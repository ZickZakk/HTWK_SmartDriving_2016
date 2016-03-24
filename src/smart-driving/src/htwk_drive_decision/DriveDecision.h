#ifndef _HTWK_DRIVE_DECISION_H_
#define _HTWK_DRIVE_DECISION_H_

#include "stdafx.h"

#include <math.h>

#include <Logger.h>
#include <DriveModule.h>
#include <tReadyModule.h>

#define OID "htwk.htwk_drive_decision"
#define FILTER_NAME "HTWK Drive Decision"

#define LANE_DRIVER_PRIO_PROPERTY "Lane driver priority"
#define TURN_MODULE_PRIO_PROPERTY "Turn priority"
#define COLLISIONPREV_PRIO_PROPERTY "CollPrev Property"
#define OBSTACLEPASS_PRIO_PROPERTY "Obstacle Passing Property"
#define CROSS_PARKING_MODULE_PRIO_PROPERTY "Cross Parking priority"
#define PARALLEL_PARKING_MODULE_PRIO_PROPERTY "Parallel Parking priority"
#define PULLOUT_PRIO_PROPERTY "PullOut priority"

#define IDLE_MODULE_PRIO_PROPERTY "Idle priority"

class DriveDecision : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        Logger logger;

        cInputPin driveInstructionsStructPin;
        cInputPin triggerPin;

        cOutputPin curvaturePin;
        cOutputPin curveAnglePin;
        cOutputPin speedPin;
        cOutputPin headLightPin;
        cOutputPin turnSignalLeftPin;
        cOutputPin turnSignalRightPin;
        cOutputPin hazardLightPin;
        cOutputPin brakeLightPin;
        cOutputPin reverseLightPin;
        cOutputPin incrementManeuverIdPin;
        cOutputPin moduleErrorPin;
        cOutputPin resetModulePin;

        cObjectPtr<IMediaType> typeSignalValue;
        cObjectPtr<IMediaType> typeBoolSignalValue;
        cObjectPtr<IMediaType> typeDriveInstructionsStructValue;
        cObjectPtr<IMediaType> typeEnumBoxValue;

        cObjectPtr<IMediaTypeDescription> descriptionSignalFloat;
        cObjectPtr<IMediaTypeDescription> descriptionDriveInstructionsStruct;
        cObjectPtr<IMediaTypeDescription> descriptionSignalBool;
        cObjectPtr<IMediaTypeDescription> descriptionEnumBox;

        tFloat32 speed;
        tFloat32 curveRadius;
        tFloat32 curveAngle;

        tBool headLightEnabled;
        tBool turnSignalLeftEnabled;
        tBool turnSignalRightEnabled;
        tBool hazardLightsEnabled;
        tBool brakeLightEnabled;
        tBool reverseLightEnabled;
        tBool moduleError;
        tBool maneuverCompleted;

        tBufferID sourceModuleID;
        tBufferID speedID;
        tBufferID curveRadiusID;
        tBufferID curveAngleID;
        tBufferID speedControlTakenID;
        tBufferID steeringControlTakenID;
        tBufferID headLightEnabledID;
        tBufferID turnSignalLeftEnabledID;
        tBufferID turnSignalRightEnabledID;
        tBufferID hazardLightsEnabledID;
        tBufferID brakeLightEnabledID;
        tBufferID reverseLightEnabledID;
        tBufferID moduleErrorID;
        tBufferID resetModuleID;
        tBufferID maneuverCompletedID;

        tInt laneDriverPrio;
        tInt turnPrio;
        tInt parallelParkingPrio;
        tInt crossParkingPrio;
        tInt idlePrio;
        tInt pullOutPrio;
        tInt collPrevPrio;
        tInt obstaclePassingPrio;

        tInt highestSteeringPrio;
        tInt highestSpeedPrio;
        tInt highestGeneralPrio;

        tReadyModule::ReadyModuleEnum resModule;

        tBool idsDriveInstructionsSet;

    private:

        void InitializeProperties();

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins();

        tResult CreateOutputPins(__exception = NULL);

        tResult TransmitBoolValue(cOutputPin *oPin, bool value);

        tResult TransmitSignalValue(cOutputPin *outputPin, tFloat32 value);

        tResult TransmitEnum(cOutputPin *outputPin, tReadyModule::ReadyModuleEnum module);

        tResult getDriveInstructions(IMediaSample *mediaSample);

        tResult sendSteeringInstructions();

        tResult sendSpeedInstructions();

        tResult increment();

        tResult errorHandling();

        tResult resetModule();

    public:
        DriveDecision(const tChar *__info);

        virtual ~DriveDecision();

    public:
        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);

        tResult EvaluatePin(IPin *source, IMediaSample *mediaSample);

        tInt64 lastIncrementWaitTime;
};

#endif // _HTWK_DRIVE_DECISION_H_
