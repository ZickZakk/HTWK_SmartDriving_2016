//
// Created by pbachmann on 2/12/16.
//

#ifndef HTWK_2016_IDLE_H
#define HTWK_2016_IDLE_H

#include "stdafx.h"

#include <tCarState.h>
#include <BaseDecisionModule.h>

#define OID "htwk.idle"
#define FILTER_NAME "HTWK Idle"

class Idle : public BaseDecisionModule
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter)

    private:
        tResult OnTrigger(tFloat32 interval);

        void TakeControl();

    public:
        Idle(const tChar *__info);

        virtual ~Idle();

    public:
        tResult Init(tInitStage eStage, IException **__exception_ptr);
};


#endif //HTWK_2016_IDLE_H
