/**
 * @author sfeig
 */

#include "DepthDetection.h"
#include "../htwk_utils/VisionUtils.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_DEPTH_DETECTION, DepthDetection)

DepthDetection::DepthDetection(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    m_nFrameCounter = 0;
    m_oKernel = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
}


DepthDetection::~DepthDetection()
{ }


tResult DepthDetection::Init(tInitStage eStage, __exception)
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
        // Depth Video Input Pin
        cObjectPtr<IMediaType> mediaType;
        RETURN_IF_FAILED(videoInputPin.GetMediaType(&mediaType));
        cObjectPtr<IMediaTypeVideo> videoType;
        RETURN_IF_FAILED(mediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &videoType));
        m_oVideoInputFormat = *(videoType->GetFormat());

        // Define Video Output Pin
        m_oVideoOutputFormat.nWidth = 400;
        m_oVideoOutputFormat.nHeight = 400;
        m_oVideoOutputFormat.nBitsPerPixel = 8;
        m_oVideoOutputFormat.nPixelFormat = cImage::PF_GREYSCALE_8;
        m_oVideoOutputFormat.nBytesPerLine = 400;
        m_oVideoOutputFormat.nSize = m_oVideoOutputFormat.nBytesPerLine * 400;
        m_oVideoOutputFormat.nPaletteSize = 0;
        videoOutputPin.SetFormat(&m_oVideoOutputFormat, NULL);
    }
    RETURN_NOERROR;
}


tResult DepthDetection::Shutdown(tInitStage eStage, __exception)
{
    if (StageFirst == eStage)
    {
    }
    return cFilter::Shutdown(eStage, __exception_ptr);
}


tResult DepthDetection::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &videoInputPin)
    {
        ExtractDepthImageFromMediaSample(mediaSample, m_oVideoInputFormat).copyTo(m_oDepth);

        if (m_nFrameCounter < 10)
        {
            m_oDepth = denoiser.process(m_oDepth);
            groundRemover.setBackground(m_oDepth);
            ++m_nFrameCounter;
            RETURN_NOERROR;
        }

        m_oDepth = groundRemover.process(m_oDepth);
        // after removing background artefacts may remain, so remove them
        cv::erode(m_oDepth, m_oDepth, m_oKernel);
        // inverse perspective mapping
        m_oBirdEye = birdsEyeViewer.process(m_oDepth);

        // std::vector<cv::Rect> obstacles = connectedComponentFinder.process(m_oBirdEye);
        RETURN_IF_POINTER_NULL(m_oBirdEye.data);

        DrawBoundingRects(m_oBirdEye);

        // cut vdar and copy to merge mat
        Mat output = Mat::zeros(400, 400, CV_8UC1);
        cv::Rect oRoi = cv::Rect(10, 40, 300, 200);
        Mat oRoiVdar = m_oBirdEye(oRoi).clone();
        Mat dst_roi = output(Rect(50, 0, 300, 200));
        oRoiVdar.copyTo(dst_roi);

        TransmitOutput(mediaSample->GetTime(), m_oVideoOutputFormat, videoOutputPin, output);
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}

Mat DepthDetection::ExtractDepthImageFromMediaSample(IMediaSample *mediaSample, const tBitmapFormat &inputFormat)
{
    const tVoid *buffer;
    static Mat image;
    if (IS_OK(mediaSample->Lock(&buffer)))
    {
        image = Mat(Size(inputFormat.nWidth, inputFormat.nHeight), CV_16UC1, (char*)buffer).clone();
        mediaSample->Unlock(buffer);
        return image;
    }
    return Mat();
}


tResult DepthDetection::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoInputPin.Create("Video_Input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoInputPin));
    RETURN_NOERROR;
}


tResult DepthDetection::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(videoOutputPin.Create("Video_Output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoOutputPin));
    RETURN_NOERROR;
}


tResult DepthDetection::TransmitOutput(const tTimeStamp &timeStamp,
                                       const tBitmapFormat &oVideoOutputFormat,
                                       cVideoPin &oOutputPin,
                                       const Mat &oImage)
{
    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE,
                                              (tVoid **) &mediaSample));
    RETURN_IF_FAILED(mediaSample->AllocBuffer(oVideoOutputFormat.nSize));
    mediaSample->Update(timeStamp, oImage.data, oVideoOutputFormat.nSize, 0);
    oOutputPin.Transmit(mediaSample);
    RETURN_NOERROR;
}

tResult DepthDetection::DrawBoundingRects(Mat &oIm)
{
    std::vector<cv::RotatedRect> boundRects;
    boundRects = connectedComponentFinder.process(oIm);

    for (unsigned int i = 0; i < boundRects.size(); ++i)
    {
        Point2f rect_points[4];
        boundRects[i].points(rect_points);

        vector<Point> contour;
        contour.push_back(rect_points[0]);
        contour.push_back(rect_points[1]);
        contour.push_back(rect_points[2]);
        contour.push_back(rect_points[3]);

        const cv::Point *pts = (const cv::Point *) Mat(contour).data;
        int npts = Mat(contour).rows;

        fillPoly(oIm, &pts, &npts, 1, 0xff);
    }
    RETURN_NOERROR;
}
