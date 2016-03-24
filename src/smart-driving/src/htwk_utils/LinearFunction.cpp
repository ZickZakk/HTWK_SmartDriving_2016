/**
 * @author pbachmann
 */

#include "LinearFunction.h"

LinearFunction::LinearFunction()
{
}

LinearFunction::LinearFunction(tFloat32 a, tFloat32 m)
{
    this->a = a;
    this->m = m;
}

tFloat32 LinearFunction::CalculateY(tFloat32 x)
{
    return (this->a * x) + this->m;
}

tFloat32 LinearFunction::CalculateX(tFloat32 y)
{
    return (y - this->m) / this->a;
}

tFloat32 LinearFunction::GetA()
{
    return this->a;
}

tFloat32 LinearFunction::GetM()
{
    return this->m;
}

LinearFunction::LinearFunction(Point2f p1, Point2f p2)
{
    m = (p1.y - p2.y) / (p1.x - p2.x);
    a = p1.y - m * p1.x;
}
