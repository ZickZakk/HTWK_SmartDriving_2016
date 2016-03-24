/**
 * @author pbachmann
 */

#ifndef _HTWK_LINEARFUNCTION_H_
#define _HTWK_LINEARFUNCTION_H_

#include <utils/base/types.h>
#include "opencv2/opencv.hpp"

using namespace cv;

class LinearFunction
{
    public:
        LinearFunction();
        LinearFunction(tFloat32 a, tFloat32 m);

        LinearFunction(Point2f p1, Point2f p2);

        tFloat32 CalculateY(tFloat32 x);

        tFloat32 CalculateX(tFloat32 y);

        tFloat32 GetA();

        tFloat32 GetM();

    private:
        tFloat32 a;
        tFloat32 m;
};

#endif //_HTWK_LINEARFUNCTION_H_
