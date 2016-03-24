/**
 *
 * ADTF Empty Filter Demo.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: voigtlpi $
 * $Date: 2013-10-19 00:53:22 +0200 (Sa, 19 Okt 2013) $
 * $Revision: 41862 $
 *
 * @remarks             This example shows how to implement a common adtf
 *                      filter for processing data.
 *
 */
#include "stdafx.h"
#include "./../include/demo_published_interface.h"
#include "./../include/interface_description.h"
#include "./demo_server_filter.h"

/// Code Macro for the Plugin creation
ADTF_FILTER_PLUGIN("Demo Server", OID_ADTF_DEMO_DYNAMIC_SERVER, cDemoServerFilter);

/**
 *   Constructor. The cFilter constructor needs to be called !!
 *   The SetProperty in the constructor is necessary if somebody wants to deal with
 *   the default values of the properties before init
 *
 */
cDemoServerFilter::cDemoServerFilter(const tChar *__info) : cServerBindingFilter(__info, IID_DEMO_PUBLISHED_INTERFACE), logger("Demo Server", 20)
{
    m_i32CurrentValue = 0;
    cMemoryBlock::MemZero(&m_sCurrentStructValue, sizeof(m_sCurrentStructValue));
    cMemoryBlock::MemZero(&m_sInternalState, sizeof(m_sInternalState));
}

/**
 *  Destructor. A Filter implementation needs always to have a virtual Destructor
 *              because the IFilter extends an IObject
 *
 */
cDemoServerFilter::~cDemoServerFilter()
{
}


tResult cDemoServerFilter::Init(cFilter::tInitStage eStage, ucom::IException **__exception_ptr/* =NULL */)
{
    //never forget t call the base implementation
    //cServerBindingFilter will register one Server Object of the given interface from
    //  Constructor parameter "IID_DEMO_PUBLISHED_INTERFACE" and
    //  template parameter cServerBindingFilter<IDemoPublishedInterface>
    RETURN_IF_FAILED(cServerBindingFilter::Init(eStage, __exception_ptr));
    if (eStage == cFilter::StageFirst)
    {
        //create the Locks
        THROW_IF_FAILED(m_oLockState.Create(cString::Format("state_lock_of_%s", OIGetInstanceName()).GetPtr()));
        //additionally this filter will register two output pins that will be used to send the
        // set Values of m_i32CurrentValue and m_sCurrentStructValue by client request on ActivateOutputs
        RETURN_IF_FAILED(
                m_oInt32Pin.Create("I32", new cMediaType(0, 0, 0, MNAME_I32, MNAME_I32_DESCRIPTION, IMediaDescription::MDF_DDL_DEFAULT_VERSION),
                                   this));
        RETURN_IF_FAILED(RegisterPin(&m_oInt32Pin));

        RETURN_IF_FAILED(m_oStructPin.Create("STRUCT", new cMediaType(0, 0, 0, MNAME_STRUCT, MNAME_STRUCT_DESCRIPTION,
                                                                      IMediaDescription::MDF_DDL_DEFAULT_VERSION), this));
        RETURN_IF_FAILED(RegisterPin(&m_oStructPin));

        logger.Log("Server Init tage First", false);
    }
    RETURN_NOERROR;
}

tResult cDemoServerFilter::Shutdown(cFilter::tInitStage eStage, ucom::IException **__exception_ptr/* =NULL */)
{
    if (eStage == cFilter::StageFirst)
    {
        m_oLockState.Release();
    }

    return cServerBindingFilter::Shutdown(eStage, __exception_ptr);
}

tResult cDemoServerFilter::Start(ucom::IException **__exception_ptr/* =NULL */)
{
    // we start a timer to trigger output pins cyclically
    RETURN_IF_FAILED(cServerBindingFilter::Start(__exception_ptr));
    m_hTimer = _kernel->TimerCreate(200000,
                                    200000,
                                    this,
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    cString::Format("%s_server_timer", OIGetInstanceName()).GetPtr());
    THROW_IF_POINTER_NULL(m_hTimer);
    RETURN_NOERROR;
}

tResult cDemoServerFilter::Stop(ucom::IException **__exception_ptr/* =NULL */)
{
    //stop the timer
    if (m_hTimer)
    {
        _kernel->TimerDestroy(m_hTimer);
    }
    cServerBindingFilter::Stop(__exception_ptr);
    RETURN_NOERROR;
}

tResult cDemoServerFilter::Run(tInt nActivationCode,
                               const tVoid *pvUserData,
                               tInt szUserDataSize,
                               ucom::IException **__exception_ptr/* =NULL */)
{
    //Implementation of the timer
    if (nActivationCode == IRunnable::RUN_TIMER)
    {
        ProcessOutputs();
    }
    RETURN_NOERROR;
}

tResult cDemoServerFilter::ProcessOutputs()
{
    tBool bTransmitActivated = tFalse;
    //cyclic trigger the outputs
    //within client server communication the request of a client is called asynchronously to any other operation
    //so we need to synchronize by mutex
    {
        //this lock is used to protect the members, but will not lock while transmit is called
        __synchronized_kernel(m_oLockState);
        bTransmitActivated = (m_sInternalState.i32ActivateCount > 0);
        m_sInternalState.i64ProcessCallCounter++;
    }

    m_i32CurrentValue++;
    m_sCurrentStructValue.i8Value = static_cast<tUInt8>(m_i32CurrentValue);
    m_sCurrentStructValue.ui32Value = m_i32CurrentValue;

    if (bTransmitActivated)
    {
        TransmitMediaSample(&m_oInt32Pin, &m_i32CurrentValue, sizeof(m_i32CurrentValue));
        TransmitMediaSample(&m_oStructPin, &m_sCurrentStructValue, sizeof(m_sCurrentStructValue));
        {
            //this lock is used to protect the members, but will not lock while transmit is called
            __synchronized_kernel(m_oLockState);
            m_sInternalState.i64TransmitCounter++;
        }
    }
    RETURN_NOERROR;
}


tResult cDemoServerFilter::GetServerName(tChar *strBuffer, const tInt32 i32BufferSize)
{
    const tChar *strName = OIGetInstanceName();
    if (strName)
    {
        cString::Copy(strBuffer, strName, -1, i32BufferSize);
        logger.Log(cString::Format("A client called and requested my name return with value %s", strBuffer).GetPtr());
        RETURN_NOERROR;
    }
    RETURN_ERROR(ERR_NOT_FOUND);
}

tResult cDemoServerFilter::GetInternalState(tDemoServerState *pState, const tInt32 i32ServerStateSize)
{
    //client request will return the value sent through m_oInt32Pin
    __synchronized_kernel(m_oLockState);
    if (pState
        && sizeof(m_sInternalState) == i32ServerStateSize)
    {
        m_sInternalState.ui8ServerState = static_cast<tUInt8>(cServerBindingFilter::GetState());
        cMemoryBlock::MemCopy(pState, &m_sInternalState, sizeof(m_sInternalState));
    }
    else
    {
        LOG_ERROR(cString::Format("A client called and requested my state, but in a wrong way").GetPtr());
    }
    LOG_INFO(cString::Format("A client called and requested my state").GetPtr());
    RETURN_NOERROR;
}

tResult cDemoServerFilter::ActivateOutputs(const tUInt8 &ui8Activate)
{
    __synchronized_kernel(m_oLockState);
    if (0 != ui8Activate)
    {
        m_sInternalState.i32ActivateCount++;
    }
    else
    {
        if (m_sInternalState.i32ActivateCount-- < 0)
        {
            LOG_ERROR(cString::Format("A client deactivates me, but i am deactivated already!").GetPtr());
            m_sInternalState.i32ActivateCount = 0;
        }
    }

    RETURN_NOERROR;
}
