#include "LaneAngle.h"

tFloat32 LaneAngleCalculator::CalculateDrivingAngle(const Mat &map, const tFloat32 currentSpeed,
                                                     const tFloat32 reactionDistance)
{
    //Mat a = Mat(range, range, CV_8UC1); // Mat_<double>(3,3); // imread("c:/full/path/to/lena.jpg");

    pair<int, int> origin = make_pair(map.rows / 2, map.cols / 2);

    pair<int, int> rightPoint;
    pair<int, int> leftPoint;

    rightPoint = searchForCorner(origin, dirRight, map);
    leftPoint = searchForCorner(origin, dirLeft, map);

    return CalcAngle(rightPoint.first, rightPoint.second);
}

void LaneAngleCalculator::CalculatePoints()
{
    const int range = 400;

    Mat a = Mat(range, range, CV_8UC1); // Mat_<double>(3,3); // imread("c:/full/path/to/lena.jpg");

    int b[range][range];
    pair<int, int> origin = make_pair(200, 200);

    pair<int, int> rightPoint;
    pair<int, int> leftPoint;

    rightPoint = searchForCorner(origin, dirRight, a, range);
    leftPoint = searchForCorner(origin, dirLeft, a, range);

    pair<int, int> furtherRight = searchForCorner(rightPoint, dirForward, a, range);
    pair<int, int> furtherLeft = searchForCorner(leftPoint, dirForward, a, range);
}

/*
 * calc angle of 3 eck
 *      diff
 * -----------------
 *  |               |
 *    |             | length
 *  c   |           |
 *        |         |
 *
 */
tFloat32 LaneAngleCalculator::CalcAngle(int diff, int length)
{
    tFloat32 fDiff = (float) diff;
    tFloat32 fLength = (float) length;
    tFloat32 c = sqrt(fDiff * fDiff + fLength * fLength);

    // sin alpha = diff / c
    return asin(fDiff / c) * 180 / M_PI;
}

pair<tInt32, tInt32 > LaneAngleCalculator::SearchForCorner(pair<int, int> origin, Direction direction, const Mat &map)
{
    /*
     * New Idea:
     * slice out blocks of 5-10 px
     * compare height of left/right begining and end
     *
     */

    int distance = 10; // var from Param
    int maxDistance = 60;

    bool onRightLane = true;
    int rightEdge = map.cols + (onRightLane ? 80 : 160); // 80 to the right from center
    int leftEdge = map.cols - (onRightLane ? 160 : 80); // 160 to the left from center

    int originalCenter = (onRightLane ? 160 : 80);

    tInt32 heightDiff = 0;

    int iteration = 0;
    bool notFound = true;
    while (notFound && (iteration * distance < maxDistance)) {
        Mat block = map(cv::Rect(leftEdge, map.rows / 2 - iteration * distance, rightEdge + leftEdge, distance));

        pair<bool, tInt32> erg = CheckSide(dirRight, map, leftEdge);

        notFound = !erg.first;
        heightDiff = erg.second;
        iteration ++;
    }

    return make_pair(heightDiff, distance * iteration);
}


/* Calculate the heightdifference between beginning and end of Block
 * last row:    --::::
 * first row:   ----::
 * -> diff = 2
 */
pair<bool, tInt32> LaneAngleCalculator::CheckSide(Direction direction, const Mat &map, const int startX)
{
    int distance = 5;

    int laneEnd = 0;
    int heightDiff = 0;

    switch (direction) {
        case dirLeft:
            distance = distance * -1;

            laneEnd = CheckSideWalking(&map, dirLeft, startX, 0, true, false);
            heightDiff = CheckSideWalking(&map, dirRight, laneEnd, map.cols - 1, false, true);
            break;
        case dirRight:
            laneEnd = CheckSideWalking(&map, dirRight, startX, map.cols - 1, true, false);
            heightDiff = CheckSideWalking(&map, dirLeft, laneEnd, 0, false, true);
            break;
        default:
            break;
    }

    int minHeightDiff = 2;
    if (heightDiff > minHeightDiff) {
        return make_pair(true, heightDiff);
    }
    else {
        return make_pair(false, 0);
    }
}

tInt32 LaneAngleCalculator::CheckSideWalking(const Mat &map, Direction direction, int start,
                                             int stop, bool obstacle, bool isTop)
{
    int y = 0;
    if (!isTop) {
         y = map.rows - 1;
    }

    for (int i = start; (direction == dirRight ? i <= stop : i >= stop); (direction == dirRight ? ++i: --i))
    {
        if (map.data[i][y] > 128 && obstacle)
        {
            return i;
        }
        else if (map.data[i][y] < 128 && !obstacle) {
            return i;
        }
    }
    return 0;
}


int GetMatType(tInt16 bitsPerPixel)
{
    switch (bitsPerPixel)
    {
        case (24): // rgb image
            return CV_8UC3;
        case (16): // depth image
            return CV_16UC1;
        case (8): // grayscale image
            return CV_8UC1;
        default:
            return 0;
    }
}