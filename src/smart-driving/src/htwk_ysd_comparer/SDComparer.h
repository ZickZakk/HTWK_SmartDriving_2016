#ifndef _HTWK_SDCOMPARER_H_
#define _HTWK_SDCOMPARER_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"

#define FILTER_OID "htwk.ysdcomp"
#define FILTER_NAME "HTWK SDComparer"

#define MAX_SPEED 20
#define MAX_REVERSE_SPEED 130
#define NEUTRAL_SPEED 90


class SDComparer : public adtf::cFilter
{
    ADTF_FILTER(FILTER_OID, FILTER_NAME, OBJCAT_DataFilter);

    public:
        SDComparer(const tChar *__info);
        virtual ~SDComparer();

        tResult OnPinEvent(IPin *sourcePin,
                           tInt eventCode,
                           tInt param1,
                           tInt param2,
                           IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);

    private:
        cInputPin senderCarDistInPin, localCarDistInPin, speedInPin;
        cOutputPin speedOutPin;

        tFloat32 speed, senderCarDist, localCarDist;
        tInt buffer;

        Logger logger;

        tFloat32 GetFloat(IMediaSample *mediaSample);

        tResult TransmitSignalValue(cOutputPin *outputPin, tFloat32 value);

        cCriticalSection iosync;

        cObjectPtr<IMediaType> typeSignalValue;
        cObjectPtr<IMediaTypeDescription> descriptionSignalValue;
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

#endif // _HTWK_SDCOMPARER_H_
