#ifndef _HTWK_IPM_FILTER_H_
#define _HTWK_IPM_FILTER_H_

#include "stdafx.h"

#include <iterator>

#include "opencv2/opencv.hpp"

#include "../htwk_logger/Logger.h"
#include "../htwk_structs/tRect.h"
#include "../htwk_structs/tReadyModule.h"
#include "../htwk_utils/HTWKMath.h"
#include "../htwk_utils/VisionUtils.h"
#include "../htwk_vision/Vision.h"

using namespace cv;

#define OID_IPM "htwk.ipm"
#define FILTER_NAME "HTWK IPM"

// Properties
#define MAX_LINECOUNT_PROPERTY "Hough Maximum Line Count"
#define THRESHHOLD_PROPERTY "Threshhold"
#define INITIAL_VP_X_PROPERTY "VP_X"
#define INITIAL_VP_Y_PROPERTY "VP_Y"

class IPM : public adtf::cFilter
{
    ADTF_FILTER(OID_IPM, FILTER_NAME, OBJCAT_DataFilter);

    private: //private members
        Logger logger;

        cVideoPin videoInputPin;
        tBitmapFormat inputFormat;

        cVideoPin ipmOutputPin;
        tBitmapFormat outputFormat;

        cVideoPin videoDebugPin;

        tBool stabilizedVP;
        cOutputPin readyPin;

        cInputPin roiPin;
        cObjectPtr<IMediaTypeDescription> descriptionRect;
        cObjectPtr<IMediaType> typeRect;

        cInputPin getReadyInput;
        cObjectPtr<IMediaTypeDescription> descriptionEnumBox;
        cObjectPtr<IMediaType> typeEnumBox;

        // Detector
        VanishingPointDetector vanishingPointDetector;

        //Image Mapper
        InversePerspectiveMapper inversePerspectiveMapper;

        Rect_<tInt16> roi;

        Mat outputImage;
        Mat debugImage;

        unsigned long maxLines;
        int threshHold;
        float VanishingPointX;
        float VanishingPointY;

        bool readySended;

    private: //private functions
        tResult ProcessImage(IMediaSample *mediaSample);

        tResult ProcessROI(IMediaSample *mediaSample);

        tResult ProcessGetReady(IMediaSample *mediaSample);

        tResult TransmitImage(const Mat image, const tTimeStamp &timeStamp, cVideoPin &pin);

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

        tResult InitDescriptions(IException **__exception_ptr);

        tResult TransmitDebug(const Mat debugImage, const tTimeStamp &timeStamp);

        tResult TransmitReady();

    public: //common implementation
        IPM(const tChar *__info);

        virtual ~IPM();

    public: // overwrites cFilter
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_IPM_FILTER_H_
