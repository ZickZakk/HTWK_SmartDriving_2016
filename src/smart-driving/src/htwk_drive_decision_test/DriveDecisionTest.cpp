#include "DriveDecisionTest.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, DriveDecisionTest)

DriveDecisionTest::DriveDecisionTest(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    speed = 0;
    curveRadius = 0;

    shouldSend = tFalse;

    sourceModule = 11;
    speedControlTaken = tFalse;
    steeringControllTaken = tFalse;

    headLightEnabled = tFalse;
    turnSignalLeftEnabled = tFalse;
    turnSignalRightEnabled = tFalse;
    hazardLightsEnabled = tFalse;
    brakeLightEnabled = tFalse;
    reverseLightEnabled = tFalse;
    emergencyStop = tFalse;
    moduleError = tFalse;

    resModule = tReadyModule::Nothing;
    resModuleTmp = -1;

    maneuverCompleted = tFalse;

    mc = tFalse;

    InitializeProperties();

}

DriveDecisionTest::~DriveDecisionTest()
{
}

void DriveDecisionTest::InitializeProperties()
{
    SetPropertyBool(SHOULD_SEND_PROPERTY, shouldSend);
    SetPropertyBool(SHOULD_SEND_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SHOULD_SEND_PROPERTY NSSUBPROP_DESCRIPTION, "Send values");

    SetPropertyInt(SEND_SOURCE_MODULE_PROPERTY, sourceModule);
    SetPropertyBool(SEND_SOURCE_MODULE_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_SOURCE_MODULE_PROPERTY NSSUBPROP_DESCRIPTION, "Source module");

    SetPropertyBool(SEND_SPEED_TAKEN_PROPERTY, speedControlTaken);
    SetPropertyBool(SEND_SPEED_TAKEN_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_SPEED_TAKEN_PROPERTY NSSUBPROP_DESCRIPTION, "Take speed controll");

    SetPropertyFloat(SEND_SPEED_PROPERTY, speed);
    SetPropertyBool(SEND_SPEED_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_SPEED_PROPERTY NSSUBPROP_DESCRIPTION, "Speed");

    SetPropertyBool(SEND_STEERING_TAKEN_PROPERTY, steeringControllTaken);
    SetPropertyBool(SEND_STEERING_TAKEN_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_STEERING_TAKEN_PROPERTY NSSUBPROP_DESCRIPTION, "Take steering controll");

    SetPropertyFloat(SEND_STEERING_PROPERTY, curveRadius);
    SetPropertyBool(SEND_STEERING_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_STEERING_PROPERTY NSSUBPROP_DESCRIPTION, "Steering (Radius)");

    SetPropertyBool(SEND_ERROR_STATE_PROPERTY, moduleError);
    SetPropertyBool(SEND_ERROR_STATE_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_ERROR_STATE_PROPERTY NSSUBPROP_DESCRIPTION, "Module error");

    SetPropertyInt(SEND_MODULE_RESET_PROPERTY, resModuleTmp);
    SetPropertyBool(SEND_MODULE_RESET_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_MODULE_RESET_PROPERTY NSSUBPROP_DESCRIPTION, "Module reset");

    SetPropertyBool(SEND_MANEUVER_COMPLETE_PROPERTY, maneuverCompleted);
    SetPropertyBool(SEND_MANEUVER_COMPLETE_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(SEND_MANEUVER_COMPLETE_PROPERTY NSSUBPROP_DESCRIPTION, "Maneuver Completed flag");

}

tResult DriveDecisionTest::Init(tInitStage eStage, __exception)
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
        RETURN_IF_FAILED(readProperties());
    }

    else if (eStage == StageGraphReady)
    {
    }

    RETURN_NOERROR;
}

tResult DriveDecisionTest::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_FAILED(readProperties());

    //RETURN_IF_POINTER_NULL(mediaSample);

    if (shouldSend)
    {
        sendStruct();
    }

    RETURN_NOERROR;
}

tResult DriveDecisionTest::CreateInputPins()
{
    RETURN_IF_FAILED(
            triggerPin.Create("trigger", new cMediaType(0, 0, 0, "tBoolSignalValue"),
                              static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&triggerPin));

    RETURN_NOERROR;
}

tResult DriveDecisionTest::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(outDriveInstructionsStructPin.Create("drive_instructions_struct_out",
                                                          typeDriveInstructionsStructValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&outDriveInstructionsStructPin));

    RETURN_NOERROR;
}

tResult DriveDecisionTest::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    tChar const *strDescDriveStruct = descManager->GetMediaDescription("tDriveInstructionsStruct");
    RETURN_IF_POINTER_NULL(strDescDriveStruct);
    typeDriveInstructionsStructValue = new cMediaType(0, 0, 0, "tDriveInstructionsStruct", strDescDriveStruct,
                                                      IMediaDescription::MDF_DDL_DEFAULT_VERSION);

    RETURN_IF_FAILED(typeDriveInstructionsStructValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
                                                                    (tVoid **) &descriptionDriveInstructionsStruct));

    RETURN_NOERROR;
}

tResult DriveDecisionTest::sendStruct()
{
    resModule = static_cast<tReadyModule::ReadyModuleEnum>(resModuleTmp);

    tFloat32 angleTmp = 0;

    cObjectPtr<IMediaSample> pMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &pMediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionDriveInstructionsStruct->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));


    {   // focus for sample write lock

        __adtf_sample_write_lock_mediadescription(descriptionDriveInstructionsStruct, pMediaSample, pCoder);

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
            pCoder->GetID("tEmergencyStop", emergencyStopID);
            pCoder->GetID("tError", moduleErrorID);
            pCoder->GetID("tResetSensor", moduleResetID);
            pCoder->GetID("tManeuverCompleted", maneuverCompletedID);
            idsDriveInstructionsSet = tTrue;
        }

        //write date to the media sample with the coder of the descriptor
        pCoder->Set(sourceModuleID, (tVoid *) &sourceModule);
        pCoder->Set(speedControlTakenID, (tVoid *) &speedControlTaken);
        pCoder->Set(steeringControlTakenID, (tVoid *) &steeringControllTaken);
        pCoder->Set(speedID, (tVoid *) &speed);
        pCoder->Set(curveRadiusID, (tVoid *) &curveRadius);
        pCoder->Set(curveAngleID, (tVoid *) &angleTmp);
        pCoder->Set(headLightEnabledID, (tVoid *) &headLightEnabled);
        pCoder->Set(hazardLightsEnabledID, (tVoid *) &hazardLightsEnabled);
        pCoder->Set(brakeLightEnabledID, (tVoid *) &brakeLightEnabled);
        pCoder->Set(reverseLightEnabledID, (tVoid *) &reverseLightEnabled);
        pCoder->Set(emergencyStopID, (tVoid *) &emergencyStop);
        pCoder->Set(turnSignalLeftEnabledID, (tVoid *) &turnSignalLeftEnabled);
        pCoder->Set(turnSignalRightEnabledID, (tVoid *) &turnSignalRightEnabled);
        pCoder->Set(moduleErrorID, (tVoid *) &moduleError);
        pCoder->Set(moduleResetID, (tVoid *) &resModule);

        if (!mc && maneuverCompleted)
        {
            pCoder->Set(maneuverCompletedID, (tVoid *) &maneuverCompleted);
            mc = tTrue;
        }
        else
        {
            maneuverCompleted = tFalse;
            pCoder->Set(maneuverCompletedID, (tVoid *) &maneuverCompleted);
        }

    }
    logger.StartLog();
    logger.Log("Send");
    logger.EndLog();

    // transmit media sample over output pin
    RETURN_IF_FAILED(outDriveInstructionsStructPin.Transmit(pMediaSample));
    RETURN_NOERROR;
}

tResult DriveDecisionTest::readProperties()
{
    shouldSend = tBool(GetPropertyInt(SHOULD_SEND_PROPERTY));
    sourceModule = tInt8(GetPropertyInt(SEND_SOURCE_MODULE_PROPERTY));
    speedControlTaken = tBool(GetPropertyBool(SEND_SPEED_TAKEN_PROPERTY));
    speed = tFloat32(GetPropertyFloat(SEND_SPEED_PROPERTY));
    steeringControllTaken = tBool(GetPropertyBool(SEND_STEERING_TAKEN_PROPERTY));
    curveRadius = tFloat32(GetPropertyFloat(SEND_STEERING_PROPERTY));
    moduleError = tBool(GetPropertyBool(SEND_ERROR_STATE_PROPERTY));
    resModuleTmp = tInt32(GetPropertyInt(SEND_MODULE_RESET_PROPERTY));
    maneuverCompleted = tBool(GetPropertyBool(SEND_MANEUVER_COMPLETE_PROPERTY));

    RETURN_NOERROR;
}
