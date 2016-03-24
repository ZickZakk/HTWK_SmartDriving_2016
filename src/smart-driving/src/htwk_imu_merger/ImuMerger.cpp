/*
@author: gjenschmischek
*/

#include "ImuMerger.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, ImuMerger);

ImuMerger::ImuMerger(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
}

ImuMerger::~ImuMerger(void)
{
}

tResult ImuMerger::Init(tInitStage eStage, ucom::IException **__exception_ptr)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }

    RETURN_NOERROR;
}

tResult ImuMerger::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (IPinEventSink::PE_MediaSampleReceived != nEventCode)
    {
        RETURN_NOERROR;
    }

    if (pSource == &YawInput)
    {
        imu.tYaw = GetValue(pMediaSample);
    }
    else if (pSource == &RollInput)
    {
        imu.tRoll = GetValue(pMediaSample);
    }
    else if (pSource == &PitchInput)
    {
        imu.tPitch = GetValue(pMediaSample);
    }
    else if (pSource == &AccXInput)
    {
        imu.tAccX = GetValue(pMediaSample);
    }
    else if (pSource == &AccYInput)
    {
        imu.tAccY = GetValue(pMediaSample);
    }
    else if (pSource == &AccZInput)
    {
        imu.tAccZ = GetValue(pMediaSample);
    }

    TransmitStruct();

    RETURN_NOERROR;
}

tResult ImuMerger::TransmitStruct()
{
    if (!imuStructOutput.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionImuStruct->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    cObjectPtr<IMediaSample> mediaSample;
    AllocMediaSample((tVoid **) &mediaSample);
    mediaSample->AllocBuffer(nSize);

    {
        __adtf_sample_write_lock_mediadescription(descriptionImuStruct, mediaSample, coderOutput);

        coderOutput->Set("tYaw", (tVoid *) &imu.tYaw);
        coderOutput->Set("tAccX", (tVoid *) &imu.tAccX);
        coderOutput->Set("tAccY", (tVoid *) &imu.tAccY);
        coderOutput->Set("tAccZ", (tVoid *) &imu.tAccZ);
        coderOutput->Set("tPitch", (tVoid *) &imu.tPitch);
        coderOutput->Set("tRoll", (tVoid *) &imu.tRoll);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    imuStructOutput.Transmit(mediaSample);

    RETURN_NOERROR;
}

tResult ImuMerger::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get Signal Value
    tChar const *signalValueDescription = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(signalValueDescription);
    typeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", signalValueDescription,
                                     IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignalValue));

    // get IMU Struct
    tChar const *imuStructDescription = descManager->GetMediaDescription("tIMU");
    RETURN_IF_POINTER_NULL(imuStructDescription);
    typeImuStruct = new cMediaType(0, 0, 0, "tIMU", imuStructDescription,
                                   IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeImuStruct->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionImuStruct));

    RETURN_NOERROR;
}

tResult ImuMerger::CreateInputPins(__exception)
{
    RETURN_IF_FAILED(
            YawInput.Create("yaw", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&YawInput));
    RETURN_IF_FAILED(
            PitchInput.Create("pitch", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&PitchInput));
    RETURN_IF_FAILED(
            RollInput.Create("roll", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&RollInput));
    RETURN_IF_FAILED(
            AccXInput.Create("accX", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&AccXInput));
    RETURN_IF_FAILED(
            AccYInput.Create("accY", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&AccYInput));
    RETURN_IF_FAILED(
            AccZInput.Create("accZ", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&AccZInput));

    RETURN_NOERROR;
}

tResult ImuMerger::CreateOutputPins(__exception)
{
    RETURN_IF_FAILED(imuStructOutput.Create("imu_struct", typeImuStruct, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&imuStructOutput));
    RETURN_NOERROR;
}

tFloat32 ImuMerger::GetValue(IMediaSample *mediaSample)
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignalValue->GetMediaSampleSerializer(&pSerializer);

    tFloat32 value;
    {
        __adtf_sample_read_lock_mediadescription(descriptionSignalValue, mediaSample, pCoder);

        pCoder->Get("f32Value", (tVoid *) &value);
    }
    return value;
}
