//
// Created by gjenschmischek on 2/10/16.
//

#include "BaseDecisionModule.h"

BaseDecisionModule::BaseDecisionModule(const tChar *info, string filterName, int driveModule) : cFilter(info), logger(filterName, 20)
{
    ResetDriveInstructions(driveModule);

    idsDriveInstructionsSet = false;
}

BaseDecisionModule::~BaseDecisionModule()
{
}

tResult BaseDecisionModule::Init(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (StageGraphReady == eStage)
    {
        RETURN_IF_FAILED(_runtime->GetObject(OID_WORLD_SERVICE, IID_WORLD_INTERFACE, (tVoid **) &worldService, __exception_ptr));
    }

    RETURN_NOERROR;
}

tResult BaseDecisionModule::Shutdown(tInitStage eStage, IException **__exception_ptr)
{
    if (eStage == StageGraphReady)
    {
        worldService = NULL;
    }
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult BaseDecisionModule::OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample)
{
    if (eventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (sourcePin == &triggerInput)
    {
        tFloat32 intervalTime;

        {
            __adtf_sample_read_lock_mediadescription(descriptionSignal, mediaSample, inputCoder);
            inputCoder->Get("f32Value", (tVoid *) &intervalTime);
        }

        if (IS_OK(OnTrigger(intervalTime)))
        {
            SendDriveInstructions();
        }
    }

    logger.EndLog();

    RETURN_NOERROR;
}

tResult BaseDecisionModule::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    tChar const *strDescDriveStruct = descManager->GetMediaDescription("tDriveInstructionsStruct");
    RETURN_IF_POINTER_NULL(strDescDriveStruct);
    typeDriveInstructions = new cMediaType(0, 0, 0, "tDriveInstructionsStruct", strDescDriveStruct,
                                           IMediaDescription::MDF_DDL_DEFAULT_VERSION);

    RETURN_IF_FAILED(
            typeDriveInstructions->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionDriveInstructions));

    // get bool
    tChar const *strDescSignal = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strDescSignal);
    typeSignal = new cMediaType(0, 0, 0, "tSignalValue", strDescSignal, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeSignal->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignal));

    RETURN_NOERROR;
}

tResult BaseDecisionModule::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(driveInstructionsOutput.Create("DriveInstructions_Output", typeDriveInstructions, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&driveInstructionsOutput));

    RETURN_NOERROR;
}

tResult BaseDecisionModule::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(triggerInput.Create("Trigger_Input", typeSignal, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&triggerInput));

    RETURN_NOERROR;
}

tResult BaseDecisionModule::SendDriveInstructions()
{
    cObjectPtr<IMediaSample> pMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &pMediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionDriveInstructions->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(descriptionDriveInstructions, pMediaSample, pCoder);

        // get the IDs for the items in the media sample
        if (!idsDriveInstructionsSet)
        {
            pCoder->GetID("tSourceModule", sourceModuleID);
            pCoder->GetID("tTakeSpeedControl", speedControlTakenID);
            pCoder->GetID("tSpeed", speedID);
            pCoder->GetID("tTakeSteeringControl", steeringControlTakenID);
            pCoder->GetID("tSteeringRadius", curveRadiusID);
            pCoder->GetID("tSteeringAngle", curveAngleID);
            pCoder->GetID("tHeadLightEnabled", headLightEnabledID);
            pCoder->GetID("tTurnSignalLeftEnabled", turnSignalLeftEnabledID);
            pCoder->GetID("tTurnSignalRightEnabled", turnSignalRightEnabledID);
            pCoder->GetID("tHazardLightsEnabled", hazardLightsEnabledID);
            pCoder->GetID("tBrakeLightEnabled", brakeLightEnabledID);
            pCoder->GetID("tReverseLightsEnabled", reverseLightEnabledID);
            pCoder->GetID("tResetSensor", resetSensorID);
            pCoder->GetID("tError", errorID);
            pCoder->GetID("tManeuverCompleted", maneuverCompletedID);
            idsDriveInstructionsSet = tTrue;
        }

        //write date to the media sample with the coder of the descriptor
        pCoder->Set(sourceModuleID, (tVoid *) &sourceModule);
        pCoder->Set(speedControlTakenID, (tVoid *) &speedControlTaken);
        pCoder->Set(steeringControlTakenID, (tVoid *) &steeringControlTaken);
        pCoder->Set(speedID, (tVoid *) &speed);
        pCoder->Set(headLightEnabledID, (tVoid *) &headLightEnabled);
        pCoder->Set(hazardLightsEnabledID, (tVoid *) &hazardLightsEnabled);
        pCoder->Set(brakeLightEnabledID, (tVoid *) &brakeLightEnabled);
        pCoder->Set(reverseLightEnabledID, (tVoid *) &reverseLightEnabled);
        pCoder->Set(curveRadiusID, (tVoid *) &curveRadius);
        pCoder->Set(curveAngleID, (tVoid *) &curveAngle);
        pCoder->Set(turnSignalLeftEnabledID, (tVoid *) &turnSignalLeftEnabled);
        pCoder->Set(turnSignalRightEnabledID, (tVoid *) &turnSignalRightEnabled);
        pCoder->Set(resetSensorID, (tVoid *) &resettableModule);
        pCoder->Set(errorID, (tVoid *) &errorActive);
        pCoder->Set(maneuverCompletedID, (tVoid *) &maneuverCompleted);
    }

    RETURN_IF_FAILED(driveInstructionsOutput.Transmit(pMediaSample));
    RETURN_NOERROR;
}

void BaseDecisionModule::ResetDriveInstructions(tInt module)
{
    logger.Log("Resetting decision struct.", false);

    speed = 0;
    curveRadius = 0;
    curveAngle = 90;

    sourceModule = module;
    speedControlTaken = tFalse;
    steeringControlTaken = tFalse;

    headLightEnabled = tTrue;
    turnSignalLeftEnabled = tFalse;
    turnSignalRightEnabled = tFalse;
    hazardLightsEnabled = tFalse;
    brakeLightEnabled = tFalse;
    reverseLightEnabled = tFalse;
    errorActive = tFalse;
    maneuverCompleted = tFalse;

    resettableModule = tReadyModule::Nothing;
}
