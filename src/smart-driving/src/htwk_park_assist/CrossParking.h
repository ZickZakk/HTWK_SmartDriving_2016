/**
 * @author pbachmann
 */

#ifndef _HTWK_CROSS_PARKING_H_
#define _HTWK_CROSS_PARKING_H_

#include <math.h>
#include <climits>

#include "ParkingBase.h"

#define OID "htwk.crossparking"
#define FILTER_NAME "HTWK Cross Parking"

#define CROSS_PARKING_CONFIG_GROUP "crossparking"

//Properties
#define STRAIGHT_BACKWARD_DISTANCE_PROPERTY "straightBackwardDistance"

#define SWING_ANGLE_PROPERTY "swingAngle"
#define SWING_RADIUS_PROPERTY "swingRadius"

#define TURN_ANGLE_PROPERTY "turnAngle"
#define TURN_RADIUS_PROPERTY "turnRadius"

#define PARKING_DISTANCE_PROPERTY "parkingDistance"

#define REAR_FREE_DISTANCE_PARKING_PROPERTY "rearFreeDistanceParking"

#define SIDE_RECOGNITION_DISTANCE_PROPERTY "sideRecognitionDistance"
#define PARKING_LOT_SIZE_PROPERTY "parkingLotSize"

class CrossParking : public ParkingBase
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        struct CrossParkingState
        {
            enum CrossParkingStateEnum
            {
                Idle, // 0
                Driving,
                FirstVehicleDetected,
                ParkingLotDetected,
                SecondVehicleDetected,
                StraightBackward, // 5
                Swing,
                Turning,
                Parking,
                Stopped,
                Wait, // 10
                ManeuverCompleted,
                Done,
                Error,
            };

            static std::string ToString(const CrossParkingState::CrossParkingStateEnum &value)
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
                    case StraightBackward:
                        return "StraightBackward";
                    case Swing:
                        return "Swing";
                    case Turning:
                        return "Turning";
                    case Parking:
                        return "Parking";
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

        // in cm
        tFloat32 drivenDistanceCurrentState;
        tFloat32 lastDistanceOverall;

        //properties
        tFloat32 sideRecognitionDistance;
        tFloat32 parkingLotSize;
        tFloat32 straightBackwardDistance;
        tInt swingAngle;
        tFloat32 swingCurveRadius;
        tInt turnAngle;
        tFloat32 turnCurveRadius;
        tFloat32 parkingDistance;
        tFloat32 rearFreeDistanceParking;

        CrossParkingState::CrossParkingStateEnum state;

    private:
        void ResetModule();

        tResult OnTrigger(tFloat32 interval);

        void SetState(const CrossParkingState::CrossParkingStateEnum newState);

        void StopOnObstacle(const tWorldModel &model);

    private:
        void CrossParkingState_Idle(const tWorldModel &model);

        void CrossParkingState_Driving(const tWorldModel &model);

        void CrossParkingState_FirstVehicleDetected(const tWorldModel &model);

        void CrossParkingState_ParkingLotDetected(const tWorldModel &model);

        void CrossParkingState_SecondVehicleDetected(const tWorldModel &model);

        void CrossParkingState_StraightBackward(const tWorldModel &model);

        void CrossParkingState_Swing(const tWorldModel &model);

        void CrossParkingState_Turning(const tWorldModel &model);

        void CrossParkingState_Parking(const tWorldModel &model);

        void CrossParkingState_Stopped(const tWorldModel &model);

        void CrossParkingState_Wait(const tWorldModel &model);

        void CrossParkingState_ManeuverCompleted(const tWorldModel &model);

        void CrossParkingState_Done(const tWorldModel &model);

        void CrossParkingState_Error(const tWorldModel &model);

    public:
        CrossParking(const tChar *__info);

        virtual ~CrossParking();

    public:
        tResult Init(tInitStage eStage, IException **__exception_ptr);

        tResult Stop(IException **__exception_ptr);

        tResult Start(IException **__exception_ptr);
};

#endif // _HTWK_CROSS_PARKING_H_
