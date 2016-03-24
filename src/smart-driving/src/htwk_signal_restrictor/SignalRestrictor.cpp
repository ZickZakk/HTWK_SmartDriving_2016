/**
 * @author pbachmann
 */

#include "SignalRestrictor.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_SIGNALRESTRICTOR, SignalRestrictor)

SignalRestrictor::SignalRestrictor(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    floatValue = 0;
    boolValue = false;
}

SignalRestrictor::~SignalRestrictor()
{
}

tResult SignalRestrictor::Init(tInitStage eStage, __exception)
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
    }
    else if (eStage == StageGraphReady)
    {
    }

    RETURN_NOERROR;
}

tResult SignalRestrictor::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2,
                                     IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &boolInputPin)
    {
        tBool newBoolValue;

        {
            __adtf_sample_read_lock_mediadescription(descriptionSignalBool, mediaSample, inputCoder);
            inputCoder->Get("bValue", (tVoid *) &newBoolValue);
        }

        if (newBoolValue != boolValue)
        {
            boolValue = newBoolValue;

            logger.Log(cString::Format("Transmitting new bool value %d", boolValue).GetPtr(), false);
            TransmitBoolValue(&boolOutputPin, boolValue);
        }
    }
    else if (source == &floatInputPin)
    {
        tFloat32 newFloatValue;

        {
            __adtf_sample_read_lock_mediadescription(descriptionSignalFloat, mediaSample, inputCoder);
            inputCoder->Get("f32Value", (tVoid *) &newFloatValue);
        }

        if (newFloatValue != floatValue)
        {
            floatValue = newFloatValue;

            logger.Log(cString::Format("Transmitting new float value %f", floatValue).GetPtr(), false);
            TransmitFloatValue(&floatOutputPin, floatValue);
        }
    }

    logger.EndLog();

    RETURN_NOERROR;
}

tResult SignalRestrictor::TransmitFloatValue(cOutputPin *outputPin, tFloat32 value)
{
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid **) &pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignalFloat->GetMediaSampleSerializer(&pSerializer);
    pMediaSample->AllocBuffer(pSerializer->GetDeserializedSize());

    tFloat32 f32Value = value;
    tUInt32 ui32TimeStamp = 0;

    //write date to the media sample with the coder of the descriptor
    {
        __adtf_sample_write_lock_mediadescription(descriptionSignalFloat, pMediaSample, pCoderOutput);

        // set value from sample
        pCoderOutput->Set("f32Value", (tVoid *) &f32Value);
        pCoderOutput->Set("ui32ArduinoTimestamp", (tVoid *) &(ui32TimeStamp));
    }

    pMediaSample->SetTime(_clock->GetStreamTime());

    //transmit media sample over output pin
    outputPin->Transmit(pMediaSample);

    RETURN_NOERROR;
}

tResult SignalRestrictor::TransmitBoolValue(cOutputPin *outputPin, tBool value)
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
    outputPin->Transmit(pMediaSample);

    RETURN_NOERROR;
}

tResult SignalRestrictor::CreateInputPins()
{
    RETURN_IF_FAILED(
            boolInputPin.Create("Bool_In", new cMediaType(0, 0, 0, "tBoolSignalValue"),
                                static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&boolInputPin));

    RETURN_IF_FAILED(
            floatInputPin.Create("Float_In", new cMediaType(0, 0, 0, "tSignalValue"),
                                 static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&floatInputPin));

    RETURN_NOERROR;
}

tResult SignalRestrictor::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(boolOutputPin.Create("Bool_Out", typeBoolSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&boolOutputPin));

    RETURN_IF_FAILED(floatOutputPin.Create("Float_out", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&floatOutputPin));

    RETURN_NOERROR;
}

tResult SignalRestrictor::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get signal value
    tChar const *signalValueDescription = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(signalValueDescription);

    typeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", signalValueDescription,
                                     IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(
            typeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignalFloat));

    // get bool
    tChar const *boolSignalValueDescription = descManager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(boolSignalValueDescription);
    typeBoolSignalValue = new cMediaType(0, 0, 0, "tBoolSignalValue",
                                         boolSignalValueDescription,
                                         IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(
            typeBoolSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignalBool));

    RETURN_NOERROR;
}
