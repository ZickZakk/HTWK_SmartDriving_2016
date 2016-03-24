/*
@author: ffreihube
*/
//#############################################################################
#ifndef _HTWK_MEDIANVALUES_FILTER_H_
#define _HTWK_MEDIANVALUES_FILTER_H_

#include "stdafx.h"
#include "./../htwk_median_values/Median_values_lib.h"
#include "../htwk_logger/Logger.h"

#include <string>
#include <typeinfo>

//#############################################################################
#define OID "htwk.htwk_median_values_filter"
#define FILTER_NAME "HTWK MedianValues"

//#############################################################################
class cMedianValues : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cInputPin m_oInput;
        cOutputPin m_oOutput;
        cMedianValuesLib *m_pMedianValues;
        Logger logger;

    public:
        /**
         * @brief Constructor.
         */
        cMedianValues(const tChar *__info = 0);

        /**
          * @brief Destructor.
          */
        virtual ~cMedianValues(void);

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

        /*! Frame Size*/
        tUInt32 m_ui32FrameSize;

        /*! Buffer Size*/
        tUInt32 m_ui32BufferSize;

        tResult ProcessInput(IMediaSample *pMediaSample);

        tResult Transmit(tFloat32 f32Value, IMediaSample *pMediaSample, tUInt32 i);

        /*! Coder Descriptor for the pins*/
        cObjectPtr<IMediaTypeDescription> m_pDescriptionSignal;
        /*! the id for the f32value of the media description for the pins */
        tBufferID m_szIDSignalF32Value;
        /*! the id for the arduino time stamp of the media description for the pins */
        tBufferID m_szIDSignalArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsSignalSet;
};

#endif // _HTWK_MEDIANVALUES_FILTER_H_
