#include "stdafx.h"
#include "BirdsEyeViewer.h"

#include "opencv2/opencv.hpp"

#define ROW_SCAN_OFFSET 8

BirdsEyeViewer::BirdsEyeViewer() : screenWidth(320), screenHeight(240)
{
    topSearchArea = 10;
    bottomSearchArea = screenHeight - 20;
    screenCenterX = screenWidth / 2;
    screenCenterY = screenHeight / 2;
}

BirdsEyeViewer::~BirdsEyeViewer()
{ }

cv::Mat &BirdsEyeViewer::process(const cv::Mat &imSrc)
{
    int nrows = imSrc.rows;
    int ncols = imSrc.cols;

    imRes.create(imSrc.rows, imSrc.cols, CV_8U);
    imRes = cv::Scalar(0, 0, 0);

    float worldX, worldZ, worldY;
    int worldXinCentimeter, worldZinCentimeter;

    for (int row = topSearchArea; row <= bottomSearchArea; row += ROW_SCAN_OFFSET)
    {
        const ushort *dataSrc = imSrc.ptr<ushort>(row);
        for (int col = 0; col < screenWidth; ++col)
        {
            coordinateConverter.depthToWorld(col, row, *dataSrc++, worldX, worldY, worldZ);
            worldXinCentimeter = worldX * -100.0f;
            // subtract distance front to camera
            worldZinCentimeter = (worldZ * 100.0f) - 20;
            try
            {
                int y = nrows - worldZinCentimeter;
                int x = screenCenterX + worldXinCentimeter;
                if ((0 < y && y < nrows) && (0 < x && x < ncols))
                {
                    imRes.ptr<uchar>(y)[x] = 0xff;
                }
            }
            catch (cv::Exception &e)
            {
                const char *err_msg = e.what();
                throw err_msg;
            }
        }
    }
    return imRes;
}
