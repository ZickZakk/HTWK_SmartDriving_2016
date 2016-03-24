
#ifndef _HTWK_LANE_ANGLE_LIB_H_
#define _HTWK_LANE_ANGLE_LIB_H_

#define _USE_MATH_DEFINES
#include <math.h>

#include <opencv2/opencv.hpp>
#include <utils/base/types.h>
#include <adtf_platform_inc.h>
#include <adtf_plugin_sdk.h>
#include "../htwk_utils/HTWKMath.h"
#include "../htwk_logger/Logger.h"

using namespace cv;

using namespace adtf;
using namespace adtf_util;

enum Direction
{
    dirLeft,
    dirRight,
    dirForward,
    dirBackwards
};


class LaneAngleCalculator
{
    public:
        tFloat32 CalculateDrivingAngle(const Mat &map, const tFloat32 currentSpeed, const tFloat32 reactionDistance);

    private:
        void CalculatePoints();
        pair<tInt32, tInt32> SearchForCorner(pair<int, int> origin, Direction direction, const Mat &map);
        pair<bool, tInt32> CheckSide(Direction direction, const Mat &map, const int startX);
        tInt32 CheckSideWalking(const Mat &map,Direction direction, int start, int stop, bool obstacle, bool isTop);

        tFloat32 CalcAngle(int diff, int length);
        //tInt32 laneWidth = 10;
};

/* Vision Utility Functions */
int GetMatType(tInt16 bitsPerPixel);

#endif // _HTWK_LANE_ANGLE_LIB_H_