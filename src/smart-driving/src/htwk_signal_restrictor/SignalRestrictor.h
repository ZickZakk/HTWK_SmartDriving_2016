/**
 * @author pbachmann
 */

#ifndef _HTWK_SIGNAL_RESTRICTOR_H_
#define _HTWK_SIGNAL_RESTRICTOR_H_

#include "stdafx.h"

#include <math.h>
#include <climits>

#include "../htwk_logger/Logger.h"

#define OID_SIGNALRESTRICTOR "htwk.signalrestrictor"
#define FILTER_NAME "HTWK Signal Restrictor"

class SignalRestrictor : public adtf::cFilter
{
    ADTF_FILTER(OID_SIGNALRESTRICTOR, FILTER_NAME, OBJCAT_DataFilter);

    private:
        Logger logger;

        cInputPin boolInputPin;
        cInputPin floatInputPin;
        cOutputPin boolOutputPin;
        cOutputPin floatOutputPin;

        cObjectPtr<IMediaType> typeSignalValue;
        cObjectPtr<IMediaType> typeBoolSignalValue;

        cObjectPtr<IMediaTypeDescription> descriptionSignalFloat;
        cObjectPtr<IMediaTypeDescription> descriptionSignalBool;

        // values
        tFloat32 floatValue;
        tBool boolValue;

    private:
        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins();

        tResult CreateOutputPins(__exception = NULL);

        tResult TransmitBoolValue(cOutputPin *outputPin, tBool value);

        tResult TransmitFloatValue(cOutputPin *outputPin, tFloat32 value);

    public:
        SignalRestrictor(const tChar *__info);

        virtual ~SignalRestrictor();

    public:
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_SIGNAL_RESTRICTOR_H_
