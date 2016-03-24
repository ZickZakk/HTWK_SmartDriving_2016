//
// Created by pbachmann on 2/9/16.
//

#include "WorldFilterOut.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, WorldFilterOut)

WorldFilterOut::WorldFilterOut(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
}

WorldFilterOut::~WorldFilterOut()
{
}

tResult WorldFilterOut::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (StageFirst == eStage)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (StageNormal == eStage)
    {
    }
    else if (StageGraphReady == eStage)
    {
        RETURN_IF_FAILED(_runtime->GetObject(OID_WORLD_SERVICE, IID_WORLD_INTERFACE, (tVoid **) &worldService, __exception_ptr));

        obstacleVideoFormat.nWidth = 400;
        obstacleVideoFormat.nHeight = 400;
        obstacleVideoFormat.nBitsPerPixel = 8;
        obstacleVideoFormat.nPixelFormat = cImage::PF_GREYSCALE_8;
        obstacleVideoFormat.nBytesPerLine = obstacleVideoFormat.nWidth;
        obstacleVideoFormat.nSize = obstacleVideoFormat.nBytesPerLine * obstacleVideoFormat.nHeight;
        obstacleVideoFormat.nPaletteSize = 0;

        obstacleVideoOutput.SetFormat(&obstacleVideoFormat, NULL);
        logger.Log(cString::Format("Obstacle Video format: %d x %d @ %d Bit", obstacleVideoFormat.nWidth, obstacleVideoFormat.nHeight,
                                   obstacleVideoFormat.nBitsPerPixel).GetPtr(), false);
    }

    RETURN_NOERROR;
}

tResult WorldFilterOut::Shutdown(tInitStage eStage, __exception)
{
    if (eStage == StageGraphReady)
    {
        worldService = NULL;
    }
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult WorldFilterOut::OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample)
{
    if (eventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (sourcePin == &triggerInput)
    {
        PullAndSend<tLane>(laneOutput, WORLD_LANE);
        PullAndSend<tBool>(noPassingOutput, WORLD_IS_NO_PASSING_ACTIVE);
        PullAndSend<tRoadSign::RoadSignEnum>(roadSignOutput, WORLD_CURRENT_ROAD_SIGN);
        PullAndSend<tFloat32>(roadSignSizeOutput, WORLD_CURRENT_ROAD_SIGN_SIZE);
        PullAndSend<tManeuver>(maneuverOutput, WORLD_CURRENT_MANEUVER);
        PullAndSend<Mat>(obstacleVideoOutput, WORLD_OBSTACLE_MAT);
        PullAndSend<tIMU>(imuOutput, WORLD_IMU);
        PullAndSend<stateCar>(carStateOutput, WORLD_CAR_STATE);
        PullAndSend<tFloat32>(distanceOverallOutput, WORLD_DISTANCE_OVERALL);
        PullAndSend<tFloat32>(currentCarSpeedOutput, WORLD_CAR_SPEED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}

template<typename Type>
void WorldFilterOut::PullAndSend(cOutputPin &pin, string key)
{
    Type value;
    if (worldService->Pull<Type>(key, value))
    {
        logger.Log(cString::Format("%s: %f", key.c_str(), value).GetPtr(), false);
        SendValue(pin, value);
    }
}

template<typename Type>
void WorldFilterOut::PullAndSend(cVideoPin &pin, string key)
{
    Type value;
    if (worldService->Pull<Type>(key, value))
    {
        SendValue(pin, value);
    }
}

tResult WorldFilterOut::SendValue(cOutputPin &pin, tLane &value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    laneDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(laneDescription, mediaSample, pCoder);

        pCoder->Set("tLeftLine.tStart.tX", (tVoid *) &value.tLeftLine.tStart.tX);
        pCoder->Set("tLeftLine.tStart.tY", (tVoid *) &value.tLeftLine.tStart.tY);
        pCoder->Set("tLeftLine.tEnd.tX", (tVoid *) &value.tLeftLine.tEnd.tX);
        pCoder->Set("tLeftLine.tEnd.tY", (tVoid *) &value.tLeftLine.tEnd.tY);
        pCoder->Set("tLeftLine.tStatus", (tVoid *) &value.tLeftLine.tStatus);
        pCoder->Set("tLeftLine.tCrossingDistance", (tVoid *) &value.tLeftLine.tCrossingDistance);

        pCoder->Set("tRightLine.tStart.tX", (tVoid *) &value.tRightLine.tStart.tX);
        pCoder->Set("tRightLine.tStart.tY", (tVoid *) &value.tRightLine.tStart.tY);
        pCoder->Set("tRightLine.tEnd.tX", (tVoid *) &value.tRightLine.tEnd.tX);
        pCoder->Set("tRightLine.tEnd.tY", (tVoid *) &value.tRightLine.tEnd.tY);
        pCoder->Set("tRightLine.tStatus", (tVoid *) &value.tRightLine.tStatus);
        pCoder->Set("tRightLine.tCrossingDistance", (tVoid *) &value.tRightLine.tCrossingDistance);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult WorldFilterOut::SendValue(cOutputPin &pin, tIMU &value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    imuDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(imuDescription, mediaSample, pCoder);

        pCoder->Set("tYaw", (tVoid *) &value.tYaw);
        pCoder->Set("tAccX", (tVoid *) &value.tAccX);
        pCoder->Set("tAccY", (tVoid *) &value.tAccY);
        pCoder->Set("tAccZ", (tVoid *) &value.tAccZ);
        pCoder->Set("tPitch", (tVoid *) &value.tPitch);
        pCoder->Set("tRoll", (tVoid *) &value.tRoll);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult WorldFilterOut::SendValue(cOutputPin &pin, tBool &value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    boolDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(boolDescription, mediaSample, pCoder);
        pCoder->Set("bValue", (tVoid *) &value);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult WorldFilterOut::SendValue(cOutputPin &pin, tFloat32 &value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    floatDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(floatDescription, mediaSample, pCoder);
        pCoder->Set("f32Value", (tVoid *) &value);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult WorldFilterOut::SendValue(cOutputPin &pin, tManeuver &value)
{
    tInt anInt = static_cast<tInt>(value);
    SendEnum(pin, anInt);
    RETURN_NOERROR;
}

tResult WorldFilterOut::SendValue(cOutputPin &pin, tRoadSign::RoadSignEnum &value)
{
    tInt anInt = static_cast<tInt>(value);
    SendEnum(pin, anInt);
    RETURN_NOERROR;
}

tResult WorldFilterOut::SendValue(cOutputPin &pin, stateCar &value)
{
    tInt anInt = static_cast<tInt>(value);
    SendEnum(pin, anInt);
    RETURN_NOERROR;
}

tResult WorldFilterOut::SendEnum(cOutputPin &pin, tInt &value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    enumDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(enumDescription, mediaSample, pCoder);
        pCoder->Set("tEnumValue", (tVoid *) &value);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult WorldFilterOut::SendValue(cVideoPin &pin, Mat &value)
{
    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &mediaSample));
    RETURN_IF_FAILED(mediaSample->AllocBuffer(obstacleVideoFormat.nSize));
    mediaSample->Update(_clock->GetStreamTime(), value.data, obstacleVideoFormat.nSize, 0);

    pin.Transmit(mediaSample);

    RETURN_NOERROR;
}

tResult WorldFilterOut::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get bool
    tChar const *boolSignalValueDescription = descManager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(boolSignalValueDescription);
    boolMediaType = new cMediaType(0, 0, 0, "tBoolSignalValue", boolSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(boolMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &boolDescription));

    // get float
    tChar const *floatSignalValueDescription = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(floatSignalValueDescription);
    floatMediaType = new cMediaType(0, 0, 0, "tSignalValue", floatSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(floatMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &floatDescription));

    // get lane
    tChar const *laneSignalValueDescription = descManager->GetMediaDescription("tLane");
    RETURN_IF_POINTER_NULL(laneSignalValueDescription);
    laneMediaType = new cMediaType(0, 0, 0, "tLane", laneSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(laneMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &laneDescription));

    // get enum
    tChar const *enumSignalValueDescription = descManager->GetMediaDescription("tEnumBox");
    RETURN_IF_POINTER_NULL(enumSignalValueDescription);
    enumMediaType = new cMediaType(0, 0, 0, "tEnumBox", enumSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(enumMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &enumDescription));

    // get imu
    tChar const *imuSignalValueDescription = descManager->GetMediaDescription("tIMU");
    RETURN_IF_POINTER_NULL(imuSignalValueDescription);
    imuMediaType = new cMediaType(0, 0, 0, "tIMU", imuSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(imuMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &imuDescription));

    RETURN_NOERROR;
}

tResult WorldFilterOut::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(obstacleVideoOutput.Create("Obstacle_Video_Output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&obstacleVideoOutput));

    RETURN_IF_FAILED(laneOutput.Create("Lane_Output", laneMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&laneOutput));

    RETURN_IF_FAILED(noPassingOutput.Create("NoPassing_Output", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&noPassingOutput));

    RETURN_IF_FAILED(roadSignOutput.Create("RoadSign_Output", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&roadSignOutput));

    RETURN_IF_FAILED(roadSignSizeOutput.Create("RoadSignSize_Output", floatMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&roadSignSizeOutput));

    RETURN_IF_FAILED(maneuverOutput.Create("Maneuver_Output", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&maneuverOutput));

    RETURN_IF_FAILED(imuOutput.Create("IMU_Output", imuMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&imuOutput));

    RETURN_IF_FAILED(carStateOutput.Create("CarState_Output", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&carStateOutput));

    RETURN_IF_FAILED(distanceOverallOutput.Create("DistanceOverall_Output", floatMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&distanceOverallOutput));

    RETURN_IF_FAILED(currentCarSpeedOutput.Create("CarSpeed_Output", floatMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&currentCarSpeedOutput));

    RETURN_NOERROR;
}

tResult WorldFilterOut::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(triggerInput.Create("Trigger_Input", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&triggerInput));

    RETURN_NOERROR;
}
