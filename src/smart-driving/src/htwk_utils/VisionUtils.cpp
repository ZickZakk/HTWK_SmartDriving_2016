//
// Created by pbachmann on 1/7/16.
//

#include "VisionUtils.h"
#include "../htwk_structs/tLine.h"
#include "../htwk_structs/tLane.h"

bool VisionUtils::ContourCompare(vector<Point> vec1, vector<Point> vec2)
{
    return (contourArea(vec1) < contourArea(vec2));
}

Mat VisionUtils::ExtractImageFromMediaSample(IMediaSample *mediaSample, const tBitmapFormat &inputFormat)
{
    int depth = 0;
    int channel = 0;
    int matType = GetMatType(inputFormat.nBitsPerPixel);

    switch (matType)
    {
        case (CV_8UC3):
            depth = IPL_DEPTH_8U;
            channel = 3;
            break;
        case (CV_16UC1):
            depth = IPL_DEPTH_16U;
            channel = 1;
            break;
        case (CV_8UC1):
            depth = IPL_DEPTH_8U;
            channel = 1;
            break;
        default:
            return Mat();
    }

    const tVoid *buffer;

    if (IS_OK(mediaSample->Lock(&buffer)))
    {
        IplImage *imageBuffer = cvCreateImageHeader(cvSize(inputFormat.nWidth, inputFormat.nHeight), depth, channel);
        imageBuffer->imageData = (char *) buffer;
        Mat image(cvarrToMat(imageBuffer));
        cvReleaseImage(&imageBuffer);
        mediaSample->Unlock(buffer);

        return image;
    }

    return Mat();
}

bool VisionUtils::IsHorizontal(Vec<float, 2> houghLine)
{
    return (houghLine[1] >= (CV_PI * 8 / 18)) && (houghLine[1] <= (CV_PI * 10 / 18));
}


int VisionUtils::GetMatType(tInt16 bitsPerPixel)
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

bool VisionUtils::StandardSmallerPoint(Point2f p1, Point2f p2)
{
    return GeneralUtils::Smaller(p1, p2, StandardPointDelta);
}

bool VisionUtils::StandardEqualsPoint(Point2f p1, Point2f p2)
{
    return GeneralUtils::Equals(p1, p2, StandardPointDelta);
}

bool VisionUtils::StandardSmaller(float f1, float f2)
{
    return GeneralUtils::Smaller(f1, f2, StandardPointDelta);
}

bool VisionUtils::StandardBigger(float f1, float f2)
{
    return GeneralUtils::Bigger(f1, f2, StandardPointDelta);
}

bool VisionUtils::StandardEquals(float f1, float f2)
{
    return GeneralUtils::Equals(f1, f2, StandardPointDelta);;
}

Point2f VisionUtils::WorldToImage(Point2f world)
{
    return Point2f(world.x + 200, 200 - world.y);
}

Point VisionUtils::WorldToImage(tPoint world)
{
    return Point(world.tX + 200, 200 - world.tY);
}

tLine VisionUtils::GenerateDefaultLine()
{
    tLine defaultLine;
    tPoint defaultPoint;

    defaultPoint.tX = 0;
    defaultPoint.tY = 0;

    defaultLine.tStart = defaultPoint;
    defaultLine.tEnd = defaultPoint;
    defaultLine.tStatus = tINVISIBLE;
    defaultLine.tCrossingDistance = 200;

    return defaultLine;
}

tLane VisionUtils::GenerateDefaultLane()
{
    tLane defaultLane;

    defaultLane.tRightLine = GenerateDefaultLine();
    defaultLane.tLeftLine = GenerateDefaultLine();

    return defaultLane;
}
