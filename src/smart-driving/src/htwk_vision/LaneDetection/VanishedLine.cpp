//
// Created by pbachmann on 1/6/16.
//

#include "VanishedLine.h"

VanishedLine::VanishedLine(const BaseLaneLine &oldLine) : BaseLaneLine(oldLine)
{
    Mode = VANISHED;
}

Ptr<BaseLaneLine> VanishedLine::UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine)
{
    logger->StartLog();

    logger->Log("Vanished!", false);

    Mat labels, stats, centroids;
    connectedComponentsWithStats(image, labels, stats, centroids, 4);

    labels.copyTo(data.Labels);
    stats.copyTo(data.Stats);

    Point2f startingPoint = border.GoMany(otherLine.Start.GetSignPoint(), StandardSafetyDistance, startToEndDirection);
    Point2f endPoint = border.GoMany(otherLine.End.GetSignPoint(), StandardSafetyDistance,
                                     IPMBorder::Invert(startToEndDirection));

    Ptr <BaseLaneLine> result = FindContinuousLabelBetween(startingPoint, endPoint, image);

    logger->EndLog();

    return result;
}

void VanishedLine::Draw(Mat &image)
{
    return;
}

float VanishedLine::GetLanePositionAt(float height)
{
    return 0;
}

int VanishedLine::GetCrossingDistance()
{
    return BaseLaneLine::GetCrossingDistance();
}
