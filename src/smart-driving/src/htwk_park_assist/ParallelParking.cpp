/**
 * @author pbachmann
 */

#include "ParallelParking.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, ParallelParking)

ParallelParking::ParallelParking(const tChar *__info) : ParkingBase(__info, FILTER_NAME, DM_PARALLEL_PARKING)
{
    logger.SetSkip(20);

    state = ParallelParkingState::Idle;
    ResetModule();
}

ParallelParking::~ParallelParking()
{
}

tResult ParallelParking::Init(tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
    }
    else if (eStage == StageNormal)
    {
        sideRecognitionDistance = tFloat32(GetPropertyFloat(SIDE_RECOGNITION_DISTANCE_PROPERTY));
        parkingLotSize = tFloat32(GetPropertyFloat(PARKING_LOT_SIZE_PROPERTY));
        straightForwardDistance = tFloat32(GetPropertyFloat(STRAIGHT_FORWARD_DISTANCE_PROPERTY));

        turnBackRightRadius = tFloat32(GetPropertyFloat(TURNBACKRIGHT_RADIUS_PROPERTY));
        turnBackRightAngle = tInt(GetPropertyInt(TURNBACKRIGHT_ANGLE_PROPERTY));

        turnBackLeftRadius = tFloat32(GetPropertyFloat(TURNBACKLEFT_RADIUS_PROPERTY));
        turnBackLeftAngle = tInt(GetPropertyInt(TURNBACKLEFT_ANGLE_PROPERTY));
        turnBackLeftRearDistance = tFloat32(GetPropertyFloat(TURNBACKLEFT_REAR_DISTANCE_PROPERTY));

        turnForwardRightRadius = tFloat32(GetPropertyFloat(TURNFORWARDRIGHT_RADIUS_PROPERTY));
        turnForwardRightAngle = tInt(GetPropertyInt(TURNFORWARDRIGHT_ANGLE_PROPERTY));
        turnForwardRightFrontDistance = tFloat32(GetPropertyFloat(TURNFORWARDRIGHT_FRONT_DISTANCE_PROPERTY));

        backwardInLotRearDistance = tFloat32(GetPropertyFloat(BACKWARDINLOT_REAR_DISTANCE));
    }
    else if (eStage == StageGraphReady)
    {
    }

    ResetModule();
    RETURN_NOERROR;
}

void ParallelParking::ResetModule()
{
    elapsedTime = 0;
    isStateInitialized = false;

    lastYaw = 0;
    yawGoal = 0;

    carDetectionCount = 0;
    stateDetectionCount = 0;

    drivenDistanceCurrentState = 0;
    lastDistanceOverall = 0;

    SetState(ParallelParkingState::Idle);
}

tResult ParallelParking::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Start(__exception_ptr));
    ResetModule();

    waitTime = 5;
    straightForwardDistance = 0.3;

    turnBackRightAngle = 33;
    turnBackRightRadius = 1;

    turnBackLeftAngle = 25;
    turnBackLeftRadius = 1;
    turnBackLeftRearDistance = 0.1;

    turnForwardRightAngle = 15;
    turnForwardRightRadius = 1;
    turnForwardRightFrontDistance = 0.15;

    backwardInLotRearDistance = 0.1;
    backwardInLotDriveDistance = 0.15;

    sideRecognitionDistance = 0.25;
    parkingLotSize = 0.70;

    CarConfigReader config;
    worldService->Pull(WORLD_CARCONFIG, config);

    logger.Log("Reading properties", false);
    ParkingBase::ReadConfiguration(config, PARALLEL_PARKING_CONFIG_GROUP);

    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, STRAIGHT_FORWARD_DISTANCE_PROPERTY, straightForwardDistance);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNBACKRIGHT_RADIUS_PROPERTY, turnBackRightRadius);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNBACKRIGHT_ANGLE_PROPERTY, turnBackRightAngle);

    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNBACKLEFT_RADIUS_PROPERTY, turnBackLeftRadius);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNBACKLEFT_ANGLE_PROPERTY, turnBackLeftAngle);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNBACKLEFT_REAR_DISTANCE_PROPERTY, turnBackLeftRearDistance);

    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNFORWARDRIGHT_RADIUS_PROPERTY, turnForwardRightRadius);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNFORWARDRIGHT_ANGLE_PROPERTY, turnForwardRightAngle);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, TURNFORWARDRIGHT_FRONT_DISTANCE_PROPERTY, turnForwardRightFrontDistance);

    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, BACKWARDINLOT_REAR_DISTANCE, backwardInLotRearDistance);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, BACKWARDINLOT_DRIVE_DISTANCE, backwardInLotDriveDistance);

    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, SIDE_RECOGNITION_DISTANCE_PROPERTY, sideRecognitionDistance);
    ReadConfigValue(config, PARALLEL_PARKING_CONFIG_GROUP, PARKING_LOT_SIZE_PROPERTY, parkingLotSize);

    RETURN_NOERROR;
}

tResult ParallelParking::Stop(IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Stop(__exception_ptr));

    ResetModule();

    RETURN_NOERROR;
}

tResult ParallelParking::OnTrigger(tFloat32 interval)
{
    tWorldModel worldModel;
    RETURN_IF_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, worldModel.CarState));

    if (tCarState::Running != worldModel.CarState)
    {
        SetState(ParallelParkingState::Idle);

        speedControlTaken = false;
        steeringControlTaken = false;

        RETURN_NOERROR;
    }

    worldModel.Interval = interval;
    RETURN_IF_FAILED(worldService->Pull<tManeuver::ManeuverEnum>(WORLD_CURRENT_MANEUVER, worldModel.Maneuver));
    RETURN_IF_FAILED(worldService->Pull<tRoadSign::RoadSignEnum>(WORLD_CURRENT_ROAD_SIGN, worldModel.RoadSign));
    RETURN_IF_FAILED(worldService->Pull<tIMU>(WORLD_IMU, worldModel.Imu));
    RETURN_IF_FAILED(worldService->Pull<tLane>(WORLD_LANE, worldModel.Lane));
    RETURN_IF_FAILED(worldService->Pull<Mat>(WORLD_OBSTACLE_MAT, worldModel.ObstacleMap));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_DISTANCE_OVERALL, worldModel.DistanceOverall));

#ifndef NDEBUG
    debugImage = worldModel.ObstacleMap.clone();
    cvtColor(debugImage, debugImage, CV_GRAY2RGB);
#endif

    drivenDistanceCurrentState += worldModel.DistanceOverall - lastDistanceOverall;
    lastDistanceOverall = worldModel.DistanceOverall;

    switch (state)
    {
        case ParallelParkingState::Idle:
            ParallelParkingState_Idle(worldModel);
            break;
        case ParallelParkingState::Driving:
            ParallelParkingState_Driving(worldModel);
            break;
        case ParallelParkingState::FirstVehicleDetected:
            ParallelParkingState_FirstVehicleDetected(worldModel);
            break;
        case ParallelParkingState::ParkingLotDetected:
            ParallelParkingState_ParkingLotDetected(worldModel);
            break;
        case ParallelParkingState::StraightForward:
            ParallelParkingState_StraightForward(worldModel);
            break;
        case ParallelParkingState::SecondVehicleDetected:
            ParallelParkingState_SecondVehicleDetected(worldModel);
            break;
        case ParallelParkingState::TurnBackRight:
            ParallelParkingState_TurnBackRight(worldModel);
            break;
        case ParallelParkingState::TurnBackLeftIntoLot:
            ParallelParkingState_TurnBackLeft(worldModel);
            break;
        case ParallelParkingState::TurnForwardRightInLot:
            ParallelParkingState_TurnForwardRightInLot(worldModel);
            break;
        case ParallelParkingState::BackwardInLot:
            ParallelParkingState_BackwardInLot(worldModel);
            break;
        case ParallelParkingState::Stopped:
            ParallelParkingState_Stopped(worldModel);
            break;
        case ParallelParkingState::Wait:
            ParallelParkingState_Wait(worldModel);
            break;
        case ParallelParkingState::ManeuverCompleted:
            ParallelParkingState_ManeuverCompleted(worldModel);
            break;
        case ParallelParkingState::Done:
            ParallelParkingState_Done(worldModel);
            break;
        case ParallelParkingState::Error:
            ParallelParkingState_Error(worldModel);
            break;
    }

#ifndef NDEBUG
    // draw car
    Point carTopLeft = VisionUtils::WorldToImage(Point(-15, 0));
    Point carBotRight = VisionUtils::WorldToImage(Point(15, -60));

    cv::rectangle(debugImage, carTopLeft, carBotRight, Scalar(100, 100, 100), CV_FILLED);

    // draw lane
    Point rightBot = VisionUtils::WorldToImage(worldModel.Lane.tRightLine.tStart);
    Point rightTop = VisionUtils::WorldToImage(worldModel.Lane.tRightLine.tEnd);
    Point leftBot = VisionUtils::WorldToImage(worldModel.Lane.tLeftLine.tStart);
    Point leftTop = VisionUtils::WorldToImage(worldModel.Lane.tLeftLine.tEnd);

    cv::line(debugImage, rightBot, rightTop, Scalar(255, 0, 0, 127));
    cv::line(debugImage, leftBot, leftTop, Scalar(255, 0, 0, 127));

    // send video
    if (debugVideoOutput.IsConnected())
    {
        cObjectPtr<IMediaSample> mediaSample;
        RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &mediaSample));
        RETURN_IF_FAILED(mediaSample->AllocBuffer(debugVideoFormat.nSize));
        mediaSample->Update(_clock->GetStreamTime(), debugImage.data, debugVideoFormat.nSize, 0);

        debugVideoOutput.Transmit(mediaSample);
    }
#endif

    lastYaw = worldModel.Imu.tYaw;

    RETURN_NOERROR;
}

void ParallelParking::ParallelParkingState_Idle(const tWorldModel &model)
{
    if (tManeuver::M_PARK_PARALLEL == model.Maneuver && tRoadSign::PARKING_AREA == model.RoadSign)
    {
        logger.Log(cString::Format("Parallel parking activated. Maneuver: %s, RoadSign: %s", tManeuver::ToString(model.Maneuver).c_str(),
                                   tRoadSign::ToString(model.RoadSign).c_str()).GetPtr(), false);
        SetState(ParallelParkingState::Driving);
    }
}

void ParallelParking::ParallelParkingState_Driving(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = driveSpeed;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalRightEnabled = false;
    turnSignalLeftEnabled = false;

    if (!IsFrontOfCarFree(model, 30))
    {
        StopOnObstacle(model);
        return;
    }

    tFloat32 distanceSideRight = MapChecker::GetFreeSideRightDistance(model.ObstacleMap);
    if (distanceSideRight < sideRecognitionDistance)
    {
        ++stateDetectionCount;

        if (stateDetectionCount > ultraSonicSamples)
        {
            logger.Log("Found first car", false);
            SetState(ParallelParkingState::FirstVehicleDetected);
            return;
        }
    }
    else
    {
        stateDetectionCount = 0;
    }
}

void ParallelParking::ParallelParkingState_FirstVehicleDetected(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = driveSpeed;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    if (!IsFrontOfCarFree(model, 30))
    {
        StopOnObstacle(model);
        return;
    }

    if (drivenDistanceCurrentState > 2 || carDetectionCount > 3)
    {
        logger.Log("Could'nt find parking lot", false);
        SetState(ParallelParkingState::Error);
        return;
    }

    tFloat32 distanceSideRight = MapChecker::GetFreeSideRightDistance(model.ObstacleMap);
    if (distanceSideRight > sideRecognitionDistance)
    {
        ++stateDetectionCount;

        if (stateDetectionCount > ultraSonicSamples)
        {
            logger.Log("Found parking lot", false);
            SetState(ParallelParkingState::ParkingLotDetected);
            return;
        }
    }
    else
    {
        stateDetectionCount = 0;
    }

    return;
}

void ParallelParking::ParallelParkingState_ParkingLotDetected(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = driveSpeed;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    if (!IsFrontOfCarFree(model, 30))
    {
        StopOnObstacle(model);
        return;
    }

    if (drivenDistanceCurrentState > 4)
    {
        logger.Log("Could'nt find second car", false);
        SetState(ParallelParkingState::Error);
        return;
    }

    tFloat32 distanceSideRight = MapChecker::GetFreeSideRightDistance(model.ObstacleMap);
    if (distanceSideRight < sideRecognitionDistance)
    {
        ++stateDetectionCount;
        if (stateDetectionCount > ultraSonicSamples)
        {

            if (drivenDistanceCurrentState < parkingLotSize)
            {
                logger.Log(cString::Format("Parking lot to small, %f m", drivenDistanceCurrentState).GetPtr(), false);
                SetState(ParallelParkingState::FirstVehicleDetected);
                return;
            }

            logger.Log(cString::Format("Found second car, parking lot size: %f m", drivenDistanceCurrentState).GetPtr(), false);
            SetState(ParallelParkingState::SecondVehicleDetected);
            return;
        }
    }
    else
    {
        stateDetectionCount = 0;
    }
}

void ParallelParking::ParallelParkingState_SecondVehicleDetected(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    // wait for gyro
    if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
    {
        SetState(ParallelParkingState::StraightForward);
    }
}

void ParallelParking::ParallelParkingState_StraightForward(const tWorldModel &model)
{
    if (!isStateInitialized)
    {
        tFloat32 angle = CalculateAngleFromDistance(straightDriveRadius, straightForwardDistance);
        yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::LEFT, false, angle);

        isStateInitialized = true;
    }

    speedControlTaken = true;
    speed = driveSpeed;
    steeringControlTaken = true;
    curveRadius = -straightDriveRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    if (!IsFrontOfCarFree(model, 15))
    {
        StopOnObstacle(model);
        return;
    }

    if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw > yawGoal)
    {
        speed = 0;
        curveRadius = INT_MAX;

        // wait for gyro
        if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
        {
            SetState(ParallelParkingState::TurnBackRight);
        }
    }
}

void ParallelParking::ParallelParkingState_TurnBackRight(const tWorldModel &model)
{
    if (!isStateInitialized)
    {
        yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::RIGHT, true, turnBackRightAngle);

        isStateInitialized = true;
    }

    speedControlTaken = true;
    speed = -turnSpeed;
    steeringControlTaken = true;
    curveRadius = turnBackRightRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = true;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    if (!IsBackOfCarFree(model, 15, 10))
    {
        StopOnObstacle(model);
        return;
    }

#ifndef NDEBUG
    Point rookPoints[1][turnBackRightAngle];
    tFloat32 radius = turnBackRightRadius * 100; // in cm
    for (int i = 0; i < turnBackRightAngle; ++i)
    {
        int x = (int) round((200 + radius) - (cos(i * M_PI / 180.0) * 100.0));
        int y = (int) round(260 + (sin(i * M_PI / 180.0) * 100.0));
        rookPoints[0][i] = Point(x, y);
    }

    const Point *ppt[1] = {rookPoints[0]};
    int npt[] = {turnBackRightAngle};
    cv::polylines(debugImage, ppt, npt, 1, false, Scalar(0, 0, 255, 127));
#endif

    if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw > yawGoal)
    {
        speed = 0;
        curveRadius = INT_MAX;

        // wait for gyro
        if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
        {
            SetState(ParallelParkingState::TurnBackLeftIntoLot);
        }
    }
}

void ParallelParking::ParallelParkingState_TurnBackLeft(const tWorldModel &model)
{
    if (!isStateInitialized)
    {
        yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::LEFT, true, turnBackLeftAngle);

        isStateInitialized = true;
    }

    speedControlTaken = true;
    speed = -turnSpeed;
    steeringControlTaken = true;
    curveRadius = -turnBackLeftRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = true;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

#ifndef NDEBUG
    Point rookPoints[1][turnBackLeftAngle];
    tFloat32 radius = turnBackLeftRadius * 100; // in cm
    for (int i = 0; i < turnBackLeftAngle; ++i)
    {
        int x = (int) round((200 + radius) + (cos(i * M_PI / 180.0) * 100.0));
        int y = (int) round(260 + (sin(i * M_PI / 180.0) * 100.0));
        rookPoints[0][i] = Point(x, y);
    }

    const Point *ppt[1] = {rookPoints[0]};
    int npt[] = {turnBackLeftAngle};
    cv::polylines(debugImage, ppt, npt, 1, false, Scalar(0, 0, 255, 127));
#endif

    if (!IsBackOfCarFree(model, turnBackLeftRearDistance * 100))
    {
        speed = 0;
        curveRadius = INT_MAX;

        logger.Log("Min back distance reached", false);
        SetState(ParallelParkingState::TurnForwardRightInLot);

        return;
    }

    if ((GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw < yawGoal))
    {
        speed = 0;
        curveRadius = INT_MAX;

        // wait for gyro
        if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
        {
            logger.Log("Yaw goal reached", false);
            SetState(ParallelParkingState::TurnForwardRightInLot);
        }
    }
}

void ParallelParking::ParallelParkingState_TurnForwardRightInLot(const tWorldModel &model)
{
    if (!isStateInitialized)
    {
        yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::RIGHT, false, turnForwardRightAngle);

        isStateInitialized = true;
    }

    speedControlTaken = true;
    speed = driveSpeed;
    steeringControlTaken = true;
    curveRadius = turnForwardRightRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    if (!IsFrontOfCarFree(model, turnForwardRightFrontDistance * 100))
    {
        speed = 0;
        curveRadius = INT_MAX;

        logger.Log("Min front distance reached", false);
        SetState(ParallelParkingState::BackwardInLot);

        return;
    }

    if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw < yawGoal)
    {
        speed = 0;
        curveRadius = INT_MAX;

        // wait for gyro
        if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
        {
            logger.Log("Yaw goal reached", false);
            SetState(ParallelParkingState::BackwardInLot);
        }
    }
}

void ParallelParking::ParallelParkingState_BackwardInLot(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = -reverseDriveSpeed;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = true;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    if (fabsf(drivenDistanceCurrentState) > backwardInLotDriveDistance)
    {
        speed = 0;
        curveRadius = INT_MAX;

        logger.Log("Drive distance reached", false);
        SetState(ParallelParkingState::Stopped);
        return;
    }

    if (!IsBackOfCarFree(model, backwardInLotRearDistance * 100))
    {
        speed = 0;
        curveRadius = INT_MAX;

        logger.Log("Min back distance reached", false);
        SetState(ParallelParkingState::Stopped);
    }
}

void ParallelParking::ParallelParkingState_Stopped(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    headLightEnabled = true;
    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    if (waitTime > 0)
    {
        SetState(ParallelParkingState::Wait);
    }
    else
    {
        SetState(ParallelParkingState::ManeuverCompleted);
    }
}

void ParallelParking::ParallelParkingState_Wait(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    headLightEnabled = true;
    brakeLightEnabled = true;
    hazardLightsEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    elapsedTime += model.Interval;
    if (elapsedTime > waitTime * 1000)
    {
        logger.Log("Wait reached", false);
        SetState(ParallelParkingState::ManeuverCompleted);
    }
}

void ParallelParking::ParallelParkingState_ManeuverCompleted(const tWorldModel &model)
{
    logger.Log("Maneuver completed, releasing control", false);

    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    headLightEnabled = true;
    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;
    reverseLightEnabled = false;

    maneuverCompleted = true;

    SetState(ParallelParkingState::Done);
}

void ParallelParking::ParallelParkingState_Done(const tWorldModel &model)
{
    ResetDriveInstructions(sourceModule);

    if (tManeuver::M_PARK_PARALLEL != model.Maneuver)
    {
        logger.Log("Going back to idle", false);
        SetState(ParallelParkingState::Idle);
    }
}

void ParallelParking::ParallelParkingState_Error(const tWorldModel &model)
{
    logger.Log("Error occured. Releasing control", true);

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
}

void ParallelParking::StopOnObstacle(const tWorldModel &model)
{
    speed = 0;
    elapsedTime += model.Interval;

    if (elapsedTime > 5000)
    {
        logger.Log("The obstacle doesn't move!", false);
        SetState(ParallelParkingState::Error);
    }
}

void ParallelParking::SetState(const ParallelParkingState::ParallelParkingStateEnum newState)
{
    if (newState == state)
    {
        return;
    }

    logger.Log(cString::Format("Switching to state %s", ParallelParkingState::ToString(newState).c_str()).GetPtr(), false);
    state = newState;
    isStateInitialized = false;

    drivenDistanceCurrentState = 0;
    stateDetectionCount = 0;
    elapsedTime = 0;

    if (ParallelParkingState::Idle == newState)
    {
        ResetDriveInstructions(sourceModule);
        carDetectionCount = 0;
    }

    if (ParallelParkingState::FirstVehicleDetected == newState)
    {
        ++carDetectionCount;
    }
}

