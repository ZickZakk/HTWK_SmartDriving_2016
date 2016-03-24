/**
 * @author pbachmann
 */

#ifndef _HTWK_FUNCTIONDRIVER_H_
#define _HTWK_FUNCTIONDRIVER_H_

#include <float.h>
#include "stdafx.h"
#include "math.h"

#include "../htwk_logger/Logger.h"
#include "../htwk_utils/HTWKMath.h"
#include "../htwk_utils/LinearFunction.h"

// Filter
#define OID_FunctionDriver "htwk.function_driver"
#define FILTER_NAME "HTWK Function Driver"

// Properties
#define SPEED_PROPERTY "Speed"
#define SINLENGTH_PROPERTY "SinLength"
#define SINWIDTH_PROPERTY "SinWidth"

class FunctionDriver : public adtf::cFilter
{
    ADTF_FILTER(OID_FunctionDriver, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cInputPin distanceOverallPin;
        cOutputPin steeringAnglePin;
        cOutputPin speedPin;

        Logger logger;
        cObjectPtr<IMediaTypeDescription> descriptionSignal;

        tFloat32 distanceOverall;
        LinearFunction wayLengthFunction;

        // Properties
        tFloat32 speed;
        tFloat32 sinLength;
        tFloat32 sinWidth;

    private: //private functions
        tResult ProcessInput(IMediaSample *mediaSample);

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

        tFloat32 CalculateSinus(tFloat32 x, tFloat32 t);

        static tFloat32 SinusLengthIntegral(tFloat32 x, tFloat32 param[]);

        tFloat32 CalculateSinusSlope(tFloat32 x, tFloat32 t);

        void SendValue(IMediaSample *mediaSample, cOutputPin &outputPin, tFloat32 value);

    public: //common implementation
        FunctionDriver(const tChar *__info);

        virtual ~FunctionDriver();

    public:
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);

        tResult Start(__exception = NULL);

        tResult Stop(__exception = NULL);

        tResult Shutdown(tInitStage eStage, __exception = NULL);
};


#endif // _HTWK_FUNCTIONDRIVER_H_
