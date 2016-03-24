//
// Created by pbachmann on 12/10/15.
//

#ifndef HTWK_2016_CALCOFFSET_H
#define HTWK_2016_CALCOFFSET_H

#include <stddef.h>

class CalcOffset
{
    private:
        float stack[3];

    public:
        CalcOffset(void);
        ~CalcOffset(void);

        float calculate();
        void stackSample(float &value);
        bool haveEnoughSamples();

};


#endif //HTWK_2016_CALCOFFSET_H
