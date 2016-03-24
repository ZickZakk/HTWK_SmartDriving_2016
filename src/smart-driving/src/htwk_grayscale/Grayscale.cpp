/**
 * @author pbachmann
 */

#include "Grayscale.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_GRAYSCALE, Grayscale
)

Grayscale::Grayscale(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
}

Grayscale::~Grayscale()
{
}

tResult Grayscale::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
    }
    else if (eStage == StageGraphReady)
    {
        // init Depth Video
        cObjectPtr <IMediaType> mediaType;
        RETURN_IF_FAILED(videoInputPin.GetMediaType(&mediaType));

        cObjectPtr <IMediaTypeVideo> videoType;
        RETURN_IF_FAILED(mediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid * *) & videoType));

        videoInputFormat = *(videoType->GetFormat());

        //set the videoformat of the output pin
        videoOutputFormat.nWidth = videoInputFormat.nWidth;
        videoOutputFormat.nHeight = videoInputFormat.nHeight;
        videoOutputFormat.nBitsPerPixel = 8;
        videoOutputFormat.nPixelFormat = cImage::PF_GREYSCALE_8;
        videoOutputFormat.nBytesPerLine = videoInputFormat.nWidth;
        videoOutputFormat.nSize = videoOutputFormat.nBytesPerLine * videoInputFormat.nHeight;
        videoOutputFormat.nPaletteSize = 0;
        videoOutputPin.SetFormat(&videoOutputFormat, NULL);

        logger.Log(cString::Format("Input format: %d x %d @ %d Bit", videoInputFormat.nWidth, videoInputFormat.nHeight,
                                   videoInputFormat.nBitsPerPixel).GetPtr(), false);
        logger.Log(cString::Format("Output format: %d x %d @ %d Bit", videoOutputFormat.nWidth, videoOutputFormat.nHeight,
                                   videoOutputFormat.nBitsPerPixel).GetPtr(), false);
    }

    RETURN_NOERROR;
}

tResult Grayscale::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &videoInputPin)
    {
        tFloat diffBefore = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;

        VisionUtils::ExtractImageFromMediaSample(mediaSample, videoInputFormat).copyTo(outputImage);
        RETURN_IF_POINTER_NULL(outputImage.data);

        int mat = VisionUtils::GetMatType(videoInputFormat.nBitsPerPixel);
        switch (mat)
        {
            case (CV_8UC3):
                // RGB image
                cvtColor(outputImage, outputImage, CV_BGR2GRAY);
                break;
            case (CV_16UC1):
                // Depth image
                outputImage = outputImage / 255;
                outputImage.convertTo(outputImage, CV_8UC1);
            case (CV_8UC1):
                // already grayscale, do nothing
                break;
            default:
                logger.Log(cString::Format("Image type %d not supported", mat).GetPtr(), false);
        }

        TransmitOutput(mediaSample->GetTime());

        tFloat diff = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;
        logger.Log(cString::Format("Before: %f, After: %f, Diff: %f", diffBefore, diff, diff - diffBefore).GetPtr());
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}

tResult Grayscale::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoInputPin.Create("Video_Input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoInputPin));

    RETURN_NOERROR;
}

tResult Grayscale::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoOutputPin.Create("Grayscale_Video_Output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoOutputPin));

    RETURN_NOERROR;
}

tResult Grayscale::TransmitOutput(const tTimeStamp &timeStamp)
{
    cObjectPtr <IMediaSample> mediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid * *) & mediaSample));
    RETURN_IF_FAILED(mediaSample->AllocBuffer(videoOutputFormat.nSize));
    mediaSample->Update(timeStamp, outputImage.data, videoOutputFormat.nSize, 0);

    videoOutputPin.Transmit(mediaSample);

    RETURN_NOERROR;
}
