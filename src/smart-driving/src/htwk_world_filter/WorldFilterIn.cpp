#include <tReadyModule.h>
#include "WorldFilterIn.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, WorldFilterIn)

WorldFilterIn::WorldFilterIn(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
}

WorldFilterIn::~WorldFilterIn()
{
}

tResult WorldFilterIn::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (StageFirst == eStage)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
    }
    else if (StageNormal == eStage)
    {
    }
    else if (StageGraphReady == eStage)
    {
        RETURN_IF_FAILED(_runtime->GetObject(OID_WORLD_SERVICE, IID_WORLD_INTERFACE, (tVoid **) &worldService, __exception_ptr));
        worldService->Clear();

        cObjectPtr<IMediaType> mediaType;
        RETURN_IF_FAILED(obstacleVideoInput.GetMediaType(&mediaType));

        cObjectPtr<IMediaTypeVideo> videoType;
        RETURN_IF_FAILED(mediaType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &videoType));

        videoInputFormat = *(videoType->GetFormat());
    }

    RETURN_NOERROR;
}

tResult WorldFilterIn::Shutdown(tInitStage eStage, __exception)
{
    if (worldService != NULL)
    {
        worldService->Clear();
        worldService = NULL;
    }

    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult WorldFilterIn::OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample)
{
    if (eventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (sourcePin == &laneInput)
    {
        tLane lane;

        {
            __adtf_sample_read_lock_mediadescription(laneDescription, mediaSample, inputCoder);
            inputCoder->Get("tLeftLine.tStart.tX", (tVoid *) &lane.tLeftLine.tStart.tX);
            inputCoder->Get("tLeftLine.tStart.tY", (tVoid *) &lane.tLeftLine.tStart.tY);
            inputCoder->Get("tLeftLine.tEnd.tX", (tVoid *) &lane.tLeftLine.tEnd.tX);
            inputCoder->Get("tLeftLine.tEnd.tY", (tVoid *) &lane.tLeftLine.tEnd.tY);
            inputCoder->Get("tLeftLine.tStatus", (tVoid *) &lane.tLeftLine.tStatus);
            inputCoder->Get("tLeftLine.tCrossingDistance", (tVoid *) &lane.tLeftLine.tCrossingDistance);

            inputCoder->Get("tRightLine.tStart.tX", (tVoid *) &lane.tRightLine.tStart.tX);
            inputCoder->Get("tRightLine.tStart.tY", (tVoid *) &lane.tRightLine.tStart.tY);
            inputCoder->Get("tRightLine.tEnd.tX", (tVoid *) &lane.tRightLine.tEnd.tX);
            inputCoder->Get("tRightLine.tEnd.tY", (tVoid *) &lane.tRightLine.tEnd.tY);
            inputCoder->Get("tRightLine.tStatus", (tVoid *) &lane.tRightLine.tStatus);
            inputCoder->Get("tRightLine.tCrossingDistance", (tVoid *) &lane.tRightLine.tCrossingDistance);
        }

        worldService->Push(WORLD_LANE, lane);
    }
    else if (sourcePin == &noPassingInput)
    {
        tBool isNoPassingActive;

        {

            __adtf_sample_read_lock_mediadescription(boolDescription, mediaSample, inputCoder);
            inputCoder->Get("bValue", (tVoid *) &isNoPassingActive);
        }

        worldService->Push(WORLD_IS_NO_PASSING_ACTIVE, isNoPassingActive);
    }
    else if (sourcePin == &roadSignInput)
    {
        tInt roadSignId;

        {
            __adtf_sample_read_lock_mediadescription(enumDescription, mediaSample, inputCoder);
            inputCoder->Get("tEnumValue", (tVoid *) &roadSignId);
        }

        tRoadSign::RoadSignEnum roadSign = static_cast<tRoadSign::RoadSignEnum>(roadSignId);
        worldService->Push(WORLD_CURRENT_ROAD_SIGN, roadSign);
    }
    else if (sourcePin == &roadSignStructInput)
    {
        tFloat32 roadSignSize;
        {
            __adtf_sample_read_lock_mediadescription(roadSignDescription, mediaSample, pCoderInput);
            pCoderInput->Get("f32Imagesize", (tVoid *) &roadSignSize);
        }

        worldService->Push(WORLD_CURRENT_ROAD_SIGN_SIZE, roadSignSize);
    }
    else if (sourcePin == &distanceOverallInput)
    {
        tFloat32 distanceOverall;

        {
            __adtf_sample_read_lock_mediadescription(floatDescription, mediaSample, inputCoder);
            inputCoder->Get("f32Value", (tVoid *) &distanceOverall);
        }

        worldService->Push(WORLD_DISTANCE_OVERALL, distanceOverall);
    }
    else if (sourcePin == &currentCarSpeedInput)
    {
        tFloat32 currentSpeed;

        {
            __adtf_sample_read_lock_mediadescription(floatDescription, mediaSample, inputCoder);
            inputCoder->Get("f32Value", (tVoid *) &currentSpeed);
        }

        worldService->Push(WORLD_CAR_SPEED, currentSpeed);
    }
    else if (sourcePin == &obstacleVideoInput)
    {
        Mat worldVideo;

        {
            VisionUtils::ExtractImageFromMediaSample(mediaSample, videoInputFormat).copyTo(worldVideo);
            RETURN_IF_POINTER_NULL(worldVideo.data);
        }

        worldService->Push(WORLD_OBSTACLE_MAT, worldVideo);
    }
    else if (sourcePin == &maneuverInput)
    {
        tManeuver::ManeuverEnum maneuver;

        {
            __adtf_sample_read_lock_mediadescription(enumDescription, mediaSample, inputCoder);
            inputCoder->Get("tEnumValue", (tVoid *) &maneuver);
        }

        worldService->Push(WORLD_CURRENT_MANEUVER, maneuver);
    }
    else if (sourcePin == &carStateInput)
    {
        tCarState::CarStateEnum carState;

        {
            __adtf_sample_read_lock_mediadescription(enumDescription, mediaSample, inputCoder);
            inputCoder->Get("tEnumValue", (tVoid *) &carState);
        }

        worldService->Push(WORLD_CAR_STATE, carState);
    }
    else if (sourcePin == &imuInput)
    {
        tIMU imu;

        {
            __adtf_sample_read_lock_mediadescription(imuDescription, mediaSample, inputCoder);
            inputCoder->Get("tYaw", (tVoid *) &imu.tYaw);
            inputCoder->Get("tAccX", (tVoid *) &imu.tAccX);
            inputCoder->Get("tAccY", (tVoid *) &imu.tAccY);
            inputCoder->Get("tAccZ", (tVoid *) &imu.tAccZ);
            inputCoder->Get("tPitch", (tVoid *) &imu.tPitch);
            inputCoder->Get("tRoll", (tVoid *) &imu.tRoll);
        }

        worldService->Push(WORLD_IMU, imu);
    }
    else if (sourcePin == &readyInput)
    {
        tReadyModule::ReadyModuleEnum module;

        {
            __adtf_sample_read_lock_mediadescription(enumDescription, mediaSample, pCoder);
            pCoder->Get("tEnumValue", (tVoid *) &module);
        }

        logger.Log(cString::Format("Got ready for %d", module).GetPtr(), false);

        if (readyModulesWithTimes.find(module) == readyModulesWithTimes.end())
        {
            logger.Log(cString::Format("Inserted %d", module).GetPtr(), false);

            readyModulesWithTimes.insert(pair<tReadyModule::ReadyModuleEnum, tTimeStamp>(module, mediaSample->GetTime()));
        }
        else
        {
            logger.Log(cString::Format("Updated %d", module).GetPtr(), false);

            readyModulesWithTimes[module] = mediaSample->GetTime();
        }

        vector<tReadyModule::ReadyModuleEnum> readyModules;
        for (map<tReadyModule::ReadyModuleEnum, tTimeStamp>::iterator it = readyModulesWithTimes.begin(); it != readyModulesWithTimes.end(); ++it)
        {
            readyModules.push_back(it->first);
        }

        worldService->Push(WORLD_READY_MODULES, readyModules);
    }
    else if (sourcePin == &resetInput)
    {
        tReadyModule::ReadyModuleEnum module;

        {
            __adtf_sample_read_lock_mediadescription(enumDescription, mediaSample, pCoder);
            pCoder->Get("tEnumValue", (tVoid *) &module);
        }

        logger.Log(cString::Format("Got reset for %d", module).GetPtr(), false);

        if (readyModulesWithTimes.find(module) != readyModulesWithTimes.end())
        {
            logger.Log(cString::Format("Found %d", module).GetPtr(), false);

            if (readyModulesWithTimes[module] < mediaSample->GetTime())
            {
                logger.Log(cString::Format("Deleted %d", module).GetPtr(), false);

                readyModulesWithTimes.erase(module);
            }
        }

        vector<tReadyModule::ReadyModuleEnum> readyModules;
        for (map<tReadyModule::ReadyModuleEnum, tTimeStamp>::iterator it = readyModulesWithTimes.begin(); it != readyModulesWithTimes.end(); ++it)
        {
            readyModules.push_back(it->first);
        }

        worldService->Push(WORLD_READY_MODULES, readyModules);
    }

    logger.EndLog();

    RETURN_NOERROR;
}

tResult WorldFilterIn::CreateDescriptions(IException **__exception_ptr)
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

    // get roadSign
    tChar const *roadSignSignalValueDescription = descManager->GetMediaDescription("tRoadSign");
    RETURN_IF_POINTER_NULL(roadSignSignalValueDescription);
    roadSignMediaType = new cMediaType(0, 0, 0, "tRoadSign", roadSignSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(roadSignMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &roadSignDescription));

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

tResult WorldFilterIn::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(obstacleVideoInput.Create("Obstacle_Video_Input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&obstacleVideoInput));

    RETURN_IF_FAILED(laneInput.Create("Lane_Input", laneMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&laneInput));

    RETURN_IF_FAILED(noPassingInput.Create("NoPassing_Input", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&noPassingInput));

    RETURN_IF_FAILED(roadSignInput.Create("RoadSign_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&roadSignInput));

    RETURN_IF_FAILED(roadSignStructInput.Create("RoadSignStruct_Input", roadSignMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&roadSignStructInput));

    RETURN_IF_FAILED(maneuverInput.Create("Maneuver_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&maneuverInput));

    RETURN_IF_FAILED(imuInput.Create("IMU_Input", imuMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&imuInput));

    RETURN_IF_FAILED(carStateInput.Create("CarState_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&carStateInput));

    RETURN_IF_FAILED(distanceOverallInput.Create("DistanceOverall_Input", floatMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&distanceOverallInput));

    RETURN_IF_FAILED(currentCarSpeedInput.Create("CarSpeed_Input", floatMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&currentCarSpeedInput));

    RETURN_IF_FAILED(readyInput.Create("Ready_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&readyInput));

    RETURN_IF_FAILED(resetInput.Create("Reset_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&resetInput));

    RETURN_NOERROR;
}
