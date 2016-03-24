//
// Created by gjenschmischek on 1/1/16.
//

#ifndef HTWK_2016_POLYNOMIALFITTER_H
#define HTWK_2016_POLYNOMIALFITTER_H

#include "PolynomialFunction.h"

using namespace std;

class PolynomialFitterData
{
    public:
        int Degrees;

        vector<Point3f> Points;

        vector<double> Parameter;
};

class PolynomialFitter
{
    private:
        static double PolynomialCalculation(double t, const double *p);

        static int degrees;

    public:
        static PolynomialFunction PolynomialFit(PolynomialFitterData &data);
};


#endif //HTWK_2016_POLYNOMIALFITTER_H
