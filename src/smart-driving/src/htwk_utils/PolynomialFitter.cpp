//
// Created by pbachmann on 1/1/16.
//

#include "PolynomialFitter.h"
#include "../../lib/lmfit/lmstruct.h"
#include "../../lib/lmfit/lmcurve_tyd.h"

PolynomialFunction PolynomialFitter::PolynomialFit(PolynomialFitterData &data)
{
    degrees = data.Degrees;

    lm_control_struct control = lm_control_float;
    lm_status_struct status;

    double xs[data.Points.size()];
    double ys[data.Points.size()];
    double ws[data.Points.size()];

    for (unsigned int i = 0; i < data.Points.size(); i++)
    {
        xs[i] = data.Points[i].x;
        ys[i] = data.Points[i].y;
        ws[i] = data.Points[i].z;
    }

    lmcurve_tyd(data.Degrees + 1, &data.Parameter[0], data.Points.size(), xs, ys, ws,
                PolynomialFitter::PolynomialCalculation, &control,
                &status);

    Mat X = Mat::zeros(degrees + 1, 1, CV_32F);

    for (int degree = 0; degree <= degrees; degree++)
    {
        X.at<float>(degree, 0) = static_cast<float>(data.Parameter[degree]);
    }

    return PolynomialFunction(X);
}

double PolynomialFitter::PolynomialCalculation(double t, const double *p)
{
    double y = 0;

    for (int degree = 0; degree <= degrees; degree++)
    {
        y += pow(t, degree) * p[degree];
    }

    return y;
}

int PolynomialFitter::degrees = 0;
