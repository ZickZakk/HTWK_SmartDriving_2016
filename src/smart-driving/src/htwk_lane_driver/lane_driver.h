#ifndef _HTWK_IPM_FILTER_H_
#define _HTWK_IPM_FILTER_H_

#include "stdafx.h"

#include <BaseDecisionModule.h>
#include <Logger.h>
#include <WorldModel.h>
#include <Vision.h>

#include "opencv2/opencv.hpp"

using namespace cv;

#define OID_LANE_DRIVER "htwk.lane_driver"
#define FILTER_NAME "HTWK Lane Driver"

#define LANE_DRIVER_CONFIG_GROUP "lanedriver"

#define CAR_WIDTH_PROPERTY "carWidth"
#define MAX_END_DISPLACEMENT_PROPERTY "maxEndDisplacement"
#define MAX_START_DISPLACEMENT_PROPERTY "maxStartDisplacement"
#define LANE_WIDTH_PROPERTY "laneWidth"
#define SAFETY_SPACE_PROPERTY "safetySpace"
#define STRAIGHT_SPEED_PROPERTY "straightSpeed"
#define CURVE_SPEED_PROPERTY "curveSpeed"
#define USE_LEFT_PROPERTY "useLeft"
#define USE_RIGHT_PROPERTY "useRight"


class LaneDriver : public BaseDecisionModule
{
    ADTF_FILTER(OID_LANE_DRIVER, FILTER_NAME, OBJCAT_DataFilter);

    private: //private members
        float carWidth;
        float maxEndDisplacement;
        float maxStartDisplacement;
        float laneWidth;
        float safetySpace;
        float curveSpeed;
        float straightSpeed;
        bool useLeft;
        bool useRight;

    private: //private functions
        void ProcessLane(tLane lane);

        tFloat32 CalculateLaneAngle(const tLane &lane);

        tFloat32 CalculateLineAngle(const tLine &line, float limit);

        tFloat32 CalculateLineSpeed(const tLine &line);

        tResult OnTrigger(tFloat32 interval);

    public: //common implementation
        LaneDriver(const tChar *__info);

        virtual ~LaneDriver();

    public: // overwrites cFilter
        virtual tResult Init(tInitStage eStage, __exception = NULL);

        tResult Start(IException **__exception_ptr);
};

#endif // _HTWK_IPM_FILTER_H_
