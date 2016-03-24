//
// Created by pbachmann on 2/25/16.
//

#include "CrossingDetector.h"
#include "../../../htwk_utils/GeneralUtils.h"

CrossingDetector::CrossingDetector(string templateImageName, Point2f pointShift, PointAcceptor *pointChecker, float threshHold)
{
    CrossingDetector::templateImage = imread(GeneralUtils::GetExePath() + "resources/" + templateImageName, CV_LOAD_IMAGE_GRAYSCALE);
    CrossingDetector::pointShift = pointShift;
    CrossingDetector::Acceptor = pointChecker;
    CrossingDetector::threshHold = threshHold;
}

bool CrossingDetector::FindCrossing(const Mat &image, Point2f &outPoint, Logger &logger)
{
    Mat matchingResult;

    int mode = CV_TM_SQDIFF_NORMED;

    /// Do the Matching and Normalize
    matchTemplate(image, templateImage, matchingResult, mode);

    /// Localizing the best match with minMaxLoc
    double minVal;
    double maxVal;
    Point minLoc;
    Point maxLoc;

    Point foundPoint;
    do
    {
        minMaxLoc(matchingResult, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
        matchingResult.at<float>(minLoc) = 1;
        foundPoint = minLoc + pointShift;

    } while (!Acceptor->check(foundPoint, image.cols) && (minVal < threshHold));

    logger.Log(cString::Format("Min Value: %f", minVal).GetPtr(), false);

    if (minVal < threshHold)
    {
        foundPoints.push_back(foundPoint);
    }
    else
    {
        foundPoints.clear();
    }

    if (foundPoints.size() < 3)
    {
        return false;
    }

    logger.Log("Checking Points", false);

    logger.Log(cString::Format("Point %d: x: %d | y: %d", 0, foundPoints[0].x, foundPoints[0].y).GetPtr(), false);

    for (unsigned int i = 1; i < foundPoints.size(); ++i)
    {
        logger.Log(cString::Format("Point %d: x: %d | y: %d", i, foundPoints[i].x, foundPoints[i].y).GetPtr(), false);

        if (!GeneralUtils::Equals(foundPoints[i], foundPoints[i - 1], 20))
        {
            logger.Log("No Success", false);
            foundPoints.clear();
            return false;
        }
    }

    outPoint = foundPoints.back();
    return true;
}

CrossingDetector::CrossingDetector()
{

}
