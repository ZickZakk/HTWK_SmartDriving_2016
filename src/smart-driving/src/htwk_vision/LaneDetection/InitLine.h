//
// Created by pbachmann on 1/6/16.
//

#ifndef HTWK_2016_INITLINE_H
#define HTWK_2016_INITLINE_H

#include "BaseLaneLine.h"
#include "../../htwk_logger/Logger.h"

class InitLine : public BaseLaneLine
{
    private:
        int leftBoundX;
        int rightBoundX;

    public:
        InitLine(int leftBoundX, int rightBoundX, IPMBorder border, BorderDirection startToEndDirection, CrossingDetector bot, CrossingDetector top,
                 Logger *logger);
        InitLine(int leftBoundX, int rightBoundX, const BaseLaneLine &olderLine);

        virtual Ptr<BaseLaneLine> UpdateLaneLine(const Mat &image, const BaseLaneLine &otherLine);

        virtual void Draw(Mat &image);

        virtual float GetLanePositionAt(float height);

        int GetCrossingDistance();
};

#endif //HTWK_2016_INITLINE_H
