//
// Created by pbachmann on 1/6/16.
//

#include "BaseLaneLine.h"
#include "VanishedLine.h"
#include "FullLine.h"
#include "LostLine.h"
#include "BrokenLine.h"
#include "CrossingLine.h"

BaseLaneLine::BaseLaneLine(const BaseLaneLine &oldLine) : currentFunction(ApproximateFunctionDegrees)
{
    Start = oldLine.Start;
    End = oldLine.End;
    border = oldLine.border;
    startToEndDirection = oldLine.startToEndDirection;
    Mode = oldLine.Mode;
    functionParameters = oldLine.functionParameters;
    currentFunction = oldLine.currentFunction;
    data = oldLine.data;
    oldLine.data.Stats.copyTo(data.Stats);
    oldLine.data.Labels.copyTo(data.Labels);
    crossingBotDetector = oldLine.crossingBotDetector;
    crossingTopDetector = oldLine.crossingTopDetector;

    logger = oldLine.logger;
}

BaseLaneLine::BaseLaneLine(IPMBorder border, BorderDirection startToEndDirection, CrossingDetector bot, CrossingDetector top, Logger *logger)
        : currentFunction(
        ApproximateFunctionDegrees)
{
    BaseLaneLine::border = border;
    BaseLaneLine::startToEndDirection = startToEndDirection;
    BaseLaneLine::crossingBotDetector = bot;
    BaseLaneLine::crossingTopDetector = top;

    functionParameters = vector<double>();

    for (int degree = 0; degree <= ApproximateFunctionDegrees; degree++)
    {
        functionParameters.push_back(degree * 10.0);
    }

    BaseLaneLine::logger = logger;
}

BaseLaneLine::BaseLaneLine() : currentFunction(ApproximateFunctionDegrees)
{
    functionParameters = vector<double>();

    for (int degree = 0; degree <= ApproximateFunctionDegrees; degree++)
    {
        functionParameters.push_back(degree * 10.0);
    }
}

BaseLaneLine::~BaseLaneLine()
{

}

void BaseLaneLine::UpdateBounds(const BaseLaneLine &otherLine, LaneDetectionData &data)
{
    Point2f innerStartBound = border.GoMany(End.GetSignPoint(), data.SafetyDistance,
                                            IPMBorder::Invert(startToEndDirection));
    Start.SetBound(innerStartBound, startToEndDirection);

    Point2f innerEndBound = border.GoMany(Start.GetSignPoint(), data.SafetyDistance, startToEndDirection);
    End.SetBound(innerEndBound, IPMBorder::Invert(startToEndDirection));

    if (otherLine.Mode == VANISHED)
    {
        Point2f outerStartBound = border.GoMany(End.GetSignPoint(), data.SafetyDistance,
                                                startToEndDirection);
        Start.SetBound(outerStartBound, IPMBorder::Invert(startToEndDirection));

        Point2f outerEndBound = border.GoMany(Start.GetSignPoint(), data.SafetyDistance,
                                              IPMBorder::Invert(startToEndDirection));
        End.SetBound(outerEndBound, startToEndDirection);
    }
    else
    {
        Point2f outerStartBound = border.GoMany(otherLine.Start.GetSignPoint(), data.SafetyDistance,
                                                startToEndDirection);
        Start.SetBound(outerStartBound, IPMBorder::Invert(startToEndDirection));

        Point2f outerEndBound = border.GoMany(otherLine.End.GetSignPoint(), data.SafetyDistance,
                                              IPMBorder::Invert(startToEndDirection));
        End.SetBound(outerEndBound, startToEndDirection);
    }
}

map<int, Point2f> BaseLaneLine::FindNearbyLabels(LaneLineEnd lineEnd, const BaseLaneLine &otherLine,
                                                 LaneDetectionData &data)
{
    map<int, Point2f> possibleLabels;

    Point2f inLineDirection = lineEnd.GetSignPoint();
    Point2f counterLineDirection = lineEnd.GetSignPoint();

    for (int i = 0; i < data.LabelSearchRadius; i++)
    {
        if (VisionUtils::StandardEqualsPoint(inLineDirection, lineEnd.GetBound(startToEndDirection)))
        {
            break;
        }

        int label = data.Labels.at<int>(inLineDirection);

        int area = data.Stats.at<int>(label, CC_STAT_AREA);

        if (label != 0 && area > MinimumLabelArea && area < MaximumLabelArea)
        {
            if (possibleLabels.find(label) == possibleLabels.end())
            {
                possibleLabels.insert(pair<int, Point2f>(label, inLineDirection));
            }
            else
            {
                possibleLabels[label] = inLineDirection;
            }
        }

        inLineDirection = border.Go(inLineDirection, startToEndDirection);
    }

    for (int i = 0; i < data.LabelSearchRadius; i++)
    {
        if (VisionUtils::StandardEqualsPoint(counterLineDirection,
                                             lineEnd.GetBound(IPMBorder::Invert(startToEndDirection))))
        {
            break;
        }

        int label = data.Labels.at<int>(counterLineDirection);

        int area = data.Stats.at<int>(label, CC_STAT_AREA);

        if (label != 0 && area > MinimumLabelArea && area < MaximumLabelArea)
        {
            possibleLabels.insert(pair<int, Point2f>(label, counterLineDirection));
        }

        counterLineDirection = border.Go(counterLineDirection, IPMBorder::Invert(startToEndDirection));
    }

    return possibleLabels;
}

void BaseLaneLine::DrawEndsInColor(Mat &image)
{
    circle(image, Start.GetSignPoint(), 2, CV_RGB(0, 255, 0), -1);
    circle(image, End.GetSignPoint(), 2, CV_RGB(255, 0, 0), -1);
}

void BaseLaneLine::DrawEndsAndBoundsInColor(Mat &image)
{
    circle(image, Start.GetBound(CLOCKWISE), 2, CV_RGB(0, 255, 0), -1);
    circle(image, Start.GetBound(COUNTER_CLOCKWISE), 2, CV_RGB(255, 0, 0), -1);

    DrawEndsInColor(image);
}

Ptr<BaseLaneLine> BaseLaneLine::BaseLaneDetection(const Mat &image, const BaseLaneLine &otherLine)
{
    UpdateBounds(otherLine, data);

    Point2f crossingPoint;
    if (Mode != CROSSING && crossingBotDetector.FindCrossing(image, crossingPoint, *logger))
    {
        logger->Log("Crossing Found!", false);
        logger->Log(cString::Format("Crossing Point: x: %f | y: %f", crossingPoint.x, crossingPoint.y).GetPtr(), false);
        logger->Log(cString::Format("Start Point: x: %f | y: %f", Start.GetSignPoint().x, Start.GetSignPoint().y).GetPtr(), false);
        logger->Log(cString::Format("End Point: x: %f | y: %f", End.GetSignPoint().x, End.GetSignPoint().y).GetPtr(), false);

        if (GeneralUtils::IsBetween(crossingPoint.x, Start.GetSignPoint().x, End.GetSignPoint().x, 5) ||
            GeneralUtils::IsBetween(crossingPoint.x, End.GetSignPoint().x, Start.GetSignPoint().x, 5))
        {
            BaseLaneLine *crossingLine = new CrossingLine(*this, crossingPoint, image);
            return Ptr<BaseLaneLine>(crossingLine);
        }
        logger->Log("No Success!", false);
    }

    if (otherLine.Mode == CROSSING)
    {
        Start.SetSignPoint(border.GoMany(otherLine.Start.GetSignPoint(), 125, startToEndDirection));
        End.SetSignPoint(border.GoMany(otherLine.End.GetSignPoint(), 100,
                                       IPMBorder::Invert(startToEndDirection)));
    }

    if (border.IsInRange(Start.GetSignPoint(), End.GetSignPoint(), VanishRange, startToEndDirection))
    {
        BaseLaneLine *vanishedLine = new VanishedLine(*this);
        return Ptr<BaseLaneLine>(vanishedLine);
    }

    Mat labels, stats, centroids;
    connectedComponentsWithStats(image, labels, stats, centroids, 4);

    labels.copyTo(data.Labels);
    stats.copyTo(data.Stats);

    int startPointLabel = labels.at<int>(Start.GetSignPoint());
    int endPointLabel = labels.at<int>(End.GetSignPoint());


    if (startPointLabel != 0 && startPointLabel == endPointLabel)
    {
        // Nothing changed - lets chill
        data.CurrentLabel = startPointLabel;
        BaseLaneLine *fullLine = new FullLine(*this);
        return Ptr<BaseLaneLine>(fullLine);
    }

    map<int, Point2f> startLabels = FindNearbyLabels(Start, otherLine, data);
    map<int, Point2f> endLabels = FindNearbyLabels(End, otherLine, data);

    data.StartLabels = startLabels;
    data.EndLabels = endLabels;

    if (startLabels.size() == 0 && endLabels.size() == 0)
    {
        //Lane vanished - oh no!
        if (border.IsInRange(Start.GetSignPoint(), End.GetSignPoint(), 400, startToEndDirection) &&
            otherLine.Mode != VANISHED)
        {
            BaseLaneLine *vanishedLine = new VanishedLine(*this);
            return Ptr<BaseLaneLine>(vanishedLine);
        }
        else
        {
            BaseLaneLine *lostLine = new LostLine(*this);
            return Ptr<BaseLaneLine>(lostLine);
        }
    }

    map<int, pair<Point2f, Point2f> > result = GeneralUtils::IntersectMaps<int, Point2f, Point2f>(startLabels,
                                                                                                  endLabels);

    if (result.size() > 0)
    {
        //Start and Endpoint are connected
        Start.SetSignPoint(result.begin()->second.first);
        End.SetSignPoint(result.begin()->second.second);

        currentFunction = ApproximateLane(result.begin()->first, image, data);

        data.CurrentLabel = result.begin()->first;
        BaseLaneLine *fullLine = new FullLine(*this);
        return Ptr<BaseLaneLine>(fullLine);
    }
    else
    {
        BrokenLine *brokenLine = new BrokenLine(*this);
        return brokenLine->BrokenLineUpdate(image, otherLine);
    }
}

Ptr<BaseLaneLine> BaseLaneLine::FindContinuousLabelBetween(const Point2f &startingPoint, const Point2f &endPoint, const Mat &image)
{
    map<int, Point2f> possibleLabels;

    pair<Point2f, int> biggestSignPoint(Point2f(-1, -1), 0);

    // TODO: Refactor this!
    pair<int, pair<int, pair<Point2f, Point2f> > > bestContinuousValue(0, pair<int, pair<Point2f, Point2f> >(0,
                                                                                                             pair<Point2f, Point2f>(Point2f(-1, -1),
                                                                                                                                    Point2f(-1,
                                                                                                                                            -1))));

    int lastLabel = 0;

    int maxSteps = border.GetDistance(startingPoint, endPoint, startToEndDirection);

    for (int step = 0; step < maxSteps; step++)
    {
        Point2f currentPoint = border.GoMany(startingPoint, step, startToEndDirection);

        if (VisionUtils::StandardEqualsPoint(currentPoint, endPoint) || VisionUtils::StandardEqualsPoint(currentPoint, Point2f(-1, -1)))
        {
            break;
        }

        int currentLabel = data.Labels.at<int>(currentPoint);

        if (currentLabel == lastLabel)
        {
            // Don't add label continuously
            continue;
        }

        lastLabel = currentLabel;

        int area = data.Stats.at<int>(currentLabel, CC_STAT_AREA);
        if (currentLabel == 0 || (area <= 50) ||
            (area > 1000) ||
            (data.Stats.at<int>(currentLabel, CC_STAT_HEIGHT) <= 100))
        {
            // Forget background and smaller labels
            continue;
        }

        if (biggestSignPoint.second < area)
        {
            biggestSignPoint = pair<Point2f, int>(currentPoint, area);
        }

        if (possibleLabels.find(currentLabel) == possibleLabels.end())
        {
            // First label occurrence
            possibleLabels.insert(pair<int, Point2f>(currentLabel, currentPoint));
            continue;
        }

        pair<int, Point2f> entry = *possibleLabels.find(currentLabel);

        int distance = border.GetDistance(entry.second, currentPoint, startToEndDirection);

        if (distance < VanishRange)
        {
            // Distance is too small
            continue;
        }

        if (distance > bestContinuousValue.first)
        {
            bestContinuousValue = pair<int, pair<int, pair<Point2f, Point2f> > >(distance, pair<int, pair<Point2f, Point2f> >(currentLabel,
                                                                                                                              pair<Point2f, Point2f>(
                                                                                                                                      entry.second,
                                                                                                                                      currentPoint)));
        }
    }

    if (bestContinuousValue.first == 0)
    {
        if (possibleLabels.size() == 0)
        {
            BaseLaneLine *vanishedLine = new VanishedLine(*this);
            return Ptr<BaseLaneLine>(vanishedLine);
        }
        else
        {
            if (biggestSignPoint.first.y > 100)
            {
                Start.SetSignPoint(biggestSignPoint.first);
            }
            else
            {
                End.SetSignPoint(biggestSignPoint.first);
            }

            data.CurrentLabel = data.Labels.at<int>(biggestSignPoint.first);
            BaseLaneLine *brokenLine = new BrokenLine(*this);
            return Ptr<BaseLaneLine>(brokenLine);
        }
    }

    Start.SetSignPoint(bestContinuousValue.second.second.first);
    End.SetSignPoint(bestContinuousValue.second.second.second);

    currentFunction = ApproximateLane(bestContinuousValue.second.first, image, data);

    data.CurrentLabel = bestContinuousValue.second.first;
    BaseLaneLine *fullLine = new FullLine(*this);
    return Ptr<BaseLaneLine>(fullLine);
}

PolynomialFunction BaseLaneLine::ApproximateLane(int label, const Mat &image, LaneDetectionData &data)
{
    Mat workingImage = image.clone();

    int height = data.Stats.at<int>(label, CC_STAT_HEIGHT);
    int width = data.Stats.at<int>(label, CC_STAT_WIDTH);

    int minX = data.Stats.at<int>(label, CC_STAT_LEFT);
    int minY = data.Stats.at<int>(label, CC_STAT_TOP);

    Mat roi(workingImage, Rect(minX, minY, width, height));
    Mat workingRoi = roi.clone();

    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    /// Find contours
    findContours(workingRoi, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_TC89_L1, Point(0, 0));

    vector<Point> maxContour;

    if (contours.size() > 0)
    {
        maxContour = *max_element(contours.begin(), contours.end(), VisionUtils::ContourCompare);
    }
    else
    {
        maxContour.push_back(Start.GetSignPoint());
        maxContour.push_back(End.GetSignPoint());
    }

    vector<Point3f> contour(maxContour.size());

    for (unsigned int i = 0; i < maxContour.size(); i++)
    {
        contour[i].x = minX + maxContour[i].x;
        contour[i].y = minY + maxContour[i].y;
        contour[i].z = 0.125;
    }

    contour.push_back(Point3f(Start.GetSignPoint().x, Start.GetSignPoint().y, 2));
    contour.push_back(Point3f(End.GetSignPoint().x, End.GetSignPoint().y, 2));
    contour.push_back(Point3f(Start.GetSignPoint().x, 199, 2));

    PolynomialFitterData fitterData;
    fitterData.Degrees = ApproximateFunctionDegrees;
    fitterData.Points = contour;
    fitterData.Parameter = functionParameters;

    PolynomialFunction func = PolynomialFitter::PolynomialFit(fitterData);

    functionParameters = fitterData.Parameter;

    currentFunction = func;

    return func;
}

void BaseLaneLine::DrawFunction(Mat &image, PolynomialFunction function)
{
    for (int x = 0; x < image.cols; x++)
    {
        float y = function.CalculateY(x);

        Point2f currentPoint(x, y);

        circle(image, currentPoint, 2, CV_RGB(255, 128, 0), -1);
    }
}

float BaseLaneLine::FindLaneCrossingAt(float height, float startX)
{
    map<float, float> results;

    for (int i = 0; i < 35; i++)
    {
        if (startX - i > 0)
        {
            results.insert(pair<float, float>(abs(currentFunction.CalculateY(startX - i) - height), startX - i));
        }

        if (startX + i < 300)
        {
            results.insert(pair<float, float>(abs(currentFunction.CalculateY(startX + i) - height), startX + i));
        }
    }

    return results.begin()->second;
}

void BaseLaneLine::Log(cString message)
{
    logger->Log(message.GetPtr(), false);
}

int BaseLaneLine::GetCrossingDistance()
{
    return 200;
}
