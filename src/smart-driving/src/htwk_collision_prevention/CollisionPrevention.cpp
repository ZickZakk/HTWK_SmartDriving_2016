//
// Created by mwinkler on 2/16/16.
//

#include "CollisionPrevention.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, CollisionPrevention)

CollisionPrevention::CollisionPrevention(const tChar *__info) : BaseDecisionModule(__info, FILTER_NAME, DM_COLL_PREV)
{
    didStopEmergency = false;
    followCalcTimeInterval = 0;
}

CollisionPrevention::~CollisionPrevention()
{
}

tResult  CollisionPrevention::Init(tInitStage eStage, IException **__exception_ptr)
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

tResult CollisionPrevention::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Start(__exception_ptr));

    speedFactor = 5.0; // Factor for taking speed into account
    followZoneLength = 50; // Length of the Follow zone
    followEnable = false; // Enable Follow

    CarConfigReader config;
    worldService->Pull(WORLD_CARCONFIG, config);

    logger.Log("Reading properties", false);

    ReadConfigValue(config, COLL_PREV_CONFIG_GROUP, SPEED_FACTOR_PROPERTY, speedFactor);
    ReadConfigValue(config, COLL_PREV_CONFIG_GROUP, FOLLOW_ZONE_PROPERTY, followZoneLength);
    ReadConfigValue(config, COLL_PREV_CONFIG_GROUP, FOLLOW_ENABLE_PROPERTY, followEnable);

    RETURN_NOERROR;
}

void CollisionPrevention::CheckMapForObstacle(tWorldModel worldModel)
{
    bool isFree = CheckObstacleMapFrontFree(worldModel.ObstacleMap);

    // obstacle is in the way -> brake
    if (!isFree && worldModel.CurrentSpeed > 0 && !didStopEmergency)
    {
        speedControlTaken = true;
        speed = 0;
        brakeLightEnabled = true;
        speedControlTaken = true;

        didStopEmergency = tTrue;
    }
        // normal driving -> check for vehicle to follow
    else if (isFree && !didStopEmergency)
    {
        brakeLightEnabled = false;
        speedControlTaken = false;

        if (followEnable)
        {
            CheckMapForMovingObstacle(worldModel);
        }
    }
        // after emergency brake, wait at least 2 sec
    else if (isFree && didStopEmergency)
    {
        frameBuffer++;

        if (frameBuffer > 120)
        {
            speedControlTaken = false;

            brakeLightEnabled = false;
            hazardLightsEnabled = false;

            didStopEmergency = false;
            frameBuffer = 0;
        }
    }
    // !isFree && didStopEmergency -> passing
}

tBool CollisionPrevention::CheckObstacleMapFrontFree(const Mat &map)
{
    int frontDistance = emergencyZoneLength;
    Point2f world1(-15, frontDistance);     // 15 left; 20 front
    Point2f world2(15, 0);                  // 15 right; 0 front    => Rect

    Point2f topLeft = VisionUtils::WorldToImage(world1);
    Point2f botRight = VisionUtils::WorldToImage(world2);

    return MapChecker::IsAreaFree(map, Rect(topLeft, botRight));
}

void CollisionPrevention::CheckMapForMovingObstacle(tWorldModel worldModel)
{
    // smaller, but further away area to check
    // takes distance over 20 frames (0.2 sec)
    // compares actual distance change with theoretical distance change
    // either follow object or surpass it, if it's speed is zero
    // emergency brake only if spontanues obstacle

    // Follow: emergency area is clear, but Follow area not and obstacle in follow area is moving

    int frontDistance = emergencyZoneLength + 5 + followZoneLength;
    if (frontDistance > 150)
    {
        frontDistance = 150;
    }    // max value so roi is not out of our map

    Point2f world1(-10, frontDistance);     // 10 left; 20 front
    Point2f world2(10, emergencyZoneLength + 5);           // 10 right; 0 front    => Rect

    Point2f topLeft = VisionUtils::WorldToImage(world1);
    Point2f rightBot = VisionUtils::WorldToImage(world2);

    Mat roi(worldModel.ObstacleMap, Rect(topLeft, rightBot));

    int dist = followZoneLength;

    for (int i = followZoneLength; i >= 1; --i) // from bottom to top
    {
        int numberZeros = cv::countNonZero(roi.row(i - 1));
        if (numberZeros != 0)
        {
            //logger.Log(cString::Format("nr Zeros : %d in line: %d", numberZeros, i).GetPtr());
            dist = followZoneLength - i;
            break;
        }
    }

    logger.Log(cString::Format("Distance: %d", dist).GetPtr(), false);
    if (dist == followZoneLength && !didStopEmergency) // all clear
    {
        FollowReset(true);
    }
    else if (dist < followZoneLength && !didStopEmergency) // obstacle somewhere in the roi
    {
        if (distances.size() > 0)
        {
            logger.Log(cString::Format("Distance: %d Last: %d", dist, distances.back()).GetPtr(), false);
            if (dist > distances.back())
            {
                logger.Log("Reset, obj to fast", false);
                FollowReset(false); // obj in front was faster than we are
            }
        }
        distances.push_back(dist);
        distanceBuffer++;

        if (distanceBuffer >= 10) // (followZoneLength / 2)
        {
            tFloat32 followVechicleSpeed = CompareDistances(
                    worldModel.CurrentSpeed);
            {
                speed = followVechicleSpeed;
                brakeLightEnabled = tTrue;
                speedControlTaken = tTrue;
            }

            FollowReset(false);
        }
    }
    else if (didStopEmergency)
    {
        FollowReset(false);
    }
}

tFloat32 CollisionPrevention::CompareDistances(tFloat32 currentSpeed)
{
    // iterate trough vector to get vehicle speed;
    //tFloat32 changePerFrame = abs(distances.at(0) - distances.at(1));
    //auto it = distances.begin()+1; it != distances.end()-1; ++it)
    /*for (std::vector<tInt>::size_type i = 1; i != distances.size() - 1; i++)
    {
        logger.Log(cString::Format("Change per frame: %f", changePerFrame).GetPtr());
        changePerFrame = (abs(distances[i] - distances[i + 1]) + changePerFrame) / 2;
    }*/

    float framesPerSecond = 30.0f;

    tFloat32 changePerFrame = abs(distances.back() - distances.front()) / static_cast<float>(distances.size());
    tFloat32 relObjectSpeed = changePerFrame * framesPerSecond / 100;

    tFloat32 changePerTime_MperS = (abs(distances.back() - distances.front()) / 100.0) / (followCalcTimeInterval / 1000.0);
    tFloat32 objectSpeed = currentSpeed - changePerTime_MperS; // relObjectSpeed;

    tFloat32 ourSpeedPerFrame = (currentSpeed * 100.0f / framesPerSecond);
    logger.Log(cString::Format("Speed (%f) myFS: %f; objFS: %f\nobjRelSpeed: %f, objSpeed: %f, obj_MperS: %f",
                               currentSpeed, ourSpeedPerFrame, changePerFrame, relObjectSpeed, objectSpeed, changePerTime_MperS).GetPtr(), false);

    if (objectSpeed > currentSpeed * 0.5) //curretnSpeed - objectSpeed > 0.5)
    {
        return objectSpeed;
        //(ourSpeedPerFrame - changePerFrame) * framesPerSecond / 100.0;
    }
    else if (objectSpeed >= currentSpeed * 0.98) // obj is faster, relative to us
    {  // prob some immovable object
        return currentSpeed * 0.75;
    }
    else
    {
        return currentSpeed;
    }
}

void CollisionPrevention::FollowReset(bool andControl)
{
    distanceBuffer = 0;
    distances.clear();

    followCalcTimeInterval = 0;

    if (andControl)
    {
        brakeLightEnabled = tFalse;
        speedControlTaken = tFalse;
    }
}

tResult CollisionPrevention::OnTrigger(tFloat32 interval)
{
    tWorldModel worldModel;
    RETURN_IF_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, worldModel.CarState));

    if (tCarState::Running != worldModel.CarState)
    {
        ResetDriveInstructions(sourceModule);

        RETURN_NOERROR;
    }

    RETURN_IF_FAILED(worldService->Pull<Mat>(WORLD_OBSTACLE_MAT, worldModel.ObstacleMap));
    RETURN_IF_FAILED(worldService->Pull<tFloat32>(WORLD_CAR_SPEED, worldModel.CurrentSpeed));

    followCalcTimeInterval += interval;
    emergencyZoneLength = 20 + cvFloor(powf(worldModel.CurrentSpeed, 2) * speedFactor);

    CheckMapForObstacle(worldModel);

    RETURN_NOERROR;
}
