/*
@author: gjenschmischek
*/
//#############################################################################
#ifndef _HTWK_VISION_LIB_H_
#define _HTWK_VISION_LIB_H_

#include <opencv2/opencv.hpp>
#include <utils/base/types.h>
#include <adtf_platform_inc.h>
#include <adtf_plugin_sdk.h>
#include "../htwk_utils/HTWKMath.h"
#include "../htwk_logger/Logger.h"
#include "LaneDetection/IPMBorder.h"
#include <numeric>
#include "../htwk_utils/GeneralUtils.h"
#include "../htwk_utils/PolynomialFitter.h"
#include "LaneDetection/BaseLaneLine.h"
#include "LaneDetection/LaneLineMode.h"
#include "../htwk_structs/tLane.h"

using namespace cv;

using namespace adtf;
using namespace adtf_util;

class VanishingPointDetector
{
    private:
        unsigned long maxLines;
        int threshHold;

        Point2f currentVanishingPoint;

        bool stabilized;
        int stabilizationCounter;

        bool initialized;

    public:
        VanishingPointDetector();

        Point2f CalculateVanishingPoint(const Mat &image);

        void Reset(unsigned long maxLines, int threshHold, float VPX, float VPY);

        bool IsStabilized();
};

class InversePerspectiveMapper
{
    public:
        Mat MapInversePerspectiveFromVanishingPoint(const Mat &image, const Point2f &vanishingPoint);
};

class ThreshHolder
{
    public:
        int Yen(const Mat &image, bool ignoreBlack, bool ignoreWhite);
};

class LaneDetector
{
    private:
        ThreshHolder threshHolder;

        vector<int> threshBuffer;
        unsigned long threshBufferIndex;
        unsigned long threshBufferSize;

        bool initialized;

        bool reseted;

        IPMBorder border;

        Ptr<BaseLaneLine> rightLine;
        Ptr<BaseLaneLine> leftLine;

        RightAcceptor IsRight;
        LeftAcceptor IsLeft;

        Logger *laneLogger;

    private:
        void ExtractBorder(Mat image);

        Mat MakeBinary(Mat ipmImage);

        tLane GenerateNormalizedLaneStruct(const Mat &image);

        tLine GenerateNormalizedLineStruct(Ptr<BaseLaneLine> line, const Mat &image);

    public:
        Mat DebugImage;

    public:
        LaneDetector();

        void Reset(unsigned long threshBufferSize);

        tLane DetectLanes(Mat &inputImage, Logger &logger);

        bool IsReady();
};

/* Vision Utility Functions */



#endif // _HTWK_VISION_LIB_H_
