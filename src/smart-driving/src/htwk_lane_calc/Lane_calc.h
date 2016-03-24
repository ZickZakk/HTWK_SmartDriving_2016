#ifndef _HTWK_LANECALCULATION_H_
#define _HTWK_LANECALCULATION_H_

#include "stdafx.h"
#include "../htwk_logger/Logger.h"
#include "../htwk_lane_angle/LaneAngle.h"

#define OID_LaneCalculation "htwk.lane_calc"
#define FILTER_NAME "HTWK Lane Calculation"

// Properties
#define REACTION_DISTANCE "Reaction Distance"
#define CALCULATION_INTERVAL "Calculation Interval"
#define SCAN_TOLERANCE "Scan Tolerance"

class LaneCalculation : public adtf::cFilter
{
    ADTF_FILTER(OID_LaneCalculation, FILTER_NAME, OBJCAT_DataFilter);

    private: //private members
        cInputPin currentSpeedPin;
        cInputPin timerInterrupt;
        cOutputPin setSteeringAngle;
        Logger logger;

        cObjectPtr<IMediaTypeDescription> descriptionSignal;

        tFloat32 currentSpeed;

        // Filter Properties
        tFloat32 reactionDistance;
        tFloat32 calculationInterval;
        tFloat32 scanTolerance;

        //
        tFloat32 straightFrontDistance;
        tFloat32 rightFrontDistance;
        tFloat32 leftFrontDistance;

        LaneAngleCalculator calculator;

    private: //private functions
        tResult ProcessInput(IMediaSample *mediaSample);

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

    public: //common implementation
        LaneCalculation(const tChar *__info);

        virtual ~LaneCalculation();

    public: // overwrites cFilter //implements IPinEventSink
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

    public: // overwrites cFilter
        tResult Init(tInitStage eStage, __exception = NULL);

        tResult Start(__exception = NULL);

        tResult Stop(__exception = NULL);

        tResult Shutdown(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_LANECALCULATION_H_
