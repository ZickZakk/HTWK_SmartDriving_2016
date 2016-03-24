#ifndef _HTWK_MOVEMENT_VISUALIZER_H_
#define _HTWK_MOVEMENT_VISUALIZER_H_

#include "stdafx.h"

#include "opencv2/opencv.hpp"
#include "../htwk_vision/Vision.h"

using namespace cv;

#include "../htwk_logger/Logger.h"

#define MOVEMENT_VISUALIZER_OID "htwk.movement_visualizer"
#define MOVEMENT_VISUALIZER_NAME "HTWK Movement Visualizer"

#define WIDTH_VIDEO 640
#define HEIGHT_VIDEO 480

#define DRAWING_SURFACE_WIDTH 3200
#define DRAWING_SURFACE_HEIGHT 2400


class MovementVisualizer : public adtf::cFilter
{
    ADTF_FILTER(MOVEMENT_VISUALIZER_OID, MOVEMENT_VISUALIZER_NAME, OBJCAT_DataFilter);

    private:
        tFloat32 oldYaw, yaw, deltaYaw;
        tFloat32 oldDistance, distance, deltaDistance;
        tInt buffer;

        cVideoPin videoOutputPin;
        cInputPin yawInPin, distanceInPin;
        Logger logger;

        cv::Point2d movementVectorInMeter, currentPosInPixel;
        std::vector<cv::Point2d> path;

        tBool videoEnabled;
        tBitmapFormat videoOutputFormat;
        cv::Mat colorImage;

        cCriticalSection syncIO;

        cObjectPtr<IMediaType> signalType;
        cObjectPtr<IMediaTypeDescription> signalTypeDescription;
        tBool idSignalSet;
        tBufferID idSignalF32Value;

        tFloat32 GetYawDelta(IMediaSample *mediaSample);

        tFloat32 GetDistanceDelta(IMediaSample *mediaSample);

        Point2d CalculateMovement(tFloat32 deltaYaw, tFloat32 deltaDistance);

        Point2d ConvertToPixel(Point2d movementVectorInMeter);

        tFloat32 GetFloat(IMediaSample *mediaSample);

        cv::Rect GetBoundingROI();

        tResult TransmitVideo();

        tVoid InitializeProperties();

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

        tResult Start(ucom::IException **__exception_ptr);

        tResult Stop(ucom::IException **__exception_ptr);

        tResult Shutdown(tInitStage eStage, ucom::IException **__exception_ptr);

    public:
        MovementVisualizer(const tChar *__info);

        virtual ~MovementVisualizer();

        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_MOVEMENT_VISUALIZER_H_
