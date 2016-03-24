#ifndef _HTWK_IMUOUTLIERREMOVER_H_
#define _HTWK_IMUOUTLIERREMOVER_H_

#include "stdafx.h"
#include <algorithm>    // std::sort

#include "../htwk_logger/Logger.h"
#include "../htwk_yaw_to_steer/YawToSteer.h"

#define IMUOUTLIERREMOVER_OID "htwk.imuoutlierremover"
#define IMUOUTLIERREMOVER_NAME "HTWK IMU Outlier Remover"

#define NUMBER_OF_INIT_VALUES  20


class IMUOutlierRemover : public adtf::cFilter
{
    ADTF_FILTER(IMUOUTLIERREMOVER_OID, IMUOUTLIERREMOVER_NAME, OBJCAT_DataFilter);

    public:
        IMUOutlierRemover(const tChar *__info);
        virtual ~IMUOutlierRemover();

        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);
        tResult Init(tInitStage eStage, __exception = NULL);

    private:
        cInputPin yawInPin;
        cOutputPin yawOutPin;

        Logger logger;

        tBool isYawInitialized;
        std::vector<float> yawInitValues;

        tFloat32 oldYaw, yaw;
        tUInt32 oldTimeStamp, timeStamp;

        tResult GetFloat(IMediaSample *mediaSample, tFloat32 &fValue, tUInt32 &nTimeStamp);

        tBool InitYaw();
        tBool IsDirectionChangePlausible();
        tResult ValidateYaw();
        tResult TransmitSignalValue(cOutputPin *outputPin, tFloat32 value);


        cObjectPtr<IMediaType> typeSignalValue;
        cObjectPtr<IMediaTypeDescription> descriptionSignalValue;

        tResult CreateDescriptions(IException **__exception_ptr);
        tResult CreateInputPins(__exception = NULL);
        tResult CreateOutputPins(__exception = NULL);
};


#endif // _HTWK_IMUOUTLIERREMOVER_H_
