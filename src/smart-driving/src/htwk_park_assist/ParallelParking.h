/**
 * @author pbachmann
 */

#ifndef _HTWK_PARALLEL_PARKING_H_
#define _HTWK_PARALLEL_PARKING_H_

#include <math.h>
#include <climits>

#include "ParkingBase.h"

#define OID "htwk.parallelparking"
#define FILTER_NAME "HTWK Parallel Parking"

#define PARALLEL_PARKING_CONFIG_GROUP "parallelparking"

//Properties
#define STRAIGHT_FORWARD_DISTANCE_PROPERTY "straightForwardDistance"

#define TURNBACKRIGHT_ANGLE_PROPERTY "turnBackRightAngle"
#define TURNBACKRIGHT_RADIUS_PROPERTY "turnBackRightRadius"

#define TURNBACKLEFT_ANGLE_PROPERTY "turnBackLeftAngle"
#define TURNBACKLEFT_RADIUS_PROPERTY "turnBackLeftRadius"
#define TURNBACKLEFT_REAR_DISTANCE_PROPERTY "turnBackLeftRearDistance"

#define TURNFORWARDRIGHT_ANGLE_PROPERTY "turnForwardRightAngle"
#define TURNFORWARDRIGHT_RADIUS_PROPERTY "turnForwardRightRadius"
#define TURNFORWARDRIGHT_FRONT_DISTANCE_PROPERTY "turnForwardRightFrontDistance"

#define BACKWARDINLOT_REAR_DISTANCE "backwardInLotRearDistance"
#define BACKWARDINLOT_DRIVE_DISTANCE "backwardInLotDriveDistance"

#define SIDE_RECOGNITION_DISTANCE_PROPERTY "sideRecognitionDistance"
#define PARKING_LOT_SIZE_PROPERTY "parkingLotSize"

class ParallelParking : public ParkingBase
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        struct ParallelParkingState
        {
            enum ParallelParkingStateEnum
            {
                Idle, // 0
                Driving,
                FirstVehicleDetected,
                ParkingLotDetected,
                SecondVehicleDetected,
                StraightForward, // 5
                TurnBackRight,
                TurnBackLeftIntoLot,
                TurnForwardRightInLot,
                BackwardInLot,
                Stopped, // 10
                Wait,
                ManeuverCompleted,
                Done,
                Error, // 14
            };

            static std::string ToString(const ParallelParkingState::ParallelParkingStateEnum &value)
            {
                switch (value)
                {
                    case Idle:
                        return "Idle";
                    case Driving:
                        return "Driving";
                    case FirstVehicleDetected:
                        return "FirstVehicleDetected";
                    case ParkingLotDetected:
                        return "ParkingLotDetected";
                    case SecondVehicleDetected:
                        return "SecondVehicleDetected";
                    case StraightForward:
                        return "StraightForward";
                    case TurnBackRight:
                        return "TurnBackRight";
                    case TurnBackLeftIntoLot:
                        return "TurnBackLeftIntoLot";
                    case TurnForwardRightInLot:
                        return "TurnForwardRightInLot";
                    case BackwardInLot:
                        return "BackwardInLot";
                    case Stopped:
                        return "Stopped";
                    case Wait:
                        return "Wait";
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

    private:
        tInt carDetectionCount;
        tInt stateDetectionCount;

        // in m
        tFloat32 drivenDistanceCurrentState;
        tFloat32 lastDistanceOverall;

        //properties
        tFloat32 sideRecognitionDistance;
        tFloat32 parkingLotSize;
        tFloat32 straightForwardDistance;

        tInt turnBackRightAngle;
        tFloat32 turnBackRightRadius;

        tInt turnBackLeftAngle;
        tFloat32 turnBackLeftRadius;
        tFloat32 turnBackLeftRearDistance;

        tInt turnForwardRightAngle;
        tFloat32 turnForwardRightRadius;
        tFloat32 turnForwardRightFrontDistance;

        tFloat32 backwardInLotRearDistance;
        tFloat32 backwardInLotDriveDistance;

        ParallelParkingState::ParallelParkingStateEnum state;

    private:
        tResult OnTrigger(tFloat32 interval);

        void ResetModule();

        void SetState(const ParallelParkingState::ParallelParkingStateEnum newState);

        void StopOnObstacle(const tWorldModel &model);

    private:
        void ParallelParkingState_Idle(const tWorldModel &model);

        void ParallelParkingState_Driving(const tWorldModel &model);

        void ParallelParkingState_FirstVehicleDetected(const tWorldModel &model);

        void ParallelParkingState_ParkingLotDetected(const tWorldModel &model);

        void ParallelParkingState_StraightForward(const tWorldModel &model);

        void ParallelParkingState_SecondVehicleDetected(const tWorldModel &model);

        void ParallelParkingState_TurnBackRight(const tWorldModel &model);

        void ParallelParkingState_TurnBackLeft(const tWorldModel &model);

        void ParallelParkingState_TurnForwardRightInLot(const tWorldModel &model);

        void ParallelParkingState_BackwardInLot(const tWorldModel &model);

        void ParallelParkingState_Wait(const tWorldModel &model);

        void ParallelParkingState_Stopped(const tWorldModel &model);

        void ParallelParkingState_ManeuverCompleted(const tWorldModel &model);

        void ParallelParkingState_Done(const tWorldModel &model);

        void ParallelParkingState_Error(const tWorldModel &model);

    public:
        ParallelParking(const tChar *__info);

        virtual ~ParallelParking();

    public:
        tResult Init(tInitStage eStage, IException **__exception_ptr);

        tResult Stop(IException **__exception_ptr);

        tResult Start(IException **__exception_ptr);


};

#endif // _HTWK_PARALLEL_PARKING_H_
