#ifndef _HTWK_YAWTOSTEER_H_
#define _HTWK_YAWTOSTEER_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"

#define FILTER_OID "htwk.yawtosteer"
#define FILTER_NAME "HTWK YawToSteer"

#define STEERING_MAX_LEFT 60
#define STEERING_MAX_RIGHT 120
#define STEERING_STRAIGHT 90

#define SPEED_STATIONARY 90

#define STEERING_COMPANSATION_FACTOR_PROPERTY "compansation factor"
#define STEERING_COMPENSATION_FACTOR 15.0f
#define DEVIATION_THRESHOLD_PROPERTY "deviation threshold"
#define DEVIATION_THRESHOLD 0.2f
#define ABRUPT_VARIATION_THRESHOLD_PROPERTY "senderYaw discard threshold"
#define ABRUPT_VARIATION_THRESHOLD 30.0f

class YawToSteer :

public adtf::cFilter
{
    ADTF_FILTER(FILTER_OID, FILTER_NAME, OBJCAT_DataFilter);

    public:
    YawToSteer(
    const tChar *__info);
    virtual ~YawToSteer();

    tResult OnPinEvent(IPin *sourcePin,
                       tInt eventCode,
                       tInt param1,
                       tInt param2,
                       IMediaSample *mediaSample);
    tResult Init(tInitStage eStage, __exception = NULL);

    private:
    cInputPin senderCarYawInPin, localCarYawInPin, senderCarSpeedInPin;
    cOutputPin steerOutPin;

    tFloat32 senderYaw, oldSenderYaw, deltaSenderYaw;
    tFloat32 localYaw, oldLocalYaw, targetLocalYaw;
    tFloat32 senderSpeed, oldSenderSpeed;
    tFloat32 directionCorrection;

    tFloat32 compensationValue;
    tFloat32 deviationThreshold;
    tFloat32 abruptVariationThreshold;
    tFloat32 steer;

    tBool isSenderYawInitialized, isLocalYawInitialized;
    Logger logger;

    tResult GetSenderYaw(IMediaSample *mediaSample);
    tResult GetLocalYaw(IMediaSample *pMediaSample);

    tFloat32 GetFloat(IMediaSample *mediaSample);

    tResult CalculateSteering();

    tFloat GetCorrectionAngle(tFloat deviationAngle);

    tResult DirectionChange(tFloat angle);

    tFloat CalcCorrectionAngle(tFloat deviationAngle);
    tResult TransmitSignalValue(cOutputPin *outputPin, tFloat32 value);

    cCriticalSection iosync;

    cObjectPtr <IMediaType> typeSignalValue;
    cObjectPtr <IMediaTypeDescription> descriptionSignalValue;
    /*! the id for the f32value of the media description for the pins */
    tBufferID m_szIDSignalF32Value;
    /*! the id for the arduino time stamp of the media description for the pins */
    tBufferID m_szIDSignalArduinoTimestamp;
    /*! indicates if bufferIDs were set */
    tBool m_bIDsSignalSet;

    void InitializeProperties();

    tResult Start(IException **__exception_ptr);

    tResult Shutdown(cFilter::tInitStage eStage, IException **__exception_ptr);
    tResult CreateDescriptions(IException **__exception_ptr);
    tResult CreateInputPins(__exception = NULL);
    tResult CreateOutputPins(__exception = NULL);
};


tFloat32 CalcYawDelta(tFloat32 oldYaw, tFloat32 newYaw)
{
    tFloat32 deltaYaw = oldYaw - newYaw;
    if (deltaYaw > 180.0f)
    {
        deltaYaw -= 360.0f;
    }
    else if (deltaYaw < -180.0f)
    {
        deltaYaw += 360.0f;
    }
    return (-deltaYaw);
    /**
    old = 170;
    new = -170;
    delta = 340 -> -20 -> 20;

    old = -170;
    new = 170;
    delta = -340 -> 20 -> -20;

    old = -10;
    new = 10;
    delta = -20 -> 20

    old = 10;
    new = -10;
    delta = 20 -> -20
                   */
}

#endif // _HTWK_YAWTOSTEER_H_
