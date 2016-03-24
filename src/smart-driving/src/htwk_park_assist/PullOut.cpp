//
// Created by pbachmann on 2/11/16.
//

#include "PullOut.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, PullOut)

PullOut::PullOut(const tChar *__info) : ParkingBase(__info, FILTER_NAME, DM_PULL_OUT)
{
    logger.SetSkip(20);

    pullOutState = PullOutState::Idle;
    pullOutCrossState = PullOutCrossState::Idle;
    pullOutParallelState = PullOutParallelState::Idle;

    ResetModule();
}

PullOut::~PullOut()
{
}

tResult PullOut::Init(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Init(eStage, __exception_ptr));

    ResetModule();
    RETURN_NOERROR;
}

void PullOut::ResetModule()
{
    elapsedTime = 0;
    isStateInitialized = false;

    lastYaw = 0;
    yawGoal = 0;

    lastDistanceOverall = 0;
    drivenDistanceCurrentState = 0;

    previousManeuver = tManeuver::M_UNKNOWN;
    detectionRuns = 0;
    undefinedParkingPositionCount = 0;
    crossParkingPositionCount = 0;
    parallelParkingPositionCount = 0;

    laneResetSend = false;
    markerEvaluatorResetSend = false;
    markerDetectionResetSend = false;

    SetState(PullOutState::Idle);
    SetState(PullOutCrossState::Idle);
    SetState(PullOutParallelState::Idle);
}

tResult PullOut::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Start(__exception_ptr));
    ResetModule();

    waitTime = 3;

    crossForwardDistance = 0.20;
    crossTurnLeftRadius = 1.15;
    crossTurnLeftAngle = 85;
    crossTurnRightRadius = 1;
    crossTurnRightAngle = 100;

    parallelForwardLeftAngle = 30;
    parallelForwardLeftRadius = 1;
    parallelForwardRightAngle = 20;
    parallelForwardRightRadius = 1;
    parallelRearCarDistance = 0.1;

    isCrossBackwardActive = tTrue;
    skipCrossRightForward = tFalse;

    CarConfigReader config;
    worldService->Pull(WORLD_CARCONFIG, config);

    logger.Log("Reading properties", false);
    ReadConfiguration(config, PULLOUT_CONFIG_GROUP);

    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, CROSS_FORWARD_DISTANCE_PROPERTY, crossForwardDistance);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, CROSS_TURN_LEFT_RADIUS_PROPERTY, crossTurnLeftRadius);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, CROSS_TURN_LEFT_ANGLE_PROPERTY, crossTurnLeftAngle);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, CROSS_TURN_RIGHT_RADIUS_PROPERTY, crossTurnRightRadius);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, CROSS_TURN_RIGHT_ANGLE_PROPERTY, crossTurnRightAngle);

    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, PARALLEL_FORWARD_LEFT_RADIUS_PROPERTY, parallelForwardLeftRadius);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, PARALLEL_FORWARD_LEFT_ANGLE_PROPERTY, parallelForwardLeftAngle);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, PARALLEL_FORWARD_RIGHT_RADIUS_PROPERTY, parallelForwardRightRadius);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, PARALLEL_FORWARD_RIGHT_ANGLE_PROPERTY, parallelForwardRightAngle);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, PARALLEL_CAR_REAR_DISTANCE_PROPERTY, parallelRearCarDistance);

    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, IS_CROSS_LEFT_BACKWARD_ACTIVE_PROPERTY, isCrossBackwardActive);
    ReadConfigValue(config, PULLOUT_CONFIG_GROUP, SKIP_CROSS_RIGHT_FORWARD_PROPERTY, skipCrossRightForward);

    RETURN_NOERROR;
}

tResult PullOut::Stop(IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Stop(__exception_ptr));

    ResetModule();

    RETURN_NOERROR;
}

tResult PullOut::OnTrigger(tFloat32 interval)
{
    tWorldModel worldModel;
    RETURN_IF_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, worldModel.CarState));

    if (tCarState::Running != worldModel.CarState)
    {
        SetState(PullOutState::Idle);

        speedControlTaken = false;
        steeringControlTaken = false;

        previousManeuver = tManeuver::M_UNKNOWN;

        RETURN_NOERROR;
    }

    worldModel.Interval = interval;
    RETURN_IF_FAILED(worldService->Pull<tManeuver::ManeuverEnum>(WORLD_CURRENT_MANEUVER, worldModel.Maneuver));
    RETURN_IF_FAILED(worldService->Pull<tIMU>(WORLD_IMU, worldModel.Imu));
    RETURN_IF_FAILED(worldService->Pull<Mat>(WORLD_OBSTACLE_MAT, worldModel.ObstacleMap));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_DISTANCE_OVERALL, worldModel.DistanceOverall));
    RETURN_IF_FAILED(worldService->Pull<vector<tReadyModule::ReadyModuleEnum> >(WORLD_READY_MODULES, worldModel.ReadyModules));

    drivenDistanceCurrentState += worldModel.DistanceOverall - lastDistanceOverall;
    lastDistanceOverall = worldModel.DistanceOverall;

    switch (pullOutState)
    {
        case PullOutState::Idle:
            PullOutState_Idle(worldModel);
            break;
        case PullOutState::Wait:
            PullOutState_Wait(worldModel);
            break;
        case PullOutState::DetectOrientation:
            PullOutState_DetectOrientation(worldModel);
            break;
        case PullOutState::PullOutCross:
            PullOutState_PullOutCross(worldModel);
            break;
        case PullOutState::PullOutParallel:
            PullOutState_PullOutParallel(worldModel);
            break;
        case PullOutState::Reset:
            PullOutState_Reset(worldModel);
            break;
        case PullOutState::WaitForReady:
            PullOutState_WaitForReady(worldModel);
            break;
        case PullOutState::ManeuverCompleted:
            PullOutState_ManeuverCompleted(worldModel);
            break;
        case PullOutState::Done:
            PullOutState_Done(worldModel);
            break;
        case PullOutState::Error:
            PullOutState_Error(worldModel);
            break;
        default:
            break;
    }

    lastYaw = worldModel.Imu.tYaw;

    RETURN_NOERROR;
}

void PullOut::PullOutState_Idle(const tWorldModel &model)
{
    if (tManeuver::M_PULL_OUT_LEFT == model.Maneuver || tManeuver::M_PULL_OUT_RIGHT == model.Maneuver)
    {
        logger.Log(cString::Format("Received pull out maneuver %s", tManeuver::ToString(model.Maneuver).c_str()).GetPtr(), false);

        speedControlTaken = true;
        speed = 0;
        steeringControlTaken = true;
        curveRadius = INT_MAX;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = true;
        hazardLightsEnabled = false;
        turnSignalLeftEnabled = tManeuver::M_PULL_OUT_LEFT == model.Maneuver;
        turnSignalRightEnabled = tManeuver::M_PULL_OUT_RIGHT == model.Maneuver;
        reverseLightEnabled = false;

        if (waitTime > 0)
        {
            PullOut::SetState(PullOutState::Wait);
        }
        else
        {
            PullOut::SetState(PullOutState::DetectOrientation);
        }
    }
    else
    {
        previousManeuver = model.Maneuver;
    }
}

void PullOut::PullOutState_Wait(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    headLightEnabled = true;
    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    turnSignalLeftEnabled = tManeuver::M_PULL_OUT_LEFT == model.Maneuver;
    turnSignalRightEnabled = tManeuver::M_PULL_OUT_RIGHT == model.Maneuver;
    reverseLightEnabled = false;

    elapsedTime += model.Interval;
    if (elapsedTime > waitTime * 1000)
    {
        logger.Log("Wait reached", false);
        SetState(PullOutState::DetectOrientation);
    }
}

void PullOut::PullOutState_DetectOrientation(const tWorldModel &model)
{
    logger.Log(cString::Format("Detecting Orientation, Run %d", detectionRuns).GetPtr(), false);

    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    headLightEnabled = true;
    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    turnSignalLeftEnabled = tManeuver::M_PULL_OUT_LEFT == model.Maneuver;
    turnSignalRightEnabled = tManeuver::M_PULL_OUT_RIGHT == model.Maneuver;
    reverseLightEnabled = false;

    if (tManeuver::M_PARK_CROSS == previousManeuver)
    {
        logger.Log(cString::Format("Using previous maneuver %s", tManeuver::ToString(previousManeuver).c_str()).GetPtr(), false);
        SetState(PullOutState::PullOutCross);
        return;
    }

    if (tManeuver::M_PARK_PARALLEL == previousManeuver)
    {
        logger.Log(cString::Format("Using previous maneuver %s", tManeuver::ToString(previousManeuver).c_str()).GetPtr(), false);
        SetState(PullOutState::PullOutParallel);
        return;
    }

    if (undefinedParkingPositionCount + crossParkingPositionCount + parallelParkingPositionCount > 10)
    {
        if (crossParkingPositionCount > 7)
        {
            logger.Log("Cross parking position detected", false);
            SetState(PullOutState::PullOutCross);
            return;
        }

        if (parallelParkingPositionCount > 7)
        {
            logger.Log("Parallel parking position detected", false);
            SetState(PullOutState::PullOutParallel);
            return;
        }

        ++detectionRuns;
        undefinedParkingPositionCount = 0;
        crossParkingPositionCount = 0;
        parallelParkingPositionCount = 0;

        logger.Log("Could'nt detect parking position", false);
        return;
    }

    if (detectionRuns > 10)
    {
        logger.Log("Max count of detection runs reached", false);
        SetState(PullOutState::Error);
        return;
    }

    PullOutState::PullOutStateEnum detectedOrientation = DetectOrientation(model.ObstacleMap);
    switch (detectedOrientation)
    {
        case PullOutState::PullOutCross:
            ++crossParkingPositionCount;
            break;
        case PullOutState::PullOutParallel:
            ++parallelParkingPositionCount;
            break;
        default:
            ++undefinedParkingPositionCount;
            break;
    }

    return;
}

void PullOut::PullOutState_PullOutCross(const tWorldModel &model)
{
    if (PullOutCrossState::Idle == pullOutCrossState)
    {
        speedControlTaken = true;
        speed = 0;
        steeringControlTaken = true;
        curveRadius = INT_MAX;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = true;
        hazardLightsEnabled = false;
        turnSignalLeftEnabled = tManeuver::M_PULL_OUT_LEFT == model.Maneuver;
        turnSignalRightEnabled = tManeuver::M_PULL_OUT_RIGHT == model.Maneuver;
        reverseLightEnabled = false;

        PullOutCrossState::PullOutCrossStateEnum state;

        if (tManeuver::M_PULL_OUT_LEFT == model.Maneuver)
        {
            state = PullOutCrossState::TurnLeft;
        }
        else
        {
            state = skipCrossRightForward ? PullOutCrossState::TurnRight : PullOutCrossState::ForwardTurnRight;
        }

        SetState(state);

        return;
    }

    if (PullOutCrossState::ForwardTurnRight == pullOutCrossState)
    {
        if (!isStateInitialized)
        {
            tFloat32 angle = CalculateAngleFromDistance(straightDriveRadius, crossForwardDistance);
            yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::LEFT, false, angle);

            isStateInitialized = true;
        }

        speedControlTaken = true;
        speed = driveSpeed;
        steeringControlTaken = true;
        curveRadius = -straightDriveRadius;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = false;
        hazardLightsEnabled = false;
        turnSignalLeftEnabled = false;
        turnSignalRightEnabled = true;
        reverseLightEnabled = false;

        // driving a bit to the left
        if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 10) && model.Imu.tYaw > yawGoal)
        {
            speed = 0;
            curveRadius = INT_MAX;
            curveAngle = 0;

            // wait for gyro
            if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
            {
                SetState(PullOutCrossState::TurnRight);
            }
        }
    }

    if (PullOutCrossState::TurnLeft == pullOutCrossState)
    {
        if (!isStateInitialized)
        {
            yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::LEFT, false, crossTurnLeftAngle);

            isStateInitialized = true;
        }

        speedControlTaken = true;
        speed = turnSpeed;
        steeringControlTaken = true;
        curveRadius = -crossTurnLeftRadius;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = false;
        hazardLightsEnabled = false;
        turnSignalLeftEnabled = true;
        turnSignalRightEnabled = false;
        reverseLightEnabled = false;

        if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw > yawGoal)
        {
            speed = 0;
            curveRadius = INT_MAX;
            curveAngle = 0;

            // wait for gyro
            if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
            {

                if (isCrossBackwardActive)
                {
                    SetState(PullOutCrossState::LeftBackward);
                }
                else
                {
                    SetState(PullOutState::Reset);
                    SetState(PullOutCrossState::Done);
                }
            }
        }
    }

    if (PullOutCrossState::TurnRight == pullOutCrossState)
    {
        if (!isStateInitialized)
        {
            yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::RIGHT, false, crossTurnRightAngle);

            isStateInitialized = true;
        }

        speedControlTaken = true;
        speed = turnSpeed;
        steeringControlTaken = true;
        curveRadius = crossTurnRightRadius;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = false;
        hazardLightsEnabled = false;
        turnSignalLeftEnabled = false;
        turnSignalRightEnabled = true;
        reverseLightEnabled = false;

        if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw < yawGoal)
        {
            speed = 0;
            curveRadius = INT_MAX;
            curveAngle = 0;

            // wait for gyro
            if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
            {
                SetState(PullOutState::Reset);
                SetState(PullOutCrossState::Done);
            }
        }
    }

    if (PullOutCrossState::LeftBackward == pullOutCrossState)
    {
        speedControlTaken = true;
        speed = -reverseDriveSpeed;
        steeringControlTaken = true;
        curveRadius = INT_MAX;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = false;
        hazardLightsEnabled = true;
        turnSignalLeftEnabled = false;
        turnSignalRightEnabled = false;
        reverseLightEnabled = true;

        if (!IsBackOfCarFree(model, 20))
        {
            speed = 0;
            elapsedTime += model.Interval;

            if (elapsedTime > 2000)
            {
                SetState(PullOutState::Reset);
                SetState(PullOutCrossState::Done);
            }
        }

        if (fabsf(drivenDistanceCurrentState) > 1)
        {
            SetState(PullOutState::Reset);
            SetState(PullOutCrossState::Done);
        }
    }
}

void PullOut::PullOutState_PullOutParallel(const tWorldModel &model)
{
    if (PullOutParallelState::Idle == pullOutParallelState)
    {
        if (tManeuver::M_PULL_OUT_RIGHT != model.Maneuver)
        {
            logger.Log(cString::Format("Maneuver %d not supported by PullOutParallel", model.Maneuver).GetPtr(), false);
            SetState(PullOutState::Error);

            return;
        }

        speedControlTaken = true;
        speed = 0;
        steeringControlTaken = true;
        curveRadius = INT_MAX;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = true;
        hazardLightsEnabled = false;
        turnSignalLeftEnabled = tManeuver::M_PULL_OUT_LEFT == model.Maneuver;
        turnSignalRightEnabled = tManeuver::M_PULL_OUT_RIGHT == model.Maneuver;
        reverseLightEnabled = false;

        SetState(PullOutParallelState::Backward);

        return;
    }

    if (PullOutParallelState::Backward == pullOutParallelState)
    {
        speedControlTaken = true;
        speed = -reverseDriveSpeed;
        steeringControlTaken = true;
        curveRadius = INT_MAX;
        curveAngle = 0;

        headLightEnabled = true;
        brakeLightEnabled = false;
        hazardLightsEnabled = false;
        turnSignalLeftEnabled = true;
        turnSignalRightEnabled = false;
        reverseLightEnabled = true;

        tFloat32 rearDistance = MapChecker::GetFreeRearCenterDistance(model.ObstacleMap);
        tFloat32 frontDistance = MapChecker::GetFreeFrontCenterDistance(model.ObstacleMap);
        if (rearDistance < parallelRearCarDistance || frontDistance > 0.30)
        {
            SetState(PullOutParallelState::PullLeft);
        }

        return;
    }

    if (PullOutParallelState::PullLeft == pullOutParallelState)
    {
        if (!isStateInitialized)
        {
            yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::LEFT, false, parallelForwardLeftAngle);

            isStateInitialized = true;
        }

        speedControlTaken = true;
        speed = turnSpeed;
        steeringControlTaken = true;
        curveRadius = -parallelForwardLeftRadius;
        curveAngle = 0;

        brakeLightEnabled = false;
        hazardLightsEnabled = false;
        headLightEnabled = true;
        reverseLightEnabled = false;
        turnSignalLeftEnabled = true;
        turnSignalRightEnabled = false;

        if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 10) && model.Imu.tYaw > yawGoal)
        {
            speed = 0;
            curveRadius = INT_MAX;
            curveAngle = 0;

            // wait for gyro
            if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
            {
                SetState(PullOutParallelState::PullStraight);
            }
        }
    }

    if (PullOutParallelState::PullStraight == pullOutParallelState)
    {
        if (!isStateInitialized)
        {
            yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::RIGHT, false, parallelForwardRightAngle);

            isStateInitialized = true;
        }

        speedControlTaken = true;
        speed = turnSpeed;
        steeringControlTaken = true;
        curveRadius = parallelForwardRightRadius;
        curveAngle = 0;

        brakeLightEnabled = false;
        hazardLightsEnabled = false;
        headLightEnabled = true;
        reverseLightEnabled = false;
        turnSignalLeftEnabled = false;
        turnSignalRightEnabled = false;

        if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 10) && model.Imu.tYaw < yawGoal)
        {
            speed = 0;
            curveRadius = INT_MAX;
            curveAngle = 0;

            // wait for gyro
            if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
            {
                SetState(PullOutState::Reset);
                SetState(PullOutParallelState::Done);
            }
        }
    }
}

void PullOut::PullOutState_Reset(const tWorldModel &model)
{
    logger.Log("Reset", false);

    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    if (tReadyModule::Nothing == resettableModule && tManeuver::M_UNKNOWN == previousManeuver)
    {
        // reset ipm, if we're starting from parking lot
        resettableModule = tReadyModule::Ipm;
        return;
    }

    if (!laneResetSend)
    {
        resettableModule = tReadyModule::LaneDetection;
        laneResetSend = true;
        return;
    }

    if (!markerDetectionResetSend)
    {
        resettableModule = tReadyModule::MarkerDetection;
        markerDetectionResetSend = true;
        return;
    }

    if (!markerEvaluatorResetSend)
    {
        resettableModule = tReadyModule::MarkerEvaluator;
        markerEvaluatorResetSend = true;
        return;
    }

    SetState(PullOutState::WaitForReady);

    return;

}

void PullOut::PullOutState_WaitForReady(tWorldModel model)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;
    resettableModule = tReadyModule::Nothing;

    if (GeneralUtils::Contains(model.ReadyModules, tReadyModule::MarkerDetection) &&
        GeneralUtils::Contains(model.ReadyModules, tReadyModule::MarkerEvaluator) &&
        GeneralUtils::Contains(model.ReadyModules, tReadyModule::LaneDetection))
    {
        SetState(PullOutState::ManeuverCompleted);
    }
}

void PullOut::PullOutState_ManeuverCompleted(const tWorldModel &model)
{
    logger.Log("Maneuver completed", false);

    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;
    resettableModule = tReadyModule::Nothing;

    maneuverCompleted = true;

    SetState(PullOutState::Done);

    return;
}

void PullOut::PullOutState_Done(const tWorldModel &model)
{
    ResetDriveInstructions(sourceModule);

    if (tManeuver::M_PULL_OUT_LEFT != model.Maneuver && tManeuver::M_PULL_OUT_RIGHT != model.Maneuver)
    {
        logger.Log("Going back to idle", false);

        SetState(PullOutState::Idle);
        SetState(PullOutCrossState::Idle);
        SetState(PullOutParallelState::Idle);
    }

    return;
}

void PullOut::PullOutState_Error(const tWorldModel &model)
{
    logger.Log("Error occured. Releasing control", false);

    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = false;

    headLightEnabled = true;
    brakeLightEnabled = true;
    hazardLightsEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    errorActive = true;

    return;
}

/*
 * Detects the current parking orientation based on the currentObstacleMap
 */
PullOut::PullOutState::PullOutStateEnum PullOut::DetectOrientation(const Mat &map)
{
    tFloat32 spaceLeft = MapChecker::GetFreeSideLeftDistance(map);
    tFloat32 spaceRight = MapChecker::GetFreeSideRightDistance(map);
    tFloat32 spaceFront = MapChecker::GetFreeFrontCenterDistance(map);
    tFloat32 spaceBack = MapChecker::GetFreeRearCenterDistance(map);

    logger.Log(cString::Format("Space Left: %f", spaceLeft).GetPtr(), false);
    logger.Log(cString::Format("Space Right: %f", spaceRight).GetPtr(), false);
    logger.Log(cString::Format("Space Front: %f", spaceFront).GetPtr(), false);
    logger.Log(cString::Format("Space Back: %f", spaceBack).GetPtr(), false);

    if ((spaceLeft < 0.30 || spaceRight < 0.30) && spaceFront > 0.50)
    {
        return PullOutState::PullOutCross;
    }

    if ((spaceBack < 0.30 || spaceFront < 0.30) && spaceLeft > 0.30)
    {
        return PullOutState::PullOutParallel;
    }

    return PullOutState::Error;
}

void PullOut::SetState(const PullOutState::PullOutStateEnum newState)
{
    if (newState == pullOutState)
    {
        return;
    }

    if (PullOutState::Idle == newState)
    {
        ResetDriveInstructions(sourceModule);
    }

    logger.Log(cString::Format("Switching to PullOutState %s", PullOutState::ToString(newState).c_str()).GetPtr(), false);
    pullOutState = newState;

    speed = 0;
    curveRadius = INT_MAX;
    curveAngle = 0;

    elapsedTime = 0;
    detectionRuns = 0;
    drivenDistanceCurrentState = 0;
    undefinedParkingPositionCount = 0;
    crossParkingPositionCount = 0;
    parallelParkingPositionCount = 0;

    laneResetSend = false;
    markerEvaluatorResetSend = false;
    markerDetectionResetSend = false;
}

void PullOut::SetState(const PullOutCrossState::PullOutCrossStateEnum newState)
{
    if (newState == pullOutCrossState)
    {
        return;
    }

    logger.Log(cString::Format("Switching to PullOutStateCross %s", PullOutCrossState::ToString(newState).c_str()).GetPtr(), false);

    pullOutCrossState = newState;
    isStateInitialized = false;
    drivenDistanceCurrentState = 0;

    speed = 0;
    curveRadius = INT_MAX;
    curveAngle = 0;
}

void PullOut::SetState(const PullOutParallelState::PullOutParallelStateEnum newState)
{
    if (newState == pullOutParallelState)
    {
        return;
    }

    logger.Log(cString::Format("Switching to PullOutParallelState %s", PullOutParallelState::ToString(newState).c_str()).GetPtr(), false);

    pullOutParallelState = newState;
    isStateInitialized = false;
    drivenDistanceCurrentState = 0;

    speed = 0;
    curveRadius = INT_MAX;
    curveAngle = 0;
}
