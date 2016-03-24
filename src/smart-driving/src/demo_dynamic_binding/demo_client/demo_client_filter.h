/**
 *
 * ADTF Demo Client Filter.
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
#ifndef _DEMO_CLIENT_FILTER_HEADER_
#define _DEMO_CLIENT_FILTER_HEADER_

#define OID_ADTF_DEMO_DYNAMIC_CLIENT "adtf.example.demo_dynamic_client"

#include "../../htwk_logger/Logger.h"

/**
 * This Demo Filter request an interface IDemoPublishedInterface by creating
 * a default connector as Client Binding Object.
 * This will add the possibility to connect this filter by Dynamic Binding.
 * To do so use the template adtf::cClientBindingFilter
 */
class cDemoClientFilter : public adtf::cClientBindingFilter<IDemoPublishedInterface>
{
    ADTF_FILTER(OID_ADTF_DEMO_DYNAMIC_CLIENT,
                "Demo Client Filter", OBJCAT_Generic);

    private:
        cObjectPtr<cDynamicInputPin> m_pInputPin;
        cObjectPtr<cDynamicOutputPin> m_pOutputPin;
        cString m_strServerName;
        tHandle m_hTimer;
        cKernelMutex m_oActivatedSync;
        tBool m_bServerActivated;

    private:
        tResult ActivateOrDeactivateTheServer();

    public: //common implementation
        cDemoClientFilter(const tChar *__info);

        virtual ~cDemoClientFilter();

        Logger logger;

    public: // overwrites cFilter
        tResult Init(tInitStage eStage, __exception = NULL);

        tResult Shutdown(cFilter::tInitStage eStage, ucom::IException **__exception_ptr = NULL);

        tResult Start(ucom::IException **__exception_ptr = NULL);

        tResult Stop(ucom::IException **__exception_ptr = NULL);

        tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample);

    public: //implements IRunnable for timer
        tResult Run(tInt nActivationCode, const tVoid *pvUserData, tInt szUserDataSize, ucom::IException **__exception_ptr/* =NULL */);


};

#endif // _DEMO_CLIENT_FILTER_HEADER_
