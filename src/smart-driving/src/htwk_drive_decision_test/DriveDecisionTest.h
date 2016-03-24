#ifndef _HTWK_DRIVE_DECISION_TEST_H_
#define _HTWK_DRIVE_DECISION_TEST_H_

#include "stdafx.h"

#include <math.h>

#include "../htwk_logger/Logger.h"
#include "../htwk_structs/DriveModule.h"
#include "../htwk_structs/tReadyModule.h"

#define OID "htwk.htwk_drive_decision_test"
#define FILTER_NAME "HTWK Drive Decision Test"

#define SHOULD_SEND_PROPERTY "Send values"
#define SEND_SPEED_TAKEN_PROPERTY "Speed controll"
#define SEND_STEERING_TAKEN_PROPERTY "Steering controll"
#define SEND_SOURCE_MODULE_PROPERTY "Send source module"
#define SEND_SPEED_PROPERTY "Send speed"
#define SEND_STEERING_PROPERTY "Send steering (Radius)"
#define SEND_ERROR_STATE_PROPERTY "Send module Error"
#define SEND_MODULE_RESET_PROPERTY "Send module Reset"
#define SEND_MANEUVER_COMPLETE_PROPERTY "Send Maneuver completed"


class DriveDecisionTest : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        Logger logger;

        cInputPin triggerPin;

        cOutputPin outDriveInstructionsStructPin;

        cObjectPtr<IMediaType> typeDriveInstructionsStructValue;

        cObjectPtr<IMediaTypeDescription> descriptionDriveInstructionsStruct;

        tBool shouldSend;

        tInt8 sourceModule;
        tBool speedControlTaken;
        tBool steeringControllTaken;

        tFloat32 speed;
        tFloat32 curveRadius;

        tBool headLightEnabled;
        tBool turnSignalLeftEnabled;
        tBool turnSignalRightEnabled;
        tBool hazardLightsEnabled;
        tBool brakeLightEnabled;
        tBool reverseLightEnabled;
        tBool emergencyStop;
        tBool moduleError;
        tBool maneuverCompleted;

        tBool mc;

        tReadyModule::ReadyModuleEnum resModule;
        tInt32 resModuleTmp;

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
        tBufferID emergencyStopID;
        tBufferID moduleErrorID;
        tBufferID moduleResetID;
        tBufferID maneuverCompletedID;

        tBool idsDriveInstructionsSet;

    private:

        void InitializeProperties();

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins();

        tResult CreateOutputPins(__exception = NULL);

        tResult sendStruct();

        tResult readProperties();

    public:
        DriveDecisionTest(const tChar *__info);

        virtual ~DriveDecisionTest();

    public:
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_DRIVE_DECISION_TEST_H_
