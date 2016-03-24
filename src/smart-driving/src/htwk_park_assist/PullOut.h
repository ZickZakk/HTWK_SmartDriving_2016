//
// Created by pbachmann on 2/11/16.
//

#ifndef HTWK_2016_PULLOUT_H
#define HTWK_2016_PULLOUT_H

#include "ParkingBase.h"

#define OID "htwk.pull_out"
#define FILTER_NAME "HTWK Pull Out"

#define PULLOUT_CONFIG_GROUP "pullout"

#define CROSS_FORWARD_DISTANCE_PROPERTY "cross_ForwardDistance"
#define CROSS_TURN_LEFT_RADIUS_PROPERTY "cross_TurnLeftRadius"
#define CROSS_TURN_LEFT_ANGLE_PROPERTY "cross_TurnLeftAngle"
#define CROSS_TURN_RIGHT_RADIUS_PROPERTY "cross_TurnRightRadius"
#define CROSS_TURN_RIGHT_ANGLE_PROPERTY "cross_TurnRightAngle"

#define PARALLEL_FORWARD_LEFT_RADIUS_PROPERTY "parallel_ForwardLeftRadius"
#define PARALLEL_FORWARD_LEFT_ANGLE_PROPERTY "parallel_ForwardLeftAngle"
#define PARALLEL_FORWARD_RIGHT_RADIUS_PROPERTY "parallel_ForwardRightRadius"
#define PARALLEL_FORWARD_RIGHT_ANGLE_PROPERTY "parallel_ForwardRightAngle"
#define PARALLEL_CAR_REAR_DISTANCE_PROPERTY "parallel_CarRearDistance"

#define IS_CROSS_LEFT_BACKWARD_ACTIVE_PROPERTY "isCrossBackWardLeftActive"
#define SKIP_CROSS_RIGHT_FORWARD_PROPERTY "skipcross_RightForward"

class PullOut : public ParkingBase
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter)

    private:
        struct PullOutState
        {
            enum PullOutStateEnum
            {
                Idle,
                Wait,
                DetectOrientation,
                PullOutCross,
                PullOutParallel,
                Reset,
                WaitForReady,
                ManeuverCompleted,
                Done,
                Error
            };

            static std::string ToString(const PullOutState::PullOutStateEnum &value)
            {
                switch (value)
                {
                    case Idle:
                        return "Idle";
                    case Wait:
                        return "Wait";
                    case DetectOrientation:
                        return "DetectOrientation";
                    case PullOutCross:
                        return "PullOutCross";
                    case PullOutParallel:
                        return "PullOutParallel";
                    case Reset:
                        return "Reset";
                    case ManeuverCompleted:
                        return "ManeuverCompleted";
                    case Done:
                        return "Done";
                    case Error:
                        return "Error";
                    default:
                        return "Unknown Value";
                }
            }
        };

        struct PullOutCrossState
        {
            enum PullOutCrossStateEnum
            {
                Idle,
                ForwardTurnRight,
                TurnLeft,
                TurnRight,
                LeftBackward,
                Done
            };

            static std::string ToString(const PullOutCrossState::PullOutCrossStateEnum &value)
            {
                switch (value)
                {
                    case Idle:
                        return "Idle";
                    case ForwardTurnRight:
                        return "ForwardTurnRight";
                    case TurnLeft:
                        return "TurnLeft";
                    case TurnRight:
                        return "TurnRight";
                    case LeftBackward:
                        return "LeftBackward";
                    case Done:
                        return "Done";
                    default:
                        return "Unknown Value";
                }
            }
        };

        struct PullOutParallelState
        {
            enum PullOutParallelStateEnum
            {
                Idle,
                Backward,
                PullLeft,
                PullStraight,
                Done
            };

            static std::string ToString(const PullOutParallelState::PullOutParallelStateEnum &value)
            {
                switch (value)
                {
                    case Idle:
                        return "Idle";
                    case Backward:
                        return "Backward";
                    case PullLeft:
                        return "PullLeft";
                    case PullStraight:
                        return "PullStraight";
                    case Done:
                        return "Done";
                    default:
                        return "Unknown Value";
                }
            }
        };

    private:
        PullOutState::PullOutStateEnum pullOutState;
        PullOutCrossState::PullOutCrossStateEnum pullOutCrossState;
        PullOutParallelState::PullOutParallelStateEnum pullOutParallelState;

        tFloat32 drivenDistanceCurrentState;
        tFloat32 lastDistanceOverall;

        // detection
        tManeuver::ManeuverEnum previousManeuver;
        tInt detectionRuns;
        tInt undefinedParkingPositionCount;
        tInt crossParkingPositionCount;
        tInt parallelParkingPositionCount;

        // Reset
        tBool laneResetSend;
        tBool markerDetectionResetSend;
        tBool markerEvaluatorResetSend;

        //Properties
        tFloat32 crossForwardDistance;
        tFloat32 crossTurnLeftRadius;
        tInt crossTurnLeftAngle;
        tFloat32 crossTurnRightRadius;
        tInt crossTurnRightAngle;

        tInt parallelForwardLeftAngle;
        tFloat32 parallelForwardLeftRadius;
        tInt parallelForwardRightAngle;
        tFloat32 parallelForwardRightRadius;

        tFloat32 parallelRearCarDistance;

        tBool skipCrossRightForward;
        tBool isCrossBackwardActive;

    private:
        tResult OnTrigger(tFloat32 interval);

        void SetState(const PullOutState::PullOutStateEnum newState);

        void SetState(const PullOutCrossState::PullOutCrossStateEnum newState);

        void SetState(const PullOutParallelState::PullOutParallelStateEnum newState);

        void ResetModule();

        PullOutState::PullOutStateEnum DetectOrientation(const Mat &map);

    private:
        void PullOutState_Idle(const tWorldModel &model);

        void PullOutState_Wait(const tWorldModel &model);

        void PullOutState_DetectOrientation(const tWorldModel &model);

        void PullOutState_PullOutCross(const tWorldModel &model);

        void PullOutState_PullOutParallel(const tWorldModel &model);

        void PullOutState_Reset(const tWorldModel &model);

        void PullOutState_Done(const tWorldModel &model);

        void PullOutState_Error(const tWorldModel &model);

        void PullOutState_ManeuverCompleted(const tWorldModel &model);

    public:
        PullOut(const tChar *__info);

        virtual ~PullOut();

    public:
        tResult Init(tInitStage eStage, IException **__exception_ptr);

        tResult Stop(IException **__exception_ptr);

        tResult Start(IException **__exception_ptr);

        void PullOutState_WaitForReady(tWorldModel model);
};


#endif //HTWK_2016_PULLOUT_H
