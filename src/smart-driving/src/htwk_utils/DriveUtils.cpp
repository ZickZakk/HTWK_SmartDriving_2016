//
// Created by pbachmann on 3/3/16.
//

#include "DriveUtils.h"

tFloat32 DriveUtils::CalculateExpectedYaw(tFloat32 yaw, Direction::DirectionEnum direction, tBool reverse, tFloat32 angle)
{
    tInt modify = reverse ? -1 : 1;
    tFloat32 expectedYaw = 0;

    switch (direction)
    {
        case Direction::LEFT:
            expectedYaw = yaw + (modify * angle);
            break;
        case Direction::RIGHT:
            expectedYaw = yaw - (modify * angle);
            break;
        default:
            return yaw;
    }

    if (expectedYaw > 180)
    {
        expectedYaw = -180 + (expectedYaw - 180);
    }

    if (expectedYaw < -180)
    {
        expectedYaw = 180 + (expectedYaw + 180);
    }

    return expectedYaw;
}
