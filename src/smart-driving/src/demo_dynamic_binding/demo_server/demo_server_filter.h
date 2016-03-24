/**
 *
 * ADTF Demo Dynamic Server Filter.
 *    This is a example to implement adtf::cServerBindingFilter.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: ABOEMI9 $
 * $Date: 2013-08-27 10:02:18 +0200 (Di, 27 Aug 2013) $
 * $Revision: 40286 $
 *
 * @remarks
 *
 */
#ifndef _DEMO_DYNAMIC_SERVER_FILTER_HEADER_
#define _DEMO_DYNAMIC_SERVER_FILTER_HEADER_

#define OID_ADTF_DEMO_DYNAMIC_SERVER "adtf.example.demo_dynamic_server"
/**
 * This Demo Filter implements a public interface IDemoPublishedInterface and provide
 * it as a Server Object. This will add the possibility to connect this filter by Dynamic Binding.
 * To do so use the template adtf::cServerBindingFilter
 */

#include "../../htwk_logger/Logger.h"

class cDemoServerFilter :

public adtf::cServerBindingFilter<IDemoPublishedInterface>
{
    ADTF_FILTER(OID_ADTF_DEMO_DYNAMIC_SERVER,
                "Demo Server Filter",
                OBJCAT_Generic);

    public: //common implementation
    cDemoServerFilter(
    const tChar *__info);
    virtual ~cDemoServerFilter();

        Logger logger;

    public: //overwrite cFilter
    tResult Init(cFilter::tInitStage eStage, ucom::IException **__exception_ptr/* =NULL */);
    tResult Shutdown(cFilter::tInitStage eStage, ucom::IException **__exception_ptr/* =NULL */);
    tResult Start(ucom::IException **__exception_ptr/* =NULL */);
    tResult Stop(ucom::IException **__exception_ptr/* =NULL */);

    public: //implements IRunnable for timer
    tResult Run(tInt nActivationCode, const tVoid *pvUserData, tInt szUserDataSize, ucom::IException **__exception_ptr/* =NULL */);

    public:
    tResult ProcessOutputs();

    public: //implements the IDemoPublishedInterface
    tResult GetServerName(tChar *strBuffer, const tInt32 i32BufferSize);
    tResult GetInternalState(tDemoServerState *pState, const tInt32 i32ServerStateSize);
    tResult
    ActivateOutputs(
    const tUInt8
    &ui8Activate);

    private:
    tInt32 m_i32CurrentValue;
    tPublishedDataStruct m_sCurrentStructValue;

    cOutputPin m_oInt32Pin;
    cOutputPin m_oStructPin;

    tDemoServerState m_sInternalState;

    tHandle m_hTimer;
    cKernelMutex m_oLockState;
};

#endif // _DEMO_DYNAMIC_SERVER_FILTER_HEADER_
