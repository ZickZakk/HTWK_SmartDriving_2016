#include "CalcOffset.h"

CalcOffset::CalcOffset(void)
{
    stack[0] = 0;
    stack[1] = 0;
    stack[2] = 0;
}

CalcOffset::~CalcOffset(void)
{

}

float CalcOffset::calculate()
{
    return (stack[0] + stack[1] + stack[2]) / 3;
}

void CalcOffset::stackSample(float &value)
{
    if(stack[0] == 0) {
        stack[0] = value;
    } else if (stack[1] == 0) {
        stack[1] = value;
    } else if (stack[2] == 0) {
        stack[2] = value;
    }
}

bool CalcOffset::haveEnoughSamples()
{
    if((stack[0] != 0) && (stack[1] != 0) && (stack[2] != 0)) {
        return true;
    } else {
        return false;
    }
}
