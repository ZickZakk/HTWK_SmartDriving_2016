#include "lane_detection.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_LANE_DRIVER, LaneDetection)

LaneDetection::LaneDetection(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    SetPropertyInt(THRESH_BUFFER_SIZE_PROPERTY, 5);
    SetPropertyBool(THRESH_BUFFER_SIZE_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(THRESH_BUFFER_SIZE_PROPERTY NSSUBPROP_DESCRIPTION, "Buffer Size for threshold average calculation.");

    inputImage = Mat::zeros(200, 300, CV_8UC1);
    ipmReady = false;

    readySended = false;
}

LaneDetection::~LaneDetection()
{
}

tResult LaneDetection::ProcessLane(IMediaSample *mediaSample)
{
    Mat workingImage;
    VisionUtils::ExtractImageFromMediaSample(mediaSample, inputFormat).copyTo(workingImage);
    RETURN_IF_POINTER_NULL(workingImage.data);

    inputImage = workingImage;

    RETURN_NOERROR;
}

tResult LaneDetection::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        // create all the pins
        RETURN_IF_FAILED(InitDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        threshBufferSize = static_cast<unsigned long>(GetPropertyInt(THRESH_BUFFER_SIZE_PROPERTY));

        laneDetector.Reset(threshBufferSize);
    }
    else if (eStage == StageGraphReady)
    {
        // get the image format of the input video pin
        cObjectPtr<IMediaType> type;
        RETURN_IF_FAILED(ipmInputPin.GetMediaType(&type));

        cObjectPtr<IMediaTypeVideo> pTypeVideo;
        RETURN_IF_FAILED(type->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &pTypeVideo));

        inputFormat = *(pTypeVideo->GetFormat());
        outputFormat = *(pTypeVideo->GetFormat());
        outputFormat.nWidth = 300;
        outputFormat.nHeight = 200;
        outputFormat.nBitsPerPixel = 24;
        outputFormat.nPixelFormat = cImage::PF_RGB_888;
        outputFormat.nBytesPerLine = 3 * 300;
        outputFormat.nSize = outputFormat.nBytesPerLine * 200;
        outputFormat.nPaletteSize = 0;

        videoDebugPin.SetFormat(&outputFormat, NULL);
    }

    RETURN_NOERROR;
}

tResult LaneDetection::OnPinEvent(IPin *source,
                                  tInt nEventCode,
                                  tInt nParam1,
                                  tInt nParam2,
                                  IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &ipmInputPin)
    {
        tFloat diffBefore = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;

        RETURN_IF_FAILED(ProcessIpm(mediaSample));

        tFloat diff = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;
        logger.Log(cString::Format("Before: %f, After: %f, Diff: %f", diffBefore, diff, diff - diffBefore).GetPtr());
    }
    else if (source == &ipmReadyPin)
    {
        ProcessIpmReady(mediaSample);
    }
    else if (source == &getReadyPin)
    {
        RETURN_IF_FAILED(ProcessGetReady(mediaSample));
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}


tResult LaneDetection::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(ipmInputPin.Create("Video_IPM_input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&ipmInputPin));

    RETURN_IF_FAILED(ipmReadyPin.Create("IPM_Ready_input", typeEnumBox, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&ipmReadyPin));

    RETURN_IF_FAILED(getReadyPin.Create("Reset_input", typeEnumBox, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&getReadyPin));

    RETURN_NOERROR;
}

tResult LaneDetection::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoDebugPin.Create("Debug_Video_output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoDebugPin));

    RETURN_IF_FAILED(lanePin.Create("Lane_output", typeLane, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&lanePin));

    RETURN_IF_FAILED(readyPin.Create("Ready_output", typeEnumBox, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&readyPin));

    RETURN_NOERROR;
}

tResult LaneDetection::TransmitDebug(const Mat &debugImage, const tTimeStamp &timeStamp)
{
    if (!videoDebugPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> newMediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &newMediaSample));
    RETURN_IF_FAILED(newMediaSample->AllocBuffer(outputFormat.nSize));
    newMediaSample->Update(timeStamp, debugImage.data, outputFormat.nSize, 0);

    videoDebugPin.Transmit(newMediaSample);

    RETURN_NOERROR;
}

tResult LaneDetection::InitDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &pDescManager, __exception_ptr));

    tChar const *laneDescription = pDescManager->GetMediaDescription("tLane");
    RETURN_IF_POINTER_NULL(laneDescription);
    typeLane = new cMediaType(0, 0, 0, "tLane", laneDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeLane->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionLane));

    tChar const *enumBoxDescription = pDescManager->GetMediaDescription("tEnumBox");
    RETURN_IF_POINTER_NULL(enumBoxDescription);
    typeEnumBox = new cMediaType(0, 0, 0, "tEnumBox", enumBoxDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeEnumBox->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionEnumBox));

    RETURN_NOERROR;
}

tResult LaneDetection::ProcessIpmReady(IMediaSample *pSample)
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionEnumBox->GetMediaSampleSerializer(&pSerializer);

    tReadyModule::ReadyModuleEnum ready;
    {
        __adtf_sample_read_lock_mediadescription(descriptionEnumBox, pSample, pCoder);
        pCoder->Get("tEnumValue", (tVoid *) &ready);
    }

    if (tReadyModule::Ipm == ready)
    {
        logger.Log("IPM Ready.", false);
        ipmReady = tTrue;
    }

    RETURN_NOERROR;
}

tResult LaneDetection::TransmitResult(tLane lane)
{
    if (!lanePin.IsConnected())
    {
        RETURN_NOERROR;
    }
    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionLane->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(descriptionLane, mediaSample, pCoder);

        pCoder->Set("tLeftLine.tStart.tX", (tVoid *) &lane.tLeftLine.tStart.tX);
        pCoder->Set("tLeftLine.tStart.tY", (tVoid *) &lane.tLeftLine.tStart.tY);
        pCoder->Set("tLeftLine.tEnd.tX", (tVoid *) &lane.tLeftLine.tEnd.tX);
        pCoder->Set("tLeftLine.tEnd.tY", (tVoid *) &lane.tLeftLine.tEnd.tY);
        pCoder->Set("tLeftLine.tStatus", (tVoid *) &lane.tLeftLine.tStatus);
        pCoder->Set("tLeftLine.tCrossingDistance", (tVoid *) &lane.tLeftLine.tCrossingDistance);

        pCoder->Set("tRightLine.tStart.tX", (tVoid *) &lane.tRightLine.tStart.tX);
        pCoder->Set("tRightLine.tStart.tY", (tVoid *) &lane.tRightLine.tStart.tY);
        pCoder->Set("tRightLine.tEnd.tX", (tVoid *) &lane.tRightLine.tEnd.tX);
        pCoder->Set("tRightLine.tEnd.tY", (tVoid *) &lane.tRightLine.tEnd.tY);
        pCoder->Set("tRightLine.tStatus", (tVoid *) &lane.tRightLine.tStatus);
        pCoder->Set("tRightLine.tCrossingDistance", (tVoid *) &lane.tRightLine.tCrossingDistance);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(lanePin.Transmit(mediaSample));

    RETURN_NOERROR;

}

tResult LaneDetection::ProcessGetReady(IMediaSample *mediaSample)
{
    __synchronized_obj(safeSection);

    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionEnumBox->GetMediaSampleSerializer(&pSerializer);

    tReadyModule::ReadyModuleEnum module;

    {
        __adtf_sample_read_lock_mediadescription(descriptionEnumBox, mediaSample, pCoder);
        pCoder->Get("tEnumValue", (tVoid *) &module);
    }

    if (tReadyModule::LaneDetection == module)
    {
        logger.Log("Resetting.", false);
        readySended = false;
        laneDetector.Reset(threshBufferSize);
        TransmitResult(VisionUtils::GenerateDefaultLane());
    }
    else if (tReadyModule::Ipm == module)
    {
        logger.Log("IPM Reseted.", false);
        ipmReady = false;
    }

    RETURN_NOERROR;
}

tResult LaneDetection::TransmitReady()
{
    if (!readyPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    if (readySended)
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid **) &pMediaSample);

    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionEnumBox->GetMediaSampleSerializer(&pSerializer);
    pMediaSample->AllocBuffer(pSerializer->GetDeserializedSize());

    tInt32 value = static_cast<tInt32>(tReadyModule::LaneDetection);
    {
        __adtf_sample_write_lock_mediadescription(descriptionEnumBox, pMediaSample, pCoderOutput);
        pCoderOutput->Set("tEnumValue", (tVoid *) &value);
    }

    pMediaSample->SetTime(_clock->GetStreamTime());

    readyPin.Transmit(pMediaSample);
    readySended = true;

    RETURN_NOERROR;
}

tResult LaneDetection::ProcessIpm(IMediaSample *mediaSample)
{
    __synchronized_obj(safeSection);

    if (!ipmReady)
    {
        logger.Log("Ipm not ready yet!");
        RETURN_NOERROR;
    }

    ProcessLane(mediaSample);
    tLane result = laneDetector.DetectLanes(inputImage, logger);

//    Mat workingImage;
//    cvtColor(laneDetector.DebugImage, workingImage, CV_RGB2GRAY);
//
//    Mat templateImage = imread(GeneralUtils::GetExePath() + "resources/" + "bot_left.bmp", CV_LOAD_IMAGE_GRAYSCALE);
//
//    /// Create the result matrix
//    int result_cols = workingImage.cols - templateImage.cols + 1;
//    int result_rows = workingImage.rows - templateImage.rows + 1;
//
//    Mat resultImage = Mat::zeros(result_rows, result_cols, CV_32FC1);
//
//    /// Do the Matching and Normalize
//    matchTemplate(workingImage, templateImage, resultImage, CV_TM_SQDIFF_NORMED);
//
//    /// Localizing the best match with minMaxLoc
//    double minVal;
//    double maxVal;
//    Point minLoc;
//    Point maxLoc;
//    Point matchLoc;
//
//    minMaxLoc(resultImage, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
//
//    matchLoc = minLoc;
//
//    /// Show me what you got
//    rectangle(laneDetector.DebugImage, matchLoc, Point(matchLoc.x + templateImage.cols, matchLoc.y + templateImage.rows), CV_RGB(255,0,0), 2, 8, 0);
//    logger.Log(cString::Format("Value: %f", minVal).GetPtr(), false);

    RETURN_IF_FAILED(TransmitDebug(laneDetector.DebugImage, mediaSample->GetTime()));
    RETURN_IF_FAILED(TransmitResult(result));

    if (laneDetector.IsReady())
    {
        RETURN_IF_FAILED(TransmitReady());
    }

    RETURN_NOERROR;
}
