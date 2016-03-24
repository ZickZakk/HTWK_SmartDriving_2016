/**
 * @author pbachmann
 */

#include "HTWKMath.h"

/**
 * Integrates the given function f from a to b using Method m
 * @param f: static function pointer used to get integral values
 * @param a: lower boundary
 * @param b: upper boundary
 * @param steps: steps used to calculate the result, use more to get a better result
 * @param m: calculation method
 */
tFloat32 HTWKMath::Integrate(tFloat32 (*f)(tFloat32), tFloat32 a, tFloat32 b, int steps, MethodEnum m)
{
    tFloat32 s = 0;
    tFloat32 h = (b - a) / steps;

    tFloat32 (*method)(tFloat32 (*f)(tFloat32), tFloat32 x, tFloat32 h);
    switch (m)
    {
        case SimpsonMethod:
            method = Integrate_Simpson;
    }

    for (int i = 0; i < steps; ++i)
    {
        s += method(f, a + h * i, h);
    }

    return h * s;
}

/**
 * Integrates the given function f from a to b using Method m
 * @param f: static function pointer used to get integral values
 * @param a: lower boundary
 * @param b: upper boundary
 * @param steps: steps used to calculate the result, use more to get a better result
 * @param m: calculation method
 */
tFloat32 HTWKMath::Integrate(tFloat32 (*f)(tFloat32, tFloat32 *), tFloat32 param[], tFloat32 a, tFloat32 b, int steps,
                             MethodEnum m)
{
    tFloat32 s = 0;
    tFloat32 h = (b - a) / steps;

    tFloat32 (*method)(tFloat32 (*f)(tFloat32, tFloat32 *), tFloat32 param[], tFloat32 x, tFloat32 h);
    switch (m)
    {
        case SimpsonMethod:
            method = Integrate_Simpson;
    }

    for (int i = 0; i < steps; ++i)
    {
        s += method(f, param, a + h * i, h);
    }

    return h * s;
}

tFloat32 HTWKMath::Integrate_Simpson(tFloat32 (*f)(tFloat32), tFloat32 x, tFloat32 h)
{
    return (f(x) + 4 * f(x + h / 2) + f(x + h)) / 6;
}

tFloat32 HTWKMath::Integrate_Simpson(tFloat32 (*f)(tFloat32, tFloat32 *), tFloat32 param[], tFloat32 x, tFloat32 h)
{
    return (f(x, param) + 4 * f(x + h / 2, param) + f(x + h, param)) / 6;
}

LinearFunction HTWKMath::LinearRegression(std::vector<Point2f> values)
{
    tFloat32 x_sum = 0;
    tFloat32 y_sum = 0;

    for (unsigned int i = 0; i < values.size(); ++i)
    {
        x_sum += values.at(i).x;
        y_sum += values.at(i).y;
    }

    x_sum = x_sum / values.size();
    y_sum = y_sum / values.size();

    tFloat32 xy_star_sum = 0;
    tFloat32 x_star_sum = 0;

    for (unsigned int i = 0; i < values.size(); ++i)
    {
        xy_star_sum += (values.at(i).x - x_sum) * (values.at(i).y - y_sum);
        x_star_sum += pow((values.at(i).x - x_sum), 2);
    }

    tFloat32 a = xy_star_sum / x_star_sum;
    tFloat32 m = y_sum - (a * x_sum);

    return LinearFunction(a, m);
}

double HTWKMath::cot(double i)
{ return (1 / tan(i)); }

float HTWKMath::cotf(float i)
{ return (1 / tanf(i)); }

float HTWKMath::CircleFunction(float radius, float x)
{
    return sqrtf(powf(radius, 2) - powf(x - radius, 2));
}
