//
// Created by pbachmann on 12/28/15.
//

#ifndef HTWK_2016_POLYNOMIALFUNCTION_H
#define HTWK_2016_POLYNOMIALFUNCTION_H

#include <utils/base/types.h>
#include "opencv2/opencv.hpp"

using namespace cv;

class PolynomialFunction
{
    public:
        PolynomialFunction(int degrees);

        PolynomialFunction(Mat m);

        tFloat32 CalculateY(tFloat32 x);

        Mat GetM();

        bool IsOk();

    private:
        Mat m;

        bool isOk;
};

#endif //HTWK_2016_POLYNOMIALFUNCTION_H