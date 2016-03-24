//
// Created by pbachmann on 1/6/16.
//

#ifndef HTWK_2016_BROKENLINE_H
#define HTWK_2016_BROKENLINE_H

#include "BaseLaneLine.h"
#include "../../htwk_utils/PolynomialFitter.h"

class BrokenLine : public BaseLaneLine
{
    private:
        int FindBiggestLabel(LaneDetectionData data);

        Point2f FindBorderCrossingWithStats(PolynomialFunction function, LaneLineEnd lineEnd, LaneDetectionData &data);

    public:
        BrokenLine(const BaseLaneLine &oldLine);

        virtual Ptr<BaseLaneLine> UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine);

        virtual void Draw(Mat &image);

        Ptr<BaseLaneLine> BrokenLineUpdate(const Mat &image, const BaseLaneLine &otherLine);

        virtual float GetLanePositionAt(float height);

        int GetCrossingDistance();
};

#endif //HTWK_2016_DUMBLINE_H
