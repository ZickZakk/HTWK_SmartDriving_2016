//
// Created by pbachmann on 1/6/16.
//

#ifndef HTWK_2016_LOSTLINE_H
#define HTWK_2016_LOSTLINE_H

#include "BaseLaneLine.h"

class LostLine : public BaseLaneLine
{
    private:
        int additionalSearchRadius;

    public:
        LostLine(const BaseLaneLine &oldLine);

        LostLine(const LostLine &oldLine);

        virtual Ptr<BaseLaneLine> UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine);

        virtual void Draw(Mat &image);

        virtual float GetLanePositionAt(float height);

        int GetCrossingDistance();
};

#endif //HTWK_2016_DUMBLINE_H
