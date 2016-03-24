/**
 * @author pbachmann
 */

#include "Turn.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, Turn)

Turn::Turn(const tChar *__info) : BaseDecisionModule(__info, FILTER_NAME, DM_TURN)
{
    SetState(TurnState::Idle); //Standard Mode

    obstacleFreeTime = 0;
    trafficFreeTime = 0;

    laneResetTriggered = false;
    laneReseted = false;

    markerEvaluatorResetTriggered = false;
    markerDetectionResetTriggered = false;
    roadSignReseted = false;
}

Turn::~Turn()
{
}

tResult Turn::Init(tInitStage eStage, __exception)
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

#ifndef NDEBUG
    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(debugVideoOutput.Create("Debug_Video_Output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
        RETURN_IF_FAILED(RegisterPin(&debugVideoOutput));
    }
    else if (eStage == StageNormal)
    {
    }
    else if (eStage == StageGraphReady)
    {
        debugVideoFormat.nWidth = 400;
        debugVideoFormat.nHeight = 400;
        debugVideoFormat.nBitsPerPixel = 24;
        debugVideoFormat.nPixelFormat = cImage::PF_RGB_888;
        debugVideoFormat.nBytesPerLine = 3 * debugVideoFormat.nWidth;
        debugVideoFormat.nSize = debugVideoFormat.nBytesPerLine * debugVideoFormat.nHeight;
        debugVideoFormat.nPaletteSize = 0;

        debugVideoOutput.SetFormat(&debugVideoFormat, NULL);
        logger.Log(cString::Format("Debug Video format: %d x %d @ %d Bit", debugVideoFormat.nWidth, debugVideoFormat.nHeight,
                                   debugVideoFormat.nBitsPerPixel).GetPtr(), false);
    }
#endif

    RETURN_NOERROR;
}

tResult Turn::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Start(__exception_ptr));

    turnSpeed = 1.6;
    nearingSpeed = 1.3;
    leftTurnRadius = 1.2; // Left turn radius in m
    leftTurnAngle = 85;
    rightTurnRadius = 1; // Right turn radius in m
    rightTurnAngle = 90;
    signSizeSlowDown = 200; // Sign Size threshold to slow down
    signSizeStop = 700; // Sign Size threshold to stop

    CarConfigReader config;
    worldService->Pull(WORLD_CARCONFIG, config);

    logger.Log("Reading properties", false);

    ReadConfigValue(config, TURN_CONFIG_GROUP, TURN_SPEED_PROPERTY, turnSpeed);
    ReadConfigValue(config, TURN_CONFIG_GROUP, NEARING_SPEED_PROPERTY, nearingSpeed);
    ReadConfigValue(config, TURN_CONFIG_GROUP, LEFT_TURN_RADIUS_PROPERTY, leftTurnRadius);
    ReadConfigValue(config, TURN_CONFIG_GROUP, LEFT_TURN_ANGLE_PROPERTY, leftTurnAngle);
    ReadConfigValue(config, TURN_CONFIG_GROUP, RIGHT_TURN_RADIUS_PROPERTY, rightTurnRadius);
    ReadConfigValue(config, TURN_CONFIG_GROUP, RIGHT_TURN_ANGLE_PROPERTY, rightTurnAngle);
    ReadConfigValue(config, TURN_CONFIG_GROUP, SIGN_SIZE_SLOW_DOWN_PROPERTY, signSizeSlowDown);
    ReadConfigValue(config, TURN_CONFIG_GROUP, SIGN_SIZE_STOP_PROPERTY, signSizeStop);

    RETURN_NOERROR;
}

tResult Turn::OnTrigger(tFloat32 interval)
{
    tWorldModel worldModel;
    RETURN_IF_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, worldModel.CarState));

    if (tCarState::Running != worldModel.CarState)
    {
        SetState(TurnState::Idle);
        ResetDriveInstructions(sourceModule);

        RETURN_NOERROR;
    }

    RETURN_IF_FAILED(worldService->Pull<tManeuver::ManeuverEnum>(WORLD_CURRENT_MANEUVER, worldModel.Maneuver));
    RETURN_IF_FAILED(worldService->Pull<tRoadSign::RoadSignEnum>(WORLD_CURRENT_ROAD_SIGN, worldModel.RoadSign));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_CURRENT_ROAD_SIGN_SIZE, worldModel.RoadSignSize));
    RETURN_IF_FAILED(worldService->Pull<tIMU>(WORLD_IMU, worldModel.Imu));
    RETURN_IF_FAILED(worldService->Pull<Mat>(WORLD_OBSTACLE_MAT, worldModel.ObstacleMap));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_DISTANCE_OVERALL, worldModel.DistanceOverall));
    RETURN_IF_FAILED(worldService->Pull<tLane>(WORLD_LANE, worldModel.Lane));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_CAR_SPEED, worldModel.CurrentSpeed));
    RETURN_IF_FAILED(worldService->Pull<vector<tReadyModule::ReadyModuleEnum> >(WORLD_READY_MODULES, worldModel.ReadyModules));

    worldModel.Interval = interval;

    worldModel.RoadSignSize = NormalizeSignSize(worldModel.RoadSignSize);

#ifndef NDEBUG
    debugImage = worldModel.ObstacleMap.clone();
    cvtColor(debugImage, debugImage, CV_GRAY2RGB);
#endif

    switch (currentTurnState)
    {

        case TurnState::Idle:
            TurnState_Idle(worldModel);
            break;
        case TurnState::Nearing:
            TurnState_Nearing(worldModel);
            break;
        case TurnState::Stopping:
            TurnState_Stopping(worldModel);
            break;
        case TurnState::CheckStraight:
            TurnState_CheckStraight(worldModel);
            break;
        case TurnState::CheckLeft:
            TurnState_CheckLeft(worldModel);
            break;
        case TurnState::CheckRight:
            TurnState_CheckRight(worldModel);
            break;
        case TurnState::TurnLeft:
            TurnState_TurnLeft(worldModel);
            break;
        case TurnState::TurnRight:
            TurnState_TurnRight(worldModel);
            break;
        case TurnState::GoStraight:
            TurnState_GoStraight(worldModel);
            break;
        case TurnState::Error:
            TurnState_Error();
            break;
        case TurnState::Done:
            TurnState_Done(worldModel);
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

    if (worldModel.Lane.tRightLine.tStatus == tVISIBLE)
    {
        cv::line(debugImage, rightBot, rightTop, Scalar(255, 0, 0, 127));
    }

    if (worldModel.Lane.tLeftLine.tStatus == tVISIBLE)
    {
        cv::line(debugImage, leftBot, leftTop, Scalar(255, 0, 0, 127));
    }

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

void Turn::SetState(TurnState::TurnStateEnum newState)
{
    logger.Log(cString::Format("Switching to state %d", newState).GetPtr(), false);
    currentTurnState = newState;
}

float Turn::CalculateRoadSignSpeed(tWorldModel worldModel)
{
    if (worldModel.RoadSign != tRoadSign::GIVE_WAY &&
        worldModel.RoadSign != tRoadSign::HAVE_WAY &&
        worldModel.RoadSign != tRoadSign::STOP_AND_GIVE_WAY &&
        worldModel.RoadSign != tRoadSign::UNMARKED_INTERSECTION)
    {
        return 100;
    }

    if (worldModel.RoadSignSize < signSizeSlowDown)
    {
        return 100;
    }

    if (worldModel.RoadSignSize < signSizeStop)
    {
        return nearingSpeed;
    }

    return 0;
}

float Turn::CalculateLaneSpeed(tWorldModel worldModel)
{
    return min(CalculateLineSpeed(worldModel.Lane.tLeftLine), CalculateLineSpeed(worldModel.Lane.tRightLine));
}

float Turn::CalculateLineSpeed(tLine line)
{
    if (line.tCrossingDistance >= 150)
    {
        return 100;
    }

    if (line.tCrossingDistance > 10)
    {
        return nearingSpeed;
    }

    return 0;
}

void Turn::TurnState_Idle(tWorldModel worldModel)
{
    ResetDriveInstructions(sourceModule);

    if (tManeuver::M_CROSSING_LEFT != worldModel.Maneuver &&
        tManeuver::M_CROSSING_RIGHT != worldModel.Maneuver &&
        tManeuver::M_CROSSING_STRAIGHT != worldModel.Maneuver)
    {
        return;
    }

    if (worldModel.RoadSign == tRoadSign::GIVE_WAY ||
        worldModel.RoadSign == tRoadSign::HAVE_WAY ||
        worldModel.RoadSign == tRoadSign::STOP_AND_GIVE_WAY ||
        worldModel.RoadSign == tRoadSign::UNMARKED_INTERSECTION)
    {
        logger.Log("Found Sign!", false);

        SetState(TurnState::Nearing);

        return;
    }

    if (worldModel.Lane.tLeftLine.tStatus == tCROSSING || worldModel.Lane.tRightLine.tStatus == tCROSSING)
    {
        logger.Log("Found Crossing!", false);

        SetState(TurnState::Nearing);
    }
}

void Turn::TurnState_Nearing(tWorldModel worldModel)
{
    speedControlTaken = false;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    float roadSignSpeed = CalculateRoadSignSpeed(worldModel);
    float laneSpeed = CalculateLaneSpeed(worldModel);

    if (min(worldModel.Lane.tLeftLine.tCrossingDistance,
            worldModel.Lane.tRightLine.tCrossingDistance) < 0)
    {
        logger.Log("Mysterious Crossing Distance!", false);

        logger.Log("Right:", false);
        logger.Log(cString::Format("Start: x: %f, y: %f", worldModel.Lane.tRightLine.tStart.tX, worldModel.Lane.tRightLine.tStart.tY).GetPtr(),
                   false);
        logger.Log(cString::Format("End: x: %f, y: %f", worldModel.Lane.tRightLine.tEnd.tX, worldModel.Lane.tRightLine.tEnd.tY).GetPtr(), false);
        logger.Log(cString::Format("Distance: %d", worldModel.Lane.tRightLine.tCrossingDistance).GetPtr(), false);
        logger.Log(cString::Format("Mode: %d", worldModel.Lane.tRightLine.tStatus).GetPtr(), false);

        logger.Log("Left:", false);
        logger.Log(cString::Format("Start: x: %f, y: %f", worldModel.Lane.tLeftLine.tStart.tX, worldModel.Lane.tLeftLine.tStart.tY).GetPtr(), false);
        logger.Log(cString::Format("End: x: %f, y: %f", worldModel.Lane.tLeftLine.tEnd.tX, worldModel.Lane.tLeftLine.tEnd.tY).GetPtr(), false);
        logger.Log(cString::Format("Distance: %d", worldModel.Lane.tLeftLine.tCrossingDistance).GetPtr(), false);
        logger.Log(cString::Format("Mode: %d", worldModel.Lane.tLeftLine.tStatus).GetPtr(), false);

    }

    speed = min(roadSignSpeed, laneSpeed);

    if (speed > 0)
    {
        logger.Log(cString::Format("Road Sign Size: %f", worldModel.RoadSignSize).GetPtr());
        logger.Log(cString::Format("Lane Crossing Distance: %d", min(worldModel.Lane.tLeftLine.tCrossingDistance,
                                                                     worldModel.Lane.tRightLine.tCrossingDistance)).GetPtr());

        logger.Log(cString::Format("Road Sign Speed: %f", roadSignSpeed).GetPtr());
        logger.Log(cString::Format("Lane Speed: %f", laneSpeed).GetPtr());
    }

    if (speed <= nearingSpeed)
    {
        speedControlTaken = true;
        brakeLightEnabled = true;

        SetSignalLights(worldModel);

        if (ObstacleInFront(worldModel))
        {
            speed = 0;
            return;
        }
    }

    if (speed == 0)
    {
        logger.Log(cString::Format("Road Sign Size: %f", worldModel.RoadSignSize).GetPtr(), false);
        logger.Log(cString::Format("Lane Crossing Distance: %d", min(worldModel.Lane.tLeftLine.tCrossingDistance,
                                                                     worldModel.Lane.tRightLine.tCrossingDistance)).GetPtr(), false);

        logger.Log(cString::Format("Road Sign Speed: %f", roadSignSpeed).GetPtr(), false);
        logger.Log(cString::Format("Lane Speed: %f", laneSpeed).GetPtr(), false);

        if (worldModel.RoadSign == tRoadSign::HAVE_WAY && worldModel.Maneuver == tManeuver::M_CROSSING_STRAIGHT)
        {
            logger.Log("Going Straight instantly.", false);

            distanceGoal = worldModel.DistanceOverall + 1.5f;
            SetState(TurnState::GoStraight);
            return;
        }

        SetState(TurnState::Stopping);
    }
}

void Turn::TurnState_Stopping(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveAngle = 90;
    curveRadius = 0;

    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    SetSignalLights(worldModel);

    if (worldModel.CurrentSpeed > 0.1)
    {
        return;
    }

    obstacleFreeTime = 0;
    trafficFreeTime = 0;

    if (tManeuver::M_CROSSING_LEFT == worldModel.Maneuver)
    {
        SetState(TurnState::CheckLeft);
    }
    else if (tManeuver::M_CROSSING_RIGHT == worldModel.Maneuver)
    {
        SetState(TurnState::CheckRight);
    }
    else
    {
        SetState(TurnState::CheckStraight);
    }
}

void Turn::SetSignalLights(tWorldModel worldModel)
{
    if (tManeuver::M_CROSSING_LEFT == worldModel.Maneuver)
    {
        turnSignalLeftEnabled = true;
    }
    else if (tManeuver::M_CROSSING_RIGHT == worldModel.Maneuver)
    {
        turnSignalRightEnabled = true;
    }
}

void Turn::TurnState_CheckLeft(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveAngle = 90;
    curveRadius = 0;

    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = true;
    turnSignalRightEnabled = false;

    trafficFreeTime += worldModel.Interval;

    if (MapChecker::IsCurveFree(worldModel.ObstacleMap, -(leftTurnRadius * 100), Direction::LEFT))
    {
        obstacleFreeTime += worldModel.Interval;
    }
    else
    {
        logger.Log("Obstacle when checking.");
        obstacleFreeTime = 0;
    }

    if (trafficFreeTime < TrafficWaitTime && Traffic(worldModel))
    {
        logger.Log("Traffic when checking.");

        obstacleFreeTime = 0;
    }

    if (obstacleFreeTime < ObstacleWaitTime)
    {
        return;
    }

    yawGoal = DriveUtils::CalculateExpectedYaw(worldModel.Imu.tYaw, Direction::LEFT, false, leftTurnAngle);
    SetState(TurnState::TurnLeft);
}

void Turn::TurnState_CheckRight(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveAngle = 90;
    curveRadius = 0;

    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    trafficFreeTime += worldModel.Interval;

    if (MapChecker::IsCurveFree(worldModel.ObstacleMap, rightTurnRadius * 100, Direction::RIGHT))
    {
        obstacleFreeTime += worldModel.Interval;
    }
    else
    {
        logger.Log("Obstacle when checking.");
        obstacleFreeTime = 0;
    }

    if (trafficFreeTime < TrafficWaitTime && Traffic(worldModel))
    {
        logger.Log("Traffic when checking.");

        obstacleFreeTime = 0;
    }

    if (obstacleFreeTime < ObstacleWaitTime)
    {
        return;
    }

    yawGoal = DriveUtils::CalculateExpectedYaw(worldModel.Imu.tYaw, Direction::RIGHT, false, leftTurnAngle);
    SetState(TurnState::TurnRight);
}

void Turn::TurnState_CheckStraight(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = true;
    curveAngle = 90;
    curveRadius = 0;

    brakeLightEnabled = true;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    Point2f worldLeftUp(-15, 100);
    Point2f worldRightDown(15, 0);

    Point2f imageLeftUp = VisionUtils::WorldToImage(worldLeftUp);
    Point2f imageRightDown = VisionUtils::WorldToImage(worldRightDown);

    trafficFreeTime += worldModel.Interval;

    if (MapChecker::IsAreaFree(worldModel.ObstacleMap, Rect(imageLeftUp, imageRightDown)))
    {
        obstacleFreeTime += worldModel.Interval;
    }
    else
    {
        logger.Log("Obstacle when checking.");
        obstacleFreeTime = 0;
    }

    if (trafficFreeTime < TrafficWaitTime && Traffic(worldModel))
    {
        logger.Log("Traffic when checking.");

        obstacleFreeTime = 0;
    }

    if (obstacleFreeTime < ObstacleWaitTime)
    {
        return;
    }

    distanceGoal = worldModel.DistanceOverall + 1.5f;
    SetState(TurnState::GoStraight);
}

void Turn::TurnState_TurnLeft(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = turnSpeed;
    steeringControlTaken = true;
    curveRadius = -leftTurnRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = true;
    turnSignalRightEnabled = false;

    if (ObstacleInFront(worldModel))
    {
        logger.Log("Obstacle detected!");

        speed = 0;
        brakeLightEnabled = true;
        return;
    }


    if (GeneralUtils::Equals(worldModel.Imu.tYaw, yawGoal, 5) && worldModel.Imu.tYaw > yawGoal)
    {
        laneResetTriggered = false;
        laneReseted = false;

        markerEvaluatorResetTriggered = false;
        markerDetectionResetTriggered = false;
        roadSignReseted = false;

        SetState(TurnState::Done);
        return;
    }
}

void Turn::TurnState_TurnRight(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = turnSpeed;
    steeringControlTaken = true;
    curveRadius = rightTurnRadius;
    curveAngle = 0;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = true;

    if (ObstacleInFront(worldModel))
    {
        logger.Log("Obstacle detected!");

        speed = 0;
        brakeLightEnabled = true;
        return;
    }

    if (GeneralUtils::Equals(worldModel.Imu.tYaw, yawGoal, 5) && worldModel.Imu.tYaw < yawGoal)
    {
        laneResetTriggered = false;
        laneReseted = false;

        markerEvaluatorResetTriggered = false;
        markerDetectionResetTriggered = false;
        roadSignReseted = false;

        SetState(TurnState::Done);
        return;
    }
}

void Turn::TurnState_GoStraight(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = nearingSpeed;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    if (ObstacleInFront(worldModel))
    {
        logger.Log("Obstacle detected!");

        speed = 0;
        brakeLightEnabled = true;
        return;
    }

    if (worldModel.DistanceOverall > distanceGoal)
    {
        laneResetTriggered = false;
        laneReseted = false;

        markerEvaluatorResetTriggered = false;
        markerDetectionResetTriggered = false;
        roadSignReseted = false;

        SetState(TurnState::Done);
        return;
    }
}

void Turn::TurnState_Error()
{
    speedControlTaken = true;
    speed = 0;
    steeringControlTaken = false;

    brakeLightEnabled = true;
    hazardLightsEnabled = true;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    errorActive = true;
}

void Turn::TurnState_Done(tWorldModel worldModel)
{
    speedControlTaken = true;
    speed = worldModel.CurrentSpeed;
    steeringControlTaken = false;

    brakeLightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

//    logger.Log("Content of ReadyModules", false);
//
//    for (unsigned int i = 0; i < worldModel.ReadyModules.size(); i++)
//    {
//        logger.Log(cString::Format("Module: %d", worldModel.ReadyModules[i]).GetPtr(), false);
//    }

    if (!laneResetTriggered)
    {
        if (worldModel.Maneuver != tManeuver::M_CROSSING_STRAIGHT)
        {
            logger.Log("Resetting Lane Detection", false);
            resettableModule = tReadyModule::LaneDetection;
        }
        laneResetTriggered = true;
        return;
    }

    if (!markerEvaluatorResetTriggered)
    {
        logger.Log("Resetting MarkerEvaluator", false);
        resettableModule = tReadyModule::MarkerEvaluator;
        markerEvaluatorResetTriggered = true;
        return;
    }

    if (!markerDetectionResetTriggered)
    {
        logger.Log("Resetting MarkerDetection", false);
        resettableModule = tReadyModule::MarkerDetection;
        markerDetectionResetTriggered = true;
        return;
    }

    resettableModule = tReadyModule::Nothing;

//    if (worldModel.RoadSign == tRoadSign::NO_MATCH)
//    {
//        roadSignReseted = true;
//    }
//    else
//    {
//        logger.Log("Waiting for Road Sign Reset!", false);
//    }
//
//    if (worldModel.Lane.tRightLine.tStatus != tINVISIBLE &&
//        worldModel.Lane.tLeftLine.tStatus != tINVISIBLE)
//    {
//        laneReseted = true;
//    }
//    else
//    {
//        logger.Log("Waiting for Road Reset!", false);
//    }

    // Wait a tick to let reset go through
    if (!roadSignReseted && !laneReseted)
    {
        roadSignReseted = true;
        laneReseted = true;
        return;
    }

    reverseLightEnabled = true;
    speed = -1.3f;

    if (GeneralUtils::Contains<tReadyModule::ReadyModuleEnum>(worldModel.ReadyModules, tReadyModule::MarkerDetection) &&
        GeneralUtils::Contains<tReadyModule::ReadyModuleEnum>(worldModel.ReadyModules, tReadyModule::LaneDetection))
    {
        laneResetTriggered = false;
        laneReseted = false;

        markerEvaluatorResetTriggered = false;
        markerDetectionResetTriggered = false;
        roadSignReseted = false;

        maneuverCompleted = true;
        SetState(TurnState::Idle);
    }
}

tFloat32 Turn::NormalizeSignSize(tFloat32 currentSize)
{
    // Ignore all false Positives
    if (currentSize < signSizeStop + 300)
    {
        lastSignSize = currentSize;
    }

    return lastSignSize;
}

tBool Turn::Traffic(tWorldModel worldModel)
{
    switch (worldModel.Maneuver)
    {

        case tManeuver::M_CROSSING_STRAIGHT:
            if (worldModel.RoadSign == tRoadSign::HAVE_WAY)
            {
                return false;
            }

            if (worldModel.RoadSign == tRoadSign::GIVE_WAY || worldModel.RoadSign == tRoadSign::STOP_AND_GIVE_WAY)
            {
                if (CheckTrafficLeft(worldModel) && CheckTrafficRight(worldModel))
                {
                    return false;
                }
                else
                {
                    logger.Log("Traffic!");
                }
            }

            if (worldModel.RoadSign == tRoadSign::UNMARKED_INTERSECTION)
            {
                if (CheckTrafficRight(worldModel))
                {
                    return false;
                }
                else
                {
                    logger.Log("Traffic!");
                }
            }
            break;
        case tManeuver::M_CROSSING_LEFT:
            if (worldModel.RoadSign == tRoadSign::HAVE_WAY)
            {
                if (checkTrafficOtherLane(worldModel))
                {
                    return false;
                }
                else
                {
                    logger.Log("Traffic!");
                }
            }

            if (worldModel.RoadSign == tRoadSign::GIVE_WAY || worldModel.RoadSign == tRoadSign::STOP_AND_GIVE_WAY)
            {
                if (CheckTrafficLeft(worldModel) && CheckTrafficRight(worldModel) && checkTrafficOtherLane(worldModel))
                {
                    return false;
                }
                else
                {
                    logger.Log("Traffic!");
                }
            }

            if (worldModel.RoadSign == tRoadSign::UNMARKED_INTERSECTION)
            {
                if (CheckTrafficRight(worldModel) && checkTrafficOtherLane(worldModel))
                {
                    return false;
                }
                else
                {
                    logger.Log("Traffic!");
                }
            }
            break;
        case tManeuver::M_CROSSING_RIGHT:
            if (worldModel.RoadSign == tRoadSign::HAVE_WAY)
            {
                return false;
            }

            if (worldModel.RoadSign == tRoadSign::GIVE_WAY || worldModel.RoadSign == tRoadSign::STOP_AND_GIVE_WAY)
            {
                if (CheckTrafficLeft(worldModel))
                {
                    return false;
                }
                else
                {
                    logger.Log("Traffic!");
                }
            }

            if (worldModel.RoadSign == tRoadSign::UNMARKED_INTERSECTION)
            {
                return false;
            }
            break;
        default:
            return false;
    }

    return true;
}

tBool Turn::CheckTrafficLeft(tWorldModel worldModel)
{
    Point2f worldLeftUp(-150, 50);
    Point2f worldRightDown(0, 5);

    Point2f imageLeftUp = VisionUtils::WorldToImage(worldLeftUp);
    Point2f imageRightDown = VisionUtils::WorldToImage(worldRightDown);

#ifndef NDEBUG
    cv::rectangle(debugImage, imageLeftUp, imageRightDown, Scalar(0, 0, 255, 127));
#endif

    return MapChecker::IsAreaFree(worldModel.ObstacleMap, Rect(imageLeftUp, imageRightDown));
}

tBool Turn::CheckTrafficRight(tWorldModel worldModel)
{
    Point2f worldLeftUp(20, 30);
    Point2f worldRightDown(125, 5);

    Point2f imageLeftUp = VisionUtils::WorldToImage(worldLeftUp);
    Point2f imageRightDown = VisionUtils::WorldToImage(worldRightDown);

#ifndef NDEBUG
    cv::rectangle(debugImage, imageLeftUp, imageRightDown, Scalar(0, 0, 255, 127));
#endif

    return MapChecker::IsAreaFree(worldModel.ObstacleMap, Rect(imageLeftUp, imageRightDown));
}

bool Turn::checkTrafficOtherLane(tWorldModel worldModel)
{
    Point2f worldLeftUp(-40, 150);
    Point2f worldRightDown(-20, 0);

    Point2f imageLeftUp = VisionUtils::WorldToImage(worldLeftUp);
    Point2f imageRightDown = VisionUtils::WorldToImage(worldRightDown);

#ifndef NDEBUG
    cv::rectangle(debugImage, imageLeftUp, imageRightDown, Scalar(0, 0, 255, 127));
#endif

    return MapChecker::IsAreaFree(worldModel.ObstacleMap, Rect(imageLeftUp, imageRightDown));
}


bool Turn::ObstacleInFront(tWorldModel model)
{
    Point2f worldLeftUp(-15, 20);
    Point2f worldRightDown(15, 0);

    Point2f imageLeftUp = VisionUtils::WorldToImage(worldLeftUp);
    Point2f imageRightDown = VisionUtils::WorldToImage(worldRightDown);

#ifndef NDEBUG
    cv::rectangle(debugImage, imageLeftUp, imageRightDown, Scalar(0, 0, 255, 127));
#endif

    return !MapChecker::IsAreaFree(model.ObstacleMap, Rect(imageLeftUp, imageRightDown));
}
