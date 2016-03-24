//
// Created by pbachmann on 1/7/16.
//

#ifndef HTWK_2016_LANEDETECTIONDATA_H
#define HTWK_2016_LANEDETECTIONDATA_H

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

class LaneDetectionData
{
    public:
        Mat Labels;
        Mat Stats;

        map<int, Point2f> StartLabels;
        map<int, Point2f> EndLabels;

        int LabelSearchRadius;
        int SafetyDistance;

        int CurrentLabel;
};


#endif //HTWK_2016_LANEDETECTIONDATA_H
