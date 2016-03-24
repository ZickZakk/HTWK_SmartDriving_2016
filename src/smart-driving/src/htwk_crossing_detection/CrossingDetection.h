//
// Created by pbachmann on 2/17/16.
//

#ifndef HTWK_2016_CROSSINGDETECTION_H
#define HTWK_2016_CROSSINGDETECTION_H

#include "opencv2/opencv.hpp"
#include <vector>


using namespace cv;

typedef struct HTWK_Triangle
{
    std::vector<Point2i> points;

};

class CrossingDetection
{
    public:
        CrossingDetection();

        virtual ~CrossingDetection();

    private:
        void CheckLeftLane();
};

/*
 * draw triangle onto road if katheses are marked -> crossing
 *
 */


#endif //HTWK_2016_CROSSINGDETECTION_H
