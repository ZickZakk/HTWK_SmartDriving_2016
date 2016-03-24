/**
 * @author sfeig
 */

#ifndef _HTWK_DEPTH_DETECTION_H_
#define _HTWK_DEPTH_DETECTION_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "NoiseRemover.h"
#include "GroundRemover.h"
#include "BirdsEyeViewer.h"

#include "opencv2/opencv.hpp"
#include "ConnectedComponentFinder.h"

using namespace cv;

#define OID_DEPTH_DETECTION "htwk.Depth_Detection"
#define FILTER_NAME "HTWK Depth Detection"

class DepthDetection : public adtf::cFilter
{
    ADTF_FILTER(OID_DEPTH_DETECTION, FILTER_NAME, OBJCAT_DataFilter);

    public:
        DepthDetection(const tChar *__info);

        virtual ~DepthDetection();

        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);
        tResult Init(tInitStage eStage, __exception = NULL);
        tResult Shutdown(tInitStage eStage, __exception);

    private:
        unsigned int m_nFrameCounter;
        cVideoPin videoInputPin;
        cVideoPin IPMInputPin;
        cVideoPin videoOutputPin;

        Logger logger;

        tBitmapFormat m_oVideoInputFormat;
        tBitmapFormat m_oVideoOutputFormat;

        Mat m_oDepth;
        Mat m_oBirdEye;
        Mat m_oOutputImage;
        Mat m_oKernel;

        NoiseRemover denoiser;
        GroundRemover groundRemover;
        BirdsEyeViewer birdsEyeViewer;
        ConnectedComponentFinder connectedComponentFinder;


        Mat ExtractDepthImageFromMediaSample(IMediaSample *mediaSample, const tBitmapFormat &inputFormat);

        tResult TransmitOutput(const tTimeStamp &timeStamp, const tBitmapFormat &oVideoOutputFormat,
                               cVideoPin &oOutputPin, const Mat &oImage);
        tResult CreateInputPins(__exception = NULL);
        tResult CreateOutputPins(__exception = NULL);

        tResult DrawBoundingRects(Mat &oIm);
};

#endif // _HTWK_DEPTH_DETECTION_H_
