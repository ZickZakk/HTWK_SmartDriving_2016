#ifndef _HTWK_IMAGE_FUSION_REACTOR_FILTER_H_
#define _HTWK_IMAGE_FUSION_REACTOR_FILTER_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "opencv2/opencv.hpp"
#include "../htwk_vision/Vision.h"

using namespace cv;

#define OID_IFR "htwk.image_fusion_reactor"
#define FILTER_NAME "HTWK Image Fusion Reactor"

// Properties
#define METHOD_PROPERTY "MethodProperty"

// Methods
#define METHOD_MEAN 1
#define METHOD_ADD 2
#define METHOD_OR 3

class ImageFusionReactor : public adtf::cFilter
{
    ADTF_FILTER(OID_IFR, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cVideoPin firstVideoInputPin;
        cVideoPin secondVideoInputPin;
        cVideoPin mergedVideoOutputPin;

        Logger logger;

        tBitmapFormat firstVideoInputFormat;
        tBitmapFormat secondVideoInputFormat;
        tBitmapFormat mergedVideoOutputFormat;

        Mat firstImage;
        Mat secondImage;
        Mat outputImage;

        tUInt8 method;

    private:
        tResult TransmitOutput(const tTimeStamp &timeStamp);

        void InitializeProperties();

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

    public:
        ImageFusionReactor(const tChar *__info);

        virtual ~ImageFusionReactor();

    public:
        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);

        void CalculateMean();

        void CalculateAnd();

        void CalculateOr();
};

#endif // _HTWK_IMAGE_FUSION_REACTOR_FILTER_H_
