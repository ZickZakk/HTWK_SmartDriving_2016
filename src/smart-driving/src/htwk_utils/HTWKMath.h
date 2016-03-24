/**
 * @author pbachmann
 */

#ifndef _HTWK_HTWKMATH_H_
#define _HTWK_HTWKMATH_H_

#include <iostream>
#include <math.h>
#include <utils/base/types.h>
#include <vector>
#include <opencv2/opencv.hpp>

#include "LinearFunction.h"
#include "PolynomialFunction.h"

using namespace std;
using namespace cv;

class HTWKMath
{
    public:
        enum MethodEnum
        {
            SimpsonMethod

            // additional methods can be found here:
            // http://rosettacode.org/wiki/Numerical_integration#C.2B.2B
        };

    public:
        static tFloat32 Integrate(tFloat32 (*f)(tFloat32), tFloat32 a, tFloat32 b, int steps, MethodEnum m);

        static tFloat32 Integrate(tFloat32 (*f)(tFloat32, tFloat32 *), tFloat32 param[], tFloat32 a, tFloat32 b, int steps, MethodEnum m);

        static LinearFunction LinearRegression(std::vector<Point2f> values);

        static double cot(double i);

        static float cotf(float i);

        static float CircleFunction(float radius, float x);


    private:
        static tFloat32 Integrate_Simpson(tFloat32 (*f)(tFloat32), tFloat32 x, tFloat32 h);

        static tFloat32 Integrate_Simpson(tFloat32(*f)(tFloat32, tFloat32 *), tFloat32 param[], tFloat32 x, tFloat32 h);
};

#endif // _HTWK_HTWKMATH_H_
