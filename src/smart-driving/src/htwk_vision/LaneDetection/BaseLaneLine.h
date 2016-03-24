//
// Created by pbachmann on 1/6/16.
//

#ifndef HTWK_2016_BASELANELINE_H
#define HTWK_2016_BASELANELINE_H

static const int MinimumLabelArea = 75;

static const int MaximumLabelArea = 3500;

static const int StandardSearchRadius = 30;

static const int StandardSafetyDistance = StandardSearchRadius * 2 + 1;

static const int VanishRange = 75;

static const int ApproximateFunctionDegrees = 3;

static const int BrokenLineSearchRadius = 125;

static const int LostLineRadiusIncrement = 20;

#include "LaneLineEnd.h"
#include "LaneLineMode.h"
#include "../../htwk_logger/Logger.h"
#include "LaneDetectionData.h"
#include "CrossingDetection/CrossingDetector.h"

class BaseLaneLine
{
    protected:
        vector<double> functionParameters;

        PolynomialFunction currentFunction;

        BorderDirection startToEndDirection;

        IPMBorder border;

        Logger *logger;

        CrossingDetector crossingBotDetector;

        CrossingDetector crossingTopDetector;

        BaseLaneLine();

        virtual void UpdateBounds(const BaseLaneLine &otherLine, LaneDetectionData &data);

        virtual map<int, Point2f> FindNearbyLabels(LaneLineEnd lineEnd, const BaseLaneLine &otherLine,
                                                   LaneDetectionData &data);

        virtual void DrawEndsInColor(Mat &image);

        virtual void DrawEndsAndBoundsInColor(Mat &image);

        virtual Ptr<BaseLaneLine> BaseLaneDetection(const Mat &image, const BaseLaneLine &otherLine);

        virtual Ptr<BaseLaneLine> FindContinuousLabelBetween(const Point2f &startingPoint, const Point2f &endPoint, const Mat &image);

        virtual void DrawFunction(Mat &image, PolynomialFunction function);

        virtual PolynomialFunction ApproximateLane(int label, const Mat &image, LaneDetectionData &data);

        virtual float FindLaneCrossingAt(float height, float startX);


    public:
        LaneLineMode Mode;

        LaneLineEnd Start;
        LaneLineEnd End;

        LaneDetectionData data;

        virtual ~BaseLaneLine();

        BaseLaneLine(const BaseLaneLine &oldLine);

        BaseLaneLine(IPMBorder border, BorderDirection startToEndDirection, CrossingDetector bot, CrossingDetector top, Logger *logger);

        virtual Ptr<BaseLaneLine> UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine) = 0;

        virtual void Draw(Mat &image) = 0;

        virtual float GetLanePositionAt(float height) = 0;

        virtual void Log(cString message);

        virtual int GetCrossingDistance();
};

#endif //HTWK_2016_BASELANELINE_H
