/**
 * @author pbachmann
 */

#include "CrossParking.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, CrossParking)

CrossParking::CrossParking(const tChar *__info) : ParkingBase(__info, FILTER_NAME, DM_CROSS_PARKING)
{
    logger.SetSkip(20);

    state = CrossParkingState::Idle;
    ResetModule();
}

void CrossParking::ResetModule()
{
    elapsedTime = 0;
    isStateInitialized = false;

    lastYaw = 0;
    yawGoal = 0;

    carDetectionCount = 0;
    stateDetectionCount = 0;

    drivenDistanceCurrentState = 0;
    lastDistanceOverall = 0;

    SetState(CrossParkingState::Idle);
}

CrossParking::~CrossParking()
{
}

tResult CrossParking::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(ParkingBase::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
    }
    else if (eStage == StageNormal)
    {
    }
    else if (eStage == StageGraphReady)
    {
    }

    ResetModule();
    RETURN_NOERROR;
}

tResult CrossParking::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Start(__exception_ptr));
    ResetModule();

    waitTime = 5;

    straightBackwardDistance = 0.2;

    swingCurveRadius = 1;
    swingAngle = 20;
    turnCurveRadius = 1;
    turnAngle = 60;

    parkingDistance = 0.1;
    rearFreeDistanceParking = 0.4;

    sideRecognitionDistance = 0.3;
    parkingLotSize = 0.5;

    CarConfigReader config;
    worldService->Pull(WORLD_CARCONFIG, config);

    logger.Log("Reading properties", false);
    ParkingBase::ReadConfiguration(config, CROSS_PARKING_CONFIG_GROUP);

    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, STRAIGHT_BACKWARD_DISTANCE_PROPERTY, straightBackwardDistance);

    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, SWING_RADIUS_PROPERTY, swingCurveRadius);
    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, SWING_ANGLE_PROPERTY, swingAngle);
    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, TURN_RADIUS_PROPERTY, turnCurveRadius);
    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, TURN_ANGLE_PROPERTY, turnAngle);

    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, PARKING_DISTANCE_PROPERTY, parkingDistance);
    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, REAR_FREE_DISTANCE_PARKING_PROPERTY, rearFreeDistanceParking);

    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, SIDE_RECOGNITION_DISTANCE_PROPERTY, sideRecognitionDistance);
    ReadConfigValue(config, CROSS_PARKING_CONFIG_GROUP, PARKING_LOT_SIZE_PROPERTY, parkingLotSize);

    RETURN_NOERROR;
}

tResult CrossParking::Stop(IException **__exception_ptr)
{
    RETURN_IF_FAILED(ParkingBase::Stop(__exception_ptr));

    RETURN_NOERROR;
}

tResult CrossParking::OnTrigger(tFloat32 interval)
{
    tWorldModel worldModel;
    RETURN_IF_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, worldModel.CarState));

    if (tCarState::Running != worldModel.CarState)
    {
        SetState(CrossParkingState::Idle);

        speedControlTaken = false;
        steeringControlTaken = false;

        RETURN_NOERROR;
    }

    worldModel.Interval = interval;
    RETURN_IF_FAILED(worldService->Pull<tManeuver::ManeuverEnum>(WORLD_CURRENT_MANEUVER, worldModel.Maneuver));
    RETURN_IF_FAILED(worldService->Pull<tRoadSign::RoadSignEnum>(WORLD_CURRENT_ROAD_SIGN, worldModel.RoadSign));
    RETURN_IF_FAILED(worldService->Pull<tIMU>(WORLD_IMU, worldModel.Imu));
    RETURN_IF_FAILED(worldService->Pull<Mat>(WORLD_OBSTACLE_MAT, worldModel.ObstacleMap));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_DISTANCE_OVERALL, worldModel.DistanceOverall));
    RETURN_IF_FAILED(worldService->Pull<tLane>(WORLD_LANE, worldModel.Lane));

#ifndef NDEBUG
    debugImage = worldModel.ObstacleMap.clone();
    cvtColor(debugImage, debugImage, CV_GRAY2RGB);
#endif

    drivenDistanceCurrentState += worldModel.DistanceOverall - lastDistanceOverall;
    lastDistanceOverall = worldModel.DistanceOverall;

    switch (state)
    {
        case CrossParkingState::Idle:
            CrossParkingState_Idle(worldModel);
            break;
        case CrossParkingState::Driving:
            CrossParkingState_Driving(worldModel);
            break;
        case CrossParkingState::FirstVehicleDetected:
            CrossParkingState_FirstVehicleDetected(worldModel);
            break;
        case CrossParkingState::ParkingLotDetected:
            CrossParkingState_ParkingLotDetected(worldModel);
            break;
        case CrossParkingState::SecondVehicleDetected:
            CrossParkingState_SecondVehicleDetected(worldModel);
            break;
        case CrossParkingState::StraightBackward:
            CrossParkingState_StraightBackward(worldModel);
            break;
        case CrossParkingState::Swing:
            CrossParkingState_Swing(worldModel);
            break;
        case CrossParkingState::Turning:
            CrossParkingState_Turning(worldModel);
            break;
        case CrossParkingState::Parking:
            CrossParkingState_Parking(worldModel);
            break;
        case CrossParkingState::Stopped:
            CrossParkingState_Stopped(worldModel);
            break;
        case CrossParkingState::Wait:
            CrossParkingState_Wait(worldModel);
            break;
        case CrossParkingState::ManeuverCompleted:
            CrossParkingState_ManeuverCompleted(worldModel);
            break;
        case CrossParkingState::Done:
            CrossParkingState_Done(worldModel);
            break;
        case CrossParkingState::Error:
            CrossParkingState_Error(worldModel);
            break;
        default:
            break;
    }

    lastYaw = worldModel.Imu.tYaw;

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

    RETURN_NOERROR;
}

void CrossParking::CrossParkingState_Idle(const tWorldModel &model)
{
    if (tManeuver::M_PARK_CROSS == model.Maneuver && tRoadSign::PARKING_AREA == model.RoadSign)
    {
        logger.Log(cString::Format("Cross parking activated. Maneuver: %s, RoadSign: %s", tManeuver::ToString(model.Maneuver).c_str(),
                                   tRoadSign::ToString(model.RoadSign).c_str()).GetPtr(), false);
        SetState(CrossParkingState::Driving);
    }
}

void CrossParking::CrossParkingState_Driving(const tWorldModel &model)
{
    speedControlTaken = true;
    speed = driveSpeed;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

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
            SetState(CrossParkingState::FirstVehicleDetected);
            return;
        }
    }
    else
    {
        stateDetectionCount = 0;
    }
}

void CrossParking::CrossParkingState_FirstVehicleDetected(const tWorldModel &model)
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

    if (drivenDistanceCurrentState > 2 || carDetectionCount > 2)
    {
        logger.Log("Could'nt find parking lot", false);
        SetState(CrossParkingState::Error);
        return;
    }

    tFloat32 distanceSideRight = MapChecker::GetFreeSideRightDistance(model.ObstacleMap);
    if (distanceSideRight > sideRecognitionDistance)
    {
        ++stateDetectionCount;

        if (stateDetectionCount > ultraSonicSamples)
        {
            logger.Log("Found parking lot", false);
            SetState(CrossParkingState::ParkingLotDetected);
            return;
        }
    }
    else
    {
        stateDetectionCount = 0;
    }

    return;
}

void CrossParking::CrossParkingState_ParkingLotDetected(const tWorldModel &model)
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

    if (drivenDistanceCurrentState > 2)
    {
        logger.Log("Could'nt find second car", false);
        SetState(CrossParkingState::Error);
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
                SetState(CrossParkingState::FirstVehicleDetected);
                return;
            }

            logger.Log(cString::Format("Found second car, parking lot size: %f m", drivenDistanceCurrentState).GetPtr(), false);
            SetState(CrossParkingState::SecondVehicleDetected);
            return;
        }
    }
    else
    {
        stateDetectionCount = 0;
    }
}

void CrossParking::CrossParkingState_SecondVehicleDetected(const tWorldModel &model)
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
        SetState(CrossParkingState::StraightBackward);
    }
}

void CrossParking::CrossParkingState_StraightBackward(const tWorldModel &model)
{
    if (!isStateInitialized)
    {
        tFloat32 angle = CalculateAngleFromDistance(straightDriveRadius, straightBackwardDistance);
        yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::RIGHT, true, angle);

        isStateInitialized = true;
    }

    speedControlTaken = true;
    speed = -turnSpeed;
    steeringControlTaken = true;
    curveRadius = straightDriveRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = true;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    if (!IsBackOfCarFree(model, 15))
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
            SetState(CrossParkingState::Swing);
        }
    }
}

void CrossParking::CrossParkingState_Swing(const tWorldModel &model)
{
    if (!isStateInitialized)
    {
        yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::LEFT, false, swingAngle);
        isStateInitialized = true;
    }

    speedControlTaken = true;
    speed = turnSpeed;
    steeringControlTaken = true;
    curveRadius = -swingCurveRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

/*
    if (!IsFrontOfCarFree(model, 10))
    {
        StopOnObstacle(model);
        return;
    }
*/

#ifndef NDEBUG
    Point rookPoints[1][swingAngle];
    tFloat32 radius = swingCurveRadius * 100; // in cm
    for (int i = 0; i < swingAngle; ++i)
    {
        int x = (int) round((200 - radius) + (cos(i * M_PI / 180.0) * 100.0));
        int y = (int) round(200 - (sin(i * M_PI / 180.0) * 100.0));
        rookPoints[0][i] = Point(x, y);
    }

    const Point *ppt[1] = {rookPoints[0]};
    int npt[] = {swingAngle};
    cv::polylines(debugImage, ppt, npt, 1, false, Scalar(0, 0, 255, 127));
#endif

    if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw > yawGoal)
    {
        speed = 0;
        curveRadius = INT_MAX;

        // wait for gyro
        if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
        {
            SetState(CrossParkingState::Turning);
        }
    }
}

void CrossParking::CrossParkingState_Turning(const tWorldModel &model)
{
    if (!isStateInitialized)
    {
        yawGoal = DriveUtils::CalculateExpectedYaw(model.Imu.tYaw, Direction::RIGHT, true, turnAngle);

        isStateInitialized = true;
    }

    speedControlTaken = true;
    speed = -turnSpeed;
    steeringControlTaken = true;
    curveRadius = turnCurveRadius;
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

    if (!IsBackOfCarFree(model, 25, -5))
    {
        StopOnObstacle(model);
        return;
    }

#ifndef NDEBUG
    Point rookPoints[1][turnAngle];
    tFloat32 radius = turnCurveRadius * 100; // in cm
    for (int i = 0; i < turnAngle; ++i)
    {
        int x = (int) round((200 + radius) - (cos(i * M_PI / 180.0) * 100.0));
        int y = (int) round(260 + (sin(i * M_PI / 180.0) * 100.0));
        rookPoints[0][i] = Point(x, y);
    }

    const Point *ppt[1] = {rookPoints[0]};
    int npt[] = {turnAngle};
    cv::polylines(debugImage, ppt, npt, 1, false, Scalar(0, 0, 255, 127));
#endif

    if (GeneralUtils::Equals(model.Imu.tYaw, yawGoal, 5) && model.Imu.tYaw > yawGoal)
    {
        speed = 0;
        curveRadius = INT_MAX;

        // wait for gyro
        if (GeneralUtils::Equals(lastYaw, model.Imu.tYaw, 0.01))
        {
            SetState(CrossParkingState::Parking);
        }
    }
}

void CrossParking::CrossParkingState_Parking(const tWorldModel &model)
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

    tFloat32 rearDistance = MapChecker::GetFreeRearCenterDistance(model.ObstacleMap);

    if (fabsf(drivenDistanceCurrentState) > parkingDistance || rearDistance < rearFreeDistanceParking)
    {
        SetState(CrossParkingState::Stopped);
    }
}

void CrossParking::CrossParkingState_Stopped(const tWorldModel &model)
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
        SetState(CrossParkingState::Wait);
    }
    else
    {
        SetState(CrossParkingState::ManeuverCompleted);
    }
}

void CrossParking::CrossParkingState_Wait(const tWorldModel &model)
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
        SetState(CrossParkingState::ManeuverCompleted);
    }
}

void CrossParking::CrossParkingState_ManeuverCompleted(const tWorldModel &model)
{
    logger.Log("Maneuver completed", false);

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

    maneuverCompleted = true;

    SetState(CrossParkingState::Done);
}

void CrossParking::CrossParkingState_Done(const tWorldModel &model)
{
    ResetDriveInstructions(sourceModule);

    if (tManeuver::M_PARK_CROSS != model.Maneuver)
    {
        logger.Log("Going back to idle", false);

        SetState(CrossParkingState::Idle);
    }
}

void CrossParking::CrossParkingState_Error(const tWorldModel &model)
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
}

void CrossParking::StopOnObstacle(const tWorldModel &model)
{
    speed = 0;
    elapsedTime += model.Interval;

    if (elapsedTime > 5000)
    {
        logger.Log("The obstacle doesn't move!", false);
        SetState(CrossParkingState::Error);
    }
}

void CrossParking::SetState(const CrossParkingState::CrossParkingStateEnum newState)
{
    if (newState == state)
    {
        return;
    }

    logger.Log(cString::Format("Switching to state %s", CrossParkingState::ToString(newState).c_str()).GetPtr(), false);
    state = newState;
    isStateInitialized = false;

    drivenDistanceCurrentState = 0;
    stateDetectionCount = 0;
    elapsedTime = 0;

    if (CrossParkingState::Idle == newState)
    {
        ResetDriveInstructions(sourceModule);
        carDetectionCount = 0;
    }

    if (CrossParkingState::FirstVehicleDetected == newState)
    {
        ++carDetectionCount;
    }
}
