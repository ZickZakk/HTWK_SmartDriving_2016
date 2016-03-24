//
// Created by pbachmann on 12/23/15.
//

#include "GeneralUtils.h"

bool GeneralUtils::Equals(float f1, float f2, float delta)
{
    return abs(f1 - f2) <= delta;
}

bool GeneralUtils::Equals(Point2f p1, Point2f p2, float delta)
{
    return Equals(p1.x, p2.x, delta) && Equals(p1.y, p2.y, delta);
}

bool GeneralUtils::Smaller(float f1, float f2, float delta)
{
    float difference = f1 - f2;
    return difference < -delta;
}


bool GeneralUtils::Smaller(Point2f p1, Point2f p2, float delta)
{
    if (Smaller(p1.y, p2.y, delta))
    {
        return true;
    }
    else if (Bigger(p1.y, p2.y, delta))
    {
        return false;
    }

    return Smaller(p1.x, p2.x, delta);
}

bool GeneralUtils::Bigger(float f1, float f2, float delta)
{
    float difference = f1 - f2;
    return difference > delta;
}

bool GeneralUtils::IsBetween(float f, float upper, float lower, float delta)
{
    return f < (upper + delta) && f > (lower - delta);
}

string GeneralUtils::GetExePath()
{
    char result[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
    return string(result, ((count - 11) > 0) ? count - 11 : 0);
}
