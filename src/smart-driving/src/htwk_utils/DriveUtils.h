//
// Created by pbachmann on 3/3/16.
//

#ifndef HTWK_2016_DRIVEUTILS_H
#define HTWK_2016_DRIVEUTILS_H

#include "stdafx.h"
#include "../htwk_structs/Direction.h"

class DriveUtils
{
    public:
        static tFloat32 CalculateExpectedYaw(tFloat32 yaw, Direction::DirectionEnum direction, tBool reverse, tFloat32 angle);

};


#endif //HTWK_2016_DRIVEUTILS_H
