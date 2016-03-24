/**
 * @author sfeig
 */

#ifndef _HTWK_DISTANCE_SENOR_MERGER_H_
#define _HTWK_DISTANCE_SENOR_MERGER_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "opencv2/opencv.hpp"
#include "../htwk_vision/Vision.h"
#include "../htwk_vdar/ConnectedComponentFinder.h"

using namespace cv;

#define OID_DISTANCE_SENOR_MERGER "htwk.distancesensormerger"
#define FILTER_NAME "HTWK Distance Sensor Merger"

class DistanceSensorMerger: public adtf::cFilter
{
    ADTF_FILTER(OID_DISTANCE_SENOR_MERGER, FILTER_NAME, OBJCAT_DataFilter);

    private:
        Logger logger;

        /////////
        // Input Pins
        cVideoPin m_oUSVideoInputPin, m_oVdarVideoInputPin, m_oIPMVideoInputPin;
        // Input Video Format
        tBitmapFormat m_oUSVideoInputFormat, m_oVdarVideoInputFormat, m_oIPMVideoInputFormat;

        /////////
        // Output Pins
        cVideoPin m_oVdarVideoOutputPin, m_oUSVideoOutputPin, m_oIPMVideoOutputPin, m_oMergedVideoOutputPin;
        // Output Video Format
        tBitmapFormat m_oUSVideoOutputFormat, m_oVdarVideoOutputFormat, m_oIPMVideoOutputFormat, m_oMergedVideoOutputFormat;

        // // mat
        Mat m_oVdar, m_oUS, m_oIPM, m_oMerged;
        Mat m_oVdarMerge, m_oUSMerge, m_oIPMMerge;

    private:
        ConnectedComponentFinder m_oConnectedComponentFinder;

        tResult ProcessInput(IMediaSample *mediaSample);
        tResult CreateInputPins(__exception = NULL);
        tResult CreateOutputPins(__exception = NULL);
        tResult InitVideoFormat(cVideoPin &oInputPin, cVideoPin &oOutputPin,
                                tBitmapFormat &oInputFormat, tBitmapFormat &oOutputFormat);
        tResult InitVideoFormatMerged();
        tResult TransmitOutput(const tTimeStamp &timeStamp, const tBitmapFormat &oVideoOutputFormat,
                               cVideoPin &oOutputPin, const Mat &oImage);

        tResult ConvertToBinary(Mat &oIm) const;
        tResult InvertBinary(Mat &oIm) const;
        tResult BoundingRects(Mat &oIm);


        tResult Merge(const Mat &oIPM, const Mat &oUS, const Mat &oVdar, Mat &oResult);

    public:
        DistanceSensorMerger(const tChar *__info);
        virtual ~DistanceSensorMerger();

    public:
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);
        tResult Init(tInitStage eStage, __exception = NULL);
        tResult Shutdown(tInitStage eStage, __exception);

};
#endif // _HTWK_DISTANCE_SENOR_MERGER_H_
