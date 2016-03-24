/**
 * @author pbachmann
 */

#ifndef _HTWK_GRAYSCALE_H_
#define _HTWK_GRAYSCALE_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "opencv2/opencv.hpp"
#include "../htwk_vision/Vision.h"

using namespace cv;

#define OID_GRAYSCALE "htwk.grayscale"
#define FILTER_NAME "HTWK Grayscale"

class Grayscale : public adtf::cFilter
{
    ADTF_FILTER(OID_GRAYSCALE, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cVideoPin videoInputPin;
        cVideoPin videoOutputPin;

        Logger logger;

        tBitmapFormat videoInputFormat;
        tBitmapFormat videoOutputFormat;

        Mat outputImage;
        Mat mask;

        //properties
        tUInt8 iterations;

    private:
        tResult ProcessInput(IMediaSample *mediaSample);

        tResult TransmitOutput(const tTimeStamp &timeStamp);

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

    public:
        Grayscale(const tChar *__info);

        virtual ~Grayscale();

    public:
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);

        void InitializeProperties();
};

#endif // _HTWK_GRAYSCALE_H_
