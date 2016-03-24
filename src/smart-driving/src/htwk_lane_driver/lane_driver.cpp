#include "lane_driver.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_LANE_DRIVER, LaneDriver)

/**
 *   Contructor. The cFilter contructor needs to be called !!
 *   The SetProperty in the constructor is necessary if somebody wants to deal with
 *   the default values of the properties before init
 *
 */
LaneDriver::LaneDriver(const tChar *__info) : BaseDecisionModule(__info, FILTER_NAME, DM_LANE_DRIVER)
{
}

/**
 *  Destructor. A Filter implementation needs always to have a virtual Destructor
 *              because the IFilter extends an IObject
 *
 */
LaneDriver::~LaneDriver()
{
}

/**
 *   The Filter Init Function.
 *    eInitStage ... StageFirst ... should be used for creating and registering Pins
 *               ... StageNormal .. should be used for reading the properies and initalizing
 *                                  everything before pin connections are made
 *   see {@link IFilter#Init IFilter::Init}.
 *
 */
tResult LaneDriver::Init(tInitStage eStage, __exception)
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

tResult LaneDriver::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Start(__exception_ptr));

    carWidth = 15; // Width of car in cm
    maxEndDisplacement = 400; // Maximum Displacement of Lane Ends in Pixel
    maxStartDisplacement = 20; // Maximum Displacement of Lane Starts in Pixel
    laneWidth = 45; // Lane Width in cm
    safetySpace = 8; // Safety Space between outer lane line and car in cm
    curveSpeed = 1.5f;
    straightSpeed = 1.5f;
    useLeft = true; // Use Left Line?
    useRight = true; // Use Right Line?

    CarConfigReader config;
    worldService->Pull(WORLD_CARCONFIG, config);

    logger.Log("Reading properties", false);

    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, CAR_WIDTH_PROPERTY, carWidth);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, MAX_END_DISPLACEMENT_PROPERTY, maxEndDisplacement);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, MAX_START_DISPLACEMENT_PROPERTY, maxStartDisplacement);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, LANE_WIDTH_PROPERTY, laneWidth);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, SAFETY_SPACE_PROPERTY, safetySpace);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, CURVE_SPEED_PROPERTY, curveSpeed);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, STRAIGHT_SPEED_PROPERTY, straightSpeed);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, USE_LEFT_PROPERTY, useLeft);
    ReadConfigValue(config, LANE_DRIVER_CONFIG_GROUP, USE_RIGHT_PROPERTY, useRight);

    RETURN_NOERROR;
}

tResult LaneDriver::OnTrigger(tFloat32 interval)
{
    tWorldModel worldModel;
    RETURN_IF_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, worldModel.CarState));

    if (tCarState::Running != worldModel.CarState)
    {
        ResetDriveInstructions(sourceModule);

        RETURN_NOERROR;
    }

    RETURN_IF_FAILED(worldService->Pull<tLane>(WORLD_LANE, worldModel.Lane));

    ProcessLane(worldModel.Lane);

    RETURN_NOERROR;
}

/**
 * Processes on mediasample. The method only shows an example, how to deal with
 * a mediasample that is received, to get the specific data.
 * Further it shows, how to create a new mediasample in the system pool, to set the
 * data and to transmit it over a pin.
 *
 * @param   mediaSample   [in]  The received sample.
 *
 * @return  Standard result code.
 */
void LaneDriver::ProcessLane(tLane lane)
{
    speedControlTaken = true;
    steeringControlTaken = true;
    curveRadius = 0;

    hazardLightsEnabled = false;
    headLightEnabled = true;
    reverseLightEnabled = false;
    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;

    curveAngle = CalculateLaneAngle(lane);
    speed = max(CalculateLineSpeed(lane.tRightLine), CalculateLineSpeed(lane.tLeftLine));

    bool isEmergency = speed == 0;
    hazardLightsEnabled = isEmergency;
    brakeLightEnabled = isEmergency;
}

float LaneDriver::CalculateLaneAngle(const tLane &lane)
{
    float rightAngle = useRight ? CalculateLineAngle(lane.tRightLine, carWidth + safetySpace) : 0;
    float leftAngle = useLeft ? CalculateLineAngle(lane.tLeftLine, -(carWidth + safetySpace + laneWidth)) : 0;

    float currentAngle = 90;

    if (rightAngle == 0)
    {
        currentAngle += leftAngle;
    }
    else if (leftAngle == 0)
    {
        currentAngle += rightAngle;
    }
    else
    {
        currentAngle += (leftAngle + rightAngle) / 2.0f;
    }

    if (currentAngle > 120)
    {
        currentAngle = 120;
    }

    if (currentAngle < 60)
    {
        currentAngle = 60;
    }
    return currentAngle;
}

float LaneDriver::CalculateLineAngle(const tLine &line, float limit)
{
    float averageAngle = 0;

    if (line.tStatus == tVISIBLE)
    {
        // Displacement to steer
        float endDisplacement = line.tEnd.tX - limit;
        if (endDisplacement < 0)
        {
            endDisplacement -= 200 - line.tEnd.tY;
        }
        else
        {
            endDisplacement += 200 - line.tEnd.tY;
        }
        averageAngle = endDisplacement / maxEndDisplacement * 30;

        float startDisplacement = line.tStart.tX - limit;
        averageAngle += startDisplacement / maxStartDisplacement * 30;

//        logger.Log(cString::Format("Right Start Steering Angle: %f", 90 + startDisplacement / MaxStartDisplacement * 30).GetPtr());
//        logger.Log(cString::Format("Right End Steering Angle: %f", 90 + endDisplacement / MaxEndDisplacement * 30).GetPtr());

        averageAngle /= 2.0;
    }

    return averageAngle;
}

tFloat32 LaneDriver::CalculateLineSpeed(const tLine &line)
{
    if (line.tStatus == tINVISIBLE)
    {
        return 0;
    }

    float delta = abs(line.tStart.tX - line.tEnd.tX);

    if (delta > 20)
    {
        return curveSpeed;
    }
    else
    {
        return straightSpeed;
    }
}


