/*
@author: ffreihube
*/
//#############################################################################
#ifndef HTWK_VELOCITYCALC_FILTER_H
#define HTWK_VELOCITYCALC_FILTER_H

#include "stdafx.h"
#include "../htwk_logger/Logger.h"
#include "opencv2/opencv.hpp"
#include "../htwk_utils/CalcOffset.h"
#include "../htwk_calc_movement/MovementCalculator.h"

#include <string>
#include <typeinfo>

//#############################################################################
#define OID "htwk.Movement_analyzer"
#define FILTER_NAME "HTWK Movement Analyzer"

//#############################################################################
class cMovementAnalyzer : public adtf::cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cInputPin m_oInputSpeed;
        cInputPin m_oInputAccX;
        cInputPin m_oInputAccY;
        cInputPin m_oInputDistance;

        cOutputPin m_oOutputCalcSpeed;
        cOutputPin m_oOutputCalcAccX;
        cOutputPin m_oOutputCalcAccY;
        cOutputPin m_oOutputCalcDistance;

        Logger logger;

        CalcOffset *calcOffsetAccX;
        CalcOffset *calcOffsetAccY;

        MovementCalculator *movementCalculator;

    public:
        /**
         * @brief Constructor.
         */
        cMovementAnalyzer(const tChar *__info = 0);

        /**
          * @brief Destructor.
          */
        virtual ~cMovementAnalyzer(void);

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

        tResult ProcessInput(IMediaSample *pMediaSample, IPin *pPin);

        tResult Transmit(tFloat32 f32Value, IMediaSample *pMediaSample, tUInt32 i, IPin *pin);

        /*! Coder Descriptor for the pins*/
        cObjectPtr<IMediaTypeDescription> m_pDescriptionSignal;
        /*! the id for the f32value of the media description for the pins */
        tBufferID m_szIDSignalF32Value;
        /*! the id for the arduino time stamp of the media description for the pins */
        tBufferID m_szIDSignalArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsSignalSet;
        /*! Adjusts the acceleration-values, calculates initial Offset */
        tFloat32 adjustAcc(tFloat32 f32Value, CalcOffset *pOffset);

        tFloat32 newDistance;


};

#endif // HTWK_VELOCITYCALC_FILTER_H
