/**
 * @author sfeig
 */

#include "DistanceSensorMerger.h"
#include "../htwk_utils/VisionUtils.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_DISTANCE_SENOR_MERGER, DistanceSensorMerger)

DistanceSensorMerger::DistanceSensorMerger(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    m_oVdarMerge = Mat::zeros(400, 400, CV_8UC1);
    m_oUSMerge = Mat::zeros(400, 400, CV_8UC1);
    m_oIPMMerge = Mat::zeros(400, 400, CV_8UC1);

    m_oVdar = Mat::zeros(320, 240, CV_8UC1);
    m_oUS = Mat::zeros(400, 400, CV_8UC1);
    m_oIPM = Mat::zeros(300, 200, CV_8UC1);
}

DistanceSensorMerger::~DistanceSensorMerger()
{
}

tResult DistanceSensorMerger::Init(tInitStage eStage, __exception)
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
        InitVideoFormat(m_oUSVideoInputPin, m_oUSVideoOutputPin,
                        m_oUSVideoInputFormat, m_oUSVideoOutputFormat);
        InitVideoFormat(m_oVdarVideoInputPin, m_oVdarVideoOutputPin,
                        m_oVdarVideoInputFormat, m_oVdarVideoOutputFormat);
        InitVideoFormat(m_oIPMVideoInputPin, m_oIPMVideoOutputPin,
                        m_oIPMVideoInputFormat, m_oIPMVideoOutputFormat);
        InitVideoFormatMerged();

        // namedWindow(cv::String("Merger"), cv::WINDOW_NORMAL);
        // cv::startWindowThread();
    }

    RETURN_NOERROR;
}


tResult DistanceSensorMerger::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(m_oUSVideoInputPin.Create("USMap_in", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oUSVideoInputPin));

    RETURN_IF_FAILED(m_oVdarVideoInputPin.Create("VdarMap_in", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oVdarVideoInputPin));

    RETURN_IF_FAILED(m_oIPMVideoInputPin.Create("IPMMap_in", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oIPMVideoInputPin));
    RETURN_NOERROR;
}


tResult DistanceSensorMerger::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(m_oUSVideoOutputPin.Create("USMap_out", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oUSVideoOutputPin));

    RETURN_IF_FAILED(m_oVdarVideoOutputPin.Create("VdarMap_out", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oVdarVideoOutputPin));

    RETURN_IF_FAILED(m_oIPMVideoOutputPin.Create("IPMMap_out", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oIPMVideoOutputPin));

    RETURN_IF_FAILED(m_oMergedVideoOutputPin.Create("MergedMap_out", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oMergedVideoOutputPin));
    RETURN_NOERROR;
}


tResult DistanceSensorMerger::InitVideoFormat(cVideoPin &oInputPin, cVideoPin &oOutputPin,
                                              tBitmapFormat &oInputFormat, tBitmapFormat &oOutputFormat)
{
    cObjectPtr<IMediaType> mediaType;
    cObjectPtr<IMediaTypeVideo> videoType;
    RETURN_IF_FAILED(oInputPin.GetMediaType(&mediaType));
    RETURN_IF_FAILED(mediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &videoType));
    oInputFormat = *(videoType->GetFormat());
    oOutputFormat = oInputFormat;
    oOutputPin.SetFormat(&oOutputFormat, NULL);
    RETURN_NOERROR;
}


tResult DistanceSensorMerger::InitVideoFormatMerged()
{
    m_oMergedVideoOutputFormat.nWidth = 400;
    m_oMergedVideoOutputFormat.nHeight = 400;
    m_oMergedVideoOutputFormat.nBytesPerLine = m_oMergedVideoOutputFormat.nWidth;
    m_oMergedVideoOutputFormat.nBitsPerPixel = 8;
    m_oMergedVideoOutputFormat.nPixelFormat = cImage::PF_GREYSCALE_8;
    m_oMergedVideoOutputFormat.nSize = m_oMergedVideoOutputFormat.nBytesPerLine * 400;
    m_oMergedVideoOutputFormat.nPaletteSize = 0;
    m_oMergedVideoOutputPin.SetFormat(&m_oMergedVideoOutputFormat, NULL);
    RETURN_NOERROR;
}


tResult DistanceSensorMerger::Shutdown(tInitStage eStage, __exception)
{
    if (StageFirst == eStage)
    {
        cv::destroyWindow("Merger");
    }
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult DistanceSensorMerger::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }
    RETURN_IF_POINTER_NULL(mediaSample);
    logger.StartLog();
    Mat dst_roi;
    if (source == &m_oUSVideoInputPin)
    {
        m_oUS = VisionUtils::ExtractImageFromMediaSample(mediaSample, m_oUSVideoInputFormat).clone();
        RETURN_IF_POINTER_NULL(m_oUS.data);

        // preprocessing us map
        InvertBinary(m_oUS);

        // cut us and copy to merge mat
        Mat oRoiUs = m_oUS(Rect(0, 0, 400, 400)).clone();
        dst_roi = m_oUSMerge(Rect(0, 0, 400, 400));
        oRoiUs.copyTo(dst_roi);

        TransmitOutput(mediaSample->GetTime(), m_oUSVideoOutputFormat, m_oUSVideoOutputPin, m_oUS);
    }
    else if (source == &m_oVdarVideoInputPin)
    {
        m_oVdar = VisionUtils::ExtractImageFromMediaSample(mediaSample, m_oVdarVideoInputFormat).clone();
        RETURN_IF_POINTER_NULL(m_oVdar.data);

        // preprocessing vdar map
        BoundingRects(m_oVdar);

        // cut vdar and copy to merge mat
        cv::Rect oRoi = cv::Rect(10, 40, 300, 200);
        Mat oRoiVdar = m_oVdar(oRoi).clone();
        dst_roi = m_oVdarMerge(Rect(50, 0, 300, 200));
        oRoiVdar.copyTo(dst_roi);

        TransmitOutput(mediaSample->GetTime(), m_oVdarVideoOutputFormat, m_oVdarVideoOutputPin, m_oVdar);
    }
    else if (source == &m_oIPMVideoInputPin)
    {
        m_oIPM = VisionUtils::ExtractImageFromMediaSample(mediaSample, m_oIPMVideoInputFormat);
        RETURN_IF_POINTER_NULL(m_oIPM.data);

        // preprocessing ipm map
        ConvertToBinary(m_oIPM);

        // cut ipm and copy to merge mat
        dst_roi = m_oIPMMerge(Rect(50, 0, 300, 200));
        m_oIPM.copyTo(dst_roi);

        TransmitOutput(mediaSample->GetTime(), m_oIPMVideoOutputFormat, m_oIPMVideoOutputPin, m_oIPM);
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }
    // merge all 3 mats
    Merge(m_oIPMMerge, m_oUSMerge, m_oVdarMerge, m_oMerged);
    // imshow("Merger", m_oMerged);
    TransmitOutput(mediaSample->GetTime(), m_oMergedVideoOutputFormat, m_oMergedVideoOutputPin, m_oMerged);
    logger.EndLog();
    RETURN_NOERROR;
}

tResult DistanceSensorMerger::BoundingRects(Mat &oIm)
{
    std::vector<cv::RotatedRect> boundRects;
    boundRects = m_oConnectedComponentFinder.process(oIm);

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

tResult DistanceSensorMerger::ConvertToBinary(Mat &oIm) const
{
    threshold(oIm, oIm, 90.0, 255.0, THRESH_BINARY);
    RETURN_NOERROR;
}


tResult DistanceSensorMerger::InvertBinary(Mat &oIm) const
{
    oIm = Scalar::all(255) - oIm;
    RETURN_NOERROR;
}


tResult DistanceSensorMerger::Merge(const Mat &oIPM, const Mat &oUS, const Mat &oVdar, Mat &oResult)
{
    oResult = Mat::zeros(oUS.rows, oUS.cols, CV_8UC1);

//    vector <Mat> channels;
//    channels.push_back(oVdar);
//    channels.push_back(oIPM);
//    channels.push_back(oUS);
//    merge(channels, oResult);

    bitwise_or(oIPM, oResult, oResult);
    bitwise_or(oUS, oResult, oResult);
    bitwise_or(oVdar, oResult, oResult);

    RETURN_IF_POINTER_NULL(oResult.data);
    RETURN_NOERROR;
}


tResult DistanceSensorMerger::TransmitOutput(const tTimeStamp &timeStamp,
                                             const tBitmapFormat &oVideoOutputFormat,
                                             cVideoPin &oOutputPin,
                                             const Mat &oImage)
{
    cObjectPtr <IMediaSample> mediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE,
                                              (tVoid **) &mediaSample));
    RETURN_IF_FAILED(mediaSample->AllocBuffer(oVideoOutputFormat.nSize));
    mediaSample->Update(timeStamp, oImage.data, oVideoOutputFormat.nSize, 0);
    oOutputPin.Transmit(mediaSample);
    RETURN_NOERROR;
}
