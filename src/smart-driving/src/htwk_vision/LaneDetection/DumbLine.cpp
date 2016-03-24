//
// Created by pbachmann on 1/6/16.
//

#include "DumbLine.h"

DumbLine::DumbLine() : BaseLaneLine()
{
    Mode = INIT;
}

Ptr<BaseLaneLine> DumbLine::UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine)
{
    logger->StartLog();
    logger->Log("Dumb!");
    logger->EndLog();

    BaseLaneLine* dumbLine = new DumbLine();
    return Ptr<BaseLaneLine>(dumbLine);
}

void DumbLine::Draw(Mat &image)
{
    return;
}

float DumbLine::GetLanePositionAt(float height)
{
    return 0;
}

int DumbLine::GetCrossingDistance()
{
    return BaseLaneLine::GetCrossingDistance();
}
