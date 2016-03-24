//
// Created by pbachmann on 2/11/16.
//

#ifndef HTWK_2016_IMUMERGERTEST_H
#define HTWK_2016_IMUMERGERTEST_H


#include "stdafx.h"
#include "../htwk_logger/Logger.h"
#include "../htwk_structs/tIMU.h"

#include <string>
#include <typeinfo>

#define OID "htwk.ImuMergerTest"
#define FILTER_NAME "HTWK IMU Merger Test"

class ImuMergerTest : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cInputPin imuStructInput;

        cObjectPtr<IMediaType> typeImuStruct;
        cObjectPtr<IMediaTypeDescription> descriptionImuStruct;

        Logger logger;

    public:
        ImuMergerTest(const tChar *__info = 0);

        virtual ~ImuMergerTest(void);

        virtual tResult Init(tInitStage eStage, ucom::IException **__exception_ptr);

        virtual tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample);

    private:
        tResult CreateInputPins(__exception = NULL);

        tResult CreateDescriptions(IException **__exception_ptr);
};

#endif //HTWK_2016_IMUMERGERTEST_H
