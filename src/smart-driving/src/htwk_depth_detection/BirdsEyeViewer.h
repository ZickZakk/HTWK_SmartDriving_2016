#include "opencv2/opencv.hpp"

#ifndef _BIRDSEYEVIEWER_H_
#define _BIRDSEYEVIEWER_H_

#include "CoordinateConverter.h"
#include "../htwk_logger/Logger.h"

class BirdsEyeViewer
{
    public:
        BirdsEyeViewer();

        ~BirdsEyeViewer();

        cv::Mat &process(const cv::Mat &imSrc);

    private:
        cv::Mat imRes;

        int screenWidth;
        int screenHeight;
        int topSearchArea;
        int bottomSearchArea;
        int screenCenterX;
        int screenCenterY;

        CoordinateConverter coordinateConverter;
};

#endif /* _BIRDSEYEVIEWER_H_ */
