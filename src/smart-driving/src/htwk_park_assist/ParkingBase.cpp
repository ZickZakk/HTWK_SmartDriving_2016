//
// Created by pbachmann on 2/11/16.
//

#include "ParkingBase.h"

ParkingBase::ParkingBase(const tChar *__info, string moduleName, int driveModule) : BaseDecisionModule(__info, moduleName, driveModule)
{
    elapsedTime = 0;
    isStateInitialized = false;

    lastYaw = 0;
    yawGoal = 0;
}

ParkingBase::~ParkingBase()
{
}

tResult ParkingBase::Init(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Init(eStage, __exception_ptr));

    if (StageFirst == eStage)
    {
    }
    else if (StageNormal == eStage)
    {
    }
    else if (StageGraphReady == eStage)
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

void ParkingBase::ReadConfiguration(CarConfigReader &config, string groupName)
{
    ReadConfigValue(config, groupName, DRIVE_SPEED_PROPERTY, driveSpeed);
    ReadConfigValue(config, groupName, REVERSE_DRIVE_SPEED_PROPERTY, reverseDriveSpeed);
    ReadConfigValue(config, groupName, TURN_SPEED_PROPERTY, turnSpeed);
    ReadConfigValue(config, groupName, WAIT_TIME_PROPERTY, waitTime);
    ReadConfigValue(config, groupName, STRAIGHT_DRIVE_OFFSET_PROPERTY, straightDriveOffset);
    ReadConfigValue(config, groupName, STRAIGHT_DRIVE_RADIUS_PROPERTY, straightDriveRadius);
    ReadConfigValue(config, groupName, ULTRASONIC_SAMPLES_PROPERTY, ultraSonicSamples);

    ReadConfigValue(config, groupName, DRIVE_SPEED_PROPERTY, driveSpeed);
}

tResult ParkingBase::Start(IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Start(__exception_ptr));

    // property default values
    driveSpeed = 1.2;
    reverseDriveSpeed = 1.4;
    turnSpeed = 1.6;
    waitTime = 0;
    straightDriveRadius = 2.5;
    straightDriveOffset = 0.08;
    ultraSonicSamples = 15;

    RETURN_NOERROR;
}

tResult ParkingBase::Stop(IException **__exception_ptr)
{
    return BaseDecisionModule::Stop(__exception_ptr);
}


/*
 * @param radius in meter
 * @param distance in meter
 */
tFloat32 ParkingBase::CalculateAngleFromDistance(tFloat32 radius, tFloat32 distance)
{
    distance = distance - straightDriveOffset;
    if (distance <= 0)
    {
        return 0;
    }

    tFloat32 u = M_PI * 2 * radius;
    tFloat32 oneDegreeDistance = u / 360;

    return distance / oneDegreeDistance;
}

bool ParkingBase::IsFrontOfCarFree(const tWorldModel &model, tInt width)
{
    Point2f topLeft = VisionUtils::WorldToImage(Point(-15, width));
    Point2f bottomRight = VisionUtils::WorldToImage(Point(15, 0));

#ifndef NDEBUG
    cv::rectangle(debugImage, topLeft, bottomRight, Scalar(0, 0, 255, 127));
#endif

    Mat roi(model.ObstacleMap.clone(), Rect(topLeft, bottomRight));

    int nonZeros = cv::countNonZero(roi);
    return nonZeros == 0;
}

bool ParkingBase::IsBackOfCarFree(const tWorldModel &model, tInt width, tInt addToSide)
{
    Point2f topLeft = VisionUtils::WorldToImage(Point(-15 - addToSide, -60));
    Point2f bottomRight = VisionUtils::WorldToImage(Point(15 + addToSide, -60 - width));

#ifndef NDEBUG
    cv::rectangle(debugImage, topLeft, bottomRight, Scalar(0, 0, 255, 127));
#endif

    Mat roi(model.ObstacleMap.clone(), Rect(topLeft, bottomRight));

    int nonZeros = cv::countNonZero(roi);
    return nonZeros == 0;
}
