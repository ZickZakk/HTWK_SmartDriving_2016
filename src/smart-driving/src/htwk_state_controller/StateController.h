/**
Copyright (c)
Audi Autonomous Driving Cup. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: �This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.�
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS �AS IS� AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@author spiesra, sfeig
*/
#ifndef _STATE_CONTROLLER_HEADER
#define _STATE_CONTROLLER_HEADER

#define OID_STATE_CONTROLLER "htwk.stateController"
#define FILTER_NAME "HTWK State Controller"
#define RUNNING_TYPE_PROPERTY "Running_type"
#define MANEUVER_PROPERTY "Maneuver"
#define STATE_PROPERTY "State"

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "../htwk_structs/juryEnums.h"
#include "../htwk_structs/tManeuver.h"
#include "../htwk_structs/Maneuverlist.h"
#include "../htwk_structs/tResettableModule.h"

/*!This filter implements a state machine that defines the several possible state of the vehicle and manages
 * the communication with the Jury module. This filter can be used by the teams as reference to build their own
 * individual state machine to communicate and respond correctly with the Jury Module.

This filter implements the following states which are defined in juryEnums.h
enum stateCar{
    stateCar_ERROR=-1,
    stateCar_READY=0,
    stateCar_RUNNING=1,
    stateCar_COMPLETE=2,
    stateCar_STARTUP=-2
};

The Jury uses the following actions to control the vehicle:
enum juryActions{
    action_STOP=-1,
    action_GETREADY=0,
    action_START=1
};

The vehicle transmits the following states to the jury:
enum stateCar{
    stateCar_ERROR=-1,
    stateCar_READY=0,
    stateCar_RUNNING=1,
    stateCar_COMPLETE=2,
    stateCar_STARTUP=-2
};

The received actions from the jury or the receiving on a sample on the pins Set_State_xxx forces the state machine to change its state.

After receiving an action_STOP the vehicle has to change to a state from which it can be activated again. Possible would be change to state_STARTUP

As already mentioned this filter can be used in two different modes:
�	The filter automatically reacts on incoming Jury Structs on the inputpin Jury_Struct
�	The filter changes its state depending on inputs on the inputpins Set_State_Ready, Set_State_Running, Set_State_Complete, Set_State_Error

Mode 1: Jury Structs:
After starting the vehicle is in state state_STARTUP. In this state no DriverStructs are generated as output and the vehicle is waiting for the action action_GETREADY from the Jury. When such an action_GETREADY is received the state changes to state_READY and starts with cyclic sending of DriverStruct with the state state_READY. In this Media Sample the current ManeuverID is also contained, after first changing to state_READY it starts with ID 0.
When the filter receives now an action_START command the filter changes its state to state_RUNNING and sends this state periodically to the Jury Module. At this state change the filter verifies if the ManeuverID of the state_READY command and the action_START command are the same. If it is unequal a warning is emitted but the state is also changed.
If a sample of type tBoolSignalValue with a TRUE in the bValue element is received on the input pin Increment_Maneuver_ID  while being in state state_RUNNING the current ManeuverID is incremented.
If the filter receives now an action_STOP command it changes its state to state_STARTUP and stops transmitting states to the jury.
Mode 2: Using the Set_State_xxx pins
Media Samples of type tBoolSignalValue have to be transmitted to the pins Set_State_Ready, Set_State_Running, Set_State_Complete, Set_State_Error, Increment_Maneuver_ID and Restart_Section. If the sample contains a TRUE in the bValue element the filter reacts as the following list shows:
�	Set_State_Ready: change to state_READY
�	Set_State_Running: change to state_RUNNING
�	Set_State_Complete: change to  state_COMPLETE
�	Set_State_Error: change to state_ERROR
�	Increment_Maneuver_ID: increments the counter for the ManeuverID
�	Restart_Section: resets the ManeuverID to the first ID in the current section.
Note: To perform the last two actions a ManeuverFile has to be set in the Properties.
The Media Samples with the tBoolSignalValues can be generated with the Filter Bool Value Generator.

*/
class StateController : public adtf::cFilter
{
    ADTF_FILTER(OID_STATE_CONTROLLER, FILTER_NAME, OBJCAT_DataFilter);
    private:
        Logger logger;
        tUInt8 m_nStateRunningCounter;

        enum RUNNING_TYPE
        {
            RT_NORMAL = 0,
            RT_AUTO = 1,
            RT_MANUAL = 2
        } runningType;

        tManeuver manualManeuver;
        stateCar manualState;
        tBool carReady;

        cObjectPtr<IMediaType> signalMediaType;
        cObjectPtr<IMediaTypeDescription> signalDescription;

        cObjectPtr<IMediaType> boolMediaType;
        cObjectPtr<IMediaTypeDescription> boolDescription;

        cObjectPtr<IMediaType> enumMediaType;
        cObjectPtr<IMediaTypeDescription> enumDescription;

        cObjectPtr<IMediaType> juryMediaType;
        cObjectPtr<IMediaTypeDescription> juryDescription;

        cObjectPtr<IMediaType> driveStructMediaType;
        cObjectPtr<IMediaTypeDescription> driveStructDescription;

        cObjectPtr<IMediaType> maneuverListMediaType;
        cObjectPtr<IMediaTypeDescription> maneuverListDescription;

    public: // construction
        StateController(const tChar *);

        virtual ~StateController();

        /*! overrides cFilter */
        virtual tResult Init(tInitStage eStage, __exception = NULL);

        /*! overrides cFilter */
        virtual tResult Start(__exception = NULL);

        /*! overrides cFilter */
        virtual tResult Stop(__exception = NULL);

        /*! overrides cFilter */
        virtual tResult Shutdown(tInitStage eStage, __exception = NULL);

        /*! overrides cFilter */
        tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample);

        /*! overrides cFilter */
        tResult Run(tInt nActivationCode, const tVoid *pvUserData, tInt szUserDataSize, ucom::IException **__exception_ptr = NULL);

        /*! signal for sending the state
        @param i8StateID state to be sent; -1: error, 0: Ready, 1: Running
        @param i16ManeuverEntry current entry to be sent
        */
        tResult SendState(stateCar state, tInt16 i16ManeuverEntry);

        /*! creates the timer for the cyclic transmits*/
        tResult createTimer();

        /*! destroys the timer for the cyclic transmits*/
        tResult destroyTimer(__exception = NULL);

        /*! increments the id of the maneuver id by one and updates the list indexes*/
        tResult incrementManeuverID();

        /*! resets the counters to the start of the current section*/
        tResult resetSection();

        /*! changes the state of the car
         @param newState the new state of the car
         */
        tResult changeState(stateCar newState);

        /*! set the maneuver id and find the correct indexes
        @param maneuverId the id of the maneuver which has to be set*/
        tResult setManeuverID(tInt maneuverId);

        /*! this functions loads the maneuver list given in the properties*/
        tResult loadManeuverList();

        /*! the id for the i8StateID of the media description */
        tBufferID m_szIDJuryStructI8ActionID;
        /*! the id for the i16ManeuverEntry of the media description data */
        tBufferID m_szIDJuryStructI16ManeuverEntry;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsJuryStructSet;

        /*! the id for the i8StateID of the media description */
        tBufferID m_szIDDriverStructI8StateID;
        /*! the id for the i16ManeuverEntry of the media description data */
        tBufferID m_szIDDriverStructI16ManeuverEntry;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsDriverStructSet;

        /*! the id for the bool value output of the media description */
        tBufferID m_szIDSetStateErrorbValue;
        /*! the id for the arduino timestamp output of the media description */
        tBufferID m_szIDSetStateErrorArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsSetStateError;

        /*! the id for the bool value output of the media description */
        tBufferID m_szIDSetStateRunningbValue;
        /*! the id for the arduino timestamp output of the media description */
        tBufferID m_szIDSetStateRunningArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsSetStateRunning;

        /*! the id for the bool value output of the media description */
        tBufferID m_szIDSetStateCompletebValue;
        /*! the id for the arduino timestamp output of the media description */
        tBufferID m_szIDSetStateArduinoTimestampComplete;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsSetStateComplete;

        /*! the id for the bool value output of the media description */
        tBufferID m_szIDSetStateReadybValue;
        /*! the id for the arduino timestamp output of the media description */
        tBufferID m_szIDSetStateReadyArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsSetStateReady;

        /*! the id for the bool value output of the media description */
        tBufferID m_szIDRestartSectionbValue;
        /*! the id for the arduino timestamp output of the media description */
        tBufferID m_szIDRestartSectionArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsRestartSection;

        /*! the id for the bool value output of the media description */
        tBufferID m_szIDIncrementManeuverbValue;
        /*! the id for the arduino timestamp output of the media description */
        tBufferID m_szIDIncrementManeuverArduinoTimestamp;
        /*! indicates if bufferIDs were set */
        tBool m_bIDsIncrementManeuver;

        /*! the id for the enum box output of the media description */
        tBufferID m_szIDEnumBoxValue;
        /*! indicates if bufferIDs were set */
        tBool m_bIDEnumBox;

        /*! input pin for the run command */
        cInputPin juryInput;

        /*! input pin for the set state error command */
        cInputPin stateErrorInput;

        /*! input pin for the set state running command */
        cInputPin stateRunningInput;

        /*! input pin for the set state stop command */
        cInputPin stateCompleteInput;

        /*! input pin for the set state ready command */
        cInputPin stateReadyInput;

        /*! input pin for the set state ready command */
        cInputPin stateRestartInput;

        cInputPin resetInput;

        /*! input pin for the set state ready command */
        cInputPin stateIncrementInput;
        /*! input pin for the maneuver list*/
        cInputPin maneuverListInput;

        /*! output pin for state from driver */
        cOutputPin driveStructOutput;

        /*! output pin for current maneuver enum */
        cOutputPin maneuverOutput;

        /*! output pin for current car state enum */
        cOutputPin carStateOutput;

        /*! this is the filename of the maneuver list*/
        cString m_strManeuverFileString;

        /*! this is the list with all the loaded sections from the maneuver list*/
        std::vector<tSector> m_sectorList;

        /*! holds the current state of the car */
        stateCar m_state;

        /*! holds the current maneuver id of the car*/
        tInt16 m_i16CurrentManeuverID;

        /*! holds the current index of the maneuvers in the list in the section */
        tInt16 m_i16ManeuverListIndex;

        /*! holds the current index in the lists of sections */
        tInt16 m_i16SectionListIndex;

        /*! handle for the timer */
        tHandle m_hTimer;

        /*! the critical section of the transmit */
        cCriticalSection m_oCriticalSectionTransmit;

        /*! the critical section of the timer setup */
        cCriticalSection m_oCriticalSectionTimerSetup;

        tResult RetrieveAndSendManeuver(tInt16 sectionIndex, tInt16 maneuverIndex);

        tManeuver DetermineManeuver(cString name);

        void GetProperties();

        tResult SendManeuver(tManeuver maneuver);

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins(IException **__exception_ptr);

        tResult CreateOutputPins(IException **__exception_ptr);
};

#endif
