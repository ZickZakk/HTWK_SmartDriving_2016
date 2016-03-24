#include "SDComparer.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, FILTER_OID, SDComparer)


SDComparer::SDComparer(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    speed = 0.0f;
    buffer = 0;
    senderCarDist = localCarDist = 0.0f;
    InitializeProperties();
}


SDComparer::~SDComparer()
{
}


void SDComparer::InitializeProperties()
{
    /*
    SetPropertyFloat(STEERING_COMPANSATION_FACTOR_PROPERTY, compensationValue);
    SetPropertyBool(STEERING_COMPANSATION_FACTOR_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(STEERING_COMPANSATION_FACTOR_PROPERTY NSSUBPROP_DESCRIPTION, "P-Part");
     */
}


tResult SDComparer::Init(tInitStage eStage, __exception)
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
        /*
        compensationValue = GetPropertyFloat(STEERING_COMPANSATION_FACTOR_PROPERTY);
         */
    }
    else if (eStage == StageGraphReady)
    {
        m_bIDsSignalSet = tFalse;
    }
    RETURN_NOERROR;
}


tResult SDComparer::Start(IException **__exception_ptr)
{
    RETURN_NOERROR;
}


tResult SDComparer::Shutdown(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    return cFilter::Shutdown(eStage, __exception_ptr);
}


tResult SDComparer::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pMediaSample);
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    logger.StartLog();

    if (pSource == &senderCarDistInPin)
    {
        buffer++;
        senderCarDist = GetFloat(pMediaSample);
        logger.Log(cString::Format("localDist %f senderDist %f speed %f", localCarDist, senderCarDist, speed).GetPtr());
        if ((localCarDist >= senderCarDist * 1.05) && buffer >= 10)
        {
            speed = NEUTRAL_SPEED;
            buffer = 0;
        }
    }
    else if (pSource == &localCarDistInPin)
    {
        localCarDist = GetFloat(pMediaSample);
    }
    else if (pSource == &speedInPin)
    {
        speed = GetFloat(pMediaSample);
    }

    TransmitSignalValue(&speedOutPin, speed);

    logger.EndLog();

    RETURN_NOERROR;
}


tResult SDComparer::CreateDescriptions(IException **__exception_ptr)
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


tResult SDComparer::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(senderCarDistInPin.Create("senderCarDist", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&senderCarDistInPin));

    RETURN_IF_FAILED(localCarDistInPin.Create("localCarDist", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&localCarDistInPin));

    RETURN_IF_FAILED(speedInPin.Create("speedIn", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&speedInPin));
    RETURN_NOERROR;
}


tResult SDComparer::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(speedOutPin.Create("speedOut", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&speedOutPin));
    RETURN_NOERROR;
}


tFloat32 SDComparer::GetFloat(IMediaSample *mediaSample)
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


tResult SDComparer::TransmitSignalValue(cOutputPin *outputPin, tFloat32 value)
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
