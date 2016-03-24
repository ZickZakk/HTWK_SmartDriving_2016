//
// Created by pbachmann on 1/6/16.
//

#include "FullLine.h"

FullLine::FullLine(const BaseLaneLine &olderLine) : BaseLaneLine(olderLine)
{
    FullLine::Mode = FULL;
}

void FullLine::Draw(Mat &image)
{
    DrawFunction(image, currentFunction);
    DrawEndsInColor(image);
    circle(image, Point2f(GetLanePositionAt(image.rows - 1), image.rows - 1), 2, CV_RGB(0, 0, 255), -1);
}

Ptr<BaseLaneLine> FullLine::UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine)
{
    logger->StartLog();

    logger->Log("Full!", false);

    data.LabelSearchRadius = StandardSearchRadius;
    data.SafetyDistance = StandardSafetyDistance;

    Ptr<BaseLaneLine> baseResult = BaseLaneDetection(image, otherLine);

    logger->EndLog();

    return baseResult;
}

float FullLine::GetLanePositionAt(float height)
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

int FullLine::GetCrossingDistance()
{
    return BaseLaneLine::GetCrossingDistance();
}
