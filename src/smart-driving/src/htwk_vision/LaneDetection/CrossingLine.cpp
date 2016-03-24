//
// Created by pbachmann on 1/6/16.
//

#include "CrossingLine.h"

Ptr<BaseLaneLine> CrossingLine::UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine)
{
    logger->StartLog();
    logger->Log("Crossing!", false);


    Point2f topPoint;
    if (!topFound && crossingTopDetector.FindCrossing(image, topPoint, *logger))
    {
        topFound = true;
        crossingPointTop = topPoint;
    };

    vector<Point2f> features_prev, features_next;
    vector<uchar> status;
    vector<float> err;

    features_prev.push_back(crossingPointBot);

    if (topFound)
    {
        features_prev.push_back(crossingPointTop);
    }

    calcOpticalFlowPyrLK(
            lastImage, image, // 2 consecutive images
            features_prev, // input point positions in first im
            features_next, // output point positions in the 2nd
            status,    // tracking success
            err      // tracking error
    );

    if (status[0] == 1 && !GeneralUtils::Equals(crossingPointBot, Start.GetSignPoint(), 2))
    {
        crossingPointBot = features_next[0];
    }

    if (topFound && status[1] == 1)
    {
        crossingPointTop = features_next[1];
    }

    image.copyTo(lastImage);

    if (!topFound)
    {
        logger->EndLog();
        BaseLaneLine *crossingLine = new CrossingLine(*this);
        return Ptr<BaseLaneLine>(crossingLine);
    }

    Ptr<BaseLaneLine> baseResult = BaseLaneDetection(image, otherLine);

    if (abs(baseResult->Start.GetSignPoint().x - crossingPointTop.x) > 5 ||
        abs(baseResult->Start.GetSignPoint().y - crossingPointTop.y) > 5)
    {
        logger->EndLog();

        BaseLaneLine *crossingLine = new CrossingLine(*this);
        return Ptr<BaseLaneLine>(crossingLine);
    }

    logger->EndLog();
    return baseResult;
}

void CrossingLine::Draw(Mat &image)
{
    circle(image, crossingPointBot, 2, CV_RGB(255, 0, 255), -1);
    if (topFound)
    {
        circle(image, crossingPointTop, 2, CV_RGB(0, 100, 0), -1);
    }

    DrawEndsInColor(image);
}

float CrossingLine::GetLanePositionAt(float height)
{
    return Start.GetSignPoint().x;
}

CrossingLine::CrossingLine(const BaseLaneLine &oldLine, Point2f point, const Mat &oldImage) : BaseLaneLine(oldLine),
                                                                                              topAcceptor(BaseLaneLine::crossingBotDetector.Acceptor,
                                                                                                          point)
{
    Mode = CROSSING;
    crossingPointBot = point;
    botFound = true;
    topFound = false;
    oldImage.copyTo(lastImage);

    Start.SetSignPoint(FindPointWithX(crossingPointBot.x, Point2f(oldImage.cols / 2.0f, border.GetLowerBound())));
    End.SetSignPoint(FindPointWithX(crossingPointBot.x, Point2f(oldImage.cols / 2.0f, border.GetUpperBound())));

    crossingTopDetector.Acceptor = &topAcceptor;
}

CrossingLine::CrossingLine(const CrossingLine &oldLine) : BaseLaneLine(oldLine),
                                                          topAcceptor(BaseLaneLine::crossingBotDetector.Acceptor, oldLine.crossingPointBot)
{
    Mode = CROSSING;
    crossingPointTop = oldLine.crossingPointTop;
    crossingPointBot = oldLine.crossingPointBot;
    botFound = oldLine.botFound;
    topFound = oldLine.topFound;
    oldLine.lastImage.copyTo(lastImage);

    Start.SetSignPoint(FindPointWithX(crossingPointBot.x, Start.GetSignPoint()));
    End.SetSignPoint(FindPointWithX(crossingPointBot.x, End.GetSignPoint()));

    crossingTopDetector.Acceptor = &topAcceptor;

    if (topFound)
    {
        Start.SetSignPoint(FindPointWithX(crossingPointTop.x, Start.GetSignPoint()));
        End.SetSignPoint(FindPointWithX(crossingPointTop.x, End.GetSignPoint()));
    }
}

Point2f CrossingLine::FindPointWithX(float x, Point2f startingPoint)
{
    for (int i = 0; i < 500; i++)
    {
        Point2f cw = border.GoMany(startingPoint, i, CLOCKWISE);

        if (GeneralUtils::Equals(cw.x, x, 1))
        {
            return cw;
        }

        Point2f ccw = border.GoMany(startingPoint, i, COUNTER_CLOCKWISE);

        if (GeneralUtils::Equals(ccw.x, x, 1))
        {
            return ccw;
        }
    }

    return startingPoint;
}

int CrossingLine::GetCrossingDistance()
{
    int botDistance = cvRound(200 - crossingPointBot.y);

    if (!topFound)
    {
        return botDistance;
    }

    int topDistance = cvRound(120 - crossingPointTop.y);

    if (topDistance < 0)
    {
        return botDistance;
    }

    return topDistance;
}
