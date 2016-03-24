//
// Created by pbachmann on 2/25/16.
//

#ifndef HTWK_2016_CROSSINGDETECTOR_H
#define HTWK_2016_CROSSINGDETECTOR_H

#include "opencv2/opencv.hpp"
#include "SideStruct.h"
#include "../../../htwk_logger/Logger.h"
#include "stdafx.h"

using namespace cv;
using namespace std;

class CrossingDetector
{
    private:
        Mat templateImage;

        Point pointShift;

        float threshHold;

        vector<Point> foundPoints;

    public:
        CrossingDetector(string templateImageName, Point2f pointShift, PointAcceptor *pointChecker, float threshHold);

        CrossingDetector();

        bool FindCrossing(const Mat &image, Point2f &outPoint, Logger &logger);

        PointAcceptor *Acceptor;
};


#endif //HTWK_2016_CROSSINGDETECTOR_H
