#ifndef _HTWK_TI_FILTER_H_
#define _HTWK_TI_FILTER_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "opencv2/opencv.hpp"
#include "../htwk_vision/Vision.h"
#include "../htwk_utils/VisionUtils.h"

using namespace cv;

#define OID_TI "htwk.temporal_image"
#define FILTER_NAME "HTWK Temporal Image"

// Properties
#define BUFFER_SIZE_PROPERTY "BufferSizeProperty"
#define METHOD_PROPERTY "MethodProperty"

// Methods
#define METHOD_MEAN 1
#define METHOD_AND 2
#define METHOD_OR 3

class TemporalImage : public adtf::cFilter
{
    ADTF_FILTER(OID_TI, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cVideoPin videoInputPin;
        cVideoPin videoOutputPin;

        Logger logger;

        tBitmapFormat videoInputFormat;
        tBitmapFormat videoOutputFormat;

        Mat workingImage;
        Mat outputImage;

        vector<Mat> buffer;
        tUInt bufferIndex;

        // Properties
        tUInt bufferSize;
        tUInt8 method;

    private:
        tResult ProcessImage();

        Mat CalculateMean();

        Mat CalculateAnd();

        Mat CalculateOr();

        tResult TransmitOutput(const tTimeStamp &timeStamp);

        void InitializeProperties();

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

    public:
        TemporalImage(const tChar *__info);

        virtual ~TemporalImage();

    public:
        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_TI_FILTER_H_
