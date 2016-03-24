//
// Created by pbachmann on 12/22/15.
//

#include "IPMBorder.h"

void IPMBorder::Initialize(Point2f UpLeft, Point2f UpRight, Point2f DownLeft,
                           Point2f DownRight)
{
    IPMBorder::UpLeft = UpLeft;
    IPMBorder::UpRight = UpRight;
    IPMBorder::DownLeft = DownLeft;
    IPMBorder::DownRight = DownRight;

    vector<Point2f> points(2);
    points[0] = UpLeft;
    points[1] = DownLeft;
    LeftBorder = HTWKMath::LinearRegression(points);

    points[0] = UpRight;
    points[1] = DownRight;
    RightBorder = HTWKMath::LinearRegression(points);

    LowerBound = cvRound(DownLeft.y);
    UpperBound = cvRound(UpLeft.y);

    Point2f current = UpLeft;

    for (int i = 0; i < 1000; i++)
    {
        current = GoClockwise(current);
        clockwiseMap.insert(i, current);

        if (VisionUtils::StandardEqualsPoint(current, UpLeft))
        {
            break;
        }
    }

    for (int i = 0; i < 1000; i++)
    {
        current = GoCounterClockwise(current);
        counterClockwiseMap.insert(i, current);

        if (VisionUtils::StandardEqualsPoint(current, UpLeft))
        {
            break;
        }
    }

    initialized = true;
}

IPMBorder::IPMBorder() : logger("IPMBorder")
{
    initialized = false;
}

Point2f IPMBorder::GoCounterClockwise(Point2f currentPoint)
{
    if (VisionUtils::StandardEqualsPoint(currentPoint, UpLeft))
    {
        currentPoint.y = cvRound(currentPoint.y + 1);
        currentPoint.x = LeftBorder.CalculateX(currentPoint.y);
        return currentPoint;
    }
    else if (VisionUtils::StandardEqualsPoint(currentPoint, UpRight))
    {
        currentPoint.y = cvRound(currentPoint.y);
        currentPoint.x = cvRound(currentPoint.x - 1);
        return currentPoint;
    }
    else if (VisionUtils::StandardEqualsPoint(currentPoint, DownRight))
    {
        currentPoint.y = cvRound(currentPoint.y - 1);
        currentPoint.x = RightBorder.CalculateX(currentPoint.y);
        return currentPoint;
    }
    else if (VisionUtils::StandardEqualsPoint(currentPoint, DownLeft))
    {
        currentPoint.y = cvRound(currentPoint.y);
        currentPoint.x = cvRound(currentPoint.x + 1);
        return currentPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, UpRight.x)
             && VisionUtils::StandardBigger(currentPoint.x, UpLeft.x)
             && VisionUtils::StandardEquals(currentPoint.y, UpLeft.y))
    {
        currentPoint.x--;
        return currentPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, DownRight.x)
             && VisionUtils::StandardBigger(currentPoint.x, DownLeft.x)
             && VisionUtils::StandardEquals(currentPoint.y, DownRight.y))
    {
        currentPoint.x++;
        return currentPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, DownLeft.x)
             && VisionUtils::StandardBigger(currentPoint.x, UpLeft.x)
             && VisionUtils::StandardSmaller(currentPoint.y, DownLeft.y)
             && VisionUtils::StandardBigger(currentPoint.y, UpLeft.y))
    {
        Point2f nextPoint;
        nextPoint.y = currentPoint.y + 1;
        nextPoint.x = LeftBorder.CalculateX(nextPoint.y);

        return nextPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, UpRight.x)
             && VisionUtils::StandardBigger(currentPoint.x, DownRight.x)
             && VisionUtils::StandardSmaller(currentPoint.y, DownRight.y)
             && VisionUtils::StandardBigger(currentPoint.y, UpRight.y))
    {
        Point2f nextPoint;
        nextPoint.y = currentPoint.y - 1;
        nextPoint.x = RightBorder.CalculateX(nextPoint.y);

        return nextPoint;
    }

    return Point2f(-1, -1);
}

Point2f IPMBorder::GoClockwise(Point2f currentPoint)
{
    if (VisionUtils::StandardEqualsPoint(currentPoint, UpLeft))
    {
        currentPoint.y = cvRound(currentPoint.y);
        currentPoint.x = cvRound(currentPoint.x + 1);
        return currentPoint;
    }
    else if (VisionUtils::StandardEqualsPoint(currentPoint, UpRight))
    {
        currentPoint.y = cvRound(currentPoint.y + 1);
        currentPoint.x = RightBorder.CalculateX(currentPoint.y);
        return currentPoint;
    }
    else if (VisionUtils::StandardEqualsPoint(currentPoint, DownRight))
    {
        currentPoint.y = cvRound(currentPoint.y);
        currentPoint.x = cvRound(currentPoint.x - 1);
        return currentPoint;
    }
    else if (VisionUtils::StandardEqualsPoint(currentPoint, DownLeft))
    {
        currentPoint.y = cvRound(currentPoint.y - 1);
        currentPoint.x = LeftBorder.CalculateX(currentPoint.y);
        return currentPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, UpRight.x)
             && VisionUtils::StandardBigger(currentPoint.x, UpLeft.x)
             && VisionUtils::StandardEquals(currentPoint.y, UpLeft.y))
    {
        currentPoint.x++;
        return currentPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, DownRight.x)
             && VisionUtils::StandardBigger(currentPoint.x, DownLeft.x)
             && VisionUtils::StandardEquals(currentPoint.y, DownRight.y))
    {
        currentPoint.x--;
        return currentPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, DownLeft.x)
             && VisionUtils::StandardBigger(currentPoint.x, UpLeft.x)
             && VisionUtils::StandardSmaller(currentPoint.y, DownLeft.y)
             && VisionUtils::StandardBigger(currentPoint.y, UpLeft.y))
    {
        Point2f nextPoint;
        nextPoint.y = currentPoint.y - 1;
        nextPoint.x = LeftBorder.CalculateX(nextPoint.y);

        return nextPoint;
    }
    else if (VisionUtils::StandardSmaller(currentPoint.x, UpRight.x)
             && VisionUtils::StandardBigger(currentPoint.x, DownRight.x)
             && VisionUtils::StandardSmaller(currentPoint.y, DownRight.y)
             && VisionUtils::StandardBigger(currentPoint.y, UpRight.y))
    {
        Point2f nextPoint;
        nextPoint.y = currentPoint.y + 1;
        nextPoint.x = RightBorder.CalculateX(nextPoint.y);

        return nextPoint;
    }

    return Point2f(-1, -1);
}

bool IPMBorder::IsInitialized()
{
    return initialized;
}

int IPMBorder::GetLowerBound()
{
    if (!initialized)
    {
        return -1;
    }

    return LowerBound;
}

int IPMBorder::GetUpperBound()
{
    if (!initialized)
    {
        return -1;
    }

    return UpperBound;
}

Point2f IPMBorder::GoManyCounterClockwise(Point2f currentPoint, int steps)
{
    int currentIndex = counterClockwiseMap.getFirst(currentPoint);

    int requestedIndex = (currentIndex + steps) % counterClockwiseMap.size();
    return counterClockwiseMap.getSecond(requestedIndex);
}

Point2f IPMBorder::GoManyClockwise(Point2f currentPoint, int steps)
{
    int currentIndex = clockwiseMap.getFirst(currentPoint);

    int requestedIndex = (currentIndex + steps) % clockwiseMap.size();
    return clockwiseMap.getSecond(requestedIndex);
}

Mat IPMBorder::Draw(Mat image)
{
    Mat workingImage = image.clone();

    line(workingImage, UpLeft, UpRight, CV_RGB(0, 255, 255), 2);
    line(workingImage, UpRight, DownRight, CV_RGB(0, 255, 255), 2);
    line(workingImage, DownRight, DownLeft, CV_RGB(0, 255, 255), 2);
    line(workingImage, DownLeft, UpLeft, CV_RGB(0, 255, 255), 2);

    return workingImage;
}

Point2f IPMBorder::Go(Point2f currentPoint, BorderDirection direction)
{
    if (!initialized)
    {
        return Point2f(-1, -1);
    }

    if (!IsOnBorder(currentPoint))
    {
        return Point2f(-1, -1);
    }

    if (direction == COUNTER_CLOCKWISE)
    {
        return GoCounterClockwise(currentPoint);
    }
    else
    {
        return GoClockwise(currentPoint);
    }
}

Point2f IPMBorder::GoMany(Point2f currentPoint, int steps, BorderDirection direction)
{
    if (!initialized)
    {
        return Point2f(-1, -1);
    }

    if (!IsOnBorder(currentPoint))
    {
        return Point2f(-1, -1);
    }

    if (direction == COUNTER_CLOCKWISE)
    {
        return GoManyCounterClockwise(currentPoint, steps);
    }
    else
    {
        return GoManyClockwise(currentPoint, steps);
    }
}

bool IPMBorder::IsInCounterClockwiseRange(Point2f startPoint, Point2f pointToFind, int range)
{
    return GetCounterClockwiseDistance(startPoint, pointToFind) <= range;
}

bool IPMBorder::IsInClockwiseRange(Point2f startPoint, Point2f pointToFind, int range)
{
    return GetClockwiseDistance(startPoint, pointToFind) <= range;
}

bool IPMBorder::IsInRange(Point2f startPoint, Point2f pointToFind, int range, BorderDirection direction)
{
    if (!initialized)
    {
        return false;
    }

    if (!IsOnBorder(startPoint) || !IsOnBorder(pointToFind))
    {
        return false;
    }

    if (direction == COUNTER_CLOCKWISE)
    {
        return IsInCounterClockwiseRange(startPoint, pointToFind, range);
    }
    else
    {
        return IsInClockwiseRange(startPoint, pointToFind, range);
    }
}

BorderDirection IPMBorder::Invert(BorderDirection direction)
{
    return direction == COUNTER_CLOCKWISE ? CLOCKWISE : COUNTER_CLOCKWISE;
}

bool IPMBorder::IsOnBorder(Point2f p)
{
    return clockwiseMap.containsSecond(p) && counterClockwiseMap.containsSecond(p);
}

int IPMBorder::GetClockwiseDistance(Point2f p1, Point2f p2)
{
    int p1Index = clockwiseMap.getFirst(p1);
    int p2Index = clockwiseMap.getFirst(p2);

    if (p1Index > p2Index)
    {
        p2Index += clockwiseMap.size();
    }

    return p2Index - p1Index;
}

int IPMBorder::GetCounterClockwiseDistance(Point2f p1, Point2f p2)
{
    int p1Index = counterClockwiseMap.getFirst(p1);
    int p2Index = counterClockwiseMap.getFirst(p2);

    if (p1Index > p2Index)
    {
        p2Index += counterClockwiseMap.size();
    }

    return p2Index - p1Index;
}

int IPMBorder::GetMinimalDistance(Point2f p1, Point2f p2)
{
    if (!initialized)
    {
        return -1;
    }

    if (!IsOnBorder(p1) || !IsOnBorder(p2))
    {
        return -1;
    }

    return min(GetClockwiseDistance(p1, p2), GetCounterClockwiseDistance(p1, p2));
}

int IPMBorder::GetDistance(Point2f p1, Point2f p2, BorderDirection direction)
{
    if (!initialized)
    {
        return -1;
    }

    if (!IsOnBorder(p1) || !IsOnBorder(p2))
    {
        return -1;
    }

    if (direction == COUNTER_CLOCKWISE)
    {
        return GetCounterClockwiseDistance(p1, p2);
    }
    else
    {
        return GetClockwiseDistance(p1, p2);
    }
}
