//
// Created by pbachmann on 2/9/16.
//

#include "Trigger.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, Trigger)

Trigger::Trigger(const tChar *__info) : cTimeTriggeredFilter(__info)
{
    interval = 30;

    SetPropertyInt(INTERVAL_PROPERTY, interval);
    SetPropertyStr(INTERVAL_PROPERTY NSSUBPROP_DESCRIPTION, "Interval time in ms");
    SetPropertyInt(INTERVAL_PROPERTY NSSUBPROP_MIN, 10);
}

Trigger::~Trigger()
{
}

tResult Trigger::Init(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(cTimeTriggeredFilter::Init(eStage, NULL));
    if (eStage == StageFirst)
    {
        CreateDescriptions(__exception_ptr);
        CreateOutputPins(__exception_ptr);
    }
    else if (eStage == StageNormal)
    {
        interval = tUInt(GetPropertyInt(INTERVAL_PROPERTY));
        SetInterval(interval * 1000);
        lastTime = _clock->GetStreamTime();
    }
    else if (eStage == StageGraphReady)
    {
    }

    RETURN_NOERROR;
}

tResult Trigger::Shutdown(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult Trigger::Cycle(IException **__exception_ptr)
{
    tTimeStamp currentTime = _clock->GetStreamTime();
    tFloat32 diff = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    RETURN_IF_FAILED(TransmitValue(triggerPin, diff));
    RETURN_NOERROR;
}

tResult Trigger::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get bool
    tChar const *strSignal = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strSignal);
    signalMediaType = new cMediaType(0, 0, 0, "tSignalValue", strSignal, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(signalMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &signalDescription));

    RETURN_NOERROR;
}

tResult Trigger::TransmitValue(cOutputPin &pin, tFloat32 value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    AllocMediaSample((tVoid **) &mediaSample);

    cObjectPtr<IMediaSerializer> serializer;
    signalDescription->GetMediaSampleSerializer(&serializer);
    mediaSample->AllocBuffer(serializer->GetDeserializedSize());

    tFloat32 f32Value = value;
    tUInt32 ui32TimeStamp = 0;

    {
        __adtf_sample_write_lock_mediadescription(signalDescription, mediaSample, pCoderOutput);
        pCoderOutput->Set("f32Value", (tVoid *) &f32Value);
        pCoderOutput->Set("ui32ArduinoTimestamp", (tVoid *) &(ui32TimeStamp));
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    pin.Transmit(mediaSample);

    RETURN_NOERROR;
}

tResult Trigger::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(triggerPin.Create("trigger", signalMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&triggerPin));

    RETURN_NOERROR;
}
