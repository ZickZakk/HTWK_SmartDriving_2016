#include "YawToSteer.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, FILTER_OID, YawToSteer)


YawToSteer::YawToSteer(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    oldSenderYaw = senderYaw = deltaSenderYaw = 0.0f;
    oldLocalYaw = localYaw = targetLocalYaw = 0.0f;
    steer = STEERING_STRAIGHT;
    isSenderYawInitialized = tFalse;
    isLocalYawInitialized = tFalse;
    compensationValue = STEERING_COMPENSATION_FACTOR;
    deviationThreshold = DEVIATION_THRESHOLD;
    abruptVariationThreshold = ABRUPT_VARIATION_THRESHOLD;
    senderSpeed = oldSenderSpeed = 0.0f;
    directionCorrection = 1;
    InitializeProperties();
}


YawToSteer::~YawToSteer()
{
}


void YawToSteer::InitializeProperties()
{
    SetPropertyFloat(STEERING_COMPANSATION_FACTOR_PROPERTY, compensationValue);
    SetPropertyBool(STEERING_COMPANSATION_FACTOR_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(STEERING_COMPANSATION_FACTOR_PROPERTY NSSUBPROP_DESCRIPTION, "P-Part");

    SetPropertyFloat(DEVIATION_THRESHOLD_PROPERTY, deviationThreshold);
    SetPropertyBool(DEVIATION_THRESHOLD_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(DEVIATION_THRESHOLD_PROPERTY NSSUBPROP_DESCRIPTION, "Threshold before steering");

    SetPropertyFloat(ABRUPT_VARIATION_THRESHOLD_PROPERTY, abruptVariationThreshold);
    SetPropertyBool(ABRUPT_VARIATION_THRESHOLD_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(ABRUPT_VARIATION_THRESHOLD_PROPERTY NSSUBPROP_DESCRIPTION, "Reinit if senderYaw suddenly changes above this value");
}


tResult YawToSteer::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));
    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        compensationValue = GetPropertyFloat(STEERING_COMPANSATION_FACTOR_PROPERTY);
        deviationThreshold = GetPropertyFloat(DEVIATION_THRESHOLD_PROPERTY);
        abruptVariationThreshold = GetPropertyFloat(ABRUPT_VARIATION_THRESHOLD_PROPERTY);
    }
    else if (eStage == StageGraphReady)
    {
        m_bIDsSignalSet = tFalse;
    }
    RETURN_NOERROR;
}


tResult YawToSteer::Start(IException **__exception_ptr)
{
    RETURN_NOERROR;
}


tResult YawToSteer::Shutdown(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    return cFilter::Shutdown(eStage, __exception_ptr);
}


tResult YawToSteer::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pMediaSample);
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    logger.StartLog();

    if (pSource == &senderCarYawInPin)
    {
        GetSenderYaw(pMediaSample);
        // init senderYaw if necessary
        if (!isSenderYawInitialized)
        {
            oldSenderYaw = senderYaw;
            isSenderYawInitialized = tTrue;
        }

        deltaSenderYaw = CalcYawDelta(oldSenderYaw, senderYaw);

        if (deltaSenderYaw > abruptVariationThreshold)
        {
            isLocalYawInitialized = tFalse;
            isSenderYawInitialized = tFalse;
            targetLocalYaw = 0;
            RETURN_NOERROR;
        }
            // ignore yaw values if deviation compared to last value less than x (0.2)
        else if (abs(deltaSenderYaw) > 0.2)
        {

            targetLocalYaw += deltaSenderYaw;
        }
    }
    else if (pSource == &localCarYawInPin)
    {
        GetLocalYaw(pMediaSample);
        // init localYaw if necessary
        if (!isLocalYawInitialized && isSenderYawInitialized)
        {
            targetLocalYaw = localYaw;
            isLocalYawInitialized = tTrue;
        }
    }
    else if (pSource == &senderCarSpeedInPin)
    {
        senderSpeed = GetFloat(pMediaSample);
        directionCorrection = (senderSpeed > SPEED_STATIONARY) ? -1 : 1;
    }

    if (isSenderYawInitialized && isLocalYawInitialized)
    {
        CalculateSteering();
        TransmitSignalValue(&steerOutPin, steer);
    }
    else
    {
        steer = STEERING_STRAIGHT;
    }

    logger.Log(cString::Format("sendYaw %f |localY %f targetY %f steer %f | %f\n"
                                       "\tSpeed: %f; corr: %f",
                               senderYaw, localYaw, targetLocalYaw, steer,
                               CalcYawDelta(targetLocalYaw, localYaw), senderSpeed, directionCorrection).GetPtr());

    logger.EndLog();

    RETURN_NOERROR;
}


tResult YawToSteer::GetSenderYaw(IMediaSample *pMediaSample)
{
    oldSenderYaw = senderYaw;
    senderYaw = GetFloat(pMediaSample);
    RETURN_NOERROR;
}


tResult YawToSteer::GetLocalYaw(IMediaSample *pMediaSample)
{
    oldLocalYaw = localYaw;
    localYaw = GetFloat(pMediaSample);
    RETURN_NOERROR;
}


tResult YawToSteer::CalculateSteering()
{
    tFloat deviation = CalcYawDelta(targetLocalYaw, localYaw);

    if (abs(deviation) < deviationThreshold)
    {
        steer = STEERING_STRAIGHT;
    }
    else
    {
        DirectionChange(CalcCorrectionAngle(deviation));
    }
    RETURN_NOERROR;
}


tFloat YawToSteer::CalcCorrectionAngle(tFloat deviationAngle)
{
    return (deviationAngle / STEERING_COMPENSATION_FACTOR);
}


tResult YawToSteer::DirectionChange(tFloat angle)
{
    if ((steer + angle) < STEERING_MAX_LEFT)
    {
        steer = STEERING_MAX_LEFT;
    }
    else if ((steer + angle) > STEERING_MAX_RIGHT)
    {
        steer = STEERING_MAX_RIGHT;
    }
    else
    {
        steer += angle;
    }
    steer = ((steer - 90) * directionCorrection) + 90;
    RETURN_NOERROR;
}


tResult YawToSteer::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));
    // Get Signal Value
    tChar const *signalValueDescription = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(signalValueDescription);
    typeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", signalValueDescription,
                                     IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignalValue));
    RETURN_NOERROR;
}


tResult YawToSteer::CreateInputPins(IException **__exception_ptr)
{
    // Sender Yaw
    RETURN_IF_FAILED(senderCarYawInPin.Create("senderYaw", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&senderCarYawInPin));
    // Local Yaw
    RETURN_IF_FAILED(localCarYawInPin.Create("localYaw", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&localCarYawInPin));
    // Sender speed
    RETURN_IF_FAILED(senderCarSpeedInPin.Create("senderSpeed", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&senderCarSpeedInPin));
    RETURN_NOERROR;
}


tResult YawToSteer::CreateOutputPins(IException **__exception_ptr)
{
    // Steering Pin, connect to SteeringController of Basic_Config
    RETURN_IF_FAILED(steerOutPin.Create("steer", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&steerOutPin));
    RETURN_NOERROR;
}


tFloat32 YawToSteer::GetFloat(IMediaSample *mediaSample)
{
    __synchronized_obj(iosync);

    tFloat32 value = 0.0f;
    {
        __adtf_sample_read_lock_mediadescription(descriptionSignalValue, mediaSample, pCoderInput);

        // get the IDs for the items in the media sample
        if (!m_bIDsSignalSet)
        {
            pCoderInput->GetID("f32Value", m_szIDSignalF32Value);
            m_bIDsSignalSet = tTrue;
        }
        //get values from media sample
        pCoderInput->Get(m_szIDSignalF32Value, (tVoid *) &value);
    }
    return value;
}


tResult YawToSteer::TransmitSignalValue(cOutputPin *outputPin, tFloat32 value)
{
    __synchronized_obj(iosync);

    tUInt32 timeStamp = 0;
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignalValue->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    cObjectPtr<IMediaSample> newMediaSample;
    AllocMediaSample((tVoid **) &newMediaSample);
    newMediaSample->AllocBuffer(nSize);

    {
        __adtf_sample_write_lock_mediadescription(descriptionSignalValue, newMediaSample, pCoderOutput);

        if (!m_bIDsSignalSet)
        {
            pCoderOutput->GetID("f32Value", m_szIDSignalF32Value);
            pCoderOutput->GetID("ui32ArduinoTimestamp", m_szIDSignalArduinoTimestamp);
            m_bIDsSignalSet = tTrue;
        }

        // set values in new media sample
        pCoderOutput->Set(m_szIDSignalF32Value, (tVoid *) &(value));
        pCoderOutput->Set(m_szIDSignalArduinoTimestamp, (tVoid *) &timeStamp);
    }

    newMediaSample->SetTime(_clock->GetTime());
    outputPin->Transmit(newMediaSample);
    RETURN_NOERROR;
}
