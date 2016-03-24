//
// Created by pbachmann on 2/28/16.
//

#include "Passing.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, Passing)

Passing::Passing(const tChar *__info) : BaseDecisionModule(__info, FILTER_NAME, DM_PASSING)
{
    state = ObstaclePassingState::Idle;

    roadYaw = 0;
    startDistance = 0;
    triggerCounter = 0;
    midPassed = tFalse;
    toClose = tFalse;
    laneResetTriggered = tFalse;
}

Passing::~Passing()
{
}

tResult Passing::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(BaseDecisionModule::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
    }
    else if (eStage == StageNormal)
    {
    }
    else if (eStage == StageGraphReady)
    {
    }

    RETURN_NOERROR;
}

tResult Passing::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Start(__exception_ptr));

    activationSpeed = 0.2;
    waitTime = 2;
    midAngle = 41; // Yaw change at which the car turns again
    distanceNextToObstacle = 0.1; // Distance the car drives next to obstacle in m
    speedWhilePassing = 1.6;
    reverseDistance = 0.25; // Distance used to decide if the care need to drive back to pass
    sideDistanceToObstacle = 0.3; // Distance to recognize a obstacle next to car in m
    detectionDistance = 0.6; // Distance to recognize a obstacle ahead of car in m
    driveWithLane = false; // Use lane detection
    safetySpace = 0; // Safety Space between outer lane line and car in cm

    CarConfigReader config;
    worldService->Pull(WORLD_CARCONFIG, config);

    logger.Log("Reading properties", false);

    ReadConfigValue(config, PASSING_CONFIG_GROUP, ACTIVATION_SPEED_PROPERTY, activationSpeed);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, WAIT_TIME_PROPERTY, waitTime);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, MID_ANGLE_PROPERTY, midAngle);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, DISTANCE_NEXT_TO_OBSTACLE_PROPERTY, distanceNextToObstacle);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, PASSING_SPEED_PROPERTY, speedWhilePassing);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, REVERSE_DISTANCE_PROPERTY, reverseDistance);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, SIDE_DISTANCE_PROPERTY, sideDistanceToObstacle);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, FRONT_DISTANCE_PROPERTY, detectionDistance);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, DRIVE_WITH_LANE_PROPERTY, driveWithLane);
    ReadConfigValue(config, PASSING_CONFIG_GROUP, SAFETY_SPACE_PROPERTY, safetySpace);

    RETURN_NOERROR;
}

tResult Passing::OnTrigger(tFloat32 interval)
{
    logger.Log(cString::Format("Current state %d", state).GetPtr());

    tWorldModel worldModel;

    RETURN_IF_FAILED(PullWorldModel(worldModel));

    logger.Log(cString::Format("Left line Start X %f", worldModel.Lane.tLeftLine.tStart.tX).GetPtr());
    logger.Log(cString::Format("Left line End X %f", worldModel.Lane.tLeftLine.tEnd.tX).GetPtr());
    logger.Log(cString::Format("Left line End Y %f", worldModel.Lane.tLeftLine.tEnd.tY).GetPtr());


    if (worldModel.CarState != tCarState::Running)
    {
        ResetDriveInstructions(sourceModule);
        Passing::SetState(ObstaclePassingState::Idle);

        RETURN_NOERROR;
    }

    if ((worldModel.Maneuver == tManeuver::M_PARK_CROSS
         || worldModel.Maneuver == tManeuver::M_PARK_PARALLEL
         || worldModel.Maneuver == tManeuver::M_PULL_OUT_LEFT
         || worldModel.Maneuver == tManeuver::M_PULL_OUT_RIGHT))
    {
        ResetDriveInstructions(sourceModule);
        Passing::SetState(ObstaclePassingState::Idle);

        RETURN_NOERROR;
    }

    switch (state)
    {
        case ObstaclePassingState::Idle:
            StateIdle(worldModel);
            break;
        case ObstaclePassingState::Passive:
            StatePassive(worldModel);
            break;
        case ObstaclePassingState::Active_switching_lane_left:
            SwitchLane(worldModel, SWITCH_LEFT);
            break;
        case ObstaclePassingState::Active_next_to_obstacle:
            NextToObstacle(worldModel);
            break;
        case ObstaclePassingState::Active_switching_lane_right:
            SwitchLane(worldModel, SWITCH_RIGHT);
            break;
        case ObstaclePassingState::Finished:
            Finished(worldModel);
            break;
    }

    RETURN_NOERROR;

}

tResult Passing::PullWorldModel(tWorldModel &worldModel)
{
    RETURN_IF_FAILED(worldService->Pull<tManeuver::ManeuverEnum>(WORLD_CURRENT_MANEUVER, worldModel.Maneuver));
    RETURN_IF_FAILED(worldService->Pull<tRoadSign::RoadSignEnum>(WORLD_CURRENT_ROAD_SIGN, worldModel.RoadSign));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_CURRENT_ROAD_SIGN_SIZE, worldModel.RoadSignSize));
    RETURN_IF_FAILED(worldService->Pull<tIMU>(WORLD_IMU, worldModel.Imu));
    RETURN_IF_FAILED(worldService->Pull<Mat>(WORLD_OBSTACLE_MAT, worldModel.ObstacleMap));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_DISTANCE_OVERALL, worldModel.DistanceOverall));
    RETURN_IF_FAILED(worldService->Pull<tBool>(WORLD_IS_NO_PASSING_ACTIVE, worldModel.IsNoPassing));
    RETURN_IF_FAILED(worldService->Pull<tLane>(WORLD_LANE, worldModel.Lane));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_CAR_SPEED, worldModel.CurrentSpeed));
    RETURN_IF_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, worldModel.CarState));
    RETURN_IF_FAILED(worldService->Pull<vector<tReadyModule::ReadyModuleEnum> >(WORLD_READY_MODULES, worldModel.ReadyModules));

    RETURN_NOERROR;
}

void Passing::SetState(ObstaclePassingState::ObstaclePassingStateEnum newState)
{
    logger.Log(cString::Format("Switching to state %d", newState).GetPtr(), tFalse);
    state = newState;
}

void Passing::StateIdle(tWorldModel model)
{
    ResetDriveInstructions(sourceModule);

    logger.Log(cString::Format("Current Speed %f", model.CurrentSpeed).GetPtr());

    if (model.CurrentSpeed > 0.2)
    {
        SetState(ObstaclePassingState::Passive);
    }

}

void Passing::StatePassive(tWorldModel model)
{
    ResetDriveInstructions(sourceModule);

    if (toClose)
    {
        if (MapChecker::GetFreeFrontCenterDistance(model.ObstacleMap) < reverseDistance)
        {
            logger.Log(cString::Format("Too close to obstacle. Reverse!").GetPtr(), tFalse);

            if (MapChecker::GetFreeRearCenterDistance(model.ObstacleMap) > reverseDistance)
            {
                speedControlTaken = tTrue;
                steeringControlTaken = tTrue;
                curveAngle = 90;
                curveRadius = 0;
                reverseLightEnabled = tTrue;
                speed = tFloat32(-speedWhilePassing);
                return;
            }
            else
            {
                logger.Log(cString::Format("Rear blocked by obstacle as well.").GetPtr(), tFalse);
            }
        }
        else
        {
            toClose = tFalse;
        }
    }


    if (!model.IsNoPassing && model.Lane.tLeftLine.tStatus != tCROSSING)
    {
        logger.Log(cString::Format("Trigger counter %d", triggerCounter).GetPtr(), tFalse);
        logger.Log(cString::Format("Current Speed %f", model.CurrentSpeed).GetPtr(), tFalse);
        logger.Log(cString::Format("Space infront of car  %f", MapChecker::GetFreeFrontCenterDistance(model.ObstacleMap)).GetPtr(), tFalse);

        if (model.CurrentSpeed < 0.05 && model.CurrentSpeed > -0.05 && MapChecker::GetFreeFrontCenterDistance(model.ObstacleMap) < detectionDistance)
        {
            triggerCounter = triggerCounter + 1;
            roadYaw = model.Imu.tYaw;
        }
        else if (model.CurrentSpeed > 0)
        {
            triggerCounter = 0;
        }

        if (triggerCounter > (15 * waitTime)) // Trigger <->  ms
        {
            logger.Log(cString::Format("Front free  %f", MapChecker::GetFreeFrontCenterDistance(model.ObstacleMap)).GetPtr(), tFalse);
            if (MapChecker::GetFreeFrontCenterDistance(model.ObstacleMap) > reverseDistance)
            {
                if (MapChecker::GetFreeSideLeftDistance(model.ObstacleMap) > sideDistanceToObstacle)
                {
                    SetState(ObstaclePassingState::Active_switching_lane_left);
                }
                else
                {
                    SetState(ObstaclePassingState::Idle);
                    logger.Log("Not enough Space on other lane.", tFalse);
                    return;
                }
            }
            else
            {
                logger.Log(cString::Format("Not enough Space  %f", MapChecker::GetFreeFrontCenterDistance(model.ObstacleMap)).GetPtr(), tFalse);
                toClose = tTrue;
            }
        }
    }
    else
    {
        if (model.IsNoPassing)
        {
            logger.Log(cString::Format("No passing allowed!").GetPtr(), tFalse);
        }
        else
        {
            logger.Log(cString::Format("No passing on intersections!").GetPtr(), tFalse);
        }

        SetState(ObstaclePassingState::Idle);
        return;
    }

}

void Passing::SwitchLane(tWorldModel model, int mode)
{
    ResetDriveInstructions(sourceModule);

    steeringControlTaken = tTrue;
    speedControlTaken = tTrue;
    logger.Log(cString::Format("Yaw  %f", model.Imu.tYaw).GetPtr());
    logger.Log(cString::Format("Mid reached %d", midPassed).GetPtr(), tFalse);
    logger.Log(cString::Format("Mode %d", mode).GetPtr());

    speed = speedWhilePassing;

    switchOnYaw(model, mode);
}

void Passing::NextToObstacle(tWorldModel model)
{
    ResetDriveInstructions(sourceModule);

    steeringControlTaken = tTrue;
    speedControlTaken = tTrue;

    if (driveWithLane)
    {
        if (model.Lane.tLeftLine.tStatus == tVISIBLE)
        {
            curveAngle = CalculateLineAngle(model.Lane.tLeftLine, -(safetySpace + 15));
            curveRadius = 0;
        }
        else
        {
            curveAngle = 90;
            curveRadius = 0;
        }
    }
    else
    {
        curveAngle = 90;
        curveRadius = 0;
    }
    speed = speedWhilePassing;

    logger.Log(cString::Format("Line angle %f", CalculateLineAngle(model.Lane.tLeftLine, -(safetySpace + 15))).GetPtr(), tFalse);

    if (model.DistanceOverall > (startDistance + distanceNextToObstacle))
    {
        if (MapChecker::GetFreeSideRightDistance(model.ObstacleMap) > sideDistanceToObstacle)
        {
            /*SetState(ObstaclePassingState::Finished);
            resettableModule = tResettableModule::LaneDetection;*/

            SetState(ObstaclePassingState::Active_switching_lane_right);
        }
    }
}

void Passing::Finished(tWorldModel worldModel)
{
    ResetDriveInstructions(sourceModule);
    roadYaw = 0;
    startDistance = 0;
    triggerCounter = 0;
    midPassed = tFalse;

    if (!laneResetTriggered)
    {
        resettableModule = tReadyModule::LaneDetection;
        laneResetTriggered = true;
        return;
    }

    resettableModule = tReadyModule::Nothing;

    if (GeneralUtils::Contains<tReadyModule::ReadyModuleEnum>(worldModel.ReadyModules, tReadyModule::LaneDetection))
    {
        laneResetTriggered = false;

        SetState(ObstaclePassingState::Idle);
    }
}

tFloat32 Passing::CalculateExpectedYaw(tFloat32 yaw, Direction::DirectionEnum direction, tBool reverse, tFloat32 angle)
{
    // direction
    // 1 steer left
    // 2 steer right

    tInt modify = reverse ? -1 : 1;

    tFloat32 expectedYaw = 0;

    switch (direction)
    {
        case Direction::LEFT:
            expectedYaw = yaw + (modify * angle);
            break;
        case Direction::RIGHT:
            expectedYaw = yaw - (modify * angle);
            break;
        default:
            // ERROR
            expectedYaw = yaw;
    }

    if (expectedYaw > 180)
    {
        expectedYaw = -180 + (expectedYaw - 180);
    }

    if (expectedYaw < -180)
    {
        expectedYaw = 180 + (expectedYaw + 180);
    }

    return expectedYaw;
}

void Passing::switchOnYaw(tWorldModel model, int mode)
{
    switch (mode)
    {
        case SWITCH_LEFT:
            turnSignalLeftEnabled = tTrue;
            if (!midPassed)
            {
                tFloat32 expected = CalculateExpectedYaw(roadYaw, Direction::LEFT, tFalse, midAngle);
                logger.Log(cString::Format("Target yaw %f", expected).GetPtr(), tFalse);
                if (GeneralUtils::Equals(model.Imu.tYaw, expected, 4) && model.Imu.tYaw > expected) //Phase 1
                {
                    midPassed = tTrue;
                }
                else
                {
                    curveAngle = 60;
                    curveRadius = 0;
                }
            }
            else //Phase 2
            {
                logger.Log(cString::Format("Target yaw %f", (roadYaw)).GetPtr(), tFalse);
                if (GeneralUtils::Equals(model.Imu.tYaw, roadYaw, 4) && model.Imu.tYaw < roadYaw)
                {
                    midPassed = tFalse;
                    startDistance = model.DistanceOverall;
                    SetState(ObstaclePassingState::Active_next_to_obstacle);
                }
                else
                {
                    curveAngle = 120;
                    curveRadius = 0;
                }
            }
            break;
        case SWITCH_RIGHT:
            turnSignalRightEnabled = tTrue;
            if (!midPassed)
            {
                tFloat32 expected = CalculateExpectedYaw(roadYaw, Direction::RIGHT, tFalse, midAngle);
                logger.Log(cString::Format("Target yaw %f", expected).GetPtr(), tFalse);
                if (GeneralUtils::Equals(model.Imu.tYaw, expected, 4) && model.Imu.tYaw < expected) //Phase 1
                {
                    midPassed = tTrue;
                }
                else
                {
                    curveAngle = 120;
                    curveRadius = 0;
                }
            }
            else //Phase 2
            {
                logger.Log(cString::Format("Target yaw %f", (roadYaw)).GetPtr(), tFalse);
                if (GeneralUtils::Equals(model.Imu.tYaw, roadYaw, 4) && model.Imu.tYaw > roadYaw)
                {
                    midPassed = tFalse;
                    laneResetTriggered = false;
                    SetState(ObstaclePassingState::Finished);
                }
                else
                {
                    curveAngle = 60;
                    curveRadius = 0;
                }
            }
            break;
        default:
            break;
    }
}

float Passing::CalculateLineAngle(const tLine &line, tFloat32 limit)
{
    tFloat32 averageAngle = 0;

    if (line.tStatus == tVISIBLE)
    {
        // Displacement to steer
        tFloat32 endDisplacement = line.tEnd.tX - limit;
        if (endDisplacement < 0)
        {
            endDisplacement -= 200 - line.tEnd.tY;
        }
        else
        {
            endDisplacement += 200 - line.tEnd.tY;
        }
        averageAngle = endDisplacement / 400 * 30;

        tFloat32 startDisplacement = line.tStart.tX - limit;
        averageAngle += startDisplacement / 20 * 30;

//        logger.Log(cString::Format("Right Start Steering Angle: %f", 90 + startDisplacement / MaxStartDisplacement * 30).GetPtr());
//        logger.Log(cString::Format("Right End Steering Angle: %f", 90 + endDisplacement / MaxEndDisplacement * 30).GetPtr());

        averageAngle /= 2.0;
    }

    return averageAngle;
}
