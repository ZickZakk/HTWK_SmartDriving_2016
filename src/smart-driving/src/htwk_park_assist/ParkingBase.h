//
// Created by pbachmann on 2/11/16.
//

#ifndef HTWK_2016_PARKINGBASE_H
#define HTWK_2016_PARKINGBASE_H

#include "stdafx.h"

#include <BaseDecisionModule.h>
#include <Direction.h>
#include <tCarState.h>
#include <tIMU.h>
#include <tManeuver.h>
#include <tRoadSign.h>
#include <WorldModel.h>
#include <VisionUtils.h>
#include <MapChecker.h>
#include <DriveUtils.h>
#include <CarConfigReader.h>

#define DRIVE_SPEED_PROPERTY "driveSpeed"
#define REVERSE_DRIVE_SPEED_PROPERTY "reverseDriveSpeed"
#define TURN_SPEED_PROPERTY "turnSpeed"
#define WAIT_TIME_PROPERTY "waitTime"
#define STRAIGHT_DRIVE_OFFSET_PROPERTY "straightDriveOffset"
#define STRAIGHT_DRIVE_RADIUS_PROPERTY "straightDriveRadius"
#define ULTRASONIC_SAMPLES_PROPERTY "ultrasonicSamples"

class ParkingBase : public BaseDecisionModule
{
    protected:
        // Properties
        tFloat32 driveSpeed;
        tFloat32 reverseDriveSpeed;
        tFloat32 turnSpeed;
        tFloat32 straightDriveOffset;
        tFloat32 straightDriveRadius;
        tInt waitTime;
        tInt ultraSonicSamples;

        // Car
        tFloat32 lastYaw;
        tFloat32 yawGoal;

        //state
        tFloat32 elapsedTime;
        tBool isStateInitialized;

#ifndef NDEBUG
        cVideoPin debugVideoOutput;
        tBitmapFormat debugVideoFormat;
        Mat debugImage;
#endif

    public:
        ParkingBase(const tChar *__info, string moduleName, int driveModule);

        virtual ~ParkingBase();

    protected:
        void ReadConfiguration(CarConfigReader &config, string string);

        tFloat32 CalculateAngleFromDistance(tFloat32 radius, tFloat32 distance);

        bool IsFrontOfCarFree(const tWorldModel &model, tInt width);

        bool IsBackOfCarFree(const tWorldModel &model, tInt width, tInt addToSide = 0);

    public:
        virtual tResult Init(tInitStage eStage, IException **__exception_ptr);

        virtual tResult Stop(IException **__exception_ptr);

        tResult Start(IException **__exception_ptr);
};


#endif //HTWK_2016_PARKINGBASE_H
