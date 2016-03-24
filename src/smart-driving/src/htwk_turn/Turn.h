/**
 * @author pbachmann
 */

#ifndef _HTWK_PROTO_TURN_H_
#define _HTWK_PROTO_TURN_H_

#include "stdafx.h"

#include <math.h>
#include <map>
#include <climits>
#include <numeric>

#include "opencv2/opencv.hpp"

#include <Logger.h>
#include <DriveUtils.h>
#include <tRoadSign.h>
#include <tLane.h>
#include <BaseDecisionModule.h>
#include <tManeuver.h>
#include <tCarState.h>
#include <WorldModel.h>
#include <MapChecker.h>
#include <VisionUtils.h>
#include <tLineStatus.h>


#define OID "htwk.turn"
#define FILTER_NAME "HTWK Turn"

#define TURN_CONFIG_GROUP "turn"

//Properties
#define TURN_SPEED_PROPERTY "turnSpeed"
#define NEARING_SPEED_PROPERTY "nearingSpeed"
#define LEFT_TURN_ANGLE_PROPERTY "leftTurnAngle"
#define LEFT_TURN_RADIUS_PROPERTY "leftTurnRadius"
#define RIGHT_TURN_ANGLE_PROPERTY "rightTurnAngle"
#define RIGHT_TURN_RADIUS_PROPERTY "rightTurnRadius"
#define SIGN_SIZE_SLOW_DOWN_PROPERTY "signSizeSlowDown"
#define SIGN_SIZE_STOP_PROPERTY "signSizeStop"

static const int TrafficWaitTime = 8000;
static const int ObstacleWaitTime = 1000;
using namespace cv;

class Turn : public BaseDecisionModule
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        struct TurnState
        {
            enum TurnStateEnum
            {
                Idle,
                Nearing,
                Stopping,
                CheckStraight,
                CheckLeft,
                CheckRight,
                TurnLeft,
                TurnRight,
                GoStraight,
                Error,
                Done
            };
        };

    private:
        // pins
        tFloat32 lastYaw;

        // goals
        tFloat32 yawGoal;
        tFloat32 distanceGoal;

        //properties
        tFloat32 turnSpeed;
        tFloat32 nearingSpeed;
        tInt leftTurnAngle;
        tInt rightTurnAngle;
        tFloat32 leftTurnRadius;
        tFloat32 rightTurnRadius;
        tInt signSizeSlowDown;
        tInt signSizeStop;

        // Inner properties
        TurnState::TurnStateEnum currentTurnState;
        int obstacleFreeTime;
        int trafficFreeTime;
        bool markerEvaluatorResetTriggered;
        bool markerDetectionResetTriggered;
        bool laneResetTriggered;
        bool roadSignReseted;
        bool laneReseted;
        tFloat32 lastSignSize;

#ifndef NDEBUG
        cVideoPin debugVideoOutput;
        tBitmapFormat debugVideoFormat;
        Mat debugImage;
#endif



    private:
        void SetState(TurnState::TurnStateEnum newState);

        tResult OnTrigger(tFloat32 interval);

    public:
        Turn(const tChar *__info);

        virtual ~Turn();

        tResult Init(tInitStage eStage, __exception = NULL);

        void TurnState_Idle(tWorldModel worldModel);

        void TurnState_Nearing(tWorldModel model);

        float CalculateRoadSignSpeed(tWorldModel worldModel);

        float CalculateLaneSpeed(tWorldModel worldModel);

        float CalculateLineSpeed(tLine line);

        void TurnState_Stopping(tWorldModel worldModel);

        void SetSignalLights(tWorldModel worldModel);

        void TurnState_CheckLeft(tWorldModel worldModel);

        void TurnState_CheckRight(tWorldModel worldModel);

        void TurnState_CheckStraight(tWorldModel worldModel);

        void TurnState_TurnLeft(tWorldModel worldModel);

        void TurnState_TurnRight(tWorldModel worldModel);

        void TurnState_GoStraight(tWorldModel worldModel);

        void TurnState_Error();

        void TurnState_Done(tWorldModel worldModel);

        tFloat32 NormalizeSignSize(tFloat32 currentSize);

        tBool Traffic(tWorldModel worldModel);

        tBool CheckTrafficLeft(tWorldModel worldModel);

        tBool CheckTrafficRight(tWorldModel worldModel);

        bool checkTrafficOtherLane(tWorldModel worldModel);

        tResult Start(IException **__exception_ptr);

        bool ObstacleInFront(tWorldModel model);
};

#endif // _HTWK_PROTO_TURN_H_
