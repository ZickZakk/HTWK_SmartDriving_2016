//
// Created by gjenschmischek on 12/22/15.
//

#ifndef HTWK_2016_IPMBORDER_H
#define HTWK_2016_IPMBORDER_H

#include <opencv2/opencv.hpp>
#include "../../htwk_utils/LinearFunction.h"
#include <adtf_platform_inc.h>
#include <adtf_plugin_sdk.h>
#include <utils/base/types.h>
#include <numeric>
#include "../../htwk_utils/GeneralUtils.h"
#include "../../htwk_utils/HTWKMath.h"
#include "../../htwk_utils/VisionUtils.h"
#include "../../htwk_logger/Logger.h"

using namespace cv;
using namespace adtf;
using namespace adtf_util;

enum BorderDirection
{
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

class IPMBorder
{
    private:
        Point2f UpLeft;
        Point2f UpRight;
        Point2f DownLeft;
        Point2f DownRight;

        LinearFunction LeftBorder;
        LinearFunction RightBorder;

        bool initialized;

        int LowerBound;
        int UpperBound;

        SimpleBimap<int, Point2f, std::greater<int>, VisionUtils::PointSmaller> clockwiseMap;
        SimpleBimap<int, Point2f, std::greater<int>, VisionUtils::PointSmaller> counterClockwiseMap;

        Logger logger;

    private :
        Point2f GoClockwise(Point2f currentPoint);

        Point2f GoManyClockwise(Point2f currentPoint, int steps);

        Point2f GoManyCounterClockwise(Point2f currentPoint, int steps);

        Point2f GoCounterClockwise(Point2f currentPoint);

        bool IsInCounterClockwiseRange(Point2f startPoint, Point2f pointToFind, int range);

        bool IsInClockwiseRange(Point2f startPoint, Point2f pointToFind, int range);

        int GetClockwiseDistance(Point2f p1, Point2f p2);

        int GetCounterClockwiseDistance(Point2f p1, Point2f p2);

        bool IsOnBorder(Point2f);

    public:
        IPMBorder();

        void Initialize(Point2f UpLeft, Point2f UpRight, Point2f DownLeft, Point2f DownRight);

        bool IsInitialized();

        Point2f Go(Point2f currentPoint, BorderDirection direction);

        Point2f GoMany(Point2f currentPoint, int steps, BorderDirection direction);

        bool IsInRange(Point2f startPoint, Point2f pointToFind, int range, BorderDirection direction);

        Mat Draw(Mat image);

        int GetLowerBound();

        int GetUpperBound();

        static BorderDirection Invert(BorderDirection direction);

        int GetMinimalDistance(Point2f p1, Point2f p2);

        int GetDistance(Point2f p1, Point2f p2, BorderDirection direction);
};


#endif //HTWK_2016_IPMBORDER_H
