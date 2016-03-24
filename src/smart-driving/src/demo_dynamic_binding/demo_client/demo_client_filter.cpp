/**
 *
 * ADTF Client Filter Demo.
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
#include "./demo_client_filter.h"


ADTF_FILTER_PLUGIN("Demo Client Filter Plugin", OID_ADTF_DEMO_DYNAMIC_CLIENT, cDemoClientFilter)

/**
 *   Constructor. The cFilter constructor needs to be called !!
 *   The SetProperty in the constructor is necessary if somebody wants to deal with
 *   the default values of the properties before init
 *
 */
cDemoClientFilter::cDemoClientFilter(const tChar *__info) : cClientBindingFilter(__info, IID_DEMO_PUBLISHED_INTERFACE), logger("Demo Client", 21)
{
    cFilter::ConfigureConfigPins(tTrue, tTrue);
    SetPropertyStr("pin_names", "default");
    // this property is required to create output pins
    SetPropertyBool("pin_names" NSSUBPROP_REQUIRED, tTrue);
    m_bServerActivated = tFalse;
}

/**
 *  Destructor. A Filter implementation needs always to have a virtual Destructor
 *              because the IFilter extends an IObject
 *
 */
cDemoClientFilter::~cDemoClientFilter()
{
}


/**
 *   The Filter Init Function.
 *    eInitStage ... StageFirst ... should be used for creating and registering Pins
 *               ... StageNormal .. should be used for reading the properties and initializing
 *                                  everything before pin connections are made
 *   see {@link IFilter#Init IFilter::Init}.
 *
 */
tResult cDemoClientFilter::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cClientBindingFilter::Init(eStage, __exception_ptr));
    if (eStage == cFilter::StageFirst)
    {
        RETURN_IF_FAILED(m_oActivatedSync.Create(cString::Format("lock_activated_%s", OIGetInstanceName()).GetPtr()));
        m_bServerActivated = tFalse;
    }
    else if (eStage == cFilter::StageNormal)
    {
        cString strPinNames = GetPropertyStr("pin_names");
        if (strPinNames.IsNotEmpty())
        {
            m_pInputPin = new cDynamicInputPin();
            m_pOutputPin = new cDynamicOutputPin();
            RETURN_IF_FAILED(m_pInputPin->Create(cString::Format("%s_in", strPinNames.GetPtr()).GetPtr(),
                                                 new cMediaType(0, 0, 0, MNAME_I32, MNAME_I32_DESCRIPTION, IMediaDescription::MDF_DDL030000),
                                                 this));
            RETURN_IF_FAILED(m_pOutputPin->Create(cString::Format("%s_out", strPinNames.GetPtr()).GetPtr(),
                                                  new cMediaType(0, 0, 0, MNAME_I32, MNAME_I32_DESCRIPTION, IMediaDescription::MDF_DDL030000),
                                                  this));
            RETURN_IF_FAILED(RegisterPin(m_pInputPin));
            RETURN_IF_FAILED(RegisterPin(m_pOutputPin));
        }
    }
    else if (eStage == StageGraphReady)
    {
        logger.Log("trying connecting Server");
        //we need to check the pointer _server always !!
        if (_server)
        {
            //send server call to get the name
            tChar strServerName[512];
            if (IS_OK(_server->GetServerName(strServerName, 512)))
            {
                m_strServerName = strServerName;
                LOG_INFO(cString::Format("A Server Filter with the name \"%s\" is connected to me (I am \"%s\")",
                                         strServerName, OIGetInstanceName()).GetPtr());
            }
            else
            {
                LOG_WARNING("The server has no name!");
            }
        }
    }
    RETURN_NOERROR;
}

tResult cDemoClientFilter::Shutdown(cFilter::tInitStage eStage, ucom::IException **__exception_ptr/* =NULL */)
{
    if (eStage == cFilter::StageFirst)
    {
        m_oActivatedSync.Release();
        m_bServerActivated = tFalse;
    }
    else if (eStage == cFilter::StageNormal)
    {
        m_pOutputPin = NULL;
        m_pInputPin = NULL;
    }
    cClientBindingFilter::Shutdown(eStage, __exception_ptr);
    RETURN_NOERROR;
}


/**
 *   The Filters Start Function. see {@link IFilter#Start IFilter::Start}.
 *
 */
tResult cDemoClientFilter::Start(__exception)
{
    // we start a timer to get the server value of Int and Struct
    RETURN_IF_FAILED(cClientBindingFilter::Start(__exception_ptr));
    m_hTimer = _kernel->TimerCreate(2000000,
                                    2000000,
                                    this,
                                    NULL,
                                    NULL,
                                    0,
                                    0,
                                    cString::Format("%s_client_timer", OIGetInstanceName()).GetPtr());
    THROW_IF_POINTER_NULL(m_hTimer);
    RETURN_NOERROR;
}

/**
 *   The Filters Start Function. see {@link IFilter#Start IFilter::Start}.
 *
 */
tResult cDemoClientFilter::Stop(__exception)
{
    if (m_hTimer)
    {
        _kernel->TimerDestroy(m_hTimer);
    }
    return cClientBindingFilter::Stop(__exception_ptr);
}


//implement the timer
tResult cDemoClientFilter::Run(tInt nActivationCode,
                               const tVoid *pvUserData,
                               tInt szUserDataSize,
                               ucom::IException **__exception_ptr/* =NULL */)
{
    if (nActivationCode == IRunnable::RUN_TIMER)
    {
        ActivateOrDeactivateTheServer();
    }
    RETURN_NOERROR;
}

tResult cDemoClientFilter::OnPinEvent(IPin *pSource,
                                      tInt nEventCode,
                                      tInt nParam1,
                                      tInt nParam2,
                                      IMediaSample *pMediaSample)
{


    if (nEventCode == IPinEventSink::PE_MediaSampleReceived
        && m_pInputPin == pSource)
    {
        tBool bActivated = tFalse;
        {
            //this will check if i requested the server to transmit samples to me
            //only if activated from my instance i will forward the media sample
            //we LOCK the mutex only to get the bool variable, we wont block the TransmitCall!!
            __synchronized_kernel(m_oActivatedSync);
            bActivated = m_bServerActivated;
        }
        if (bActivated)
        {
            //This code just forward the media sample in a new instance
            cObjectPtr<IMediaSample> pSampleToTransmit;
            if (IS_OK(AllocMediaSample(&pSampleToTransmit)))
            {
                pSampleToTransmit->CloneFrom(pMediaSample);
                m_pOutputPin->Transmit(pSampleToTransmit);
            }
        }
    }
    RETURN_NOERROR;
}

tResult cDemoClientFilter::ActivateOrDeactivateTheServer()
{
    //we need to check the pointer _server always to ensure we are connected
    //if we are not connected this filter is implemented to startup anyway
    //The _server pointer will not be locked by mutex assuming that:
    //  * the server stay connected during this call
    //  * the server stay connected and untouched within RL_Running (Stop is where we stop the timer)
    if (_server)
    {
        tUInt8 ui8ActivateServer = 0;
        {
            __synchronized_kernel(m_oActivatedSync);
            m_bServerActivated = !m_bServerActivated;
            ui8ActivateServer = m_bServerActivated ? 1 : 0;
        }

        //get the current state
        IDemoPublishedInterface::tDemoServerState sCurrentServerState;
        cMemoryBlock::MemZero(&sCurrentServerState, sizeof(sCurrentServerState));
        if (IS_OK(_server->ActivateOutputs(ui8ActivateServer)))
        {
            if (IS_OK(_server->GetInternalState(&sCurrentServerState, tInt32(sizeof(sCurrentServerState)))))
            {
                //check if really activated
                if (ui8ActivateServer != 0)
                {
                    if (sCurrentServerState.i32ActivateCount > 0)
                    {
                        //nothing to do
                        //everything alright
                    }
                    else
                    {
                        LOG_ERROR("Server did not react on my activation call!");
                    }
                }
            }
        }

    }
    RETURN_NOERROR;
}





