/**
 *
 * Service standard includes.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: A1RHEND $
 * $Date: 2013-04-11 16:13:01 +0200 (Thu, 11 Apr 2013) $
 * $Revision: 37965 $
 *
 * @remarks
 *
 */

// Standard includes
#include "stdafx.h"
#include "raw_filter.h"

/// Create filter shell
ADTF_FILTER_PLUGIN("DX raw receive", OID_ADTF_DX_FILTER, cExchangeRawFilter)

//#############################################################################
cExchangeRawFilter::cExchangeRawFilter(const tChar *i_pcInfo) : cFilter(i_pcInfo)
{
}

//#############################################################################
cExchangeRawFilter::~cExchangeRawFilter()
{
}

//#############################################################################
tResult cExchangeRawFilter::Init(tInitStage i_eInitStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(i_eInitStage, __exception_ptr));

    if (i_eInitStage == StageFirst)
    {
        cObjectPtr<IMediaType> ptrIMediaTypeInput1 = new cMediaType(MEDIA_TYPE_STRUCTURED_DATA,
                                                                    MEDIA_SUBTYPE_STRUCT_FLOAT64);
        RETURN_IF_FAILED(m_oInput1.Create("Input1", ptrIMediaTypeInput1, static_cast<IPinEventSink *> (this)))
        RETURN_IF_FAILED(RegisterPin(&m_oInput1));

        cObjectPtr<IMediaType> ptrIMediaTypeInput2 = new cMediaType(MEDIA_TYPE_STRUCTURED_DATA,
                                                                    MEDIA_SUBTYPE_STRUCT_FLOAT64);
        RETURN_IF_FAILED(m_oInput2.Create("Input2", ptrIMediaTypeInput2, static_cast<IPinEventSink *> (this)))
        RETURN_IF_FAILED(RegisterPin(&m_oInput2));
    }

    RETURN_NOERROR;
}

//#############################################################################
tResult cExchangeRawFilter::OnPinEvent(IPin *i_pIPin,
                                       tInt i_iEventCode,
                                       tInt i_iParam1,
                                       tInt i_iParam2,
                                       IMediaSample *i_pIMediaSample)
{
    if (i_iEventCode == IPinEventSink::PE_MediaSampleReceived)
    {
        // Check pointer validity.
        RETURN_IF_POINTER_NULL(i_pIMediaSample);

        if (i_pIPin == &m_oInput1)
        {
            LOG_INFO("Input 1");
        }
        else if (i_pIPin == &m_oInput2)
        {
            LOG_INFO("Input 2");
        }
    }

    return ERR_NOERROR;
}
