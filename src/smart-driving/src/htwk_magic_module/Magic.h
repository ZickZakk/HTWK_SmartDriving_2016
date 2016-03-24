//
// Created by pbachmann on 2/17/16.
//

#ifndef HTWK_2016_MAGIC_H
#define HTWK_2016_MAGIC_H

#include "stdafx.h"
#include "../htwk_base_decision_module/BaseDecisionModule.h"
#include "../htwk_structs/DriveModule.h"

#define OID "htwk.magic"
#define FILTER_NAME "HTWK Magic Module"

#define SPEED_FACTOR_PROPERTY "Speed"

#define LEFT_TURN_MODE 1
#define RIGHT_TURN_MODE 2

using namespace cv;

class Magic : public BaseDecisionModule
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    public:
    Magic(const tChar *__info);

    virtual ~Magic();

    tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

    tResult Init(tInitStage eStage, __exception = NULL);

    tResult CreateInputPins();

    private:
    void InitializeProperties();

    // Pins
    cInputPin distancePin;
    cInputPin imuPin;

    // properties
    tFloat32 speedFactor;

    // current values
    tFloat32 distanceSinceLastAction;
    tFloat32 distanceOverall;
        tFloat32 yaw;
        tFloat32 lastYaw;

        tInt32 state;


        void drive();

        tBool WaitForGyro();

        tFloat32 CalculateExpectedYaw(tFloat32 yaw, tInt direction, tBool reverse, tFloat32 angle);

        void turn(int turnMode);

        tFloat32 yawGoal;

        void drive(tFloat distance, tInt32 mode);

        tFloat32 lastDistanceOverall;

        void calcDistance();

        tBool turnFinished;
};


#endif //HTWK_2016_MAGIC_H
