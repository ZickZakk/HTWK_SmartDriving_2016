//
// Created by pbachmann on 2/11/16.
//

#ifndef HTWK_2016_STATECONTROLLERTEST_H
#define HTWK_2016_STATECONTROLLERTEST_H

#define OID_STATE_CONTROLLER_TEST "htwk.stateControllerTest"
#define FILTER_NAME "HTWK State Controller Test"

#include "stdafx.h"

#include <tCarState.h>
#include <JuryEnums.h>
#include <Logger.h>
#include <tManeuver.h>

class StateControllerTest : public cFilter
{
    ADTF_FILTER(OID_STATE_CONTROLLER_TEST, FILTER_NAME, OBJCAT_DataFilter)

    private:
        Logger logger;
    tCarState::CarStateEnum state;
    tManeuver::ManeuverEnum maneuver;

        /*! Coder Descriptor for tEnumBox*/
        cObjectPtr<IMediaTypeDescription> descriptionEnumBox;
        /*! the id for the enum box output of the media description */
        tBufferID m_szIDEnumBoxValue;
        /*! indicates if bufferIDs were set */
        tBool m_bIDEnumBox;

        /*! output pin for current maneuver enum */
        cInputPin m_ManeuverInputPin;

        /*! output pin for current car state enum */
        cInputPin m_CarStateInputPin;

    public: // construction
        StateControllerTest(const tChar *);

        virtual ~StateControllerTest();

        /*! overrides cFilter */
        virtual tResult Init(tInitStage eStage, __exception = NULL);

        /*! overrides cFilter */
        virtual tResult Shutdown(tInitStage eStage, __exception = NULL);

        /*! overrides cFilter */
        tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample);
};


#endif //HTWK_2016_STATECONTROLLERTEST_H
