/**
 * @author pbachmann
 */

#include "NoiseRemover.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_NOISE_REMOVER, NoiseRemover)

NoiseRemover::NoiseRemover(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    // | 1 1 1 |
    // | 1 1 1 |
    // | 1 1 1 |
    mask = getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));

    InitializeProperties();
}

NoiseRemover::~NoiseRemover()
{
}

void NoiseRemover::InitializeProperties()
{
    iterations = 2;

    SetPropertyInt(ITERATION_PROPERTY, iterations);
    SetPropertyBool(ITERATION_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(ITERATION_PROPERTY NSSUBPROP_DESCRIPTION, "Iterations");
    SetPropertyInt(ITERATION_PROPERTY NSSUBPROP_MIN, 0);
    SetPropertyInt(ITERATION_PROPERTY NSSUBPROP_MAX, 255);
}

tResult NoiseRemover::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        iterations = tUInt8(GetPropertyInt(ITERATION_PROPERTY));
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

tResult NoiseRemover::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &videoInputPin)
    {
        VisionUtils::ExtractImageFromMediaSample(mediaSample, videoInputFormat).copyTo(outputImage);
        RETURN_IF_POINTER_NULL(outputImage.data);

        ProcessInput();

        TransmitOutput(mediaSample->GetTime());

        tFloat diff = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;
        logger.Log(cString::Format("Total depth image processing time: %f ms", diff).GetPtr());
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}

tResult NoiseRemover::ProcessInput()
{
    for (int i = 0; i < iterations; ++i)
    {
        cv::dilate(outputImage, outputImage, mask);
        cv::dilate(outputImage, outputImage, mask);
        cv::erode(outputImage, outputImage, mask);
    }

    RETURN_NOERROR;
}

tResult NoiseRemover::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoInputPin.Create("Grayscale_Video_Input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoInputPin));

    RETURN_NOERROR;
}

tResult NoiseRemover::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoOutputPin.Create("Grayscale_Video_Output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoOutputPin));

    RETURN_NOERROR;
}

tResult NoiseRemover::TransmitOutput(const tTimeStamp &timeStamp)
{
    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &mediaSample));
    RETURN_IF_FAILED(mediaSample->AllocBuffer(videoOutputFormat.nSize));
    mediaSample->Update(timeStamp, outputImage.data, videoOutputFormat.nSize, 0);

    videoOutputPin.Transmit(mediaSample);

    RETURN_NOERROR;
}
