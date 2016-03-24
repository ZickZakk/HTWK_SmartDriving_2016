//
// Created by pbachmann on 12/28/15.
//

#include "PolynomialFunction.h"

PolynomialFunction::PolynomialFunction(Mat m)
{
    if (m.data == NULL)
    {
        PolynomialFunction::m = Mat::zeros(1, 1, CV_32F);
        isOk = false;
    }
    else
    {
        PolynomialFunction::m = m;
        isOk = true;
    }

}

Mat PolynomialFunction::GetM()
{
    return m;
}

bool PolynomialFunction::IsOk()
{
    return isOk;
}

tFloat32 PolynomialFunction::CalculateY(tFloat32 x)
{
    float y = m.at<float>(0, 0);

    for (int degree = 1; degree < m.rows; degree++)
    {
        y += m.at<float>(degree, 0) * powf(x, degree);
    }

    return y;
}

PolynomialFunction::PolynomialFunction(int degrees)
{
    m = Mat::zeros(degrees + 1, 1, CV_32F);

    for (int degree = 0; degree <= degrees; degree++)
    {
        m.at<float>(degree, 0) = 0;
    }
}
