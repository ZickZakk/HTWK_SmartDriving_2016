//
// Created by pbachmann on 1/6/16.
//

#include "LostLine.h"

LostLine::LostLine(const BaseLaneLine &oldLine) : BaseLaneLine(oldLine)
{
    LostLine::additionalSearchRadius = 0;
    Mode = LOST;
}

Ptr<BaseLaneLine> LostLine::UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine)
{
    logger->StartLog();

    logger->Log("Lost!", false);

    data.LabelSearchRadius = StandardSearchRadius + additionalSearchRadius;
    data.SafetyDistance = 2 * data.LabelSearchRadius + 1;

    Ptr<BaseLaneLine> baseResult = BaseLaneDetection(image, otherLine);

    logger->EndLog();

    return baseResult;
}

void LostLine::Draw(Mat &image)
{
    DrawEndsInColor(image);
    circle(image, Point2f(GetLanePositionAt(image.rows - 1), image.rows - 1), 2, CV_RGB(0, 0, 255), -1);
}

LostLine::LostLine(const LostLine &oldLine) : BaseLaneLine(oldLine)
{
    LostLine::additionalSearchRadius = oldLine.additionalSearchRadius + LostLineRadiusIncrement;
    Mode = LOST;
}

float LostLine::GetLanePositionAt(float height)
{
    if (abs(Start.GetSignPoint().y - height) < 20)
    {
        return Start.GetSignPoint().x, height;
    }

    if (abs(Start.GetSignPoint().x - End.GetSignPoint().x) < 20)
    {
        return Start.GetSignPoint().x, height;
    }

    return FindLaneCrossingAt(height, Start.GetSignPoint().x);
}

int LostLine::GetCrossingDistance()
{
    return BaseLaneLine::GetCrossingDistance();
}
