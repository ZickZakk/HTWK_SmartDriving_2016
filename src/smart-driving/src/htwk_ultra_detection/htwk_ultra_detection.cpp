/*
@author: ffreihube
*/
//#############################################################################
#include "stdafx.h"
#include "htwk_ultra_detection.h"

//#############################################################################
ADTF_FILTER_PLUGIN(FILTER_NAME, OID, cUltraDetection);

//#############################################################################
cUltraDetection::cUltraDetection(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    obstacleGrid = new cUltraGrid();
}

//#############################################################################
cUltraDetection::~cUltraDetection(void)
{
}

//#############################################################################
tResult cUltraDetection::Init(tInitStage eStage, ucom::IException **__exception_ptr)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    switch (eStage)
    {
        case StageFirst:
        {
            // create pins
            RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
            RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
            RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));

            logger.Log("StageFirst done", false);

            break;
        }

        case StageNormal:
        {
            outputFormat = new tBitmapFormat();
            outputFormat->nWidth = 400;
            outputFormat->nHeight = 400;
            outputFormat->nBitsPerPixel = 8;
            outputFormat->nPixelFormat = cImage::PF_GREYSCALE_8;
            outputFormat->nBytesPerLine = 3200;
            outputFormat->nSize = 400 * 400;
            outputFormat->nPaletteSize = 0;

            videoOutputPin.SetFormat(outputFormat, NULL);
            break;
        }

        case StageGraphReady:
        {
            break;
        }
    }

    RETURN_NOERROR;
}

//#############################################################################
tResult cUltraDetection::Start(ucom::IException **__exception_ptr)
{
    return cFilter::Start(__exception_ptr);;
}

//#############################################################################
tResult cUltraDetection::Stop(ucom::IException **__exception_ptr)
{
    return cFilter::Stop(__exception_ptr);;
}

//#############################################################################
tResult cUltraDetection::Shutdown(tInitStage eStage, ucom::IException **__exception_ptr)
{
    if (this->obstacleGrid != NULL)
    {
        delete this->obstacleGrid;
        this->obstacleGrid = NULL;
    }

    return cFilter::Shutdown(eStage, __exception_ptr);
}

//#############################################################################
tResult cUltraDetection::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    logger.Log("OnPinEvent", false);

    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (nEventCode == IPinEventSink::PE_MediaSampleReceived)
    {
        if (pSource == &this->m_oInputUFL)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_FRONT_LEFT));
        }

        if (pSource == &this->m_oInputUFCL)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_FRONT_CENTER_LEFT));
        }

        if (pSource == &this->m_oInputUFC)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_FRONT_CENTER));
        }

        if (pSource == &this->m_oInputUFCR)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_FRONT_CENTER_RIGHT));
        }

        if (pSource == &this->m_oInputUFR)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_FRONT_RIGHT));
        }

        if (pSource == &this->m_oInputUSR)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_SIDE_RIGHT));
        }

        if (pSource == &this->m_oInputUSL)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_SIDE_LEFT));
        }

        if (pSource == &this->m_oInputURL)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_REAR_LEFT));
        }

        if (pSource == &this->m_oInputURC)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_REAR_CENTER));
        }

        if (pSource == &this->m_oInputURR)
        {
            RETURN_IF_FAILED(ProcessSingleSensorInput(pMediaSample, ULTRASONIC_REAR_RIGHT));
        }

        if (pSource == &this->ultraSonicStructPin)
        {
            {
                tFloat32 frontLeft = 0;
                tFloat32 frontCenterLeft = 0;
                tFloat32 frontCenter = 0;
                tFloat32 frontCenterRight = 0;
                tFloat32 frontRight = 0;
                tFloat32 sideLeft = 0;
                tFloat32 sideRight = 0;
                tFloat32 rearLeft = 0;
                tFloat32 rearCenter = 0;
                tFloat32 rearRight = 0;
                tUInt32 ui32TimeStamp = 0;

                logger.Log("Reading struct", false);

                __adtf_sample_read_lock_mediadescription(descriptionUltraSonicStruct, pMediaSample, inputCoder);
                inputCoder->Get("tFrontLeft.f32Value", (tVoid *) &frontLeft);
                inputCoder->Get("tFrontCenterLeft.f32Value", (tVoid *) &frontCenterLeft);
                inputCoder->Get("tFrontCenter.f32Value", (tVoid *) &frontCenter);
                inputCoder->Get("tFrontCenterRight.f32Value", (tVoid *) &frontCenterRight);
                inputCoder->Get("tFrontRight.f32Value", (tVoid *) &frontRight);
                inputCoder->Get("tSideLeft.f32Value", (tVoid *) &sideLeft);
                inputCoder->Get("tSideRight.f32Value", (tVoid *) &sideRight);
                inputCoder->Get("tRearLeft.f32Value", (tVoid *) &rearLeft);
                inputCoder->Get("tRearCenter.f32Value", (tVoid *) &rearCenter);
                inputCoder->Get("tRearRight.f32Value", (tVoid *) &rearRight);
                inputCoder->Get("m_szIDSignalF32Value", (tVoid *) &ui32TimeStamp);

                logger.Log("Setting values to Grid", false);

                SetValueToGrid(ULTRASONIC_FRONT_LEFT, frontLeft);
                SetValueToGrid(ULTRASONIC_FRONT_CENTER_LEFT, frontCenterLeft);
                SetValueToGrid(ULTRASONIC_FRONT_CENTER, frontCenter);
                SetValueToGrid(ULTRASONIC_FRONT_CENTER_RIGHT, frontCenterRight);
                SetValueToGrid(ULTRASONIC_FRONT_RIGHT, frontRight);
                SetValueToGrid(ULTRASONIC_SIDE_LEFT, sideLeft);
                SetValueToGrid(ULTRASONIC_SIDE_RIGHT, sideRight);
                SetValueToGrid(ULTRASONIC_REAR_LEFT, rearLeft);
                SetValueToGrid(ULTRASONIC_REAR_CENTER, rearCenter);
                Mat outputImage = SetValueToGrid(ULTRASONIC_REAR_RIGHT, rearRight);

                logger.Log("Transmit output", false);

                RETURN_IF_FAILED(Transmit(outputImage, pMediaSample, ui32TimeStamp));
            }
        }
    }

    RETURN_NOERROR;
}

//#############################################################################
tResult cUltraDetection::ProcessSingleSensorInput(IMediaSample *pMediaSample, tUInt32 sensorCode)
{
    tFloat32 f32Value = 0;
    tUInt32 ui32TimeStamp = 0;

    {
        __adtf_sample_read_lock_mediadescription(descriptionSignalFloat, pMediaSample, pCoderInput);

        pCoderInput->Get("f32Value", (tVoid *) &f32Value);
        pCoderInput->Get("m_szIDSignalF32Value", (tVoid *) &ui32TimeStamp);
    }

    Mat outputImage = SetValueToGrid(sensorCode, f32Value);

    RETURN_IF_FAILED(Transmit(outputImage, pMediaSample, ui32TimeStamp));
    RETURN_NOERROR;
}

Mat cUltraDetection::SetValueToGrid(tUInt32 sensorCode, tFloat32 f32Value) const
{
    Mat outputImage;

    if (((f32Value * 100) < 250))
    {
        obstacleGrid->write2Grid(f32Value, sensorCode);
        outputImage = obstacleGrid->printGrid();
    }
    else
    {
        outputImage = obstacleGrid->printMax(sensorCode);
    }
    return outputImage;
}

//#############################################################################
tResult cUltraDetection::Transmit(Mat outputImage, IMediaSample *pMediaSample, tUInt32 ui32TimeStamp)
{
    cObjectPtr<IMediaSample> pNewDepthSample;
    if (IS_OK(AllocMediaSample(&pNewDepthSample)))
    {
        pNewDepthSample->Update(_clock->GetStreamTime(), outputImage.data, outputFormat->nSize, 0);
        videoOutputPin.Transmit(pNewDepthSample);
    }

    RETURN_NOERROR;
}

tResult cUltraDetection::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get signal value
    tChar const *signalValueDescription = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(signalValueDescription);
    typeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", signalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignalFloat));

    // get ultra sonic struct
    tChar const *ultraSonicStructDescription = descManager->GetMediaDescription("tUltrasonicStruct");
    RETURN_IF_POINTER_NULL(ultraSonicStructDescription);
    typeUltraSonicStructSignalValue = new cMediaType(0, 0, 0, "tUltrasonicStruct", ultraSonicStructDescription,
                                                     IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeUltraSonicStructSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
                                                                   (tVoid **) &descriptionUltraSonicStruct));

    RETURN_NOERROR;
}

tResult cUltraDetection::CreateInputPins(__exception)
{
    RETURN_IF_FAILED(m_oInputUFL.Create("UFL", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputUFL));

    RETURN_IF_FAILED(m_oInputUFCL.Create("UFCL", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputUFCL));

    RETURN_IF_FAILED(m_oInputUFC.Create("UFC", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputUFC));

    RETURN_IF_FAILED(m_oInputUFCR.Create("UFCR", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputUFCR));

    RETURN_IF_FAILED(m_oInputUFR.Create("UFR", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputUFR));

    RETURN_IF_FAILED(m_oInputUSR.Create("USR", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputUSR));

    RETURN_IF_FAILED(m_oInputUSL.Create("USL", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputUSL));

    RETURN_IF_FAILED(m_oInputURL.Create("URL", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputURL));

    RETURN_IF_FAILED(m_oInputURC.Create("URC", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputURC));

    RETURN_IF_FAILED(m_oInputURR.Create("URR", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputURR));

    RETURN_IF_FAILED(
            ultraSonicStructPin.Create("ultrasonicStruct", new cMediaType(0, 0, 0, "tUltrasonicStruct"), static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&ultraSonicStructPin));

    RETURN_NOERROR;
}

tResult cUltraDetection::CreateOutputPins(__exception)
{
    RETURN_IF_FAILED(videoOutputPin.Create("output_video", IPin::PD_Output, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&videoOutputPin));

    RETURN_NOERROR;
}
