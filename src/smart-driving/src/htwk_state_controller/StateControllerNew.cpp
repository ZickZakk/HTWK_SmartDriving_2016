//
// Created by pbachmann on 3/4/16.
//

#include "StateControllerNew.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, StateControllerNew);

StateControllerNew::StateControllerNew(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    waitTimer = NULL;

    modulesToReset.clear();
    modulesReady.clear();

    carState = tCarState::Startup;
    InitializeProperties();
}

StateControllerNew::~StateControllerNew()
{
}

void StateControllerNew::InitializeProperties()
{
    controllerMode = tControllerMode::Manual;

    SetPropertyInt(CONTROLLER_MODE_PROPERTY, controllerMode);
    SetPropertyStr(CONTROLLER_MODE_PROPERTY NSSUBPROP_VALUELIST, "0@Manual|1@Jury");
    SetPropertyStr(CONTROLLER_MODE_PROPERTY NSSUBPROP_DESCRIPTION, "Defines which mode should be used.");

    SetPropertyInt(MANEUVER_PROPERTY, tManeuver::M_CROSSING_STRAIGHT);
    SetPropertyBool(MANEUVER_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(MANEUVER_PROPERTY NSSUBPROP_VALUELIST,
                   "100@Straight|101@Left|102@Right|103@Park_Cross|104@Park_Parallel|105@Pull_Out_Left|106@Pull_Out_Right|-100@Unknown");
    SetPropertyStr(MANEUVER_PROPERTY NSSUBPROP_DESCRIPTION, "Defines which Maneuver should be send in manual mode");

    SetPropertyInt(STATE_PROPERTY, carState);
    SetPropertyBool(STATE_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(STATE_PROPERTY NSSUBPROP_VALUELIST, "0@StartUp|1@GetReady|2@Ready|3@Running|4@Completed|5@Error");
    SetPropertyStr(STATE_PROPERTY NSSUBPROP_DESCRIPTION, "Defines which State should be send in manual mode");
}

tResult StateControllerNew::Init(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (StageFirst == eStage)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (StageNormal == eStage)
    {
        controllerMode = static_cast<tControllerMode::ControllerModeEnum>(GetPropertyInt(CONTROLLER_MODE_PROPERTY));
        logger.Log(cString::Format("StateController running in %s mode.", tControllerMode::ToString(controllerMode).c_str()).GetPtr());

        if (tControllerMode::Manual == controllerMode && waitTimer == NULL)
        {
            tInt waitTime = 500 * 1000; // 500ms interval
            waitTimer = _kernel->TimerCreate(tTimeStamp(waitTime), 0, static_cast<IRunnable *>(this), NULL, NULL, 0, 0,
                                             cString::Format("%s.timer", OIGetInstanceName()));
        }
    }
    else if (StageGraphReady == eStage)
    {
        currentManeuverID = 0;
        sectionListIndex = -1;
        maneuverListIndex = -1;
        SetState(tCarState::Startup);
    }

    RETURN_NOERROR;
}

tResult StateControllerNew::Shutdown(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    if (NULL != waitTimer)
    {
        _kernel->TimerDestroy(waitTimer);
        waitTimer = NULL;
    }

    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult StateControllerNew::OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample)
{
    if (eventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();
    EvaluatePin(sourcePin, mediaSample);
    logger.EndLog();

    RETURN_NOERROR;
}

void StateControllerNew::EvaluatePin(IPin *sourcePin, IMediaSample *mediaSample)
{
    if (sourcePin == &maneuverListInput)
    {
        cString maneuverFileString;
        {
            __adtf_sample_read_lock_mediadescription(maneuverListDescription, mediaSample, pCoder);

            std::vector<tSize> vecDynamicIDs;

            // retrieve number of elements by providing NULL as first parameter
            tSize bufferSize = 0;
            if (IS_OK(pCoder->GetDynamicBufferIDs(NULL, bufferSize)))
            {
                // create a buffer depending on the size element
                tChar pcBuffer[bufferSize];
                vecDynamicIDs.resize(bufferSize);

                // get the dynamic ids (we already got the first "static" size element)
                if (IS_OK(pCoder->GetDynamicBufferIDs(&(vecDynamicIDs.front()), bufferSize)))
                {
                    // iterate over all elements
                    for (tUInt32 nIdx = 0; nIdx < vecDynamicIDs.size(); ++nIdx)
                    {
                        // get the value and put it into the buffer
                        pCoder->Get(vecDynamicIDs[nIdx], (tVoid *) &pcBuffer[nIdx]);
                    }

                    maneuverFileString = (const tChar *) pcBuffer;
                }
            }
        }

        LoadManeuverList(maneuverFileString);

        return;
    }

    if (sourcePin == &juryInput)
    {
        tInt8 juryActionId = -2;
        tInt16 maneuverId = -1;

        {
            __adtf_sample_read_lock_mediadescription(juryDescription, mediaSample, pCoder);

            pCoder->Get("i8ActionID", (tVoid *) &juryActionId);
            pCoder->Get("i16ManeuverEntry", (tVoid *) &maneuverId);
        }

        switch (juryActions(juryActionId))
        {
            case action_GETREADY:
                JuryGetReady(maneuverId);
                break;
            case action_START:
                JuryStart(maneuverId);
                break;
            case action_STOP:
                JuryStop(maneuverId);
                break;
        }

        return;
    }

    if (sourcePin == &errorInput)
    {
        tBool isError = tFalse;
        {
            __adtf_sample_read_lock_mediadescription(boolDescription, mediaSample, pCoder);
            pCoder->Get("bValue", (tVoid *) &isError);
        }

        if (isError)
        {
            logger.Log("Received Error");
            SetState(tCarState::Error);
        }

        return;
    }

    if (sourcePin == &emergencyStopInput)
    {
        tBool isEmergencyStop = tFalse;
        {
            __adtf_sample_read_lock_mediadescription(boolDescription, mediaSample, pCoder);
            pCoder->Get("bEmergencyStop", (tVoid *) &isEmergencyStop);
        }

        if (isEmergencyStop)
        {
            logger.Log("Received EmergencyStop");
            SetState(tCarState::Startup);
        }

        return;
    }

    if (sourcePin == &incrementManeuverInput)
    {
        tBool incrementManeuver = tFalse;
        {
            __adtf_sample_read_lock_mediadescription(boolDescription, mediaSample, pCoder);
            pCoder->Get("bValue", (tVoid *) &incrementManeuver);
        }

        if (incrementManeuver)
        {
            IncrementManeuver();
        }

        return;
    }

    if (sourcePin == &readyInput)
    {
        tReadyModule::ReadyModuleEnum module = tReadyModule::Nothing;
        {
            __adtf_sample_read_lock_mediadescription(enumDescription, mediaSample, pCoder);
            pCoder->Get("tEnumValue", (tVoid *) &module);
        }

        logger.Log(cString::Format("Received ready from %s", tReadyModule::ToString(module).c_str()).GetPtr());
        modulesReady.push_back(module);

        if (tCarState::GetReady == carState && AreAllModulesReady())
        {
            logger.Log("All modules ready");
            SetState(tCarState::Ready);
        }

        return;
    }
}

/*
 * IRunnable implementation for manual mode
 */
tResult StateControllerNew::Run(tInt nActivationCode, const tVoid *pvUserData, tInt szUserDataSize, IException **__exception_ptr)
{
    if (tControllerMode::Manual == controllerMode)
    {
        tCarState::CarStateEnum manualCarState = static_cast<tCarState::CarStateEnum>(GetPropertyInt(STATE_PROPERTY));
        tManeuver::ManeuverEnum manualManeuver = static_cast<tManeuver::ManeuverEnum>(GetPropertyInt(MANEUVER_PROPERTY));

        logger.Log(cString::Format("Manual: CarState: %s, Maneuver: %s", tCarState::ToString(manualCarState).c_str(),
                                   tManeuver::ToString(manualManeuver).c_str()).GetPtr());

        SendEnum(maneuverOutput, static_cast<tInt>(manualManeuver));
        SendEnum(carStateOutput, static_cast<tInt>(manualCarState));

        RETURN_NOERROR;
    }

    /* if (tControllerMode::Jury == controllerMode)
     {
         if (tCarState::GetReady == carState && AreAllModulesReady())
         {
             logger.Log("All modules ready.");
             SetState(tCarState::Ready);
         }

         tManeuver::ManeuverEnum maneuver = tManeuver::M_UNKNOWN;

         if (tCarState::Ready == carState || tCarState::Running == carState)
         {
             cString currentManeuverString = sectorList[sectionListIndex].maneuverList[maneuverListIndex].action;
             maneuver = ConvertManeuver(currentManeuverString);
         }

         logger.Log(cString::Format("Jury: CarState: %s, Maneuver: %s", tCarState::ToString(carState).c_str(),
                                    tManeuver::ToString(maneuver).c_str()).GetPtr());

         SendEnum(maneuverOutput, static_cast<tInt>(maneuver));
         SendEnum(carStateOutput, static_cast<tInt>(carState));

         stateCar stateCar = Convert(carState);
         tInt16 maneuverId = tInt16(currentManeuverID);
         SendValue(driveStructOutput, stateCar, maneuverId);
     }*/

    RETURN_NOERROR;
}

tBool StateControllerNew::AreAllModulesReady()
{
    for (unsigned int i = 0; i < modulesToReset.size(); ++i)
    {
        if (!GeneralUtils::Contains(modulesReady, modulesToReset[i]))
        {
            return tFalse;
        }
    }

    return tTrue;
}

tResult StateControllerNew::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get signal
    tChar const *strSignal = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strSignal);
    signalMediaType = new cMediaType(0, 0, 0, "tSignalValue", strSignal, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(signalMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &signalDescription));

    // get bool
    tChar const *strBool = descManager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(strBool);
    boolMediaType = new cMediaType(0, 0, 0, "tBoolSignalValue", strBool, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(boolMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &boolDescription));

    // get enum
    tChar const *strEnumBox = descManager->GetMediaDescription("tEnumBox");
    RETURN_IF_POINTER_NULL(strEnumBox);
    enumMediaType = new cMediaType(0, 0, 0, "tEnumBox", strEnumBox, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(enumMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &enumDescription));

    // get drive
    tChar const *strDriveStruct = descManager->GetMediaDescription("tDriverStruct");
    RETURN_IF_POINTER_NULL(strDriveStruct);
    driveStructMediaType = new cMediaType(0, 0, 0, "tDriverStruct", strDriveStruct, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(driveStructMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &driveStructDescription));

    // get jury
    tChar const *strJuryStruct = descManager->GetMediaDescription("tJuryStruct");
    RETURN_IF_POINTER_NULL(strJuryStruct);
    juryMediaType = new cMediaType(0, 0, 0, "tJuryStruct", strJuryStruct, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(juryMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &juryDescription));

    // get maneuverlist
    tChar const *strManeuverList = descManager->GetMediaDescription("tManeuverList");
    RETURN_IF_POINTER_NULL(strManeuverList);
    maneuverListMediaType = new cMediaType(0, 0, 0, "tManeuverList", strManeuverList, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(maneuverListMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &maneuverListDescription));

    // get juryStop
    tChar const *strJuryStop = descManager->GetMediaDescription("tJuryEmergencyStop");
    RETURN_IF_POINTER_NULL(strJuryStop);
    juryStopMediaType = new cMediaType(0, 0, 0, "tJuryEmergencyStop", strJuryStop, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(juryStopMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &juryStopDescription));

    RETURN_NOERROR;
}

tResult StateControllerNew::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(driveStructOutput.Create("Driver_Struct", driveStructMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&driveStructOutput));

    RETURN_IF_FAILED(maneuverOutput.Create("Maneuver_Enum", enumMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&maneuverOutput));

    RETURN_IF_FAILED(carStateOutput.Create("CarState_Enum", enumMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&carStateOutput));

    RETURN_IF_FAILED(resetOutput.Create("Reset_Output", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&resetOutput));

    RETURN_NOERROR;
}

tResult StateControllerNew::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(juryInput.Create("Jury_Struct", juryMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&juryInput));

    RETURN_IF_FAILED(incrementManeuverInput.Create("Increment_Maneuver_ID", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&incrementManeuverInput));

    RETURN_IF_FAILED(readyInput.Create("Ready_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&readyInput));

    RETURN_IF_FAILED(errorInput.Create("Error_Input", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&errorInput));

    RETURN_IF_FAILED(emergencyStopInput.Create("Emergency_Stop_Input", juryStopMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&emergencyStopInput));

    RETURN_IF_FAILED(maneuverListInput.Create("Maneuver_List", maneuverListMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&maneuverListInput));

    RETURN_NOERROR;
}

void StateControllerNew::SetState(tCarState::CarStateEnum newState)
{
    if (newState == carState)
    {
        return;
    }

    logger.Log(cString::Format("Switching to state %s", tCarState::ToString(newState).c_str()).GetPtr());
    carState = newState;
    SendCurrentState();
}

void StateControllerNew::SendResets()
{
    if (tCarState::GetReady == carState)
    {
        modulesToReset.clear();

        cString currentManeuverString = sectorList[sectionListIndex].maneuverList[maneuverListIndex].action;
        tManeuver::ManeuverEnum currentManeuver = ConvertManeuver(currentManeuverString);

        if (currentManeuver != tManeuver::M_PULL_OUT_LEFT && currentManeuver != tManeuver::M_PULL_OUT_RIGHT)
        {
            modulesToReset.push_back(tReadyModule::Ipm);
            modulesToReset.push_back(tReadyModule::LaneDetection);
        }
        modulesToReset.push_back(tReadyModule::MarkerEvaluator);
        modulesToReset.push_back(tReadyModule::MarkerDetection);

        modulesReady.clear();

        for (unsigned int i = 0; i < modulesToReset.size(); ++i)
        {
            logger.Log(cString::Format("Resetting module %s", tReadyModule::ToString(modulesToReset[i]).c_str()).GetPtr());
            SendEnum(resetOutput, modulesToReset[i]);
        }
    }
}

void StateControllerNew::SendCurrentState()
{
    if (tControllerMode::Jury == controllerMode)
    {
        __synchronized_obj(transmitLock);

        tManeuver::ManeuverEnum maneuver = tManeuver::M_UNKNOWN;

        if (tCarState::Ready == carState || tCarState::Running == carState)
        {
            cString currentManeuverString = sectorList[sectionListIndex].maneuverList[maneuverListIndex].action;
            maneuver = ConvertManeuver(currentManeuverString);
        }

        logger.Log(cString::Format("Jury: CarState: %s, Maneuver: %s", tCarState::ToString(carState).c_str(),
                                   tManeuver::ToString(maneuver).c_str()).GetPtr());
        SendEnum(carStateOutput, static_cast<tInt>(carState));
        SendEnum(maneuverOutput, static_cast<tInt>(maneuver));

        stateCar stateCar = Convert(carState);
        tInt16 maneuverId = tInt16(currentManeuverID);
        SendValue(driveStructOutput, stateCar, maneuverId);
    }
}

void StateControllerNew::IncrementManeuver()
{
    if (maneuverListIndex < 0 && sectionListIndex < 0)
    {
        logger.Log("Could not set new maneuver id because no maneuver list was loaded");
        SetState(tCarState::Error);
        return;
    }

    if (sectorList[sectionListIndex].maneuverList.size() > tUInt(maneuverListIndex + 1))
    {
        maneuverListIndex++;
        currentManeuverID++;
    }
    else
    {
        if (sectorList.size() > tUInt(sectionListIndex + 1))
        {
            sectionListIndex++;
            maneuverListIndex = 0;
            currentManeuverID++;
            if (sectorList[sectionListIndex].maneuverList[maneuverListIndex].id != currentManeuverID)
            {
                logger.Log("Inconsistancy in maneuverfile detected. Please check the file ");
                SetState(tCarState::Error);
                return;
            }
        }
        else
        {
            logger.Log("End of maneuverlist reached, cannot increment any more");
            SetState(tCarState::Complete);
            return;
        }
    }

    SendCurrentState();
    logger.Log(cString::Format("Increment ManeuverId: SectionIndex is %d, ManeuverIndex is %d, ID is %d",
                               sectionListIndex, maneuverListIndex, currentManeuverID).GetPtr());
}

void StateControllerNew::LoadManeuverList(cString maneuverString)
{
    sectorList.clear();
    cDOM oDOM;
    oDOM.FromString(maneuverString);
    cDOMElementRefList oSectorElems;
    cDOMElementRefList oManeuverElems;

    if (IS_OK(oDOM.FindNodes("AADC-Maneuver-List/AADC-Sector", oSectorElems)))
    {
        for (cDOMElementRefList::iterator itSectorElem = oSectorElems.begin(); itSectorElem != oSectorElems.end(); ++itSectorElem)
        {
            //if sector found
            tSector sector;
            sector.id = (*itSectorElem)->GetAttributeUInt32("id");

            if (IS_OK((*itSectorElem)->FindNodes("AADC-Maneuver", oManeuverElems)))
            {
                //iterate through maneuvers
                for (cDOMElementRefList::iterator itManeuverElem = oManeuverElems.begin();
                     itManeuverElem != oManeuverElems.end(); ++itManeuverElem)
                {
                    tAADC_Maneuver man;
                    man.id = (*itManeuverElem)->GetAttributeUInt32("id");
                    man.action = (*itManeuverElem)->GetAttribute("action");
                    sector.maneuverList.push_back(man);
                    logger.Log(cString::Format("id = %d action = %s", man.id, man.action.GetPtr()).GetPtr());
                }
            }

            sectorList.push_back(sector);
        }
    }

    if (oSectorElems.size() > 0)
    {
        logger.Log("Loaded Maneuver file successfully.");
        sectionListIndex = 0;
        maneuverListIndex = 0;
    }
    else
    {
        logger.Log("No valid Maneuver Data found!");
        sectionListIndex = -1;
        maneuverListIndex = -1;
    }
}

void StateControllerNew::SetManeuver(tInt maneuverId)
{
    for (unsigned int i = 0; i < sectorList.size(); i++)
    {
        for (unsigned int j = 0; j < sectorList[i].maneuverList.size(); j++)
        {
            if (maneuverId == sectorList[i].maneuverList[j].id)
            {
                sectionListIndex = i;
                maneuverListIndex = j;
                currentManeuverID = maneuverId;
                logger.Log(cString::Format("SectionIndex: %d, ManeuverIndex: %d, ManeuverId: %d",
                                           sectionListIndex, maneuverListIndex, currentManeuverID).GetPtr());

                SendCurrentState();
                return;
            }
        }
    }

    logger.Log(cString::Format("ERROR: Maneuver %d not found in maneuver list", maneuverId).GetPtr());
    SetState(tCarState::Error);
}

void StateControllerNew::JuryGetReady(tInt maneuverId)
{
    logger.Log(cString::Format("GETREADY received. ManeuverId: %d", maneuverId).GetPtr());

    if (sectionListIndex < 0 && maneuverListIndex < 0)
    {
        logger.Log("Maneuverlist missing");
        SetState(tCarState::Error);
        return;
    }

    SetState(tCarState::GetReady);
    SetManeuver(maneuverId);
    SendResets();
}

void StateControllerNew::JuryStart(tInt maneuverId)
{
    logger.Log(cString::Format("START received. ManeuverId: %d", maneuverId).GetPtr());

    if (tCarState::Running == carState)
    {
        logger.Log("Car is already running");
        return;
    }

    if (tCarState::Ready != carState)
    {
        logger.Log("Car is not ready yet");
        return;
    }

    SetState(tCarState::Running);
    SetManeuver(maneuverId);
}

void StateControllerNew::JuryStop(tInt maneuverId)
{
    logger.Log(cString::Format("STOP received").GetPtr());
    SetState(tCarState::Startup);
}

tResult StateControllerNew::SendEnum(cOutputPin &pin, tInt value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    enumDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(enumDescription, mediaSample, pCoder);
        pCoder->Set("tEnumValue", (tVoid *) &value);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult StateControllerNew::SendValue(cOutputPin &pin, stateCar state, tInt16 maneuver)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> driverSerializer;
    driveStructDescription->GetMediaSampleSerializer(&driverSerializer);
    tInt nSize = driverSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    tInt8 carState = tInt8(state);
    {
        __adtf_sample_write_lock_mediadescription(driveStructDescription, mediaSample, pCoder);
        pCoder->Set("i8StateID", (tVoid *) &carState);
        pCoder->Set("i16ManeuverEntry", (tVoid *) &maneuver);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

stateCar StateControllerNew::Convert(tCarState::CarStateEnum carState)
{
    switch (carState)
    {
        case tCarState::Startup:
        case tCarState::GetReady:
            return stateCar_STARTUP;
        case tCarState::Ready:
            return stateCar_READY;
        case tCarState::Running:
            return stateCar_RUNNING;
        case tCarState::Complete:
            return stateCar_COMPLETE;
        case tCarState::Error:
            return stateCar_ERROR;
        default:
            logger.Log(cString::Format("Cannot convert CarState %d to stateCar", carState).GetPtr());
            return stateCar_ERROR;
    }
}

tManeuver::ManeuverEnum StateControllerNew::ConvertManeuver(cString name)
{
    if (name == "straight")
    {
        return tManeuver::M_CROSSING_STRAIGHT;
    }
    else if (name == "left")
    {
        return tManeuver::M_CROSSING_LEFT;
    }
    else if (name == "right")
    {
        return tManeuver::M_CROSSING_RIGHT;
    }
    else if (name == "cross_parking")
    {
        return tManeuver::M_PARK_CROSS;
    }
    else if (name == "parallel_parking")
    {
        return tManeuver::M_PARK_PARALLEL;
    }
    else if (name == "pull_out_left")
    {
        return tManeuver::M_PULL_OUT_LEFT;
    }
    else if (name == "pull_out_right")
    {
        return tManeuver::M_PULL_OUT_RIGHT;
    }

    return tManeuver::M_UNKNOWN;
}
