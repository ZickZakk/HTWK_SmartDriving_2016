//
// Created by pbachmann on 2/15/16.
//

#ifndef HTWK_2016_MAPCHECKER_H
#define HTWK_2016_MAPCHECKER_H

#include "opencv2/opencv.hpp"

#include "../htwk_structs/ObstacleGrid.h"
#include "../htwk_structs/Direction.h"
#include "HTWKMath.h"

using namespace cv;

class MapChecker
{
    public:
        static float GetFreeFrontCenterDistance(const Mat &map);

        static float GetFreeSideRightDistance(const Mat &map);

        static float GetFreeSideLeftDistance(const Mat &map);

        static float GetFreeRearCenterDistance(const Mat &map);

        static bool IsAreaFree(const Mat &map, const Rect &rect);

        static bool IsCurveFree(const Mat &map, float radius, Direction::DirectionEnum direction);
};


#endif //HTWK_2016_MAPCHECKER_H
