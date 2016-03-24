//
// Created by pbachmann on 2/9/16.
//

#ifndef HTWK_2016_TRIGGER_H
#define HTWK_2016_TRIGGER_H

#include "stdafx.h"

#include <math.h>
#include <climits>

#define OID "htwk.trigger"
#define FILTER_NAME "HTWK Trigger"

#define INTERVAL_PROPERTY "Interval"

class Trigger : public adtf::cTimeTriggeredFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_Tool)

    private:
        cOutputPin triggerPin;
        cObjectPtr<IMediaType> signalMediaType;
        cObjectPtr<IMediaTypeDescription> signalDescription;

        tUInt interval;
        tTimeStamp lastTime;

    private:
        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateOutputPins(IException **__exception_ptr);

        tResult TransmitValue(cOutputPin &pin, tFloat32 value);

    public:
        Trigger(const tChar *__info);

        virtual ~Trigger();

        tResult Init(tInitStage eStage, IException **__exception_ptr);

        tResult Shutdown(tInitStage eStage, IException **__exception_ptr);

        tResult Cycle(IException **__exception_ptr);
};


#endif //HTWK_2016_TRIGGER_H
