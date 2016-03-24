/**
 *
 * ADTF Demo Filter.
 *    This is only for demo a Filter application.
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
#ifndef _DEMO_EMPTY_FILTER_HEADER_
#define _DEMO_EMPTY_FILTER_HEADER_


#define OID_ADTF_DEMO_EMPTY "adtf.example.demo_empty"

class cDemoFilter :
public adtf::cFilter,
publicucom::IScriptable
{
    ADTF_DECLARE_FILTER_VERSION(OID_ADTF_DEMO_EMPTY, "Demo Filter", OBJCAT_DataFilter, "DemoFilterVersion", 2, 2, 20, "MARGE")


    public: //common implementation
    cDemoFilter(
    const tChar *__info);
    virtual ~cDemoFilter();

    private: //private members
    cInputPin m_oInput;
    cOutputPin m_oOutput;
    cObjectPtr <IMediaTypeDescription> m_pCoderDesc;

    private: //private functions
    tResult ProcessInput(IMediaSample *pSample);

    public: // overwrites cFilter //implements IPinEventSink
    tResult OnPinEvent(IPin *pSource,
                       tInt nEventCode,
                       tInt nParam1,
                       tInt nParam2,
                       IMediaSample *pMediaSample);

    public: // overwrites cFilter
    tResult
    Init(tInitStage
    eStage, __exception = NULL);
    tResult Start(__exception = NULL);
    tResult Stop(__exception = NULL);
    tResult
    Shutdown(tInitStage
    eStage, __exception = NULL);

    public: // implements IObject
    tResult GetInterface(const tChar *idInterface, tVoid **ppvObject);
    tUInt Ref();
    tUInt Unref();
    tVoid Destroy();

    public: // overwrites IScriptable
    tResult ScriptRunCommand(const tChar *strCommand,
                             tVoid *pvData = NULL,
    tInt szData = 0,
    IException * *__exception_ptr = NULL);


};

#endif // _DEMO_EMPTY_FILTER_HEADER_
