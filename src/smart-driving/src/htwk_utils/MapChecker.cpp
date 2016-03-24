//
// Created by pbachmann on 2/15/16.
//

#include "MapChecker.h"
#include "VisionUtils.h"

/**
 * returns free distance in meter
 */
float MapChecker::GetFreeFrontCenterDistance(const Mat &map)
{
    int frontX = 200 + ULTRASONIC_FRONT_CENTER_POS_X;
    int frontY = 200 + ULTRASONIC_FRONT_CENTER_POS_Y;

    int distance = 0;
    for (int i = frontY; i > 0; --i)
    {
        uchar color = map.at<uchar>(i, frontX);

        if (color == 255)
        {
            break;
        }

        distance = frontY - i;
    }

    return distance / 100.0f;
}

/**
 * returns free distance in meter
 */
float MapChecker::GetFreeSideRightDistance(const Mat &map)
{
    int rightX = 200 + ULTRASONIC_SIDE_RIGHT_POS_X;
    int rightY = 200 + ULTRASONIC_SIDE_RIGHT_POS_Y;

    int distance = 0;
    for (int i = rightX; i < map.cols; ++i)
    {
        uchar color = map.at<uchar>(rightY, i);

        if (color == 255)
        {
            break;
        }

        distance = i - rightX;
    }

    return distance / 100.0f;
}

/**
 * returns free distance in meter
 */
float MapChecker::GetFreeSideLeftDistance(const Mat &map)
{
    int leftX = 200 + ULTRASONIC_SIDE_LEFT_POS_X;
    int leftY = 200 + ULTRASONIC_SIDE_LEFT_POS_Y;

    int distance = 0;
    for (int i = leftX; i > 0; --i)
    {
        uchar color = map.at<uchar>(leftY, i);

        if (color == 255)
        {
            break;
        }

        distance = leftX - i;
    }

    return distance / 100.0f;
}

/**
 * returns free distance in meter
 */
float MapChecker::GetFreeRearCenterDistance(const Mat &map)
{
    int backX = 200 + ULTRASONIC_REAR_CENTER_POS_X;
    int backY = 200 + ULTRASONIC_REAR_CENTER_POS_Y;

    int distance = 0;
    for (int i = backY; i < map.rows; ++i)
    {
        uchar color = map.at<uchar>(i, backX);

        if (color == 255)
        {
            break;
        }

        distance = i - backY;
    }

    return distance / 100.0f;
}

bool MapChecker::IsAreaFree(const Mat &map, const Rect &rect)
{
    Mat roi(map, rect);
    return cv::countNonZero(roi) == 0;
}

bool MapChecker::IsCurveFree(const Mat &map, float radius, Direction::DirectionEnum direction)
{
    if (direction == Direction::LEFT)
    {
        for (int x = 0; x > radius; x--)
        {
            Point2f worldPoint(x, HTWKMath::CircleFunction(radius, x));
            Point2f imagePoint = VisionUtils::WorldToImage(worldPoint);

            if (map.at<uchar>(imagePoint) > 0)
            {
                return false;
            }
        }

        return true;
    }
    else if (direction == Direction::RIGHT)
    {
        for (int x = 0; x < radius; x++)
        {
            Point2f worldPoint(x, HTWKMath::CircleFunction(radius, x));
            Point2f imagePoint = VisionUtils::WorldToImage(worldPoint);

            if (map.at<uchar>(imagePoint) > 0)
            {
                return false;
            }
        }

        return true;
    }

    return false;
}
