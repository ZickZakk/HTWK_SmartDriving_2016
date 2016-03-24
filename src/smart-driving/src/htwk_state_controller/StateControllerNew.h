//
// Created by pbachmann on 3/4/16.
//

#ifndef HTWK_2016_STATECONTROLLERNEW_H
#define HTWK_2016_STATECONTROLLERNEW_H

#include "stdafx.h"

#include "JuryEnums.h"
#include <Logger.h>
#include <tCarState.h>
#include <Maneuverlist.h>
#include <tManeuver.h>
#include <tReadyModule.h>
#include <GeneralUtils.h>

#define FILTER_NAME "HTWK StateController New"
#define OID "htwk.statecontroller.new"

#define CONTROLLER_MODE_PROPERTY "Controller Mode"
#define STATE_PROPERTY "State"
#define MANEUVER_PROPERTY "Maneuver"

class StateControllerNew : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        struct tControllerMode
        {
            enum ControllerModeEnum
            {
                Manual = 0,
                Jury = 1,
            };

            static std::string ToString(ControllerModeEnum value)
            {
                switch (value)
                {
                    case Manual:
                        return "Manual";
                    case Jury:
                        return "Jury";
                    default:
                        return "Unknown Value";
                }
            }
        };

    private:
        Logger logger;
        cCriticalSection transmitLock;

        // used in manual mode
        tHandle waitTimer;

        cInputPin maneuverListInput;
        cInputPin juryInput;
        cInputPin incrementManeuverInput;
        cInputPin errorInput;
        cInputPin readyInput;
        cInputPin emergencyStopInput;

        cOutputPin driveStructOutput;
        cOutputPin carStateOutput;
        cOutputPin maneuverOutput;
        cOutputPin resetOutput;

        cObjectPtr<IMediaType> signalMediaType;
        cObjectPtr<IMediaTypeDescription> signalDescription;

        cObjectPtr<IMediaType> boolMediaType;
        cObjectPtr<IMediaTypeDescription> boolDescription;

        cObjectPtr<IMediaType> enumMediaType;
        cObjectPtr<IMediaTypeDescription> enumDescription;

        cObjectPtr<IMediaType> juryMediaType;
        cObjectPtr<IMediaTypeDescription> juryDescription;

        cObjectPtr<IMediaType> driveStructMediaType;
        cObjectPtr<IMediaTypeDescription> driveStructDescription;

        cObjectPtr<IMediaType> maneuverListMediaType;
        cObjectPtr<IMediaTypeDescription> maneuverListDescription;

        cObjectPtr<IMediaType> juryStopMediaType;
        cObjectPtr<IMediaTypeDescription> juryStopDescription;

        tControllerMode::ControllerModeEnum controllerMode;
        tCarState::CarStateEnum carState;

        std::vector<tSector> sectorList;

        tInt currentManeuverID;
        tInt maneuverListIndex;
        tInt sectionListIndex;

        vector<tReadyModule::ReadyModuleEnum> modulesToReset;
        vector<tReadyModule::ReadyModuleEnum> modulesReady;

    public:
        StateControllerNew(const tChar *__info);

        ~StateControllerNew();

    private:
        void InitializeProperties();

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateOutputPins(IException **__exception_ptr);

        tResult CreateInputPins(IException **__exception_ptr);

    private:
        void EvaluatePin(IPin *sourcePin, IMediaSample *mediaSample);

        void SetState(tCarState::CarStateEnum newState);

        void IncrementManeuver();

        void LoadManeuverList(cString maneuverString);

        void SetManeuver(tInt maneuverId);

        void JuryGetReady(tInt maneuverId);

        void JuryStart(tInt maneuverId);

        void JuryStop(tInt maneuverId);

        tResult SendEnum(cOutputPin &pin, tInt value);

        tResult SendValue(cOutputPin &pin, stateCar state, tInt16 maneuver);

        stateCar Convert(tCarState::CarStateEnum carState);

        tBool AreAllModulesReady();

        tManeuver::ManeuverEnum ConvertManeuver(cString name);

        void SendCurrentState();

    public:
        virtual tResult Init(tInitStage eStage, IException **__exception_ptr);

        virtual tResult Shutdown(tInitStage eStage, IException **__exception_ptr);

        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Run(tInt nActivationCode, const void *pvUserData, tInt szUserDataSize, IException **__exception_ptr);

        void SendResets();
};


#endif //HTWK_2016_STATECONTROLLERNEW_H
