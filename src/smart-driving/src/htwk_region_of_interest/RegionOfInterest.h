#ifndef _HTWK_ROI_FILTER_H_
#define _HTWK_ROI_FILTER_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "opencv2/opencv.hpp"
#include "../htwk_vision/Vision.h"
#include <tRect.h>

using namespace cv;

#define OID_ROI "htwk.roi"
#define FILTER_NAME "HTWK Region of Interest"

// Properties
#define RGB_VIDEO_MANIPULATION_PROPERTY "RGB video manipulation method"
#define DEPTH_VIDEO_MANIPULATION_PROPERTY "Depth video manipulation method"
#define ROOM_HEIGHT_MANIPULATION_PROPERTY "Room height manipulation"
#define HOOD_SCANLINE_NUMBER_PROPERTY "Hood scanline number"
#define ROOM_SCANLINE_NUMBER_PROPERTY "Room scanline number"
#define MAX_HOOD_DETECTION_COUNT_PROPERTY "Max hood detection count"
#define DETECT_HOOD_PROPERTY "Detect hood"
#define DETECT_ROOM_PROPERTY "Detect room"

// Video Manipulation
#define VIDEO_NONE 1
#define VIDEO_CROP 2
#define VIDEO_RECT 3

class ROI : public adtf::cFilter
{
    ADTF_FILTER(OID_ROI, FILTER_NAME, OBJCAT_DataFilter);

    private:
        Logger logger;

        cVideoPin rgbVideoInputPin;
        cVideoPin depthVideoInputPin;
        cVideoPin rgbVideoOutputPin;
        cVideoPin depthVideoOutputPin;
        cOutputPin rgbRoiOutputPin;
        cOutputPin depthRoiOutputPin;

        cObjectPtr<IMediaTypeDescription> tRectDescriptionSignal;
        cObjectPtr<IMediaType> tRectType;

        tBitmapFormat rgbVideoInputFormat;
        tBitmapFormat depthVideoInputFormat;
        tBitmapFormat rgbVideoOutputFormat;
        tBitmapFormat depthVideoOutputFormat;

        // output data
        Mat depthOutputImage;
        Mat rgbOutputImage;
        tRect rgbROI;
        tRect depthROI;

        // calculated values
        tInt hoodHeight;
        tInt roomHeight;
        tInt hoodDetectionCount;
        tInt processingWidthPercentage;

        // properties
        tInt8 rgbVideoManipulation;
        tInt8 depthVideoManipulation;
        tInt hoodScanLineNumber;
        tInt roomScanLineNumber;
        tInt maxHoodDetectionCount;
        tBool isHoodDetectionEnabled;
        tBool isRoomDetectionEnabled;
        tFloat32 roomHeightManipulation;

        struct ProcessingData
        {
            tInt processingWidth;
            tInt startOffset;
            tInt hoodScanLineStepWidth;
            tInt roomScanLineStepWidth;
        } processingData;

    private:
        void AnalyzeDepthImage();

        void ProcessDepthImage();

        void ProcessRgbImage();

        void CalculateRgbRoi();

        void CalculateDepthRoi();

        void DetectHood();

        void DetectRoom();

        tResult SendRoi(const tRect &roi, cOutputPin &outputPin);

        tResult TransmitRGBOutput(const tTimeStamp &timeStamp);

        tResult TransmitDepthOutput(const tTimeStamp &timeStamp);

        void InitializeProperties();

        tResult InitDescriptions(IException **__exception_ptr);

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

    public:
        ROI(const tChar *__info);

        virtual ~ROI();

    public:
        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_ROI_FILTER_H_
