//
// Created by pbachmann on 1/6/16.
//

#include "LaneLineEnd.h"

void LaneLineEnd::SetBound(Point2f point, BorderDirection direction)
{
    switch(direction)
    {
        case CLOCKWISE:
            maxClockwiseBound = point;
            return;
        case COUNTER_CLOCKWISE:
            maxCounterClockwiseBound = point;
            return;
    }
}

void LaneLineEnd::SetSignPoint(Point2f point)
{
    signPoint = point;
}

Point2f LaneLineEnd::GetBound(BorderDirection direction) const
{
    switch (direction)
    {
        case CLOCKWISE:
            return maxClockwiseBound;
        case COUNTER_CLOCKWISE:
            return maxCounterClockwiseBound;
        default:
            return Point2f(-1,-1);
    }
}

Point2f LaneLineEnd::GetSignPoint() const
{
    return signPoint;
}
