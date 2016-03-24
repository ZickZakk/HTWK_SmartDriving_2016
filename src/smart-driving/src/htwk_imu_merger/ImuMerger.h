/*
@author: gjenschmischek
*/

#ifndef _HTWK_IMU_MERGER_FILTER_H_
#define _HTWK_IMU_MERGER_FILTER_H_

#include "stdafx.h"
#include "../htwk_logger/Logger.h"
#include "../htwk_structs/tIMU.h"

#include <string>
#include <typeinfo>

#define OID "htwk.ImuMerger"
#define FILTER_NAME "HTWK IMU Merger"

class ImuMerger : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cInputPin YawInput;
        cInputPin PitchInput;
        cInputPin RollInput;
        cInputPin AccXInput;
        cInputPin AccYInput;
        cInputPin AccZInput;

        cOutputPin imuStructOutput;

        cObjectPtr<IMediaType> typeSignalValue;
        cObjectPtr<IMediaTypeDescription> descriptionSignalValue;

        cObjectPtr<IMediaType> typeImuStruct;
        cObjectPtr<IMediaTypeDescription> descriptionImuStruct;

        Logger logger;

        tIMU imu;

    public:
        ImuMerger(const tChar *__info = 0);

        virtual ~ImuMerger(void);

        virtual tResult Init(tInitStage eStage, ucom::IException **__exception_ptr);

        virtual tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample);

    private:
        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult TransmitStruct();

        tFloat32 GetValue(IMediaSample *mediaSample);
};

#endif // _HTWK_ULTRAMEDIAN_FILTER_H_
