#include "IMUOutlierRemover.h"

ADTF_FILTER_PLUGIN(IMUOUTLIERREMOVER_NAME, IMUOUTLIERREMOVER_OID, IMUOutlierRemover)


IMUOutlierRemover::IMUOutlierRemover(const tChar *__info) : cFilter(__info), logger(IMUOUTLIERREMOVER_NAME, 20)
{
    yaw = oldYaw = 0.0f;
    timeStamp = oldTimeStamp = 0.0f;
    isYawInitialized = tFalse;
    yawInitValues.clear();
}


IMUOutlierRemover::~IMUOutlierRemover()
{
}


tResult IMUOutlierRemover::Init(tInitStage eStage, __exception)
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
    }
    else if (eStage == StageGraphReady)
    {
    }
    RETURN_NOERROR;
}


tResult IMUOutlierRemover::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pMediaSample);
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    logger.StartLog();

    if (pSource == &yawInPin)
    {
        GetFloat(pMediaSample, yaw, timeStamp);

        if (!isYawInitialized)
        {
            isYawInitialized = InitYaw();
            RETURN_NOERROR;
        }

        if (IsDirectionChangePlausible())
        {
            ValidateYaw();
            TransmitSignalValue(&yawOutPin, yaw);
        }
        else
        {
            logger.Log(cString::Format("removed: oldYaw %f (%d), yaw %f (%d)",
                                       oldYaw, oldTimeStamp, yaw, timeStamp).GetPtr(), tFalse);
        }
    }

    logger.EndLog();

    RETURN_NOERROR;
}


tBool IMUOutlierRemover::InitYaw()
{
    // collect some yaw and calculate median
    if (yawInitValues.size() < NUMBER_OF_INIT_VALUES)
    {
        yawInitValues.push_back(yaw);
        return tFalse;
    }
    sort(yawInitValues.begin(), yawInitValues.end());
    oldYaw = yawInitValues[NUMBER_OF_INIT_VALUES/2];
    oldTimeStamp = timeStamp;
    return tTrue;
}


tBool IMUOutlierRemover::IsDirectionChangePlausible()
{
    return !(fabs(CalcYawDelta(oldYaw, yaw)) > 60.0f && (timeStamp - oldTimeStamp) < 1750000);
}


tResult IMUOutlierRemover::ValidateYaw()
{
    oldYaw = yaw;
    oldTimeStamp = timeStamp;
    RETURN_NOERROR;
}


tResult IMUOutlierRemover::CreateDescriptions(IException **__exception_ptr)
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


tResult IMUOutlierRemover::CreateInputPins(IException **__exception_ptr)
{
    // Yaw
    RETURN_IF_FAILED(yawInPin.Create("raw_yaw", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&yawInPin));
   RETURN_NOERROR;
}


tResult IMUOutlierRemover::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(yawOutPin.Create("adjusted_yaw", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&yawOutPin));
    RETURN_NOERROR;
}


tResult IMUOutlierRemover::GetFloat(IMediaSample *mediaSample, tFloat32& fValue, tUInt32& nTimeStamp)
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignalValue->GetMediaSampleSerializer(&pSerializer);

    {
        __adtf_sample_read_lock_mediadescription(descriptionSignalValue, mediaSample, pCoder);

        pCoder->Get("f32Value", (tVoid *) &fValue);
        pCoder->Get("ui32ArduinoTimestamp", (tVoid *) &nTimeStamp);
    }

    RETURN_NOERROR;
}


tResult IMUOutlierRemover::TransmitSignalValue(cOutputPin *outputPin, tFloat32 value)
{
    tUInt32 timeStamp = 0;
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignalValue->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    cObjectPtr<IMediaSample> newMediaSample;
    AllocMediaSample((tVoid **) &newMediaSample);
    newMediaSample->AllocBuffer(nSize);

    {
        __adtf_sample_write_lock_mediadescription(descriptionSignalValue, newMediaSample, outputCoder);
        outputCoder->Set("f32Value", (tVoid *) &(value));
        outputCoder->Set("ui32ArduinoTimestamp", (tVoid *) &timeStamp);
    }

    newMediaSample->SetTime(_clock->GetTime());
    outputPin->Transmit(newMediaSample);

    RETURN_NOERROR;
}

