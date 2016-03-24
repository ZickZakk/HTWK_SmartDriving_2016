//
// Created by pbachmann on 2/10/16.
//

#ifndef HTWK_2016_BASEDECISIONMODULE_H
#define HTWK_2016_BASEDECISIONMODULE_H

#include "stdafx.h"
#include <Logger.h>
#include <DriveModule.h>
#include <tReadyModule.h>
#include <WorldService.h>
#include <CarConfigReader.h>

#include "opencv2/opencv.hpp"

using namespace cv;

class BaseDecisionModule : public adtf::cFilter
{
    private:
        cInputPin triggerInput;
        cOutputPin driveInstructionsOutput;

        cObjectPtr<IMediaType> typeSignal;
        cObjectPtr<IMediaTypeDescription> descriptionSignal;

        cObjectPtr<IMediaType> typeDriveInstructions;
        cObjectPtr<IMediaTypeDescription> descriptionDriveInstructions;

        tBufferID speedID;
        tBufferID curveRadiusID;
        tBufferID curveAngleID;
        tBufferID resetSensorID;
        tBufferID sourceModuleID;
        tBufferID speedControlTakenID;
        tBufferID steeringControlTakenID;
        tBufferID headLightEnabledID;
        tBufferID turnSignalLeftEnabledID;
        tBufferID turnSignalRightEnabledID;
        tBufferID hazardLightsEnabledID;
        tBufferID brakeLightEnabledID;
        tBufferID reverseLightEnabledID;
        tBufferID errorID;
        tBufferID maneuverCompletedID;

        tBool idsDriveInstructionsSet;

    private:
        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateOutputPins(IException **__exception_ptr);

        tResult CreateInputPins(IException **__exception_ptr);

        tResult SendDriveInstructions();

    protected:
        Logger logger;
        cObjectPtr<WorldService> worldService;

        tInt sourceModule;
        tBool speedControlTaken;
        tBool steeringControlTaken;

        tFloat32 speed;
        tFloat32 curveRadius;
        tFloat32 curveAngle;

        tBool headLightEnabled;
        tBool turnSignalLeftEnabled;
        tBool turnSignalRightEnabled;
        tBool hazardLightsEnabled;
        tBool brakeLightEnabled;
        tBool reverseLightEnabled;
        tBool errorActive;
        tBool maneuverCompleted;

        tReadyModule::ReadyModuleEnum resettableModule;

    protected:
        virtual tResult OnTrigger(tFloat32 interval) = 0;

        void ResetDriveInstructions(tInt module);

        template<typename Type>
        void ReadConfigValue(CarConfigReader &config, string group, string key, Type &value)
        {
            string isDefault = "(Default)";

            if (!IS_FAILED(config.Pull(group, key, value)))
            {
                isDefault = "";
            }

            if (typeid(Type) == typeid(tInt))
            {
                logger.Log(cString::Format("%s: %d %s", key.c_str(), value, isDefault.c_str()).GetPtr(), false);
            }
            else if (typeid(Type) == typeid(tFloat32))
            {
                logger.Log(cString::Format("%s: %f %s", key.c_str(), value, isDefault.c_str()).GetPtr(), false);
            }
            else if (typeid(Type) == typeid(tBool))
            {
                string boolVal = value ? "true" : "false";
                logger.Log(cString::Format("%s: %s %s", key.c_str(), boolVal.c_str(), isDefault.c_str()).GetPtr(), false);
            }
        }

    public:
        BaseDecisionModule(const tChar *info, string filterName, int driveModule);

        virtual ~BaseDecisionModule();

    public:
        virtual tResult Init(tInitStage eStage, __exception = NULL);

        virtual tResult Shutdown(tInitStage eStage, IException **__exception_ptr);

        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);
};


#endif //HTWK_2016_BASEDECISIONMODULE_H
