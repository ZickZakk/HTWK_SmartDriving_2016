//
// Created by gjenschmischek on 1/6/16.
//

#include "BrokenLine.h"

BrokenLine::BrokenLine(const BaseLaneLine &oldLine) : BaseLaneLine(oldLine)
{
    Mode = BROKEN;
}

Ptr<BaseLaneLine> BrokenLine::UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine)
{
    logger->StartLog();
    logger->Log("Broken!", false);

    data.LabelSearchRadius = StandardSearchRadius;
    data.SafetyDistance = StandardSafetyDistance;

    Ptr<BaseLaneLine> result = BaseLaneDetection(image, otherLine);

    logger->EndLog();
    return result;
}

Ptr<BaseLaneLine> BrokenLine::BrokenLineUpdate(const Mat &image, const BaseLaneLine &otherLine)
{
    Point2f startingPoint = border.GoMany(Start.GetSignPoint(), StandardSearchRadius,
                                          IPMBorder::Invert(startToEndDirection));
    Point2f endPoint = border.GoMany(End.GetSignPoint(), StandardSearchRadius, startToEndDirection);

    Ptr<BaseLaneLine> continuousResult = FindContinuousLabelBetween(startingPoint, endPoint, image);

    if (continuousResult->Mode == FULL)
    {
        return continuousResult;
    }

    int biggestLabel = FindBiggestLabel(data);
    data.CurrentLabel = biggestLabel;
//
//    if ((abs(Start.GetSignPoint().x - End.GetSignPoint().x) <= 25) // Small x-distance between endpoints
//        && data.Stats.at<int>(biggestLabel, CC_STAT_WIDTH) > 10 // Nevertheless high width
//        && data.Stats.at<int>(biggestLabel, CC_STAT_HEIGHT) < 190) // a good amount smaller than picture height
//    {
//        // Possible Crossing
//        BaseLaneLine *crossingLine = new CrossingLine(*this);
//        return Ptr<BaseLaneLine>(crossingLine);
//    }

    PolynomialFunction function = ApproximateLane(biggestLabel, image, data);

    Point2f newStartPoint = FindBorderCrossingWithStats(function, Start, data);
    Start.SetSignPoint(newStartPoint);

    Point2f newEndPoint = FindBorderCrossingWithStats(function, End, data);
    End.SetSignPoint(newEndPoint);

    BaseLaneLine *brokenLine = new BrokenLine(*this);
    return Ptr<BaseLaneLine>(brokenLine);
}

void BrokenLine::Draw(Mat &image)
{
    DrawFunction(image, currentFunction);
//    DrawContour(image, currentContour);
    DrawEndsInColor(image);
    circle(image, Point2f(GetLanePositionAt(image.rows - 1), image.rows - 1), 2, CV_RGB(0, 0, 255), -1);
}

Point2f BrokenLine::FindBorderCrossingWithStats(PolynomialFunction function, LaneLineEnd lineEnd, LaneDetectionData &data)
{
    Point2f inLineDirection = lineEnd.GetSignPoint();
    Point2f counterLineDirection = lineEnd.GetSignPoint();

    map<float, Point2f> possiblePoints;

    for (int i = 0; i < BrokenLineSearchRadius; i++)
    {
        if (VisionUtils::StandardEqualsPoint(inLineDirection, lineEnd.GetBound(startToEndDirection)))
        {
            break;
        }

        int functionY = cvRound(function.CalculateY(inLineDirection.x));

        if (abs(functionY - inLineDirection.y) <= 1)
        {
            possiblePoints.insert(pair<float, Point2f>(abs(functionY - inLineDirection.y), inLineDirection));
        }

        inLineDirection = border.Go(inLineDirection, startToEndDirection);
    }

    for (int i = 0; i < BrokenLineSearchRadius; i++)
    {
        if (VisionUtils::StandardEqualsPoint(counterLineDirection,
                                             lineEnd.GetBound(IPMBorder::Invert(startToEndDirection))))
        {
            break;
        }

        int functionY = cvRound(function.CalculateY(counterLineDirection.x));

        if (abs(functionY - counterLineDirection.y) <= 1)
        {
            possiblePoints.insert(pair<float, Point2f>(abs(functionY - counterLineDirection.y), counterLineDirection));
        }

        counterLineDirection = border.Go(counterLineDirection, IPMBorder::Invert(startToEndDirection));
    }

    if (possiblePoints.size() > 0)
    {
        return possiblePoints.begin()->second;
    }

    return lineEnd.GetSignPoint();
}

int BrokenLine::FindBiggestLabel(LaneDetectionData data)
{
    int biggestLabel = 0;
    int biggestStat = 0;

    for (map<int, Point2f>::iterator it = data.StartLabels.begin(); it != data.StartLabels.end(); it++)
    {
        int currentStat = data.Stats.at<int>(it->first, CC_STAT_AREA);
        if (currentStat > biggestStat)
        {
            biggestLabel = it->first;
            biggestStat = currentStat;
        }
    }

    for (map<int, Point2f>::iterator it = data.EndLabels.begin(); it != data.EndLabels.end(); it++)
    {
        int currentStat = data.Stats.at<int>(it->first, CC_STAT_AREA);
        if (currentStat > biggestStat)
        {
            biggestLabel = it->first;
            biggestStat = currentStat;
        }
    }

    return biggestLabel;
}

float BrokenLine::GetLanePositionAt(float height)
{
    if (abs(Start.GetSignPoint().y - height) < 20)
    {
        return Start.GetSignPoint().x;
    }

    if (abs(Start.GetSignPoint().x - End.GetSignPoint().x) < 20)
    {
        return Start.GetSignPoint().x;
    }

    return FindLaneCrossingAt(height, Start.GetSignPoint().x);
}

int BrokenLine::GetCrossingDistance()
{
    return BaseLaneLine::GetCrossingDistance();
}
