#include "RegionOfInterest.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_ROI, ROI)

ROI::ROI(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    hoodHeight = 0;
    roomHeight = 0;
    hoodDetectionCount = 0;
    processingWidthPercentage = 75;

    InitializeProperties();
}

ROI::~ROI()
{
}

void ROI::InitializeProperties()
{
    hoodScanLineNumber = 4;
    roomScanLineNumber = 4;
    maxHoodDetectionCount = 5;
    isHoodDetectionEnabled = true;
    isRoomDetectionEnabled = true;
    rgbVideoManipulation = VIDEO_NONE;
    depthVideoManipulation = VIDEO_NONE;
    roomHeightManipulation = 0.85f;

    SetPropertyInt(HOOD_SCANLINE_NUMBER_PROPERTY, hoodScanLineNumber);
    SetPropertyBool(HOOD_SCANLINE_NUMBER_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(HOOD_SCANLINE_NUMBER_PROPERTY NSSUBPROP_DESCRIPTION, "Number of hood scanlines");
    SetPropertyInt(HOOD_SCANLINE_NUMBER_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyFloat(ROOM_HEIGHT_MANIPULATION_PROPERTY, roomHeightManipulation);
    SetPropertyBool(ROOM_HEIGHT_MANIPULATION_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(ROOM_HEIGHT_MANIPULATION_PROPERTY NSSUBPROP_DESCRIPTION, "Reduces the room height by the given percentage");
    SetPropertyFloat(ROOM_HEIGHT_MANIPULATION_PROPERTY NSSUBPROP_MIN, 0);
    SetPropertyFloat(ROOM_HEIGHT_MANIPULATION_PROPERTY NSSUBPROP_MAX, 1);

    SetPropertyInt(ROOM_SCANLINE_NUMBER_PROPERTY, roomScanLineNumber);
    SetPropertyBool(ROOM_SCANLINE_NUMBER_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(ROOM_SCANLINE_NUMBER_PROPERTY NSSUBPROP_DESCRIPTION, "Number of room scanlines");
    SetPropertyInt(ROOM_SCANLINE_NUMBER_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyInt(MAX_HOOD_DETECTION_COUNT_PROPERTY, maxHoodDetectionCount);
    SetPropertyBool(MAX_HOOD_DETECTION_COUNT_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(MAX_HOOD_DETECTION_COUNT_PROPERTY NSSUBPROP_DESCRIPTION, "Max hood detection dount");
    SetPropertyInt(MAX_HOOD_DETECTION_COUNT_PROPERTY NSSUBPROP_MIN, 0);

    SetPropertyBool(DETECT_HOOD_PROPERTY, isHoodDetectionEnabled);
    SetPropertyBool(DETECT_HOOD_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(DETECT_HOOD_PROPERTY NSSUBPROP_DESCRIPTION, "Detect hood");

    SetPropertyBool(DETECT_ROOM_PROPERTY, isRoomDetectionEnabled);
    SetPropertyBool(DETECT_ROOM_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(DETECT_ROOM_PROPERTY NSSUBPROP_DESCRIPTION, "Detect room");

    SetPropertyInt(RGB_VIDEO_MANIPULATION_PROPERTY, rgbVideoManipulation);
    SetPropertyBool(RGB_VIDEO_MANIPULATION_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(RGB_VIDEO_MANIPULATION_PROPERTY NSSUBPROP_VALUELIST, "1@None|2@Crop|3@Rect");
    SetPropertyStr(RGB_VIDEO_MANIPULATION_PROPERTY NSSUBPROP_DESCRIPTION, "Defines method which is used to manipulate the RGB image");

    SetPropertyInt(DEPTH_VIDEO_MANIPULATION_PROPERTY, depthVideoManipulation);
    SetPropertyBool(DEPTH_VIDEO_MANIPULATION_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(DEPTH_VIDEO_MANIPULATION_PROPERTY NSSUBPROP_VALUELIST, "1@None|2@Crop|3@Rect");
    SetPropertyStr(DEPTH_VIDEO_MANIPULATION_PROPERTY NSSUBPROP_DESCRIPTION, "Defines method which is used to manipulate the Depth image");
}

tResult ROI::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(InitDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        hoodScanLineNumber = GetPropertyInt(HOOD_SCANLINE_NUMBER_PROPERTY);
        roomScanLineNumber = GetPropertyInt(ROOM_SCANLINE_NUMBER_PROPERTY);
        maxHoodDetectionCount = GetPropertyInt(MAX_HOOD_DETECTION_COUNT_PROPERTY);
        rgbVideoManipulation = GetPropertyInt(RGB_VIDEO_MANIPULATION_PROPERTY);
        depthVideoManipulation = GetPropertyInt(DEPTH_VIDEO_MANIPULATION_PROPERTY);
        isHoodDetectionEnabled = GetPropertyBool(DETECT_HOOD_PROPERTY);
        isRoomDetectionEnabled = GetPropertyBool(DETECT_ROOM_PROPERTY);
        roomHeightManipulation = GetPropertyFloat(ROOM_HEIGHT_MANIPULATION_PROPERTY);

        logger.Log(cString::Format("roomHeightManipulation: %d", roomHeightManipulation).GetPtr(), false);
        logger.Log(cString::Format("hoodScanLineNumber: %d", hoodScanLineNumber).GetPtr(), false);
        logger.Log(cString::Format("roomScanLineNumber: %d", roomScanLineNumber).GetPtr(), false);
        logger.Log(cString::Format("processingWidthPercentage: %d", processingWidthPercentage).GetPtr(), false);
        logger.Log(cString::Format("maxHoodDetectionCount: %d", maxHoodDetectionCount).GetPtr(), false);
        logger.Log(cString::Format("rgbVideoManipulation: %d", rgbVideoManipulation).GetPtr(), false);
        logger.Log(cString::Format("depthVideoManipulation: %d", depthVideoManipulation).GetPtr(), false);
        logger.Log(cString::Format("isHoodDetectionEnabled: %d", isHoodDetectionEnabled).GetPtr(), false);
        logger.Log(cString::Format("isRoomDetectionEnabled: %d", isRoomDetectionEnabled).GetPtr(), false);
    }
    else if (eStage == StageGraphReady)
    {
        // init RGB Video
        cObjectPtr<IMediaType> rgbMediaType;
        RETURN_IF_FAILED(rgbVideoInputPin.GetMediaType(&rgbMediaType));

        cObjectPtr<IMediaTypeVideo> rgbVideoType;
        RETURN_IF_FAILED(rgbMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &rgbVideoType));

        rgbVideoInputFormat = *(rgbVideoType->GetFormat());
        rgbVideoOutputFormat = *(rgbVideoType->GetFormat());
        rgbVideoOutputPin.SetFormat(&rgbVideoOutputFormat, NULL);

        // init Depth Video
        cObjectPtr<IMediaType> depthMediaType;
        RETURN_IF_FAILED(depthVideoInputPin.GetMediaType(&depthMediaType));

        cObjectPtr<IMediaTypeVideo> depthVideoType;
        RETURN_IF_FAILED(depthMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &depthVideoType));

        depthVideoInputFormat = *(depthVideoType->GetFormat());
        depthVideoOutputFormat = *(depthVideoType->GetFormat());
        depthVideoOutputPin.SetFormat(&depthVideoOutputFormat, NULL);

        logger.Log(cString::Format("RGB Input format: %d x %d @ %d Bit", rgbVideoInputFormat.nWidth, rgbVideoInputFormat.nHeight,
                                   rgbVideoInputFormat.nBitsPerPixel).GetPtr(), false);
        logger.Log(cString::Format("RGB Output format: %d x %d @ %d Bit", rgbVideoOutputFormat.nWidth, rgbVideoOutputFormat.nHeight,
                                   rgbVideoOutputFormat.nBitsPerPixel).GetPtr(), false);

        logger.Log(cString::Format("Depth Input format: %d x %d @ %d Bit", depthVideoInputFormat.nWidth, depthVideoInputFormat.nHeight,
                                   depthVideoInputFormat.nBitsPerPixel).GetPtr(), false);
        logger.Log(cString::Format("Depth Output format: %d x %d @ %d Bit", depthVideoOutputFormat.nWidth, depthVideoOutputFormat.nHeight,
                                   depthVideoOutputFormat.nBitsPerPixel).GetPtr(), false);

        if (depthVideoOutputFormat.nBitsPerPixel != 8)
        {
            THROW_ERROR_DESC(depthVideoOutputFormat.nBitsPerPixel, "Wrong depth video format. Use HTWK_Grayscale in front of this filter.");
        }

        // init processing parameters
        processingData.processingWidth = depthVideoInputFormat.nWidth * (processingWidthPercentage / 100.0);
        processingData.startOffset = (depthVideoInputFormat.nWidth - processingData.processingWidth) / 2;
        processingData.hoodScanLineStepWidth = processingData.processingWidth / (hoodScanLineNumber - 1);
        processingData.roomScanLineStepWidth = processingData.processingWidth / (roomScanLineNumber - 1);

        logger.Log(cString::Format("hoodScanLineNumber: %d", hoodScanLineNumber).GetPtr(), false);
        logger.Log(cString::Format("processingWidthPercentage: %d", processingWidthPercentage).GetPtr(), false);
        logger.Log(cString::Format("processingWidth: %d", processingData.processingWidth).GetPtr(), false);
        logger.Log(cString::Format("startOffset: %d", processingData.startOffset).GetPtr(), false);
        logger.Log(cString::Format("hoodScanLineStepWidth: %d", processingData.hoodScanLineStepWidth).GetPtr(), false);
        logger.Log(cString::Format("roomScanLineStepWidth: %d", processingData.roomScanLineStepWidth).GetPtr(), false);
    }

    RETURN_NOERROR;
}

tResult ROI::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &rgbVideoInputPin)
    {
        tFloat diffBefore = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;

        VisionUtils::ExtractImageFromMediaSample(mediaSample, rgbVideoInputFormat).copyTo(rgbOutputImage);
        RETURN_IF_POINTER_NULL(rgbOutputImage.data);

        ProcessRgbImage();

        TransmitRGBOutput(mediaSample->GetTime());

        tFloat diff = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;
        logger.Log(cString::Format("RGB: Before: %f, After: %f, Diff: %f", diffBefore, diff, diff - diffBefore).GetPtr());
    }
    else if (source == &depthVideoInputPin)
    {
        tFloat diffBefore = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;

        VisionUtils::ExtractImageFromMediaSample(mediaSample, depthVideoInputFormat).copyTo(depthOutputImage);
        RETURN_IF_POINTER_NULL(depthOutputImage.data);

        AnalyzeDepthImage();

        ProcessDepthImage();

        TransmitDepthOutput(mediaSample->GetTime());
        SendRoi(rgbROI, rgbRoiOutputPin);
        SendRoi(depthROI, depthRoiOutputPin);

        tFloat diff = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;
        logger.Log(cString::Format("Depth: Before: %f, After: %f, Diff: %f", diffBefore, diff, diff - diffBefore).GetPtr());
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();
    RETURN_NOERROR;
}

void ROI::AnalyzeDepthImage()
{
    DetectRoom();
    DetectHood();

    CalculateRgbRoi();
    CalculateDepthRoi();
}

void ROI::DetectHood()
{
    if (!isHoodDetectionEnabled || hoodDetectionCount > maxHoodDetectionCount)
    {
        return;
    }

    tInt hoodStartY = 0;
    for (tInt x = 0; x <= processingData.processingWidth; x = x + processingData.hoodScanLineStepWidth)
    {
        // do not process more then the lower half of the image
        for (tInt y = depthOutputImage.rows - 1; y > depthOutputImage.rows / 2; --y)
        {
            tInt color = depthOutputImage.at<tUInt8>(y, x + processingData.startOffset);

            if (color != 0)
            {
                if (y > hoodStartY)
                {
                    hoodStartY = y;
                }

                break;
            }
        }
    }

    // storing height of hood for rgb image cropping
    // reducing impact of new value by adding the old one two times
    hoodHeight = ((hoodHeight * 2) + (depthOutputImage.rows - hoodStartY)) / 3;

    // increasing hood height with magic number
    hoodHeight += 4;

    hoodDetectionCount++;

    logger.Log(cString::Format("HoodHeight: %d", hoodHeight).GetPtr());
}

void ROI::DetectRoom()
{
    if (!isRoomDetectionEnabled)
    {
        return;
    }

    vector<tInt> roomEndValues;

    for (tInt x = 0; x <= processingData.processingWidth; x = x + processingData.roomScanLineStepWidth)
    {
        // ignore lines where first pixel isn't white
        tInt color = depthOutputImage.at<tUInt8>(0, x + processingData.startOffset);
        if (color < 255)
        {
            continue;
        }

        // do not process more then the upper half of the image
        for (tInt y = 0; y < depthOutputImage.rows / 2; ++y)
        {
            color = depthOutputImage.at<tUInt8>(y, x + processingData.startOffset);
            if (color < 255)
            {
                roomEndValues.push_back(y);
                continue;
            }
        }
    }

    if (roomEndValues.size() == 0)
    {
        roomEndValues.push_back(depthOutputImage.rows / 2);
    }

    tInt sum = 0;
    for (unsigned int i = 0; i < roomEndValues.size(); ++i)
    {
        sum += roomEndValues[i];
    }

    tInt mean = (sum / roomEndValues.size()) * roomHeightManipulation;
    roomHeight = ((roomHeight * 2) + mean) / 3;
}

void ROI::CalculateRgbRoi()
{
    tInt rgbRoomEndY = 0;
    tInt rgbHoodStartY = rgbVideoInputFormat.nHeight;

    if (isRoomDetectionEnabled)
    {
        rgbRoomEndY = tFloat32(roomHeight) / depthVideoInputFormat.nHeight * rgbVideoInputFormat.nHeight;
    }

    if (isHoodDetectionEnabled)
    {
        rgbHoodStartY = rgbVideoInputFormat.nHeight -
                        (tFloat32(hoodHeight) / depthVideoInputFormat.nHeight * rgbVideoInputFormat.nHeight);
    }

    rgbROI.tX = 0;
    rgbROI.tY = rgbRoomEndY;
    rgbROI.tWidth = rgbVideoInputFormat.nWidth;
    rgbROI.tHeight = rgbHoodStartY - rgbRoomEndY;

    logger.Log(cString::Format("RGB ROI x: %d, y: %d, width: %d, height: %d", rgbROI.tX, rgbROI.tY, rgbROI.tWidth, rgbROI.tHeight)
                       .GetPtr());
}

void ROI::CalculateDepthRoi()
{
    tInt depthRoomEndY = 0;
    tInt depthHoodStartY = depthVideoInputFormat.nHeight;

    if (isRoomDetectionEnabled)
    {
        depthRoomEndY = roomHeight;
    }

    if (isHoodDetectionEnabled)
    {
        depthHoodStartY = depthVideoInputFormat.nHeight - hoodHeight;
    }

    depthROI.tX = 0;
    depthROI.tY = roomHeight;
    depthROI.tWidth = depthVideoInputFormat.nWidth;
    depthROI.tHeight = depthHoodStartY - depthRoomEndY;
}

void ROI::ProcessRgbImage()
{
    Rect cvRect = Rect(rgbROI.tX, rgbROI.tY, rgbROI.tWidth, rgbROI.tHeight);

    switch (rgbVideoManipulation)
    {
        case VIDEO_NONE:
            break;
        case VIDEO_CROP:
            rgbOutputImage = rgbOutputImage(Rect(rgbROI.tX, rgbROI.tY, rgbROI.tWidth, rgbROI.tHeight)).clone();
            rgbVideoOutputFormat.nHeight = rgbROI.tHeight;
            rgbVideoOutputPin.SetFormat(&rgbVideoOutputFormat, NULL);
            break;
        case VIDEO_RECT:
            rectangle(rgbOutputImage, cvRect, Scalar(0, 0, 255));
        default:
            break;
    }
}

void ROI::ProcessDepthImage()
{
    Rect cvRect = Rect(depthROI.tX, depthROI.tY, depthROI.tWidth, depthROI.tHeight);
    switch (depthVideoManipulation)
    {
        case VIDEO_NONE:
            break;
        case VIDEO_CROP:
            depthOutputImage = depthOutputImage(cvRect).clone();
            depthVideoOutputFormat.nHeight = depthROI.tHeight;
            depthVideoOutputPin.SetFormat(&depthVideoOutputFormat, NULL);
            break;
        case VIDEO_RECT:
            rectangle(depthOutputImage, cvRect, Scalar::all(127));
        default:
            break;
    }
}

tResult ROI::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(rgbVideoInputPin.Create("Video_RGB_In", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&rgbVideoInputPin));

    RETURN_IF_FAILED(depthVideoInputPin.Create("Grayscale_Video_Depth_In", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&depthVideoInputPin));

    RETURN_NOERROR;
}

tResult ROI::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(rgbRoiOutputPin.Create("Rect_RGB_Out", tRectType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&rgbRoiOutputPin));

    RETURN_IF_FAILED(depthRoiOutputPin.Create("Rect_Depth_Out", tRectType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&depthRoiOutputPin));

    RETURN_IF_FAILED(rgbVideoOutputPin.Create("Video_RGB_Out", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&rgbVideoOutputPin));

    RETURN_IF_FAILED(depthVideoOutputPin.Create("Grayscale_Video_Depth_Out", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&depthVideoOutputPin));

    RETURN_NOERROR;
}

tResult ROI::TransmitRGBOutput(const tTimeStamp &timeStamp)
{
    if (!rgbVideoOutputPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &mediaSample));
    RETURN_IF_FAILED(mediaSample->AllocBuffer(rgbVideoOutputFormat.nSize));
    mediaSample->Update(timeStamp, rgbOutputImage.data, rgbVideoOutputFormat.nSize, 0);

    rgbVideoOutputPin.Transmit(mediaSample);

    RETURN_NOERROR;
}

tResult ROI::TransmitDepthOutput(const tTimeStamp &timeStamp)
{
    if (!depthVideoOutputPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &mediaSample));
    RETURN_IF_FAILED(mediaSample->AllocBuffer(depthVideoOutputFormat.nSize));
    mediaSample->Update(timeStamp, depthOutputImage.data, depthVideoOutputFormat.nSize, 0);

    depthVideoOutputPin.Transmit(mediaSample);

    RETURN_NOERROR;
}

tResult ROI::SendRoi(const tRect &roi, cOutputPin &outputPin)
{
    if (!outputPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    tRectDescriptionSignal->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(tRectDescriptionSignal, mediaSample, pCoder);

        pCoder->Set("tX", (tVoid *) &roi.tX);
        pCoder->Set("tY", (tVoid *) &roi.tY);
        pCoder->Set("tWidth", (tVoid *) &roi.tWidth);
        pCoder->Set("tHeight", (tVoid *) &roi.tHeight);
    }

    //transmit media sample over output pin
    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(outputPin.Transmit(mediaSample));

    RETURN_NOERROR;
}


tResult ROI::InitDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &pDescManager, __exception_ptr));

    tChar const *tRectDescription = pDescManager->GetMediaDescription("tRect");
    RETURN_IF_POINTER_NULL(tRectDescription);

    tRectType = new cMediaType(0, 0, 0, "tRect", tRectDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(tRectType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &tRectDescriptionSignal));

    RETURN_NOERROR;
}
