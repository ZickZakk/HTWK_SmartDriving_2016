#include "MovementVisualizer.h"
#include "../htwk_utils/VisionUtils.h"
#include "../htwk_yaw_to_steer/YawToSteer.h"


ADTF_FILTER_PLUGIN(MOVEMENT_VISUALIZER_NAME, MOVEMENT_VISUALIZER_OID, MovementVisualizer)

MovementVisualizer::MovementVisualizer(const tChar *__info)
        : cFilter(__info), logger(MOVEMENT_VISUALIZER_NAME, 20)
{
    colorImage = Mat::zeros(DRAWING_SURFACE_HEIGHT, DRAWING_SURFACE_WIDTH, CV_8UC3);
    oldYaw = yaw = deltaYaw = 0.0f;
    oldDistance = distance = deltaDistance = 0.0f;
    buffer = 0;
    currentPosInPixel.x = DRAWING_SURFACE_WIDTH / 2;
    currentPosInPixel.y = DRAWING_SURFACE_HEIGHT / 2;
    path.clear();
    InitializeProperties();
}


MovementVisualizer::~MovementVisualizer()
{
}


tVoid MovementVisualizer::InitializeProperties()
{
}


tResult MovementVisualizer::Init(tInitStage eStage, __exception)
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
        videoEnabled = videoOutputPin.IsConnected();

        //set the videoformat of the output pin
        videoOutputFormat.nWidth = WIDTH_VIDEO;
        videoOutputFormat.nHeight = HEIGHT_VIDEO;
        videoOutputFormat.nBitsPerPixel = 24;
        videoOutputFormat.nPixelFormat = cImage::PF_RGB_888;
        videoOutputFormat.nBytesPerLine = WIDTH_VIDEO * 3;
        videoOutputFormat.nSize = videoOutputFormat.nBytesPerLine * HEIGHT_VIDEO;
        videoOutputFormat.nPaletteSize = 0;
        videoOutputPin.SetFormat(&videoOutputFormat, NULL);
    }
    else if (eStage == StageGraphReady)
    {
    }

    RETURN_NOERROR;
}


tResult MovementVisualizer::Start(ucom::IException **__exception_ptr)
{
    return cFilter::Start(__exception_ptr);
}

tResult MovementVisualizer::Stop(ucom::IException **__exception_ptr)
{
    return cFilter::Stop(__exception_ptr);
}

tResult MovementVisualizer::Shutdown(tInitStage eStage, ucom::IException **__exception_ptr)
{
    return cFilter::Shutdown(eStage, __exception_ptr);
}


tResult MovementVisualizer::OnPinEvent(IPin *source, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample)
{
    RETURN_IF_POINTER_NULL(mediaSample);

    if (eventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    logger.StartLog();

    if (source == &yawInPin)
    {
        // get current yaw
        deltaYaw = GetYawDelta(mediaSample);
    }
    else if (source == &distanceInPin)
    {
        if (buffer >= 10)
        {
            // draw point after certain distance
            deltaDistance = GetDistanceDelta(mediaSample);
            movementVectorInMeter = CalculateMovement(deltaYaw, deltaDistance);
            buffer = 0;
        }
        buffer++;
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    //logger.Log(cString::Format("deltaInMeter %f %f", deltaYaw, deltaDistance).GetPtr(), tFalse);
    logger.Log(cString::Format("CurYaw %f ; AllDist %f", yaw, distance).GetPtr());

    Point2d movementVectorInPixel = ConvertToPixel(movementVectorInMeter);

    currentPosInPixel.x += movementVectorInPixel.x;
    currentPosInPixel.y += movementVectorInPixel.y;
    logger.Log(cString::Format("CurDrawPoint %f %f",
                               currentPosInPixel.x, currentPosInPixel.y).GetPtr());

    //logger.Log(cString::Format("currentPosInPixel %f %f",currentPosInPixel.x, currentPosInPixel.y).GetPtr(), tFalse);

    // draw currentPos to image
    circle(colorImage, currentPosInPixel, 1, Scalar(255, 255, 255), -1);

    TransmitVideo();

    logger.EndLog();

    RETURN_NOERROR;
}


Point2d MovementVisualizer::ConvertToPixel(Point2d movementVectorInMeter)
{
    tFloat32 conversionFactor = 1.25;
    return Point2d((movementVectorInMeter.x * conversionFactor), (movementVectorInMeter.y * conversionFactor));
    // 1m = 1.5px; 1px = 80cm
}


Point2d MovementVisualizer::CalculateMovement(tFloat32 deltaYaw, tFloat32 deltaDistance)
{
    tFloat32 xpiece = 1 - (abs(90 - abs(yaw)) / 90);
    if (yaw < 0)
    { xpiece *= -1; }

    tFloat32 ypiece = abs(abs(yaw) - 90) / 90;
    if (abs(yaw) > 90)
    { ypiece *= -1; }

    logger.Log(cString::Format("CurYaw %f ; DeltaDist: %f; XChange %f; yChange %f",
                               yaw, deltaDistance, xpiece, ypiece).GetPtr());

    return Point2d(xpiece * deltaDistance,
                   ypiece * deltaDistance);
    //return Point2d(tan(deltaYaw) * deltaDistance, deltaDistance);
}


cv::Rect MovementVisualizer::GetBoundingROI()
{
    cv::Point2d topLeft(currentPosInPixel.x - (WIDTH_VIDEO / 2),
                        currentPosInPixel.y - (HEIGHT_VIDEO / 2));
    cv::Point2d bottomRight(currentPosInPixel.x + (WIDTH_VIDEO / 2),
                            currentPosInPixel.y + (HEIGHT_VIDEO / 2));

    logger.Log(cString::Format("tx %f ty %f bx %f by %f",
                               topLeft.x, topLeft.y, bottomRight.x, bottomRight.y).GetPtr());
    logger.Log("Rect erstellt");
    return cv::Rect(topLeft, bottomRight);
}


tResult MovementVisualizer::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));
    // Get Signal Description
    tChar const *signalValueDescription = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(signalValueDescription);
    signalType = new cMediaType(0, 0, 0, "tSignalValue", signalValueDescription,
                                IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(signalType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
                                              (tVoid **) &signalTypeDescription));
    RETURN_NOERROR;
}

tFloat32 MovementVisualizer::GetYawDelta(IMediaSample *mediaSample)
{
    oldYaw = yaw;
    yaw = GetFloat(mediaSample);
    return CalcYawDelta(oldYaw, yaw);
}

tFloat32 MovementVisualizer::GetDistanceDelta(IMediaSample *mediaSample)
{
    oldDistance = distance;
    distance = GetFloat(mediaSample);
    return distance - oldDistance;
}

tResult MovementVisualizer::CreateInputPins(IException **__exception_ptr)
{
    // Yaw
    RETURN_IF_FAILED(yawInPin.Create("yawIn", signalType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&yawInPin));
    // Distance
    RETURN_IF_FAILED(distanceInPin.Create("distanceIn", signalType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&distanceInPin));
    RETURN_NOERROR;
}


tResult MovementVisualizer::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoOutputPin.Create("VideoOut", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoOutputPin));
    RETURN_NOERROR;
}


tFloat32 MovementVisualizer::GetFloat(IMediaSample *mediaSample)
{
    __synchronized_obj(syncIO);

    tFloat32 value = 0.0f;
    {
        __adtf_sample_read_lock_mediadescription(signalTypeDescription, mediaSample, pCoderInput);

        // get the IDs for the items in the media sample
        if (!idSignalSet)
        {
            pCoderInput->GetID("f32Value", idSignalF32Value);
            idSignalSet = tTrue;
        }
        //get values from media sample
        pCoderInput->Get(idSignalF32Value, (tVoid *) &value);
    }
    return value;
}


tResult MovementVisualizer::TransmitVideo()
{
    cv::Mat roi = cv::Mat(colorImage, GetBoundingROI()).clone();
    logger.Log("Roi erstellt");
    // transmit data in media sample over the output pin
    cObjectPtr<IMediaSample> pNewRGBSample;
    if (IS_OK(AllocMediaSample(&pNewRGBSample)))
    {
        tTimeStamp tmStreamTime = _clock ? _clock->GetStreamTime() : adtf_util::cHighResTimer::GetTime();
        pNewRGBSample->Update(tmStreamTime, roi.data, tInt32(videoOutputFormat.nSize), 0);
        videoOutputPin.Transmit(pNewRGBSample);
    }
    RETURN_NOERROR;
}

