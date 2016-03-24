//
// Created by pbachmann on 1/6/16.
//

#ifndef HTWK_2016_DUMBLINE_H
#define HTWK_2016_DUMBLINE_H

#include "BaseLaneLine.h"

class DumbLine : public BaseLaneLine
{
    public:
        DumbLine();

        virtual Ptr<BaseLaneLine> UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine);

        virtual void Draw(Mat &image);

        virtual float GetLanePositionAt(float height);

        int GetCrossingDistance();
};

#endif //HTWK_2016_DUMBLINE_H
