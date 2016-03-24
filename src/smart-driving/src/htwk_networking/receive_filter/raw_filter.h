/**
 *
 * Filter to create LOG Information if a MediaSample is received
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

#ifndef _RAW_FILTER_HEADER_
#define _RAW_FILTER_HEADER_

#define OID_ADTF_DX_FILTER "adtf.demo.dx.raw.filter"

//#############################################################################
class cExchangeRawFilter : public cFilter
{
    ADTF_FILTER(OID_ADTF_DX_FILTER, "DX raw receive", adtf::OBJCAT_Generic)

    public:
        cExchangeRawFilter(const tChar *i_pcInfo);

        virtual ~cExchangeRawFilter();

    public:
        tResult Init(tInitStage i_eInitStage, __exception = NULL);


    public:
        tResult OnPinEvent(IPin *i_pIPin,
                           tInt i_iEventCode,
                           tInt i_iParam1,
                           tInt i_iParam2,
                           IMediaSample *i_pIMediaSample);

    private:
        // Input pin
        cInputPin m_oInput1;
        cInputPin m_oInput2;

};


#endif  // _RAW_FILTER_HEADER_
