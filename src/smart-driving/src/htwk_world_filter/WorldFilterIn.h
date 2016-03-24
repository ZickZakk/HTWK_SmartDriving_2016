#ifndef _WORLD_FILTER_HEADER_
#define _WORLD_FILTER_HEADER_

#include "stdafx.h"

#include <WorldService.h>
#include <Logger.h>
#include <tLane.h>
#include <tManeuver.h>
#include <tIMU.h>
#include <tCarState.h>
#include <tRoadSign.h>
#include <VisionUtils.h>

#define OID "htwk.world_filter_in"
#define FILTER_NAME "HTWK World Filter In"

class WorldFilterIn : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter)

    private:
        Logger logger;

        cObjectPtr<WorldService> worldService;

        cVideoPin obstacleVideoInput;
        cInputPin laneInput;
        cInputPin noPassingInput;
        cInputPin roadSignInput;
        cInputPin roadSignStructInput;
        cInputPin maneuverInput;
        cInputPin imuInput;
        cInputPin carStateInput;
        cInputPin distanceOverallInput;
        cInputPin currentCarSpeedInput;
        cInputPin resetInput;
        cInputPin readyInput;

        tBitmapFormat videoInputFormat;

        cObjectPtr<IMediaType> boolMediaType;
        cObjectPtr<IMediaTypeDescription> boolDescription;

        cObjectPtr<IMediaType> floatMediaType;
        cObjectPtr<IMediaTypeDescription> floatDescription;

        cObjectPtr<IMediaType> roadSignMediaType;
        cObjectPtr<IMediaTypeDescription> roadSignDescription;

        cObjectPtr<IMediaType> laneMediaType;
        cObjectPtr<IMediaTypeDescription> laneDescription;

        cObjectPtr<IMediaType> imuMediaType;
        cObjectPtr<IMediaTypeDescription> imuDescription;

        cObjectPtr<IMediaType> enumMediaType;
        cObjectPtr<IMediaTypeDescription> enumDescription;

        map<tReadyModule::ReadyModuleEnum, tTimeStamp> readyModulesWithTimes;

    public:
        WorldFilterIn(const tChar *__info);

        virtual ~WorldFilterIn();

        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);

    private:
        tResult Shutdown(tInitStage eStage, IException **__exception_ptr);

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins(IException **__exception_ptr);
};

#endif // _WORLD_FILTER_HEADER_
