//
// Created by gjenschmischek on 1/6/16.
//

#ifndef HTWK_2016_LANELINEEND_H
#define HTWK_2016_LANELINEEND_H

#include "opencv2/opencv.hpp"
#include "IPMBorder.h"

using namespace cv;

class LaneLineEnd
{
    private:
        Point2f signPoint;

        Point2f maxClockwiseBound;
        Point2f maxCounterClockwiseBound;

    public:
        void SetBound(Point2f point, BorderDirection direction);
        void SetSignPoint(Point2f point);

        Point2f GetBound(BorderDirection direction) const;

        Point2f GetSignPoint() const;
};

#endif //HTWK_2016_LANELINEEND_H
