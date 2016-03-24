#ifndef _HTWK_IPM_FILTER_H_
#define _HTWK_IPM_FILTER_H_

#include "stdafx.h"

#include "opencv2/opencv.hpp"

#include "../htwk_logger/Logger.h"
#include "../htwk_structs/tReadyModule.h"
#include "../htwk_structs/tRoadSign.h"
#include "../htwk_vision/Vision.h"

using namespace cv;

#define OID_LANE_DRIVER "htwk.lane_detection"
#define FILTER_NAME "HTWK Lane Detection"

// Properties
#define BRIGHTNESS_FACTOR_PROPERTY "Brightness Factor"
#define THRESH_BUFFER_SIZE_PROPERTY "Buffer Size"

class LaneDetection : public adtf::cFilter
{
    ADTF_FILTER(OID_LANE_DRIVER, FILTER_NAME, OBJCAT_DataFilter);

    private: //private members
        cVideoPin ipmInputPin;
        cVideoPin videoDebugPin;
        Logger logger;

        /*! bitmapformat of input image */
        tBitmapFormat inputFormat;
        /*! bitmapformat of output image */
        tBitmapFormat outputFormat;

        cInputPin ipmReadyPin;

        cInputPin getReadyPin;
        cOutputPin readyPin;
        cObjectPtr<IMediaTypeDescription> descriptionEnumBox;
        cObjectPtr<IMediaType> typeEnumBox;

        cOutputPin lanePin;
        cObjectPtr<IMediaTypeDescription> descriptionLane;
        cObjectPtr<IMediaType> typeLane;

        Mat outputImage;
        Mat inputImage;

        // Properties
        LaneDetector laneDetector;
        tBool ipmReady;
        bool readySended;
        unsigned long threshBufferSize;

        cCriticalSection safeSection;

    private: //private functions
        tResult ProcessLane(IMediaSample *mediaSample);

        tResult ProcessGetReady(IMediaSample *mediaSample);

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

        tResult TransmitDebug(const Mat &debugImage, const tTimeStamp &timeStamp);

        tResult ProcessIpmReady(IMediaSample *pSample);

        tResult InitDescriptions(IException **__exception_ptr);

        tResult TransmitResult(tLane lane);

        tResult TransmitReady();

        tResult ProcessIpm(IMediaSample *mediaSample);

    public: //common implementation
        LaneDetection(const tChar *__info);

        virtual ~LaneDetection();

    public: // overwrites cFilter
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_IPM_FILTER_H_
