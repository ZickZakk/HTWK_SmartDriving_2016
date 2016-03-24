//
// Created by ffreihube on 2/28/16.
//

#ifndef HTWK_2016_PASSING_H
#define HTWK_2016_PASSING_H

#include "stdafx.h"

#include <BaseDecisionModule.h>
#include <DriveModule.h>
#include <WorldModel.h>
#include <tManeuver.h>
#include <MapChecker.h>
#include <tIMU.h>
#include <tLane.h>
#include <GeneralUtils.h>

#define OID "htwk.obstacle.passing"
#define FILTER_NAME "HTWK Obstacle Passing"

#define PASSING_CONFIG_GROUP "passing"

#define ACTIVATION_SPEED_PROPERTY "activationSpeed"
#define WAIT_TIME_PROPERTY "waitTime"
#define MID_ANGLE_PROPERTY "midAngle"
#define DISTANCE_NEXT_TO_OBSTACLE_PROPERTY "distanceNextObstacle"
#define PASSING_SPEED_PROPERTY "passingSpeed"
#define REVERSE_DISTANCE_PROPERTY "reverseSpeed"
#define SIDE_DISTANCE_PROPERTY "sideDistance"
#define FRONT_DISTANCE_PROPERTY "frontDistance"
#define DRIVE_WITH_LANE_PROPERTY "useLane"
#define SAFETY_SPACE_PROPERTY "laneSafetySpace"

#define SWITCH_LEFT 0
#define SWITCH_RIGHT 1

class Passing : public BaseDecisionModule
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        struct ObstaclePassingState
        {
            enum ObstaclePassingStateEnum
            {
                Idle,
                Passive,
                Active_switching_lane_left,
                Active_switching_lane_right,
                Active_next_to_obstacle,
                Finished
            };
        };

    public:
        Passing(const tChar *__info);

        virtual ~Passing();

    private:
        void InitializeProperties();

        tResult OnTrigger(tFloat32 interval);

        void SetState(ObstaclePassingState::ObstaclePassingStateEnum newState);

        tResult Init(tInitStage eStage, IException **__exception_ptr);

        tResult PullWorldModel(tWorldModel &worldModel);

        void StateIdle(tWorldModel model);

        void StatePassive(tWorldModel model);

        void SwitchLane(tWorldModel model, int mode);

        void NextToObstacle(tWorldModel model);

        void Finished();

        tFloat32 CalculateExpectedYaw(tFloat32 yaw, Direction::DirectionEnum direction, tBool reverse, tFloat32 angle);

        void switchOnYaw(tWorldModel model, int mode);

        float CalculateLineAngle(const tLine &line, tFloat32 limit);

    private:
        ObstaclePassingState::ObstaclePassingStateEnum state;

        tFloat32 activationSpeed;

        tFloat32 waitTime;

        tInt32 triggerCounter;

        tFloat32 roadYaw;

        tBool midPassed;

        tFloat32 midAngle;

        tFloat32 startDistance;

        tFloat32 distanceNextToObstacle;

        tFloat32 speedWhilePassing;

        tBool toClose;

        tFloat32 reverseDistance;

        tFloat32 sideDistanceToObstacle;

        tFloat32 detectionDistance;

        tBool driveWithLane;

        tFloat32 safetySpace;

        tBool laneResetTriggered;

        tResult Start(IException **__exception_ptr);

        void Finished(tWorldModel worldModel);
};


#endif //HTWK_2016_PASSING_H
