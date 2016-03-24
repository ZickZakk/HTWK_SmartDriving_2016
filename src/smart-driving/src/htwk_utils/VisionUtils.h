//
// Created by pbachmann on 1/7/16.
//

#ifndef HTWK_2016_VISIONUTILS_H
#define HTWK_2016_VISIONUTILS_H

#include <opencv2/opencv.hpp>
#include <utils/base/types.h>
#include <utils/base/structs.h>
#include <adtf_platform_inc.h>
#include <adtf_plugin_sdk.h>
#include "GeneralUtils.h"
#include "../htwk_structs/tPoint.h"
#include "../htwk_structs/tLine.h"
#include "../htwk_structs/tLane.h"

using namespace adtf;
using namespace adtf_util;

using namespace cv;
using namespace std;

const float StandardPointDelta = 0.01f;

const float CV_PI_F = 3.14159265358979f;

class VisionUtils
{
    public:
        static bool ContourCompare(vector<Point> vec1, vector<Point> vec2);

        static Mat ExtractImageFromMediaSample(IMediaSample *mediaSample, const tBitmapFormat &inputFormat);

        static int GetMatType(tInt16 bitsPerPixel);

        static bool IsHorizontal(Vec<float, 2> houghLine);

        static bool StandardSmallerPoint(Point2f p1, Point2f p2);

        static bool StandardSmaller(float f1, float f2);

        static bool StandardEqualsPoint(Point2f p1, Point2f p2);

        static bool StandardEquals(float f1, float f2);

        static bool StandardBigger(float f1, float f2);

        static Point2f WorldToImage(Point2f world);

        static Point WorldToImage(tPoint world);

        class PointSmaller
        {
            public:
                bool operator()(const Point2f p1, const Point2f p2) const
                {
                    return VisionUtils::StandardSmallerPoint(p1, p2);
                }
        };

        static tLine GenerateDefaultLine();

        static tLane GenerateDefaultLane();
};

#endif //HTWK_2016_VISIONUTILS_H
