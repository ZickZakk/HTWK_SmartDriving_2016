/*
@author: pbachmann
*/

#ifndef _HTWK_ULTRAMEDIAN_FILTER_H_
#define _HTWK_ULTRAMEDIAN_FILTER_H_

#include "stdafx.h"
#include "../htwk_logger/Logger.h"

#include <string>
#include <typeinfo>

#define OID "htwk.ultra_median"
#define FILTER_NAME "HTWK Ultra Median"

#define BUFFER_SIZE_PROPERTY "Buffer Size"

class UltraMedian : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cInputPin ultraSonicInput;
        cOutputPin ultraSonicOutput;

        cObjectPtr<IMediaType> typeUltraSonicStructSignalValue;
        cObjectPtr<IMediaTypeDescription> descriptionUltraSonicStruct;

        //properties
        unsigned long bufferSize;

        unsigned long readIndex;
        tFloat32 frontLeftTotal;
        tFloat32 frontCenterLeftTotal;
        tFloat32 frontCenterTotal;
        tFloat32 frontCenterRightTotal;
        tFloat32 frontRightTotal;
        tFloat32 sideLeftTotal;
        tFloat32 sideRightTotal;
        tFloat32 rearLeftTotal;
        tFloat32 rearCenterTotal;
        tFloat32 rearRightTotal;
        vector<tFloat32> frontLeftBuffer;
        vector<tFloat32> frontCenterLeftBuffer;
        vector<tFloat32> frontCenterBuffer;
        vector<tFloat32> frontCenterRightBuffer;
        vector<tFloat32> frontRightBuffer;
        vector<tFloat32> sideLeftBuffer;
        vector<tFloat32> sideRightBuffer;
        vector<tFloat32> rearLeftBuffer;
        vector<tFloat32> rearCenterBuffer;
        vector<tFloat32> rearRightBuffer;

        Logger logger;

    public:
        UltraMedian(const tChar *__info = 0);

        virtual ~UltraMedian(void);

        virtual tResult Init(tInitStage eStage, ucom::IException **__exception_ptr);

        virtual tResult Start(ucom::IException **__exception_ptr);

        virtual tResult Stop(ucom::IException **__exception_ptr);

        virtual tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample);

    private:
        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

        tResult CreateDescriptions(IException **__exception_ptr);

        void InitializeProperties();

        tResult TransmitStruct();
};

#endif // _HTWK_ULTRAMEDIAN_FILTER_H_
