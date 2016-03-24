#include "TemporalImage.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_TI, TemporalImage)

TemporalImage::TemporalImage(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    bufferIndex = 0;
    InitializeProperties();
}

TemporalImage::~TemporalImage()
{
}

void TemporalImage::InitializeProperties()
{
    bufferSize = 5;
    method = METHOD_MEAN;

    SetPropertyInt(BUFFER_SIZE_PROPERTY, bufferSize);
    SetPropertyBool(BUFFER_SIZE_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(BUFFER_SIZE_PROPERTY NSSUBPROP_DESCRIPTION, "Buffer Size");
    SetPropertyInt(BUFFER_SIZE_PROPERTY NSSUBPROP_MIN, 0);
    SetPropertyInt(BUFFER_SIZE_PROPERTY NSSUBPROP_MAX, 255);

    SetPropertyInt(METHOD_PROPERTY, method);
    SetPropertyBool(METHOD_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(METHOD_PROPERTY NSSUBPROP_VALUELIST, "1@Mean|2@And|3@Or");
    SetPropertyStr(METHOD_PROPERTY NSSUBPROP_DESCRIPTION, "Defines method which is used to create the output image");
}

tResult TemporalImage::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        bufferSize = tUInt(GetPropertyInt(BUFFER_SIZE_PROPERTY));
        method = tUInt8(GetPropertyInt(METHOD_PROPERTY));

        buffer = vector<Mat>(bufferSize);
        for (unsigned int i = 0; i < buffer.size(); ++i)
        {
            buffer[i] = Mat(0, 0, 0);
        }
    }
    else if (eStage == StageGraphReady)
    {
        cObjectPtr<IMediaType> mediaType;
        RETURN_IF_FAILED(videoInputPin.GetMediaType(&mediaType));

        cObjectPtr<IMediaTypeVideo> videoType;
        RETURN_IF_FAILED(mediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &videoType));

        videoInputFormat = *(videoType->GetFormat());
        videoOutputFormat = *(videoType->GetFormat());
        videoOutputPin.SetFormat(&videoOutputFormat, NULL);

        logger.Log(cString::Format("Input format: %d x %d @ %d Bit", videoInputFormat.nWidth, videoInputFormat.nHeight,
                                   videoInputFormat.nBitsPerPixel).GetPtr(), false);
        logger.Log(cString::Format("Output format: %d x %d @ %d Bit", videoOutputFormat.nWidth, videoOutputFormat.nHeight,
                                   videoOutputFormat.nBitsPerPixel).GetPtr(), false);
    }

    RETURN_NOERROR;
}

tResult TemporalImage::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &videoInputPin)
    {
        VisionUtils::ExtractImageFromMediaSample(mediaSample, videoInputFormat).copyTo(workingImage);
        RETURN_IF_POINTER_NULL(workingImage.data);

        ProcessImage();

        TransmitOutput(mediaSample->GetTime());

        tFloat diff = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;
        logger.Log(cString::Format("Total depth image processing time: %f ms", diff).GetPtr());
    }
    else if (source == &videoOutputPin)
    {
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}

tResult TemporalImage::ProcessImage()
{
    if (bufferSize == 0)
    {
        outputImage = workingImage;
        RETURN_NOERROR;
    }

    buffer[bufferIndex] = workingImage.clone();
    bufferIndex = (bufferIndex + 1) % bufferSize;

    switch (method)
    {
        case (METHOD_AND):
            outputImage = CalculateAnd();
            break;
        case (METHOD_OR):
            outputImage = CalculateOr();
            break;
        case (METHOD_MEAN):
            outputImage = CalculateMean();
            break;
        default:
            logger.Log(cString::Format("Invalid method %d", method).GetPtr(), false);
            outputImage = workingImage;
            break;
    }

    RETURN_NOERROR;
}

Mat TemporalImage::CalculateMean()
{
    // use Mat with 16Bit per pixel
    Mat img = Mat::zeros(videoInputFormat.nHeight, videoInputFormat.nWidth, CV_16UC1);

    for (tUInt i = 0; i < bufferSize; ++i)
    {
        // skip buffered image if empty
        if (0 == buffer[i].rows)
        {
            continue;
        }

        Mat bufferedImage;
        buffer[i].convertTo(bufferedImage, CV_16UC1);

        img += bufferedImage;
    }

    // calculate mean
    img = (img / bufferSize);

    // convert back to 8Bit per pixel Mat
    img.convertTo(img, CV_8UC1);

    return img;
}

Mat TemporalImage::CalculateAnd()
{
    Mat img(videoInputFormat.nHeight, videoInputFormat.nWidth, CV_8UC1, Scalar(255));

    for (tUInt i = 0; i < bufferSize; ++i)
    {
        // skip buffered image if empty
        if (0 == buffer[i].rows)
        {
            continue;
        }

        bitwise_and(img, buffer[i], img);
    }

    return img;
}

Mat TemporalImage::CalculateOr()
{
    Mat img(videoInputFormat.nHeight, videoInputFormat.nWidth, CV_8UC1, Scalar(0));

    for (tUInt i = 0; i < bufferSize; ++i)
    {
        // skip buffered image if empty
        if (0 == buffer[i].rows)
        {
            continue;
        }

        bitwise_or(img, buffer[i], img);
    }

    return img;
}

tResult TemporalImage::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoInputPin.Create("Greyscale_Video_Input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoInputPin));

    RETURN_NOERROR;
}

tResult TemporalImage::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoOutputPin.Create("Greyscale_Video_Output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoOutputPin));

    RETURN_NOERROR;
}

tResult TemporalImage::TransmitOutput(const tTimeStamp &timeStamp)
{
    cObjectPtr<IMediaSample> outputMediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &outputMediaSample));
    RETURN_IF_FAILED(outputMediaSample->AllocBuffer(videoOutputFormat.nSize));
    outputMediaSample->Update(timeStamp, outputImage.data, videoOutputFormat.nSize, 0);

    videoOutputPin.Transmit(outputMediaSample);

    RETURN_NOERROR;
}
