/*
@author: ffreihube
*/
//#############################################################################
#ifndef _ULTRA_DETECTION_FILTER_H_
#define _ULTRA_DETECTION_FILTER_H_

#include "../htwk_logger/Logger.h"
#include "htwk_ultra_grid.h"

#include <string>
#include <typeinfo>

//#############################################################################
#define OID "htwk.smart_driving.htwk_ultra_detection"
#define FILTER_NAME "HTWK Ultra Detection"
//#############################################################################

#define ULTRASONIC_FRONT_LEFT 0
#define ULTRASONIC_FRONT_CENTER_LEFT 1
#define ULTRASONIC_FRONT_CENTER 2
#define ULTRASONIC_FRONT_CENTER_RIGHT 3
#define ULTRASONIC_FRONT_RIGHT 4
#define ULTRASONIC_SIDE_RIGHT 5
#define ULTRASONIC_SIDE_LEFT 6
#define ULTRASONIC_REAR_LEFT 7
#define ULTRASONIC_REAR_CENTER 8
#define ULTRASONIC_REAR_RIGHT 9


class cUltraDetection : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cInputPin m_oInputUFL;
        cInputPin m_oInputUFCL;
        cInputPin m_oInputUFC;
        cInputPin m_oInputUFCR;
        cInputPin m_oInputUFR;
        cInputPin m_oInputUSL;
        cInputPin m_oInputUSR;
        cInputPin m_oInputURL;
        cInputPin m_oInputURC;
        cInputPin m_oInputURR;
        cInputPin ultraSonicStructPin;
        cVideoPin videoOutputPin;

        cObjectPtr<IMediaType> typeSignalValue;
        cObjectPtr<IMediaType> typeUltraSonicStructSignalValue;

        cObjectPtr<IMediaTypeDescription> descriptionSignalFloat;
        cObjectPtr<IMediaTypeDescription> descriptionUltraSonicStruct;

        Logger logger;
        cUltraGrid *obstacleGrid;

    public:
        /**
         * @brief Constructor.
         */
        cUltraDetection(const tChar *__info = 0);

        /**
          * @brief Destructor.
          */
        virtual ~cUltraDetection(void);

        virtual tResult Init(tInitStage eStage, ucom::IException **__exception_ptr);

        virtual tResult Start(ucom::IException **__exception_ptr);

        virtual tResult Stop(ucom::IException **__exception_ptr);

        virtual tResult Shutdown(tInitStage eStage, ucom::IException **__exception_ptr);

        virtual tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2,
                                   IMediaSample *pMediaSample);

    private:
        /*! creates all the input Pins*/
        tResult CreateInputPins(__exception = NULL);

        /*! creates all the output Pins*/
        tResult CreateOutputPins(__exception = NULL);

        tResult ProcessSingleSensorInput(IMediaSample *pMediaSample, tUInt32 sensorCode);

        /*! bitmapformat of output image */
        tBitmapFormat *outputFormat;

        /*! Coder Descriptor for the pins*/
        cObjectPtr <IMediaTypeDescription> m_pDescriptionSignal;
        /*! the id for the f32value of the media description for the pins */
        tBufferID m_szIDSignalF32Value;
        /*! the id for the arduino time stamp of the media description for the pins */
        tBufferID m_szIDSignalArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsSignalSet;

        tResult Transmit(Mat outputImage, IMediaSample *pMediaSample, tUInt32 ui32TimeStamp);

        cCriticalSection m_oProcessUsDataCritSection;

        tResult CreateDescriptions(IException **__exception_ptr);

        Mat SetValueToGrid(tUInt32 sensorCode, tFloat32 f32Value) const;
};

#endif
