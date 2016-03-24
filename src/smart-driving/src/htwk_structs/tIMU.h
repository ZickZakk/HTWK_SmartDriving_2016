/**
 * @author pbachmann
 */

#ifndef _T_IMU
#define _T_IMU

#include "stdafx.h"

typedef struct
{
    tFloat32 tYaw;
    tFloat32 tPitch;
    tFloat32 tRoll;
    tFloat32 tAccX;
    tFloat32 tAccY;
    tFloat32 tAccZ;
} tIMU;

#endif
