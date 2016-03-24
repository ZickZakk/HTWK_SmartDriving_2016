//
// Created by pbachmann on 2/11/16.
//

#include "StateControllerTest.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_STATE_CONTROLLER_TEST, StateControllerTest);

StateControllerTest::StateControllerTest(const tChar *aChar) : logger(FILTER_NAME)
{
}

StateControllerTest::~StateControllerTest()
{
}

tResult StateControllerTest::Init(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    // pins need to be created at StageFirst
    if (eStage == StageFirst)
    {

        cObjectPtr<IMediaDescriptionManager> pDescManager;
        RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                             IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                             (tVoid **) &pDescManager,
                                             __exception_ptr));

        // output maneuver enum
        tChar const *strEnumBox = pDescManager->GetMediaDescription("tEnumBox");
        RETURN_IF_POINTER_NULL(strEnumBox);
        cObjectPtr<IMediaType> pTypeEnumBox = new cMediaType(0, 0, 0, "tEnumBox", strEnumBox, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
        RETURN_IF_FAILED(pTypeEnumBox->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionEnumBox));

        RETURN_IF_FAILED(m_ManeuverInputPin.Create("Maneuver_Enum", pTypeEnumBox, this));
        RETURN_IF_FAILED(RegisterPin(&m_ManeuverInputPin));
        RETURN_IF_FAILED(m_CarStateInputPin.Create("CarState_Enum", pTypeEnumBox, this));
        RETURN_IF_FAILED(RegisterPin(&m_CarStateInputPin));

        state = tCarState::Startup;
        maneuver = tManeuver::M_UNKNOWN;
    }

    RETURN_NOERROR;
}

tResult StateControllerTest::Shutdown(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult StateControllerTest::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pMediaSample);
    RETURN_IF_POINTER_NULL(pSource);
    if (nEventCode == IPinEventSink::PE_MediaSampleReceived)
    {
        //process the request to set state error pin
        if (pSource == &m_CarStateInputPin)
        {
            // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(descriptionEnumBox, pMediaSample, pCoder);

            // set the id if not already done
            if (!m_bIDEnumBox)
            {
                pCoder->GetID("tEnumValue", m_szIDEnumBoxValue);
                m_bIDEnumBox = tTrue;
            }
            // get value
            pCoder->Get(m_szIDEnumBoxValue, (tVoid *) &state);

            string stateString = tCarState::ToString(state);

            logger.Log(cString::Format("State Received: %s", stateString.c_str()).GetPtr());
        }
        else if (pSource == &m_ManeuverInputPin)
        {
            tInt maneuverId;

            {
                __adtf_sample_read_lock_mediadescription(descriptionEnumBox, pMediaSample, pCoder);
                pCoder->Get("tEnumValue", (tVoid *) &maneuverId);
            }

            maneuver = static_cast<tManeuver::ManeuverEnum>(maneuverId);
            string maneuverString = tManeuver::ToString(maneuver);

            logger.Log(cString::Format("Maneuver Received: %s", maneuverString.c_str()).GetPtr());
        }
    }

    RETURN_NOERROR;
}
