/**
Copyright (c)
Audi Autonomous Driving Cup. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: �This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.�
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS �AS IS� AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@author spiesra, sfeig, gjenschmischek
*/

#include "StateController.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_STATE_CONTROLLER, StateController);

#define SC_PROP_FILENAME "Maneuver File"
#define DEBUG_MAX_STATE_RUNNING_COUNTER 8 // if max to low inconsistency with maneuverlistid occur, complete state is never reached

StateController::StateController(const tChar *__info) : cFilter(__info), logger(FILTER_NAME), m_hTimer(NULL)
{
    carReady = false;

    SetPropertyInt(RUNNING_TYPE_PROPERTY, RT_NORMAL);
    SetPropertyBool(RUNNING_TYPE_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(RUNNING_TYPE_PROPERTY NSSUBPROP_VALUELIST, "0@Normal|1@Auto|2@Manual");
    SetPropertyStr(RUNNING_TYPE_PROPERTY NSSUBPROP_DESCRIPTION, "Defines which mode should be used.");

    SetPropertyInt(MANEUVER_PROPERTY, M_CROSSING_STRAIGHT);
    SetPropertyBool(MANEUVER_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(MANEUVER_PROPERTY NSSUBPROP_VALUELIST,
                   "100@Straight|101@Left|102@Right|103@Park_Cross|104@Park_Parallel|105@Pull_Out_Left|106@Pull_Out_Right|-100@Unknown");
    SetPropertyStr(MANEUVER_PROPERTY NSSUBPROP_DESCRIPTION, "Defines which Maneuver should be send in manual mode");

    SetPropertyInt(STATE_PROPERTY, stateCar_STARTUP);
    SetPropertyBool(STATE_PROPERTY NSSUBPROP_ISCHANGEABLE, tTrue);
    SetPropertyStr(STATE_PROPERTY NSSUBPROP_VALUELIST, "-2@StartUp|-1@Error|0@Ready|1@Running|2@Completed");
    SetPropertyStr(STATE_PROPERTY NSSUBPROP_DESCRIPTION, "Defines which State should be send in manual mode");

    m_nStateRunningCounter = 0;
}

StateController::~StateController()
{
}

tResult StateController::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    // pins need to be created at StageFirst
    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));

        m_i16CurrentManeuverID = 0;

        m_state = stateCar_STARTUP;
    }
    else if (eStage == StageGraphReady)
    {
        m_i16SectionListIndex = -1;
        m_i16ManeuverListIndex = -1;

        // no ids were set yet
        m_bIDsDriverStructSet = tFalse;
        m_bIDsJuryStructSet = tFalse;
        m_bIDsSetStateError = tFalse;
        m_bIDsSetStateRunning = tFalse;
        m_bIDsSetStateComplete = tFalse;
        m_bIDsSetStateReady = tFalse;
        m_bIDsRestartSection = tFalse;
        m_bIDsIncrementManeuver = tFalse;

        logger.StartLog();
    }
    RETURN_NOERROR;
}

tResult StateController::Start(__exception)
{
    createTimer();
    SendState(stateCar_STARTUP, 0);
    return cFilter::Start(__exception_ptr);
}

tResult StateController::Run(tInt nActivationCode, const tVoid *pvUserData, tInt szUserDataSize, ucom::IException **__exception_ptr/* =NULL */)
{
    GetProperties();

    if (runningType == RT_MANUAL)
    {
        RETURN_IF_FAILED(SendState(manualState, 0));
        RETURN_IF_FAILED(SendManeuver(manualManeuver));
        return cFilter::Run(nActivationCode, pvUserData, szUserDataSize, __exception_ptr);
    }

    if (stateCar_RUNNING == m_state || stateCar_READY == m_state)
    {
        SendState(m_state, m_i16CurrentManeuverID);
        RetrieveAndSendManeuver(m_i16SectionListIndex, m_i16ManeuverListIndex);
    }

    if (runningType == RT_AUTO &&
        stateCar_RUNNING == m_state &&
        DEBUG_MAX_STATE_RUNNING_COUNTER <= m_nStateRunningCounter)
    {
        m_nStateRunningCounter = 0;
        incrementManeuverID();
    }

    return cFilter::Run(nActivationCode, pvUserData, szUserDataSize, __exception_ptr);
}

tResult StateController::Stop(__exception)
{
    __synchronized_obj(m_oCriticalSectionTimerSetup);

    destroyTimer(__exception_ptr);

    // flushes the cache
    logger.EndLog();

    return cFilter::Stop(__exception_ptr);
}

tResult StateController::Shutdown(tInitStage eStage, __exception)
{
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult StateController::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pMediaSample);
    RETURN_IF_POINTER_NULL(pSource);

    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    //process the request to set state error pin
    if (pSource == &stateErrorInput)
    {
        tBool bValue = tFalse;
        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(boolDescription, pMediaSample, pCoder);

            // set the id if not already done
            if (!m_bIDsSetStateError)
            {
                pCoder->GetID("bValue", m_szIDSetStateErrorbValue);
                pCoder->GetID("ui32ArduinoTimestamp", m_szIDSetStateErrorArduinoTimestamp);
                m_bIDsSetStateError = tTrue;
            }
            // get value
            pCoder->Get(m_szIDSetStateErrorbValue, (tVoid *) &bValue);
        }
        if (bValue)
        {
            changeState(stateCar_ERROR);
        }
    }
        //process the request to set state running pin
    else if (pSource == &stateRunningInput)
    {
        tBool bValue = tFalse;
        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(boolDescription, pMediaSample, pCoder);
            // set the id if not already done
            if (!m_bIDsSetStateRunning)
            {
                pCoder->GetID("bValue", m_szIDSetStateRunningbValue);
                pCoder->GetID("ui32ArduinoTimestamp", m_szIDSetStateRunningArduinoTimestamp);
                m_bIDsSetStateRunning = tTrue;
            }
            // get value
            pCoder->Get(m_szIDSetStateRunningbValue, (tVoid *) &bValue);
        }
        if (bValue)
        {
            changeState(stateCar_RUNNING);
        }
    }
        //process the request to set state stop pin
    else if (pSource == &stateCompleteInput)
    {
        tBool bValue = tFalse;
        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(boolDescription, pMediaSample, pCoder);
            // set the id if not already done
            if (!m_bIDsSetStateComplete)
            {
                pCoder->GetID("bValue", m_szIDSetStateCompletebValue);
                pCoder->GetID("ui32ArduinoTimestamp", m_szIDIncrementManeuverArduinoTimestamp);
                m_bIDsSetStateComplete = tTrue;
            }
            // get value
            pCoder->Get(m_szIDSetStateCompletebValue, (tVoid *) &bValue);
        }
        if (bValue)
        {
            changeState(stateCar_COMPLETE);
        }
    }
        //process the request to set state ready pin
    else if (pSource == &stateReadyInput)
    {
        tBool bValue = tFalse;
        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(boolDescription, pMediaSample, pCoder);
            // set the id if not already done
            if (!m_bIDsSetStateReady)
            {
                pCoder->GetID("bValue", m_szIDSetStateReadybValue);
                pCoder->GetID("ui32ArduinoTimestamp", m_szIDSetStateReadyArduinoTimestamp);
                m_bIDsSetStateReady = tTrue;
            }
            // get value
            pCoder->Get(m_szIDSetStateReadybValue, (tVoid *) &bValue);
        }

        if (bValue)
        {
            carReady = true;
        }
    }
        //process the increment of maneuver id pin
    else if (pSource == &stateIncrementInput)
    {
        tBool bValue = tFalse;
        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(boolDescription, pMediaSample, pCoder);

            // set the id if not already done
            if (!m_bIDsIncrementManeuver)
            {
                pCoder->GetID("bValue", m_szIDIncrementManeuverbValue);
                pCoder->GetID("ui32ArduinoTimestamp", m_szIDIncrementManeuverArduinoTimestamp);
                m_bIDsIncrementManeuver = tTrue;
            }
            // get value
            pCoder->Get(m_szIDIncrementManeuverbValue, (tVoid *) &bValue);
        }
        if (bValue)
        {
            incrementManeuverID();
        }
    }
        //process the increment of restart section pin
    else if (pSource == &stateRestartInput)
    {
        tBool bValue = tFalse;
        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(boolDescription, pMediaSample, pCoder);

            // set the id if not already done
            if (!m_bIDsRestartSection)
            {
                pCoder->GetID("bValue", m_szIDRestartSectionbValue);
                pCoder->GetID("ui32ArduinoTimestamp", m_szIDRestartSectionArduinoTimestamp);
                m_bIDsRestartSection = tTrue;
            }

            // get value
            pCoder->Get(m_szIDRestartSectionbValue, (tVoid *) &bValue);
        }
        if (bValue)
        {
            resetSection();
        }
    }
        //process the request to the jury struct input pin
    else if (pSource == &juryInput)
    {
        tInt8 i8ActionID = -2;
        tInt16 i16entry = -1;

        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(juryDescription, pMediaSample, pCoder);

            // get the IDs for the items in the media sample
            if (!m_bIDsJuryStructSet)
            {
                pCoder->GetID("i8ActionID", m_szIDJuryStructI8ActionID);
                pCoder->GetID("i16ManeuverEntry", m_szIDJuryStructI16ManeuverEntry);
                m_bIDsJuryStructSet = tTrue;
            }

            pCoder->Get(m_szIDJuryStructI8ActionID, (tVoid *) &i8ActionID);
            pCoder->Get(m_szIDJuryStructI16ManeuverEntry, (tVoid *) &i16entry);
        }

        //change the state depending on the input
        // action_GETREADY --> stateCar_READY
        // action_START --> stateCar_RUNNING
        // action_STOP --> stateCar_STARTUP
        switch (juryActions(i8ActionID))
        {
            case action_GETREADY:
                logger.Log(cString::Format("Received: Request Ready with maneuver ID %d", i16entry).GetPtr());
                changeState(stateCar_READY);
                setManeuverID(i16entry);
                break;
            case action_START:
                logger.Log(cString::Format("Received: Run with maneuver ID %d", i16entry).GetPtr());
                if (i16entry == m_i16CurrentManeuverID)
                {
                    changeState(stateCar_RUNNING);
                }
                else
                {
                    logger.Log("The id of the action_START corresponds not with the id of the last action_GETREADY");
                    setManeuverID(i16entry);
                    changeState(stateCar_RUNNING);
                }
                break;
            case action_STOP:
                logger.Log(cString::Format("Received: Stop with maneuver ID %d", i16entry).GetPtr());
                changeState(stateCar_STARTUP);
                break;
        }
    }
    else if (pSource == &maneuverListInput && maneuverListDescription != NULL)
    {
        {   // focus for sample read lock
            __adtf_sample_read_lock_mediadescription(maneuverListDescription, pMediaSample, pCoder);

            std::vector<tSize> vecDynamicIDs;

            // retrieve number of elements by providing NULL as first parameter
            tSize szBufferSize = 0;
            if (IS_OK(pCoder->GetDynamicBufferIDs(NULL, szBufferSize)))
            {
                // create a buffer depending on the size element
                tChar *pcBuffer = new tChar[szBufferSize];
                vecDynamicIDs.resize(szBufferSize);
                // get the dynamic ids (we already got the first "static" size element)
                if (IS_OK(pCoder->GetDynamicBufferIDs(&(vecDynamicIDs.front()), szBufferSize)))
                {
                    // iterate over all elements
                    for (tUInt32 nIdx = 0; nIdx < vecDynamicIDs.size(); ++nIdx)
                    {
                        // get the value and put it into the buffer
                        pCoder->Get(vecDynamicIDs[nIdx], (tVoid *) &pcBuffer[nIdx]);
                    }

                    // set the resulting char buffer to the string object
                    m_strManeuverFileString = (const tChar *) pcBuffer;
                }
                // cleanup the buffer
                delete pcBuffer;
            }
        }
        // trigger loading maneuver list and update the ui
        loadManeuverList();
    }
    else if (pSource == &resetInput)
    {
        tResettableModule::ResettableModuleEnum reset;

        {
            __adtf_sample_read_lock_mediadescription(enumDescription, pMediaSample, pCoder);
            pCoder->Get("tEnumValue", (tVoid *) &reset);
        }

        if (tResettableModule::Maneuver == reset)
        {
            logger.Log("Resetting.", false);
            SendManeuver(M_UNKNOWN);
        }
    }

    RETURN_NOERROR;
}

tResult StateController::SendState(stateCar state, tInt16 i16ManeuverEntry)
{
    __synchronized_obj(m_oCriticalSectionTransmit);

    cObjectPtr<IMediaSample> driverMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &driverMediaSample));

    cObjectPtr<IMediaSerializer> driverSerializer;
    driveStructDescription->GetMediaSampleSerializer(&driverSerializer);
    tInt nSize = driverSerializer->GetDeserializedSize();

    tInt8 bValue = tInt8(state);

    RETURN_IF_FAILED(driverMediaSample->AllocBuffer(nSize));
    {   // focus for sample write lock
        __adtf_sample_write_lock_mediadescription(driveStructDescription, driverMediaSample, pCoder);
        // get the IDs for the items in the media sample
        if (!m_bIDsDriverStructSet)
        {
            pCoder->GetID("i8StateID", m_szIDDriverStructI8StateID);
            pCoder->GetID("i16ManeuverEntry", m_szIDDriverStructI16ManeuverEntry);
            m_bIDsDriverStructSet = tTrue;
        }
        pCoder->Set(m_szIDDriverStructI8StateID, (tVoid *) &bValue);
        pCoder->Set(m_szIDDriverStructI16ManeuverEntry, (tVoid *) &i16ManeuverEntry);
    }

    driverMediaSample->SetTime(_clock->GetStreamTime());
    driveStructOutput.Transmit(driverMediaSample);

    switch (state)
    {
        case stateCar_ERROR:
            logger.Log(cString::Format("Send state: ERROR, Maneuver ID %d", i16ManeuverEntry).GetPtr());
            break;
        case stateCar_READY:
            logger.Log(cString::Format("Send state: READY, Maneuver ID %d", i16ManeuverEntry).GetPtr());
            break;
        case stateCar_RUNNING:
            ++m_nStateRunningCounter;
            logger.Log(cString::Format("Send state: RUNNING, Maneuver ID %d, RunningCounter %d",
                                       i16ManeuverEntry, m_nStateRunningCounter).GetPtr());
            break;
        case stateCar_COMPLETE:
            logger.Log(cString::Format("Send state: COMPLETE, Maneuver ID %d", i16ManeuverEntry).GetPtr());
            break;
        case stateCar_STARTUP:
            logger.Log(cString::Format("Send state: STARTUP, Maneuver ID %d", i16ManeuverEntry).GetPtr());
            break;
    }

    cObjectPtr<IMediaSample> stateMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &stateMediaSample));

    cObjectPtr<IMediaSerializer> stateSerializer;
    enumDescription->GetMediaSampleSerializer(&stateSerializer);
    nSize = stateSerializer->GetDeserializedSize();

    RETURN_IF_FAILED(stateMediaSample->AllocBuffer(nSize));
    {   // focus for sample write lock
        __adtf_sample_write_lock_mediadescription(enumDescription, stateMediaSample, pCoder);
        // get the IDs for the items in the media sample
        if (!m_bIDEnumBox)
        {
            pCoder->GetID("tEnumValue", m_szIDEnumBoxValue);
            m_bIDEnumBox = tTrue;
        }
        pCoder->Set(m_szIDEnumBoxValue, (tVoid *) &state);
    }
    stateMediaSample->SetTime(_clock->GetStreamTime());
    carStateOutput.Transmit(stateMediaSample);

    RETURN_NOERROR;
}

tResult StateController::loadManeuverList()
{
    m_sectorList.clear();
    cDOM oDOM;
    oDOM.FromString(m_strManeuverFileString);
    cDOMElementRefList oSectorElems;
    cDOMElementRefList oManeuverElems;

    //read first Sector Elem
    if (IS_OK(oDOM.FindNodes("AADC-Maneuver-List/AADC-Sector", oSectorElems)))
    {
        //iterate through sectors
        for (cDOMElementRefList::iterator itSectorElem = oSectorElems.begin(); itSectorElem != oSectorElems.end(); ++itSectorElem)
        {
            //if sector found
            tSector sector;
            sector.id = (*itSectorElem)->GetAttributeUInt32("id");

            if (IS_OK((*itSectorElem)->FindNodes("AADC-Maneuver", oManeuverElems)))
            {
                //iterate through maneuvers
                for (cDOMElementRefList::iterator itManeuverElem = oManeuverElems.begin(); itManeuverElem != oManeuverElems.end(); ++itManeuverElem)
                {
                    tAADC_Maneuver man;
                    man.id = (*itManeuverElem)->GetAttributeUInt32("id");
                    man.action = (*itManeuverElem)->GetAttribute("action");
                    sector.maneuverList.push_back(man);
                    logger.Log(cString::Format("id = %d action = %s", man.id, man.action.GetPtr()).GetPtr());
                }
            }
            m_sectorList.push_back(sector);
        }
    }
    if (oSectorElems.size() > 0)
    {
        logger.Log("Loaded Maneuver file successfully.");
        m_i16SectionListIndex = 0;
        m_i16ManeuverListIndex = 0;
    }
    else
    {
        logger.Log("No valid Maneuver Data found!");
        m_i16SectionListIndex = -1;
        m_i16ManeuverListIndex = -1;
        RETURN_ERROR(ERR_INVALID_FILE);
    }
    RETURN_NOERROR;
}

tResult StateController::changeState(stateCar newState)
{
    if (runningType == RT_MANUAL)
    {
        RETURN_NOERROR;
    }

    // if state is the same do nothing
    if (m_state == newState)
    {
        RETURN_NOERROR;
    }

    // don't switch to ready if car is not ready
    if (stateCar_READY == newState && !carReady)
    {
        RETURN_NOERROR;
    }

    //to secure the state is sent at least one time
    SendState(newState, m_i16CurrentManeuverID);

    //handle the timer depending on the state
    switch (newState)
    {
        case stateCar_ERROR:
            carReady = false;
            destroyTimer();
            logger.Log("State ERROR reached");
            break;
        case stateCar_STARTUP:
            carReady = false;
            destroyTimer();
            logger.Log("State STARTUP reached");
            break;
        case stateCar_READY:
            carReady = false;
            logger.Log(cString::Format("State READY reached (ID %d)", m_i16CurrentManeuverID).GetPtr());
            break;
        case stateCar_RUNNING:
            carReady = false;
            if (m_state != stateCar_READY)
            {
                logger.Log("Invalid state change to Car_RUNNING. Car_READY was not reached before");
            }
            logger.Log("State RUNNING reached");
            break;
        case stateCar_COMPLETE:
            carReady = false;
            destroyTimer();
            logger.Log("State COMPLETE reached");
            break;
    }

    m_state = newState;
    RETURN_NOERROR;
}

tResult StateController::createTimer()
{
    // creates timer with 0.5 sec
    __synchronized_obj(m_oCriticalSectionTimerSetup);
    // additional check necessary because input jury structs can be mixed up because every signal is sent three times
    if (m_hTimer == NULL)
    {
        m_hTimer = _kernel->TimerCreate(tTimeStamp(0.5 * 1000000), 0, static_cast<IRunnable *>(this),
                                        NULL, NULL, 0, 0, adtf_util::cString::Format("%s.timer", OIGetInstanceName()));
    }
    else
    {
        LOG_ERROR("Timer is already running. Unable to create a new one.");
    }
    RETURN_NOERROR;
}

tResult StateController::destroyTimer(__exception)
{
    __synchronized_obj(m_oCriticalSectionTimerSetup);
    //destroy timer
    if (m_hTimer != NULL)
    {
        tResult nResult = _kernel->TimerDestroy(m_hTimer);
        if (IS_FAILED(nResult))
        {
            LOG_ERROR("Unable to destroy the timer.");
            THROW_ERROR(nResult);
        }
        m_hTimer = NULL;
    }
        //check if handle for some unknown reason still exists
    else
    {
        LOG_WARNING("Timer handle not set, but I should destroy the timer. Try to find a timer with my name.");
        tHandle hFoundHandle = _kernel->FindHandle(adtf_util::cString::Format("%s.timer", OIGetInstanceName()));
        if (hFoundHandle)
        {
            tResult nResult = _kernel->TimerDestroy(hFoundHandle);
            if (IS_FAILED(nResult))
            {
                LOG_ERROR("Unable to destroy the found timer.");
                THROW_ERROR(nResult);
            }
        }
    }

    RETURN_NOERROR;
}

tResult StateController::incrementManeuverID()
{
    //check if list was successfully loaded in init
    if (m_i16ManeuverListIndex != -1 && m_i16SectionListIndex != -1)
    {
        //check if end of section is reached
        if (m_sectorList[m_i16SectionListIndex].maneuverList.size() > tUInt(m_i16ManeuverListIndex + 1))
        {
            //increment only maneuver index
            m_i16ManeuverListIndex++;
            m_i16CurrentManeuverID++;
        }
        else
        {
            //end of section was reached and another section is in list
            if (m_sectorList.size() > tUInt(m_i16SectionListIndex + 1))
            {
                //reset maneuver index to zero and increment section list index
                m_i16SectionListIndex++;
                m_i16ManeuverListIndex = 0;
                m_i16CurrentManeuverID++;
                if (m_sectorList[m_i16SectionListIndex].maneuverList[m_i16ManeuverListIndex].id != m_i16CurrentManeuverID)
                {
                    logger.Log("Inconsistancy in maneuverfile detected. Please check the file ");
                }
            }
            else
            {
                logger.Log("End of maneuverlist reached, cannot increment any more");
                changeState(stateCar_COMPLETE);
            }
        }
    }
    else
    {
        logger.Log("Could not set new maneuver id because no maneuver list was loaded");
    }
    logger.Log(cString::Format("Increment Manevuer ID: Sectionindex is %d, Maneuverindex is %d, ID is %d",
                               m_i16SectionListIndex, m_i16ManeuverListIndex, m_i16CurrentManeuverID).GetPtr());
    RETURN_NOERROR;
}

tResult StateController::resetSection()
{
    //maneuver list index to zero, and current maneuver id to first element in list
    m_i16ManeuverListIndex = 0;
    m_i16CurrentManeuverID = m_sectorList[m_i16SectionListIndex].maneuverList[m_i16ManeuverListIndex].id;
    logger.Log(cString::Format("Reset section: Sectionindex is %d, Maneuverindex is %d, ID is %d",
                               m_i16SectionListIndex, m_i16ManeuverListIndex, m_i16CurrentManeuverID).GetPtr());
    RETURN_NOERROR;
}

tResult StateController::setManeuverID(tInt maneuverId)
{
    //look for the right section id and write it to section combobox
    for (unsigned int i = 0; i < m_sectorList.size(); i++)
    {
        for (unsigned int j = 0; j < m_sectorList[i].maneuverList.size(); j++)
        {
            if (m_i16CurrentManeuverID == m_sectorList[i].maneuverList[j].id)
            {
                m_i16SectionListIndex = i;
                m_i16ManeuverListIndex = j;
                m_i16CurrentManeuverID = maneuverId;
                logger.Log(cString::Format("Sectionindex is %d, Maneuverindex is %d, ID is %d",
                                           m_i16SectionListIndex, m_i16ManeuverListIndex, m_i16CurrentManeuverID).GetPtr());
                break;
            }
        }
    }
    RETURN_NOERROR;
}

tResult StateController::RetrieveAndSendManeuver(tInt16 sectionIndex, tInt16 maneuverIndex)
{
    cString currentManeuverString = m_sectorList[sectionIndex].maneuverList[maneuverIndex].action;
    tManeuver maneuver = DetermineManeuver(currentManeuverString);

    return SendManeuver(maneuver);
}

tResult StateController::SendManeuver(tManeuver maneuver)
{
    __synchronized_obj(m_oCriticalSectionTransmit);

    cObjectPtr<IMediaSample> pMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &pMediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    enumDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));
    {   // focus for sample write lock
        __adtf_sample_write_lock_mediadescription(enumDescription, pMediaSample, pCoder);
        // get the IDs for the items in the media sample
        if (!m_bIDEnumBox)
        {
            pCoder->GetID("tEnumValue", m_szIDEnumBoxValue);
            m_bIDEnumBox = tTrue;
        }
        pCoder->Set(m_szIDEnumBoxValue, (tVoid *) &maneuver);
    }
    pMediaSample->SetTime(_clock->GetStreamTime());
    maneuverOutput.Transmit(pMediaSample);

    RETURN_NOERROR;
}

tManeuver StateController::DetermineManeuver(cString name)
{
    if (name == "straight")
    {
        return M_CROSSING_STRAIGHT;
    }
    else if (name == "left")
    {
        return M_CROSSING_LEFT;
    }
    else if (name == "right")
    {
        return M_CROSSING_RIGHT;
    }
    else if (name == "cross_parking")
    {
        return M_PARK_CROSS;
    }
    else if (name == "parallel_parking")
    {
        return M_PARK_PARALLEL;
    }
    else if (name == "pull_out_left")
    {
        return M_PULL_OUT_LEFT;
    }
    else if (name == "pull_out_right")
    {
        return M_PULL_OUT_RIGHT;
    }

    return M_UNKNOWN;
}

void StateController::GetProperties()
{
    runningType = static_cast<RUNNING_TYPE>(GetPropertyInt(RUNNING_TYPE_PROPERTY));
    manualManeuver = static_cast<tManeuver>(GetPropertyInt(MANEUVER_PROPERTY));
    manualState = static_cast<stateCar>(GetPropertyInt(STATE_PROPERTY));

    if (RT_MANUAL == runningType)
    {
        logger.Log(cString::Format("RT: %d", runningType).GetPtr());
        logger.Log(cString::Format("Man: %d", manualManeuver).GetPtr());
        logger.Log(cString::Format("State: %d", manualState).GetPtr());
    }
}

tResult StateController::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get signal
    tChar const *strSignal = descManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strSignal);
    signalMediaType = new cMediaType(0, 0, 0, "tSignalValue", strSignal, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(signalMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &signalDescription));

    // get bool
    tChar const *strBool = descManager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(strBool);
    boolMediaType = new cMediaType(0, 0, 0, "tBoolSignalValue", strBool, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(boolMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &boolDescription));

    // get enum
    tChar const *strEnumBox = descManager->GetMediaDescription("tEnumBox");
    RETURN_IF_POINTER_NULL(strEnumBox);
    enumMediaType = new cMediaType(0, 0, 0, "tEnumBox", strEnumBox, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(enumMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &enumDescription));

    // get drive
    tChar const *strDriveStruct = descManager->GetMediaDescription("tDriverStruct");
    RETURN_IF_POINTER_NULL(strDriveStruct);
    driveStructMediaType = new cMediaType(0, 0, 0, "tDriverStruct", strDriveStruct, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(driveStructMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &driveStructDescription));

    // get jury
    tChar const *strJuryStruct = descManager->GetMediaDescription("tJuryStruct");
    RETURN_IF_POINTER_NULL(strJuryStruct);
    juryMediaType = new cMediaType(0, 0, 0, "tJuryStruct", strJuryStruct, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(juryMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &juryDescription));

    // get jury
    tChar const *strManeuverList = descManager->GetMediaDescription("tManeuverList");
    RETURN_IF_POINTER_NULL(strManeuverList);
    maneuverListMediaType = new cMediaType(0, 0, 0, "tManeuverList", strManeuverList, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(maneuverListMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &maneuverListDescription));

    RETURN_NOERROR;
}

tResult StateController::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(juryInput.Create("Jury_Struct", juryMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&juryInput));

    RETURN_IF_FAILED(stateReadyInput.Create("Set_State_Ready", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&stateReadyInput));

    RETURN_IF_FAILED(stateRunningInput.Create("Set_State_Running", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&stateRunningInput));

    RETURN_IF_FAILED(stateCompleteInput.Create("Set_State_Complete", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&stateCompleteInput));

    RETURN_IF_FAILED(stateErrorInput.Create("Set_State_Error", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&stateErrorInput));

    RETURN_IF_FAILED(stateIncrementInput.Create("Increment_Maneuver_ID", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&stateIncrementInput));

    RETURN_IF_FAILED(stateRestartInput.Create("Restart_Section", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&stateRestartInput));

    RETURN_IF_FAILED(resetInput.Create("Reset_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&resetInput));

    RETURN_IF_FAILED(maneuverListInput.Create("Maneuver_List", maneuverListMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&maneuverListInput));

    RETURN_NOERROR;
}

tResult StateController::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(driveStructOutput.Create("Driver_Struct", driveStructMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&driveStructOutput));

    RETURN_IF_FAILED(maneuverOutput.Create("Maneuver_Enum", enumMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&maneuverOutput));

    RETURN_IF_FAILED(carStateOutput.Create("CarState_Enum", enumMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&carStateOutput));

    RETURN_NOERROR;
}
