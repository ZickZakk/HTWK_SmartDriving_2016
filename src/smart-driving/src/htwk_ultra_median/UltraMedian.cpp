/*
@author: pbachmann
*/

#include "UltraMedian.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, UltraMedian);

UltraMedian::UltraMedian(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    InitializeProperties();
    readIndex = 0;
}

void UltraMedian::InitializeProperties()
{
    bufferSize = 5;

    SetPropertyInt(BUFFER_SIZE_PROPERTY, tInt8(bufferSize));
    SetPropertyInt(BUFFER_SIZE_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(BUFFER_SIZE_PROPERTY NSSUBPROP_DESCRIPTION, "Buffersize");
}

UltraMedian::~UltraMedian(void)
{
}

tResult UltraMedian::Init(tInitStage eStage, ucom::IException **__exception_ptr)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    switch (eStage)
    {
        case StageFirst:
        {
            RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
            RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
            RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
            break;
        }
        case StageNormal:
        {
            bufferSize = tUInt32(GetPropertyInt(BUFFER_SIZE_PROPERTY));

            frontLeftBuffer = vector<tFloat32>(bufferSize);
            frontCenterLeftBuffer = vector<tFloat32>(bufferSize);
            frontCenterBuffer = vector<tFloat32>(bufferSize);
            frontCenterRightBuffer = vector<tFloat32>(bufferSize);
            frontRightBuffer = vector<tFloat32>(bufferSize);
            sideLeftBuffer = vector<tFloat32>(bufferSize);
            sideRightBuffer = vector<tFloat32>(bufferSize);
            rearLeftBuffer = vector<tFloat32>(bufferSize);
            rearCenterBuffer = vector<tFloat32>(bufferSize);
            rearRightBuffer = vector<tFloat32>(bufferSize);

            for (unsigned long i = 0; i < bufferSize; ++i)
            {
                frontLeftBuffer.push_back(0);
                frontCenterLeftBuffer.push_back(0);
                frontCenterBuffer.push_back(0);
                frontCenterRightBuffer.push_back(0);
                frontRightBuffer.push_back(0);
                sideLeftBuffer.push_back(0);
                sideRightBuffer.push_back(0);
                rearLeftBuffer.push_back(0);
                rearCenterBuffer.push_back(0);
                rearRightBuffer.push_back(0);
            }

            frontLeftTotal = 0;
            frontCenterLeftTotal = 0;
            frontCenterTotal = 0;
            frontCenterRightTotal = 0;
            frontRightTotal = 0;
            sideLeftTotal = 0;
            sideRightTotal = 0;
            rearLeftTotal = 0;
            rearCenterTotal = 0;
            rearRightTotal = 0;

            break;
        }
        case StageGraphReady:
        {
            break;
        }
    }

    RETURN_NOERROR;
}


tResult UltraMedian::Start(ucom::IException **__exception_ptr)
{
    return cFilter::Start(__exception_ptr);;
}


tResult UltraMedian::Stop(ucom::IException **__exception_ptr)
{
    return cFilter::Stop(__exception_ptr);;
}

tResult UltraMedian::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (IPinEventSink::PE_MediaSampleReceived != nEventCode)
    {
        RETURN_NOERROR;
    }

    if (pSource == &this->ultraSonicInput)
    {
        {
            tFloat32 val;

            __adtf_sample_read_lock_mediadescription(descriptionUltraSonicStruct, pMediaSample, inputCoder);

            inputCoder->Get("tFrontLeft.f32Value", (tVoid *) &val);
            frontLeftTotal -= frontLeftBuffer.at(readIndex);
            frontLeftBuffer[readIndex] = val;
            frontLeftTotal += frontLeftBuffer.at(readIndex);

            inputCoder->Get("tFrontCenterLeft.f32Value", (tVoid *) &val);
            frontCenterLeftTotal -= frontCenterLeftBuffer.at(readIndex);
            frontCenterLeftBuffer[readIndex] = val;
            frontCenterLeftTotal += frontCenterLeftBuffer.at(readIndex);

            inputCoder->Get("tFrontCenter.f32Value", (tVoid *) &val);
            frontCenterTotal -= frontCenterBuffer.at(readIndex);
            frontCenterBuffer[readIndex] = val;
            frontCenterTotal += frontCenterBuffer.at(readIndex);

            inputCoder->Get("tFrontCenterRight.f32Value", (tVoid *) &val);
            frontCenterRightTotal -= frontCenterRightBuffer.at(readIndex);
            frontCenterRightBuffer[readIndex] = val;
            frontCenterRightTotal += frontCenterRightBuffer.at(readIndex);

            inputCoder->Get("tFrontRight.f32Value", (tVoid *) &val);
            frontRightTotal -= frontRightBuffer.at(readIndex);
            frontRightBuffer[readIndex] = val;
            frontRightTotal += frontRightBuffer.at(readIndex);

            inputCoder->Get("tSideLeft.f32Value", (tVoid *) &val);
            sideLeftTotal -= sideLeftBuffer.at(readIndex);
            sideLeftBuffer[readIndex] = val;
            sideLeftTotal += sideLeftBuffer.at(readIndex);

            inputCoder->Get("tSideRight.f32Value", (tVoid *) &val);
            sideRightTotal -= sideRightBuffer.at(readIndex);
            sideRightBuffer[readIndex] = val;
            sideRightTotal += sideRightBuffer.at(readIndex);

            inputCoder->Get("tRearLeft.f32Value", (tVoid *) &val);
            rearLeftTotal -= rearLeftBuffer.at(readIndex);
            rearLeftBuffer[readIndex] = val;
            rearLeftTotal += rearLeftBuffer.at(readIndex);

            inputCoder->Get("tRearCenter.f32Value", (tVoid *) &val);
            rearCenterTotal -= rearCenterBuffer.at(readIndex);
            rearCenterBuffer[readIndex] = val;
            rearCenterTotal += rearCenterBuffer.at(readIndex);

            inputCoder->Get("tRearRight.f32Value", (tVoid *) &val);
            rearRightTotal -= rearRightBuffer.at(readIndex);
            rearRightBuffer[readIndex] = val;
            rearRightTotal += rearRightBuffer.at(readIndex);

            readIndex = (readIndex + 1) % bufferSize;
        }

        TransmitStruct();
    }

    RETURN_NOERROR;
}

tResult UltraMedian::TransmitStruct()
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionUltraSonicStruct->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    cObjectPtr<IMediaSample> mediaSample;
    AllocMediaSample((tVoid **) &mediaSample);
    mediaSample->AllocBuffer(nSize);

    tFloat32 frontLeft = frontLeftTotal / bufferSize;
    tFloat32 frontCenterLeft = frontCenterLeftTotal / bufferSize;
    tFloat32 frontCenter = frontCenterTotal / bufferSize;
    tFloat32 frontCenterRight = frontCenterRightTotal / bufferSize;
    tFloat32 frontRight = frontRightTotal / bufferSize;
    tFloat32 sideLeft = sideLeftTotal / bufferSize;
    tFloat32 sideRight = sideRightTotal / bufferSize;
    tFloat32 rearLeft = rearLeftTotal / bufferSize;
    tFloat32 rearCenter = rearCenterTotal / bufferSize;
    tFloat32 rearRight = rearRightTotal / bufferSize;

    {
        __adtf_sample_write_lock_mediadescription(descriptionUltraSonicStruct, mediaSample, coderOutput);

        coderOutput->Set("tFrontLeft.f32Value", (tVoid *) &frontLeft);
        coderOutput->Set("tFrontCenterLeft.f32Value", (tVoid *) &frontCenterLeft);
        coderOutput->Set("tFrontCenter.f32Value", (tVoid *) &frontCenter);
        coderOutput->Set("tFrontCenterRight.f32Value", (tVoid *) &frontCenterRight);
        coderOutput->Set("tFrontRight.f32Value", (tVoid *) &frontRight);
        coderOutput->Set("tSideLeft.f32Value", (tVoid *) &sideLeft);
        coderOutput->Set("tSideRight.f32Value", (tVoid *) &sideRight);
        coderOutput->Set("tRearLeft.f32Value", (tVoid *) &rearLeft);
        coderOutput->Set("tRearCenter.f32Value", (tVoid *) &rearCenter);
        coderOutput->Set("tRearRight.f32Value", (tVoid *) &rearRight);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    ultraSonicOutput.Transmit(mediaSample);

    RETURN_NOERROR;
}

tResult UltraMedian::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get ultra sonic struct
    tChar const *ultraSonicStructDescription = descManager->GetMediaDescription("tUltrasonicStruct");
    RETURN_IF_POINTER_NULL(ultraSonicStructDescription);
    typeUltraSonicStructSignalValue = new cMediaType(0, 0, 0, "tUltrasonicStruct", ultraSonicStructDescription,
                                                     IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeUltraSonicStructSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionUltraSonicStruct));

    RETURN_NOERROR;
}

tResult UltraMedian::CreateInputPins(__exception)
{
    RETURN_IF_FAILED(
            ultraSonicInput.Create("ultrasonicStruct_in", new cMediaType(0, 0, 0, "tUltrasonicStruct"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&ultraSonicInput));
    RETURN_NOERROR;
}

tResult UltraMedian::CreateOutputPins(__exception)
{
    RETURN_IF_FAILED(ultraSonicOutput.Create("ultrasonicStruct_out", typeUltraSonicStructSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&ultraSonicOutput));
    RETURN_NOERROR;
}
