//
// Created by pbachmann on 1/6/16.
//

#include "InitLine.h"
#include "FullLine.h"
#include "BrokenLine.h"
#include "CrossingLine.h"

InitLine::InitLine(int leftBoundX, int rightBoundX, IPMBorder border, BorderDirection startToEndDirection, CrossingDetector bot, CrossingDetector top,
                   Logger *logger)
        : BaseLaneLine(border, startToEndDirection, bot, top, logger)
{
    InitLine::leftBoundX = leftBoundX;
    InitLine::rightBoundX = rightBoundX;
    InitLine::Mode = INIT;
}

InitLine::InitLine(int leftBoundX, int rightBoundX, const BaseLaneLine &olderLine) : BaseLaneLine(olderLine)
{
    InitLine::leftBoundX = leftBoundX;
    InitLine::rightBoundX = rightBoundX;
    InitLine::Mode = INIT;
}

Ptr<BaseLaneLine> InitLine::UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine)
{
    logger->StartLog();

    logger->Log("Init!", false);

    if (otherLine.Mode != INIT)
    {
        Start.SetSignPoint(border.GoMany(otherLine.Start.GetSignPoint(), 100, startToEndDirection));
        End.SetSignPoint(border.GoMany(otherLine.End.GetSignPoint(), 100,
                                       IPMBorder::Invert(startToEndDirection)));

        logger->EndLog();

        BaseLaneLine *brokenLine = new BrokenLine(*this);
        return Ptr<BaseLaneLine>(brokenLine);
    }

    Point2f crossingPoint;
    if (crossingBotDetector.FindCrossing(image, crossingPoint, *logger))
    {
        logger->Log("Crossing Found!", false);

        logger->EndLog();

        BaseLaneLine *crossingLine = new CrossingLine(*this, crossingPoint, image);
        return Ptr<BaseLaneLine>(crossingLine);
    }

    Mat workingImage = image.clone();

    Mat labels, stats, centroids;
    int nLabels = connectedComponentsWithStats(workingImage, labels, stats, centroids, 4);

    int biggestArea = 0;
    int candidateLabel = 0;

    for (int label = 1; label < nLabels; ++label)
    {
        int left = stats.at<int>(label, CC_STAT_LEFT);
        int width = stats.at<int>(label, CC_STAT_WIDTH);

        if (left < leftBoundX || (left + width) > rightBoundX)
        {
            logger->Log("Not in Place!", false);

            continue;
        }

        int area = stats.at<int>(label, CC_STAT_AREA);
        if (area < 150 || area > 3500)
        {
            logger->Log("To Small or to Big!", false);

            continue;
        }

        int height = stats.at<int>(label, CC_STAT_HEIGHT);

        if (height < 100)
        {
            logger->Log("Not high enough!", false);

            continue;
        }

        float heightRatio = height / (width + height * 1.0f);

        if (heightRatio > 0.8)
        {
            if (area > biggestArea)
            {
                candidateLabel = label;
            }
            continue;
        }

        logger->Log("Height Ratio not ok!", false);
    }

    if (candidateLabel == 0)
    {
        logger->Log("No good Label found!", false);
        logger->EndLog();

        BaseLaneLine *initLine = new InitLine(leftBoundX, rightBoundX, *this);
        return Ptr<BaseLaneLine>(initLine);
    }

    int goalX = stats.at<int>(candidateLabel, CC_STAT_LEFT);

    End.SetSignPoint(Point2f(goalX, border.GetUpperBound()));

    Point2f startPoint = border.GoMany(End.GetSignPoint(), 100, IPMBorder::Invert(startToEndDirection));
    while (!GeneralUtils::Equals(cvRound(startPoint.x), goalX, 5))
    {
        startPoint = border.Go(startPoint, IPMBorder::Invert(startToEndDirection));

        if (startPoint == Point2f(-1, -1))
        {
            logger->Log("-1 when searching for start!", false);
            logger->EndLog();

            BaseLaneLine *initLine = new InitLine(leftBoundX, rightBoundX, *this);
            return Ptr<BaseLaneLine>(initLine);
        }
    }

    Start.SetSignPoint(startPoint);


    LaneDetectionData data;
    data.Labels = labels;
    data.Stats = stats;
    currentFunction = ApproximateLane(candidateLabel, image, data);

    logger->EndLog();

    BaseLaneLine *fullLine = new FullLine(*this);
    return Ptr<BaseLaneLine>(fullLine);
}

void InitLine::Draw(Mat &image)
{
    line(image, Point2f(image.cols / 2.0f - 20, 0), Point2f(image.cols / 2.0f - 20, image.rows - 1), CV_RGB(255, 0, 0), 2);

    line(image, Point2f(0, image.rows - 50), Point2f(image.cols - 1, image.rows - 50), CV_RGB(0, 0, 255), 2);
    line(image, Point2f(0, image.rows - 100), Point2f(image.cols - 1, image.rows - 100), CV_RGB(0, 0, 255), 2);
    return;
}

float InitLine::GetLanePositionAt(float height)
{
    return 0;
}

int InitLine::GetCrossingDistance()
{
    return 200;
}
