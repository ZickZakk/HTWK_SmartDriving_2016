//
// Created by pbachmann on 1/6/16.
//

#include "BaseLaneLine.h"

#ifndef HTWK_2016_VANISHEDLINE_H
#define HTWK_2016_VANISHEDLINE_H

class VanishedLine : public BaseLaneLine
{
    public:
        VanishedLine(const BaseLaneLine &oldLine);

        virtual Ptr<BaseLaneLine> UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine);

        virtual void Draw(Mat &image);

        float GetLanePositionAt(float height);

        int GetCrossingDistance();
};

#endif //HTWK_2016_VANISHEDLINE_H
