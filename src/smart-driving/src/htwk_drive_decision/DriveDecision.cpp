/*IMPORTANT NOTE FOR MODULES WHICH USE THIS FILTER
 * A drive Module can send either a curveRadius or a curveAngle for steering.
 * The one the module does not use have to be set to zero!
 * */

#include "DriveDecision.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, DriveDecision)

DriveDecision::DriveDecision(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    speed = 0;
    curveRadius = 0;
    curveAngle = 90;

    headLightEnabled = tFalse;
    turnSignalLeftEnabled = tFalse;
    turnSignalRightEnabled = tFalse;
    hazardLightsEnabled = tFalse;
    brakeLightEnabled = tFalse;
    reverseLightEnabled = tFalse;
    moduleError = tFalse;
    maneuverCompleted = tFalse;

    highestSteeringPrio = 0;
    highestSpeedPrio = 0;
    highestGeneralPrio = 0;

    resModule = tReadyModule::Nothing;

    InitializeProperties();

    lastIncrementWaitTime = 10;
    idsDriveInstructionsSet = tFalse;
}

DriveDecision::~DriveDecision()
{
}

void DriveDecision::InitializeProperties()
{
    laneDriverPrio = 1;
    collPrevPrio = 2;
    obstaclePassingPrio = 3;
    turnPrio = 4;
    crossParkingPrio = 10;
    parallelParkingPrio = 11;
    pullOutPrio = 12;
    idlePrio = 999;

    SetPropertyInt(LANE_DRIVER_PRIO_PROPERTY, laneDriverPrio);
    SetPropertyBool(LANE_DRIVER_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(LANE_DRIVER_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Lane tracking module priority");
    SetPropertyFloat(LANE_DRIVER_PRIO_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(CROSS_PARKING_MODULE_PRIO_PROPERTY, crossParkingPrio);
    SetPropertyBool(CROSS_PARKING_MODULE_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(CROSS_PARKING_MODULE_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Cross parking module priority");
    SetPropertyFloat(CROSS_PARKING_MODULE_PRIO_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(PARALLEL_PARKING_MODULE_PRIO_PROPERTY, parallelParkingPrio);
    SetPropertyBool(PARALLEL_PARKING_MODULE_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(PARALLEL_PARKING_MODULE_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Parallel parking module priority");
    SetPropertyFloat(PARALLEL_PARKING_MODULE_PRIO_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(TURN_MODULE_PRIO_PROPERTY, turnPrio);
    SetPropertyBool(TURN_MODULE_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(TURN_MODULE_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Turn module priority");
    SetPropertyFloat(TURN_MODULE_PRIO_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(IDLE_MODULE_PRIO_PROPERTY, idlePrio);
    SetPropertyBool(IDLE_MODULE_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(IDLE_MODULE_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Idle module priority");
    SetPropertyFloat(IDLE_MODULE_PRIO_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(PULLOUT_PRIO_PROPERTY, pullOutPrio);
    SetPropertyBool(PULLOUT_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(PULLOUT_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Pullout priority");
    SetPropertyFloat(PULLOUT_PRIO_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(COLLISIONPREV_PRIO_PROPERTY, collPrevPrio);
    SetPropertyBool(COLLISIONPREV_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(COLLISIONPREV_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Collision prevention priority");
    SetPropertyFloat(COLLISIONPREV_PRIO_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(OBSTACLEPASS_PRIO_PROPERTY, obstaclePassingPrio);
    SetPropertyBool(OBSTACLEPASS_PRIO_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(OBSTACLEPASS_PRIO_PROPERTY NSSUBPROP_DESCRIPTION, "Obstacle passing priority");
    SetPropertyFloat(OBSTACLEPASS_PRIO_PROPERTY NSSUBPROP_MIN, 0);
}

tResult DriveDecision::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins());
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }

    else if (eStage == StageNormal)
    {
        laneDriverPrio = GetPropertyInt(LANE_DRIVER_PRIO_PROPERTY);
        turnPrio = GetPropertyInt(TURN_MODULE_PRIO_PROPERTY);
        parallelParkingPrio = GetPropertyInt(PARALLEL_PARKING_MODULE_PRIO_PROPERTY);
        crossParkingPrio = GetPropertyInt(CROSS_PARKING_MODULE_PRIO_PROPERTY);
        idlePrio = GetPropertyInt(IDLE_MODULE_PRIO_PROPERTY);
        pullOutPrio = GetPropertyInt(PULLOUT_PRIO_PROPERTY);

        logger.StartLog();
        logger.Log(cString::Format("Lane Prio: %d", laneDriverPrio).GetPtr());
        logger.Log(cString::Format("Idle Prio: %d", idlePrio).GetPtr());
        logger.Log(cString::Format("Parallel Park Prio: %d", parallelParkingPrio).GetPtr());
        logger.EndLog();
    }

    else if (eStage == StageGraphReady)
    {
    }

    RETURN_NOERROR;
}

tResult DriveDecision::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();
    tResult ret = EvaluatePin(source, mediaSample);
    logger.EndLog();

    RETURN_ERROR(ret);
}

tResult DriveDecision::EvaluatePin(IPin *source, IMediaSample *mediaSample)
{
    if (source == &driveInstructionsStructPin)
    {
        RETURN_IF_FAILED(getDriveInstructions(mediaSample));
    }

    else if (source == &triggerPin)
    {
        if (resModule != tReadyModule::Nothing)
        {
            resetModule();
        }

        if (moduleError)
        {
            errorHandling();
        }

        if (maneuverCompleted)
        {
            increment();
        }

        lastIncrementWaitTime++;

        RETURN_IF_FAILED(sendSpeedInstructions());
        RETURN_IF_FAILED(sendSteeringInstructions());
    }

    RETURN_NOERROR;
}

tResult DriveDecision::CreateInputPins()
{
    RETURN_IF_FAILED(
            driveInstructionsStructPin.Create("drive_instructions_struct", typeDriveInstructionsStructValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&driveInstructionsStructPin));

    RETURN_IF_FAILED(triggerPin.Create("Trigger", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&triggerPin));

    RETURN_NOERROR;
}

tResult DriveDecision::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(speedPin.Create("speed", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&speedPin));

    RETURN_IF_FAILED(curvaturePin.Create("curveRadius", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&curvaturePin));

    RETURN_IF_FAILED(curveAnglePin.Create("curveAngle", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&curveAnglePin));

    RETURN_IF_FAILED(headLightPin.Create("headLightEnabled", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&headLightPin));

    RETURN_IF_FAILED(turnSignalLeftPin.Create("turnSignalLeftEnabled", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&turnSignalLeftPin));

    RETURN_IF_FAILED(turnSignalRightPin.Create("turnSignalRightEnabled", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&turnSignalRightPin));

    RETURN_IF_FAILED(hazardLightPin.Create("hazardLightEnabled", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&hazardLightPin));

    RETURN_IF_FAILED(brakeLightPin.Create("brakeLightEnabled", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&brakeLightPin));

    RETURN_IF_FAILED(reverseLightPin.Create("reverseLightEnabled", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&reverseLightPin));

    RETURN_IF_FAILED(incrementManeuverIdPin.Create("Increment_Maneuver_ID", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&incrementManeuverIdPin));

    RETURN_IF_FAILED(moduleErrorPin.Create("Set_Error_State", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&moduleErrorPin));

    RETURN_IF_FAILED(resetModulePin.Create("Module_Reset", typeEnumBoxValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&resetModulePin));

    RETURN_NOERROR;
}

tResult DriveDecision::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get signal value
    tChar const *signalValueDescription = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(signalValueDescription);
    typeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", signalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignalFloat));

    // get bool
    tChar const *boolSignalValueDescription = descManager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(boolSignalValueDescription);
    typeBoolSignalValue = new cMediaType(0, 0, 0, "tBoolSignalValue", boolSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeBoolSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignalBool));

    tChar const *strDescDriveStruct = descManager->GetMediaDescription("tDriveInstructionsStruct");
    RETURN_IF_POINTER_NULL(strDescDriveStruct);
    typeDriveInstructionsStructValue = new cMediaType(0, 0, 0, "tDriveInstructionsStruct", strDescDriveStruct,
                                                      IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeDriveInstructionsStructValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionDriveInstructionsStruct));

    tChar const *strDescEnumBox = descManager->GetMediaDescription("tEnumBox");
    RETURN_IF_POINTER_NULL(strDescEnumBox);
    typeEnumBoxValue = new cMediaType(0, 0, 0, "tEnumBox", strDescEnumBox, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeEnumBoxValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionEnumBox));

    RETURN_NOERROR;
}

tResult DriveDecision::TransmitBoolValue(cOutputPin *oPin, bool value)
{
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid **) &pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignalBool->GetMediaSampleSerializer(&pSerializer);
    pMediaSample->AllocBuffer(pSerializer->GetDeserializedSize());

    tBool bValue = value;
    tUInt32 ui32TimeStamp = 0;

    //write date to the media sample with the coder of the descriptor
    {
        __adtf_sample_write_lock_mediadescription(descriptionSignalBool, pMediaSample, pCoderOutput);

        // set value from sample
        pCoderOutput->Set("bValue", (tVoid *) &bValue);
        pCoderOutput->Set("ui32ArduinoTimestamp", (tVoid *) &(ui32TimeStamp));
    }

    pMediaSample->SetTime(_clock->GetStreamTime());

    //transmit media sample over output pin
    oPin->Transmit(pMediaSample);

    RETURN_NOERROR;
}


tResult DriveDecision::TransmitSignalValue(cOutputPin *outputPin, tFloat32 value)
{
    tUInt32 timeStamp = 0;

    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignalFloat->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    cObjectPtr<IMediaSample> newMediaSample;
    AllocMediaSample((tVoid **) &newMediaSample);
    newMediaSample->AllocBuffer(nSize);
    {
        __adtf_sample_write_lock_mediadescription(descriptionSignalFloat, newMediaSample, outputCoder);
        outputCoder->Set("f32Value", (tVoid *) &(value));
        outputCoder->Set("ui32ArduinoTimestamp", (tVoid *) &timeStamp);
    }

    newMediaSample->SetTime(_clock->GetStreamTime());
    outputPin->Transmit(newMediaSample);

    RETURN_NOERROR;
}

tResult DriveDecision::TransmitEnum(cOutputPin *outputPin, tReadyModule::ReadyModuleEnum module)
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionEnumBox->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    cObjectPtr<IMediaSample> newMediaSample;
    AllocMediaSample((tVoid **) &newMediaSample);
    newMediaSample->AllocBuffer(nSize);
    {
        __adtf_sample_write_lock_mediadescription(descriptionEnumBox, newMediaSample, outputCoder);
        outputCoder->Set("tEnumValue", (tVoid *) &module);
    }

    newMediaSample->SetTime(_clock->GetStreamTime());
    outputPin->Transmit(newMediaSample);

    RETURN_NOERROR;
}

tResult DriveDecision::getDriveInstructions(IMediaSample *mediaSample)
{
    tInt sourceModule = 0;
    tBool speedControlTaken = false;
    tBool steeringControlTaken = false;

    {
        __adtf_sample_read_lock_mediadescription(descriptionDriveInstructionsStruct, mediaSample, pCoderInput);

        if (!idsDriveInstructionsSet)
        {
            pCoderInput->GetID("tSourceModule", sourceModuleID);
            pCoderInput->GetID("tTakeSpeedControl", speedControlTakenID);
            pCoderInput->GetID("tSpeed", speedID);
            pCoderInput->GetID("tTakeSteeringControl", steeringControlTakenID);
            pCoderInput->GetID("tSteeringRadius", curveRadiusID);
            pCoderInput->GetID("tSteeringAngle", curveAngleID);
            pCoderInput->GetID("tHeadLightEnabled", headLightEnabledID);
            pCoderInput->GetID("tTurnSignalLeftEnabled", turnSignalLeftEnabledID);
            pCoderInput->GetID("tTurnSignalRightEnabled", turnSignalRightEnabledID);
            pCoderInput->GetID("tHazardLightsEnabled", hazardLightsEnabledID);
            pCoderInput->GetID("tBrakeLightEnabled", brakeLightEnabledID);
            pCoderInput->GetID("tReverseLightsEnabled", reverseLightEnabledID);
            pCoderInput->GetID("tError", moduleErrorID);
            pCoderInput->GetID("tResetSensor", resetModuleID);
            pCoderInput->GetID("tManeuverCompleted", maneuverCompletedID);
            idsDriveInstructionsSet = tTrue;
        }

        pCoderInput->Get(sourceModuleID, (tVoid *) &sourceModule);
        pCoderInput->Get(speedControlTakenID, (tVoid *) &speedControlTaken);
        pCoderInput->Get(steeringControlTakenID, (tVoid *) &steeringControlTaken);

        tInt modulePrio;
        switch (sourceModule)
        {
            case DM_LANE_DRIVER:
                modulePrio = laneDriverPrio;
                break;

            case DM_PARALLEL_PARKING:
                modulePrio = parallelParkingPrio;
                break;

            case DM_CROSS_PARKING:
                modulePrio = crossParkingPrio;
                break;

            case DM_TURN:
                modulePrio = turnPrio;
                break;

            case DM_IDLE:
                modulePrio = idlePrio;
                break;

            case DM_PULL_OUT:
                modulePrio = pullOutPrio;
                break;

            case DM_COLL_PREV:
                modulePrio = collPrevPrio;
                break;

            case DM_PASSING:
                modulePrio = obstaclePassingPrio;
                break;

/*
            case DM_MAGIC:
                modulePrio = 50;
                break;
*/

            default:
                logger.Log(cString::Format("Unknown Module: %d!", sourceModule).GetPtr());
                modulePrio = -1;
                break;
        }

        if (((speedControlTaken == tTrue) || (steeringControlTaken == tTrue)) && (modulePrio >= highestGeneralPrio))
        {
            pCoderInput->Get(headLightEnabledID, (tVoid *) &headLightEnabled);
            pCoderInput->Get(hazardLightsEnabledID, (tVoid *) &hazardLightsEnabled);
            pCoderInput->Get(brakeLightEnabledID, (tVoid *) &brakeLightEnabled);
            pCoderInput->Get(reverseLightEnabledID, (tVoid *) &reverseLightEnabled);
            pCoderInput->Get(moduleErrorID, (tVoid *) &moduleError);
            pCoderInput->Get(resetModuleID, (tVoid *) &resModule);
            pCoderInput->Get(maneuverCompletedID, (tVoid *) &maneuverCompleted);
            pCoderInput->Get(turnSignalLeftEnabledID, (tVoid *) &turnSignalLeftEnabled);
            pCoderInput->Get(turnSignalRightEnabledID, (tVoid *) &turnSignalRightEnabled);

            highestGeneralPrio = modulePrio;

            logger.Log(cString::Format("General from %d", sourceModule).GetPtr(), false);
        }
        else if ((steeringControlTaken == tFalse) && (speedControlTaken == tFalse) && (modulePrio == highestGeneralPrio))
        {
            logger.Log(cString::Format("General control released by: %d", sourceModule).GetPtr(), false);
            highestGeneralPrio = 0;
        }

        if ((speedControlTaken == tTrue) && (modulePrio >= highestSpeedPrio))
        {
            pCoderInput->Get(speedID, (tVoid *) &speed);

            highestSpeedPrio = modulePrio;

            logger.Log(cString::Format("Speed from %d: %f", sourceModule, speed).GetPtr(), false);

        }
        else if ((speedControlTaken == tFalse) && (modulePrio == highestSpeedPrio))
        {
            logger.Log(cString::Format("Speed control released by: %d", sourceModule).GetPtr(), false);
            highestSpeedPrio = 0;
        }

        if ((steeringControlTaken == tTrue) && (modulePrio >= highestSteeringPrio))
        {
            pCoderInput->Get(curveRadiusID, (tVoid *) &curveRadius);
            pCoderInput->Get(curveAngleID, (tVoid *) &curveAngle);

            highestSteeringPrio = modulePrio;

            logger.Log(cString::Format("Steering from %d: %f m / %f °", sourceModule, curveRadius, curveAngle).GetPtr(), false);
        }
        else if ((steeringControlTaken == tFalse) && (modulePrio == highestSteeringPrio))
        {
            logger.Log(cString::Format("Steering control released by: %d", sourceModule).GetPtr(), false);
            highestSteeringPrio = 0;
        }
    }

    RETURN_NOERROR;
}

tResult DriveDecision::sendSteeringInstructions()
{
    if (curveRadius == 0 && curveAngle != 0)
    {
        TransmitSignalValue(&curveAnglePin, curveAngle);
    }
    else if (curveAngle == 0 && curveRadius != 0)
    {
        TransmitSignalValue(&curvaturePin, curveRadius);
    }
    else if (curveAngle == 0 && curveRadius == 0)
    {
        TransmitSignalValue(&curveAnglePin, 90);
    }
    else
    {
        RETURN_ERROR(ERR_UNEXPECTED);
    }

    TransmitBoolValue(&turnSignalLeftPin, turnSignalLeftEnabled);
    TransmitBoolValue(&turnSignalRightPin, turnSignalRightEnabled);

    RETURN_NOERROR;
}

tResult DriveDecision::sendSpeedInstructions()
{
    TransmitSignalValue(&speedPin, speed);
    TransmitBoolValue(&headLightPin, headLightEnabled);
    TransmitBoolValue(&hazardLightPin, hazardLightsEnabled);
    TransmitBoolValue(&brakeLightPin, brakeLightEnabled);
    TransmitBoolValue(&reverseLightPin, reverseLightEnabled);

    RETURN_NOERROR;
}

tResult DriveDecision::increment()
{
    if (lastIncrementWaitTime > 2)
    {
        logger.Log("Maneuver ID increased", false);
        TransmitBoolValue(&incrementManeuverIdPin, tTrue);
        lastIncrementWaitTime = 0;
    }

    RETURN_NOERROR;
}

tResult DriveDecision::errorHandling()
{
    curveAngle = 90;
    curveRadius = 0;
    speed = 0;
    hazardLightsEnabled = tTrue;
    brakeLightEnabled = tTrue;

    logger.Log("Error state was triggered", false);

    TransmitBoolValue(&moduleErrorPin, tTrue);

    RETURN_NOERROR;
}

tResult DriveDecision::resetModule()
{
    logger.Log(cString::Format("Reset was triggered for module %d", resModule).GetPtr(), false);
    TransmitEnum(&resetModulePin, resModule);
    RETURN_NOERROR;
}


