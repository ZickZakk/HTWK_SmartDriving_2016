//
// Created by pbachmann on 1/6/16.
//

#include "BaseLaneLine.h"

#ifndef HTWK_2016_CORNERLINE_H
#define HTWK_2016_CORNERLINE_H

class CrossingLine : public BaseLaneLine
{
    private:
        Point2f crossingPointTop;
        Point2f crossingPointBot;
        bool botFound;
        bool topFound;
        Mat lastImage;

        TopAcceptor topAcceptor;

    public:
        CrossingLine(const BaseLaneLine &oldLine, Point2f point, const Mat &oldImage);

        CrossingLine(const CrossingLine &oldLine);

        virtual Ptr<BaseLaneLine> UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine);

        virtual void Draw(Mat &image);

        virtual float GetLanePositionAt(float height);

        Point2f FindPointWithX(float x, Point2f startingPoint);

        int GetCrossingDistance();
};

#endif //HTWK_2016_VANISHEDLINE_H
