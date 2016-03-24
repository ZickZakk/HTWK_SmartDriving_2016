/*
@author: gjenschmischek
*/
//#############################################################################
#include "Vision.h"
#include "LaneDetection/InitLine.h"
#include "LaneDetection/DumbLine.h"

//#############################################################################
void LaneDetector::ExtractBorder(Mat image)
{
    int leftUp = 0, leftDown = 0, rightUp = image.cols - 1, rightDown = image.cols - 1;
    int lowerBound = image.rows - 1;

    while (leftUp < image.cols && image.at<tUInt8>(0, leftUp) == 5)
    {
        leftUp++;
    }

    while (leftDown < image.cols && image.at<tUInt8>(lowerBound, leftDown) == 5)
    {
        leftDown++;
    }

    while (rightUp > 0 && image.at<tUInt8>(0, rightUp) == 5)
    {
        rightUp--;
    }

    while (rightDown > 0 && image.at<tUInt8>(lowerBound, rightDown) == 5)
    {
        rightDown--;
    }

    Point2f LeftUp(leftUp + 1, 1);
    Point2f LeftDown(leftDown + 1, lowerBound - 1);
    Point2f RightDown(rightDown - 1, lowerBound - 1);
    Point2f RightUp(rightUp - 1, 1);

    border.Initialize(LeftUp, RightUp, LeftDown, RightDown);
}

Point2f VanishingPointDetector::CalculateVanishingPoint(const Mat &image)
{
    if (!initialized)
    {
        return Point2f(-1, -1);
    }

    if (stabilized)
    {
        return currentVanishingPoint;
    }

    float A = 0.0;
    float B = 0.0;
    float C = 0.0;
    float D = 0.0;
    float E = 0.0;

    // Pre-Filtering
    Mat workingCopy = image.clone();
//    cvtColor(workingCopy, workingCopy, CV_RGB2GRAY);
    Sobel(workingCopy, workingCopy, -1, 1, 0);
    threshold(workingCopy, workingCopy, threshHold, 255, THRESH_BINARY);

    vector<Vec2f> lines;
    vector<Vec2f> filteredLines;
    HoughLines(workingCopy, lines, 2, CV_PI / 60, cvRound(125), 0, 0, CV_PI / 18, CV_PI * 17 / 18);
    remove_copy_if(lines.begin(), lines.end(), std::back_inserter(filteredLines), VisionUtils::IsHorizontal);

    map<float, vector<float> > groupedLines;
    for (size_t i = 0; i < min(filteredLines.size(), maxLines); i++)
    {
        Vec2f line = filteredLines[i];
        float roundedTheta = cvRound(line[1] * 100) / 100.0f;
        groupedLines[roundedTheta].push_back(line[0]);
    }

    int i = 0;
    for (map<float, vector<float> >::iterator it = groupedLines.begin(); it != groupedLines.end(); ++it)
    {
        i++;
        float rho = it->second.front(), theta = it->first;

        float p = exp2f(
                -powf(rho - currentVanishingPoint.x * cos(theta) - currentVanishingPoint.y * sin(theta), 2) / 20000);
        float a = cos(theta);
        float b = sin(theta);
        float H = 10.0f / (i + 1);

        A += p * H * a * a;
        B += p * H * b * b;
        C += p * H * a * b;
        D += p * H * a * rho;
        E += p * H * b * rho;
    }

    Mat L = Mat_<float>(2, 2);
    L.at<float>(0) = A;
    L.at<float>(1) = C;
    L.at<float>(2) = C;
    L.at<float>(3) = B;

    Mat R = Mat_<float>(2, 1);
    R.at<float>(0) = D;
    R.at<float>(1) = E;

    Mat X;

    if (solve(L, R, X, DECOMP_CHOLESKY))
    {
        Point2f newVanishingPoint(X.at<float>(0), X.at<float>(1));

        if (GeneralUtils::Equals(newVanishingPoint, currentVanishingPoint, 5))
        {
            stabilizationCounter++;
            stabilized = stabilizationCounter >= 10;
        }
        else
        {
            stabilizationCounter = 0;
        }
        currentVanishingPoint = newVanishingPoint;
    }

    return currentVanishingPoint;

}

void VanishingPointDetector::Reset(unsigned long maxLines, int threshHold, float VPX, float VPY)
{
    VanishingPointDetector::maxLines = maxLines;
    VanishingPointDetector::threshHold = threshHold;
    VanishingPointDetector::currentVanishingPoint.x = VPX;
    VanishingPointDetector::currentVanishingPoint.y = VPY;
    VanishingPointDetector::stabilized = false;
    VanishingPointDetector::stabilizationCounter = 0;

    VanishingPointDetector::initialized = true;
}

VanishingPointDetector::VanishingPointDetector()
{
    VanishingPointDetector::initialized = false;

    VanishingPointDetector::stabilized = false;
    VanishingPointDetector::stabilizationCounter = 0;
}

bool VanishingPointDetector::IsStabilized()
{
    return stabilized;
}

Mat InversePerspectiveMapper::MapInversePerspectiveFromVanishingPoint(const Mat &image, const Point2f &vanishingPoint)
{
    float abstand = abs(vanishingPoint.y - (image.rows / 2.0f));
    float focalLength = 575;
    float alphaKlein = 23 * CV_PI_F / 180;
    float alphaGross = 29 * CV_PI_F / 180;

    float cameraTilt = atan(abstand / focalLength);

    float remainingHeight = image.rows - cvRound(vanishingPoint.y);

    float xDistance = cvCeil(image.cols / 3.0f);
    float yDistance = cvCeil(remainingHeight / 3.0f);

    pair<vector<cv::Point2f>, vector<cv::Point2f> > sourceDestinationPairs;

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            float x = xDistance + i * xDistance;
            float y = vanishingPoint.y + yDistance + j * yDistance;

            float u = 22 * HTWKMath::cotf((cameraTilt - alphaKlein) + y * (2 * alphaKlein / (image.rows - 1))) *
                      sinf(-alphaGross + x * (2 * alphaGross / (image.cols - 1))) - 3;
            float v = 22 * HTWKMath::cotf((cameraTilt - alphaKlein) + y * (2 * alphaKlein / (image.rows - 1))) *
                      cosf(-alphaGross + x * (2 * alphaGross / (image.cols - 1))) - 50;
            // set from 50 to 55 remove hood for car 20 after crash

            // Moved coordinates to IPM
            u = u + 150;
            v = 200 - v;

            sourceDestinationPairs.first.push_back(Point2f(x, y));
            sourceDestinationPairs.second.push_back(Point2f(u, v));
        }
    }

    Mat lambda = getPerspectiveTransform(sourceDestinationPairs.first, sourceDestinationPairs.second);
    Mat output(200, 300, CV_8UC1, CV_RGB(0, 0, 0));

    warpPerspective(image, output, lambda, output.size(), INTER_LINEAR, BORDER_CONSTANT, CV_RGB(5, 5, 5));

    return output.clone();
}


int ThreshHolder::Yen(const Mat &image, bool ignoreBlack, bool ignoreWhite)
{
    Mat workingImage = image.clone();

    /// Establish the number of bins
    int histSize = 256;
    /// Set the ranges ( for B,G,R) )
    float range[] = {0, 256};
    const float *histRange = {range};
    Mat data;
    calcHist(&workingImage, 1, 0, Mat(), data, 1, &histSize, &histRange);

    //Ignore full Black and White
    if (ignoreBlack)
    {
        data.at<float>(0) = 0.0f;
    }
    if (ignoreWhite)
    {
        data.at<float>(255) = 0.0f;
    }

    int threshold;
    int ih, it;
    float crit;
    float max_crit;
    float norm_histo[histSize]; /* normalized histogram */
    float P1[histSize]; /* cumulative normalized histogram */
    float P1_sq[histSize];
    float P2_sq[histSize];

    int total = 0;
    for (ih = 0; ih < histSize; ih++)
    {
        total += data.at<float>(ih);
    }

    for (ih = 0; ih < histSize; ih++)
    {
        norm_histo[ih] = data.at<float>(ih) / total;
    }

    P1[0] = norm_histo[0];
    for (ih = 1; ih < histSize; ih++)
    {
        P1[ih] = P1[ih - 1] + norm_histo[ih];
    }

    P1_sq[0] = norm_histo[0] * norm_histo[0];
    for (ih = 1; ih < histSize; ih++)
    {
        P1_sq[ih] = P1_sq[ih - 1] + norm_histo[ih] * norm_histo[ih];
    }

    P2_sq[histSize - 1] = 0.0;
    for (ih = histSize - 2; ih >= 0; ih--)
    {
        P2_sq[ih] = P2_sq[ih + 1] + norm_histo[ih + 1] * norm_histo[ih + 1];
    }

    /* Find the threshold that maximizes the criterion */
    threshold = -1;
    max_crit = INT_MIN;
    for (it = 0; it < histSize; it++)
    {
        crit = -1.0f * ((P1_sq[it] * P2_sq[it]) > 0.0f ? log(P1_sq[it] * P2_sq[it]) : 0.0f) +
               2 * ((P1[it] * (1.0f - P1[it])) > 0.0f ? log(P1[it] * (1.0f - P1[it])) : 0.0f);
        if (crit > max_crit)
        {
            max_crit = crit;
            threshold = it;
        }
    }
    return threshold;
}

Mat LaneDetector::MakeBinary(Mat ipmImage)
{
    if (!initialized)
    {
        return Mat::zeros(200, 300, CV_8UC1);
    }

    int thresh = threshHolder.Yen(ipmImage, true, true);
    threshBuffer[threshBufferIndex] = thresh;
    threshBufferIndex = (threshBufferIndex + 1) % threshBufferSize;

    int average_thresh = cvRound(std::accumulate(threshBuffer.begin(), threshBuffer.end(), 0.0f) / threshBuffer.size());

    Mat result;
    threshold(ipmImage, result, average_thresh, 255, THRESH_BINARY);

    return result;
}

LaneDetector::LaneDetector()
{
    BaseLaneLine *left = new DumbLine();
    BaseLaneLine *right = new DumbLine();

    leftLine = Ptr<BaseLaneLine>(left);
    rightLine = Ptr<BaseLaneLine>(right);

    initialized = false;

    laneLogger = new Logger("Lanes", 20);
}

void LaneDetector::Reset(unsigned long threshBufferSize)
{
    LaneDetector::threshBufferSize = threshBufferSize;
    LaneDetector::threshBuffer = vector<int>(threshBufferSize);

    for (unsigned long i = 0; i < threshBufferSize; i++)
    {
        threshBuffer[i] = 255;
    }

    LaneDetector::threshBufferIndex = 0;

    LaneDetector::initialized = true;
    reseted = true;
}

tLane LaneDetector::DetectLanes(Mat &inputImage, Logger &logger)
{
    DebugImage = Mat::zeros(200, 300, CV_8UC3);

    Mat binaryImage = MakeBinary(inputImage);

//    Mat kernel = getStructuringElement(CV_SHAPE_ELLIPSE, Size(5, 5));
//    morphologyEx(workingImage, workingImage, MORPH_CLOSE, kernel);

    if (reseted)
    {
        ExtractBorder(inputImage);

        CrossingDetector leftBot("left_bot.bmp", Point(28, 24), &IsLeft, 0.6);
        CrossingDetector leftTop("left_top.bmp", Point(25, 39), &IsLeft, 0.55);
        BaseLaneLine *left = new InitLine(0, cvRound(binaryImage.cols / 2.0) - 20, border, CLOCKWISE, leftBot, leftTop, laneLogger);

        CrossingDetector rightBot("right_bot.bmp", Point(13, 12), &IsRight, 0.55);
        CrossingDetector rightTop("right_top.bmp", Point(9, 25), &IsRight, 0.55);
        BaseLaneLine *right = new InitLine(cvRound(binaryImage.cols / 2.0) - 20 + 1, binaryImage.cols - 1, border,
                                           COUNTER_CLOCKWISE, rightBot, rightTop, laneLogger);

        leftLine = Ptr<BaseLaneLine>(left);
        rightLine = Ptr<BaseLaneLine>(right);

        reseted = false;

        tLane result = GenerateNormalizedLaneStruct(binaryImage);

        return result;
    }
    leftLine->Log("Left:");
    leftLine = leftLine->UpdateLaneLine(binaryImage, *rightLine);
    leftLine->Log("------");
    rightLine->Log("Right:");
    rightLine = rightLine->UpdateLaneLine(binaryImage, *leftLine);
    rightLine->Log("------");


#ifndef NDEBUG
    cvtColor(binaryImage, DebugImage, CV_GRAY2RGB);

    leftLine->Draw(DebugImage);
    rightLine->Draw(DebugImage);

    line(DebugImage, Point2f(DebugImage.cols / 2.0f - (15 + 8 + 47), 0), Point2f(DebugImage.cols / 2.0f - (15 + 8 + 47), 199), CV_RGB(0, 0, 255));
    line(DebugImage, Point2f(DebugImage.cols / 2.0f + (15 + 8), 0), Point2f(DebugImage.cols / 2.0f + (15 + 8), 199), CV_RGB(0, 0, 255));
#endif

    tLane result = GenerateNormalizedLaneStruct(binaryImage);

    return result;
}

tLine LaneDetector::GenerateNormalizedLineStruct(Ptr<BaseLaneLine> line, const Mat &image)
{
    tLine result;
    result.tStart.tX = line->GetLanePositionAt(image.rows - 1) - image.cols / 2.0f;
    result.tStart.tY = 0;
    result.tEnd.tX = line->End.GetSignPoint().x - image.cols / 2.0f;
    result.tEnd.tY = image.rows - line->End.GetSignPoint().y;
    result.tCrossingDistance = line->GetCrossingDistance();

    switch (line->Mode)
    {
        case FULL:
        case BROKEN:
        case LOST:
            result.tStatus = tVISIBLE;
            break;

        case CROSSING:
            result.tStatus = tCROSSING;
            break;

        default:
            result.tStatus = tINVISIBLE;
    }

    return result;
}

tLane LaneDetector::GenerateNormalizedLaneStruct(const Mat &image)
{
    tLane result;
    result.tLeftLine = GenerateNormalizedLineStruct(leftLine, image);
    result.tRightLine = GenerateNormalizedLineStruct(rightLine, image);

    return result;
}

bool LaneDetector::IsReady()
{
    return leftLine->Mode != INIT && rightLine->Mode != INIT;
}
