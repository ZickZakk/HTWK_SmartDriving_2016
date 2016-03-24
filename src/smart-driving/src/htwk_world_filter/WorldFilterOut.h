#ifndef _WORLD_FILTER_HEADER_
#define _WORLD_FILTER_HEADER_

#include "stdafx.h"

#include "opencv2/opencv.hpp"

#include "../htwk_world_service_h/WorldService.h"
#include "../htwk_logger/Logger.h"
#include "../htwk_structs/tLane.h"
#include "../htwk_structs/tManeuver.h"
#include "../htwk_structs/tIMU.h"
#include "../htwk_structs/juryEnums.h"
#include "../htwk_structs/tRoadSign.h"
#include "../htwk_structs/tLine.h"

using namespace cv;

#define OID "htwk.world_filter_out"
#define FILTER_NAME "HTWK World Filter Out"

class WorldFilterOut : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter)

    private:
        Logger logger;

        cObjectPtr<WorldService> worldService;

        cOutputPin laneOutput;
        cVideoPin obstacleVideoOutput;
        cOutputPin noPassingOutput;
        cOutputPin roadSignOutput;
        cOutputPin roadSignSizeOutput;
        cOutputPin maneuverOutput;
        cOutputPin imuOutput;
        cOutputPin carStateOutput;
        cOutputPin distanceOverallOutput;
        cOutputPin currentCarSpeedOutput;
        cInputPin triggerInput;

        tBitmapFormat obstacleVideoFormat;

        cObjectPtr<IMediaType> boolMediaType;
        cObjectPtr<IMediaTypeDescription> boolDescription;

        cObjectPtr<IMediaType> floatMediaType;
        cObjectPtr<IMediaTypeDescription> floatDescription;

        cObjectPtr<IMediaType> laneMediaType;
        cObjectPtr<IMediaTypeDescription> laneDescription;

        cObjectPtr<IMediaType> enumMediaType;
        cObjectPtr<IMediaTypeDescription> enumDescription;

        cObjectPtr<IMediaType> imuMediaType;
        cObjectPtr<IMediaTypeDescription> imuDescription;

    public:
        WorldFilterOut(const tChar *__info);

        virtual ~WorldFilterOut();

        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, IException **__exception_ptr);

    private:
        tResult Shutdown(tInitStage eStage, IException **__exception_ptr);

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins(IException **__exception_ptr);

        tResult CreateOutputPins(IException **__exception_ptr);

        tResult SendValue(cOutputPin &pin, tLane &lane);

        tResult SendValue(cOutputPin &pin, tBool &value);

        tResult SendValue(cOutputPin &pin, tFloat32 &value);

        tResult SendValue(cVideoPin &pin, cv::Mat &value);

        template<typename Type>
        void PullAndSend(cOutputPin &pin, string key);

        template<typename Type>
        void PullAndSend(cVideoPin &pin, string key);

        tResult SendEnum(cOutputPin &pin, tInt &value);

        tResult SendValue(cOutputPin &pin, tManeuver &value);

        tResult SendValue(cOutputPin &pin, tRoadSign::RoadSignEnum &value);

        tResult SendValue(cOutputPin &pin, stateCar &value);

        tResult SendValue(cOutputPin &pin, tIMU &value);
};

#endif // _WORLD_FILTER_HEADER_
