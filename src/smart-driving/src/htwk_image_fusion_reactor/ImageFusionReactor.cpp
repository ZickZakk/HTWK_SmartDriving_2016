#include "ImageFusionReactor.h"
#include "../htwk_utils/VisionUtils.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_IFR, ImageFusionReactor)

ImageFusionReactor::ImageFusionReactor(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    method = METHOD_MEAN;
    InitializeProperties();
}

ImageFusionReactor::~ImageFusionReactor()
{
}

void ImageFusionReactor::InitializeProperties()
{
    SetPropertyInt(METHOD_PROPERTY, method);
    SetPropertyBool(METHOD_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(METHOD_PROPERTY NSSUBPROP_VALUELIST, "1@Mean|2@And|3@Or");
    SetPropertyStr(METHOD_PROPERTY NSSUBPROP_DESCRIPTION, "Defines method which is used to create the output image");
}

tResult ImageFusionReactor::Init(tInitStage eStage, __exception)
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
        cObjectPtr<IMediaType> firstMediaType;
        RETURN_IF_FAILED(firstVideoInputPin.GetMediaType(&firstMediaType));
        cObjectPtr<IMediaTypeVideo> firstVideoType;
        RETURN_IF_FAILED(firstMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &firstVideoType));

        cObjectPtr<IMediaType> secondMediaType;
        RETURN_IF_FAILED(secondVideoInputPin.GetMediaType(&secondMediaType));
        cObjectPtr<IMediaTypeVideo> secondVideoType;
        RETURN_IF_FAILED(secondMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &secondVideoType));

        firstVideoInputFormat = *(firstVideoType->GetFormat());
        secondVideoInputFormat = *(secondVideoType->GetFormat());
        mergedVideoOutputFormat = *(firstVideoType->GetFormat());
        mergedVideoOutputPin.SetFormat(&mergedVideoOutputFormat, NULL);

        logger.Log(cString::Format("First Input format: %d x %d @ %d Bit", firstVideoInputFormat.nWidth, firstVideoInputFormat.nHeight,
                                   firstVideoInputFormat.nBitsPerPixel).GetPtr(), false);
        logger.Log(cString::Format("Second Input format: %d x %d @ %d Bit", secondVideoInputFormat.nWidth, secondVideoInputFormat.nHeight,
                                   secondVideoInputFormat.nBitsPerPixel).GetPtr(), false);
        logger.Log(cString::Format("Output format: %d x %d @ %d Bit", mergedVideoOutputFormat.nWidth, mergedVideoOutputFormat.nHeight,
                                   mergedVideoOutputFormat.nBitsPerPixel).GetPtr(), false);
    }

    RETURN_NOERROR;
}

tResult ImageFusionReactor::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &firstVideoInputPin)
    {
        VisionUtils::ExtractImageFromMediaSample(mediaSample, firstVideoInputFormat).copyTo(firstImage);

        RETURN_IF_POINTER_NULL(firstImage.data);
        RETURN_IF_POINTER_NULL(secondImage.data)

        switch (method)
        {
            case (METHOD_ADD):
                CalculateAnd();
                break;
            case (METHOD_OR):
                CalculateOr();
                break;
            case (METHOD_MEAN):
                CalculateMean();
                break;
            default:
                logger.Log(cString::Format("Invalid method %d", method).GetPtr(), false);
                outputImage = Mat::zeros(firstVideoInputFormat.nHeight, firstVideoInputFormat.nWidth, CV_8UC1);
                break;
        }

        threshold(outputImage, outputImage, 5, 255, CV_THRESH_BINARY);

        TransmitOutput(mediaSample->GetTime());
    }
    else if (source == &secondVideoInputPin)
    {
        VisionUtils::ExtractImageFromMediaSample(mediaSample, secondVideoInputFormat).copyTo(secondImage);
        RETURN_IF_POINTER_NULL(secondImage.data);
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}

tResult ImageFusionReactor::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(firstVideoInputPin.Create("First_Video_In", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&firstVideoInputPin));

    RETURN_IF_FAILED(secondVideoInputPin.Create("Second_Video_In", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&secondVideoInputPin));

    RETURN_NOERROR;
}

tResult ImageFusionReactor::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(mergedVideoOutputPin.Create("Fusion_Video_Out", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&mergedVideoOutputPin));

    RETURN_NOERROR;
}

tResult ImageFusionReactor::TransmitOutput(const tTimeStamp &timeStamp)
{
    if (!mergedVideoOutputPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> outputMediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &outputMediaSample));
    RETURN_IF_FAILED(outputMediaSample->AllocBuffer(mergedVideoOutputFormat.nSize));
    outputMediaSample->Update(timeStamp, outputImage.data, mergedVideoOutputFormat.nSize, 0);

    mergedVideoOutputPin.Transmit(outputMediaSample);

    RETURN_NOERROR;
}

void ImageFusionReactor::CalculateMean()
{
    logger.Log("CalculateMean");

    // use Mat with 16Bit per pixel
    Mat img = Mat::zeros(firstVideoInputFormat.nHeight, firstVideoInputFormat.nWidth, CV_16UC1);

    Mat bufferedImage;

    firstImage.convertTo(bufferedImage, CV_16UC1);
    img += bufferedImage;

    secondImage.convertTo(bufferedImage, CV_16UC1);
    img += bufferedImage;

    logger.Log(cString::Format("Calculate mean").GetPtr());
    // calculate mean
    img = (img / 2.0);

    // convert back to 8Bit per pixel Mat
    img.convertTo(outputImage, CV_8UC1);
}

void ImageFusionReactor::CalculateOr()
{
    logger.Log("CalculateOr");

    bitwise_or(firstImage, secondImage, outputImage);
}

void ImageFusionReactor::CalculateAnd()
{
    logger.Log("CalculateAnd");

    bitwise_and(firstImage, secondImage, outputImage);
}
