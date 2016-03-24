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
#include "./demoemptyfilter.h"


ADTF_FILTER_PLUGIN("Demo Filter", OID_ADTF_DEMO_EMPTY, cDemoFilter)

/**
 *   Contructor. The cFilter contructor needs to be called !!
 *   The SetProperty in the constructor is necessary if somebody wants to deal with
 *   the default values of the properties before init
 *
 */
cDemoFilter::cDemoFilter(const tChar *__info) : cFilter(__info)
{
    //SetPropertInt("myintvalue", 1);
}

/**
 *  Destructor. A Filter implementation needs always to have a virtual Destructor
 *              because the IFilter extends an IObject
 *
 */
cDemoFilter::~cDemoFilter()
{
}

/**
 * Processes on mediasample. The method only shows an example, how to deal with
 * a mediasample that is received, to get the specific data.
 * Further it shows, how to create a new mediasample in the system pool, to set the
 * data and to transmit it over a pin.
 *
 * @param   pSample   [in]  The received sample.
 *
 * @return  Standard result code.
 */
tResult cDemoFilter::ProcessInput(IMediaSample *pSample)
{
    RETURN_IF_POINTER_NULL(pSample);
    if (pSample != NULL && m_pCoderDesc != NULL)
    {
        // read-out the incoming Media Sample
        cObjectPtr<IMediaCoder> pCoder;
        RETURN_IF_FAILED(m_pCoderDesc->Lock(pSample, (IMediaCoder * *) & pCoder));

        tUInt8 ui8Value = 0;
        tUInt16 ui16Value = 0;
        tUInt32 ui32Value = 0;
        tInt32 i32Value = 0;
        tInt64 i64Value = 0;
        tFloat64 f64Value = 0.0;
        tFloat32 f32Value = 0.0f;

        pCoder->Get("sHeaderStruct.ui32HeaderVal", (tVoid *) &ui32Value);
        pCoder->Get("sHeaderStruct.f64HeaderVal", (tVoid *) &f64Value);
        pCoder->Get("sSimpleStruct.ui8Val", &ui8Value);
        pCoder->Get("sSimpleStruct.ui16Val", &ui16Value);
        pCoder->Get("sSimpleStruct.ui32Val", &ui32Value);
        pCoder->Get("sSimpleStruct.i32Val", &i32Value);
        pCoder->Get("sSimpleStruct.i64Val", &i64Value);
        pCoder->Get("sSimpleStruct.f64Val", &f64Value);
        pCoder->Get("sSimpleStruct.f32Val", &f32Value);

        m_pCoderDesc->Unlock(pCoder);


        // Now we create a new Media Sample and fill it with the values of the incoming sample
        cObjectPtr<IMediaSample> pMediaSample;
        RETURN_IF_FAILED(AllocMediaSample((tVoid **) &pMediaSample));

        cObjectPtr<IMediaSerializer> pSerializer;
        m_pCoderDesc->GetMediaSampleSerializer(&pSerializer);
        tInt nSize = pSerializer->GetDeserializedSize();

        // to allocate the buffer, you have to know the exact size of DDL-Type-Description for this Pin:
        // either count by yourself the values within the .description file, or use the "De-SerializedSize"
        RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));

        RETURN_IF_FAILED(m_pCoderDesc->WriteLock(pMediaSample, &pCoder));

        // to fill the buffer, you have to know the exact data-types!
        // DON'T forget to initialize the whole memory, e.g. fill-in ALL values!

        //we access the elements element by element
        //the coder can handle the point-operator.
        pCoder->Set("sHeaderStruct.ui32HeaderVal", (tVoid *) &ui32Value);
        pCoder->Set("sHeaderStruct.f64HeaderVal", (tVoid *) &f64Value);
        pCoder->Set("sSimpleStruct.ui8Val", (tVoid *) &ui8Value);
        pCoder->Set("sSimpleStruct.ui16Val", (tVoid *) &ui16Value);
        pCoder->Set("sSimpleStruct.ui32Val", (tVoid *) &ui32Value);
        pCoder->Set("sSimpleStruct.i32Val", (tVoid *) &i32Value);
        pCoder->Set("sSimpleStruct.i64Val", (tVoid *) &i64Value);
        pCoder->Set("sSimpleStruct.f64Val", (tVoid *) &f64Value);
        pCoder->Set("sSimpleStruct.f32Val", (tVoid *) &f32Value);

        m_pCoderDesc->Unlock(pCoder);

        pMediaSample->SetTime(_clock->GetStreamTime());
        m_oOutput.Transmit(pMediaSample);
    }
    RETURN_NOERROR;
}


/**
 *   The Filter Init Function.
 *    eInitStage ... StageFirst ... should be used for creating and registering Pins
 *               ... StageNormal .. should be used for reading the properies and initalizing
 *                                  everything before pin connections are made
 *   see {@link IFilter#Init IFilter::Init}.
 *
 */
tResult cDemoFilter::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        cObjectPtr<IMediaDescriptionManager> pDescManager;
        RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                             IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                             (tVoid **) &pDescManager,
                                             __exception_ptr));
        /*
        * the MediaDescription for <struct name="tNestedStruct" .../> has to exist in a description file (e.g. in $ADTF_DIR\description\ or $ADTF_DIR\src\examples\src\description
        * before (!) you start adtf_devenv !! if not: the Filter-Plugin will not loaded because cPin.Create() and so ::Init() failes !
        */
        tChar const *strDesc = pDescManager->GetMediaDescription("tNestedStruct");
        RETURN_IF_POINTER_NULL(strDesc);
        cObjectPtr<IMediaType> pType = new cMediaType(0, 0, 0, "tNestedStruct", strDesc, IMediaDescription::MDF_DDL020000);

        // register the input pin with the type "tNestedStruct", which has to be defined in a Media Description File
        RETURN_IF_FAILED(m_oInput.Create("input", pType, this));
        RETURN_IF_FAILED(RegisterPin(&m_oInput));
        RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &m_pCoderDesc));

        // register the output pin with the same media type as the input pin
        RETURN_IF_FAILED(m_oOutput.Create("output", pType, this));
        RETURN_IF_FAILED(RegisterPin(&m_oOutput));
        RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &m_pCoderDesc));
    }
    else if (eStage == StageNormal)
    {
        //tInt nValue = GetPropertInt("myintvalue");
    }
    RETURN_NOERROR;
}

/**
 *   The Filters Start Function. see {@link IFilter#Start IFilter::Start}.
 *
 */
tResult cDemoFilter::Start(__exception)
{
    //nothing to do
    return cFilter::Start(__exception_ptr);
}

/**
 *   The Filters Stop Function. see {@link IFilter#Stop IFilter::Stop}.
 *
 */
tResult cDemoFilter::Stop(__exception)
{
    //nothing to do
    return cFilter::Stop(__exception_ptr);
}

/**
 *   The Filters Shutdown Function. see {@link IFilter#Shutdown IFilter::Shutdown}.
 *
 */
tResult cDemoFilter::Shutdown(tInitStage eStage, __exception)
{
    //nothing to do
    return cFilter::Shutdown(eStage, __exception_ptr);
}


/**
 *   The Filters Pin Event Implementation. see {@link IPinEventSink#OnPinEvent IPinEventSink::OnPinEvent}.
 *   Here the receiving Pin (cInputPin) will call the OnPinEvent.
 *
 */
tResult cDemoFilter::OnPinEvent(IPin *pSource,
                                tInt nEventCode,
                                tInt nParam1,
                                tInt nParam2,
                                IMediaSample *pMediaSample)
{
    if (nEventCode == IPinEventSink::PE_MediaSampleReceived)
    {
        if (pSource == &m_oInput)
        {
            ProcessInput(pMediaSample);
        }
        else
        {
            RETURN_ERROR(ERR_NOT_SUPPORTED);
        }
    }
    RETURN_NOERROR;
}

/**
 * The IObject implementation
 */
tResult cDemoFilter::GetInterface(const tChar *idInterface, tVoid **ppvObject)
{
    if (ppvObject == NULL)
    {
        RETURN_ERROR(ERR_POINTER);
    }

    if (idmatch(idInterface, IID_SCRIPTABLE))
    {
        *ppvObject = static_cast<IScriptable *> (this);
        Ref();
        RETURN_NOERROR;
    }

    return cFilter::GetInterface(idInterface, ppvObject);
}

tUInt cDemoFilter::Ref()
{
    return cFilter::Ref();
}

tUInt cDemoFilter::Unref()
{
    return cFilter::Unref();
}

tVoid cDemoFilter::Destroy()
{
    delete this;
}

/**
 * The Filter's scriptable interface
 */
tResult cDemoFilter::ScriptRunCommand(const tChar *strCommand,
                                      tVoid *pvData,
                                      tInt szData,
                                      IException **__exception_ptr)
{
    cDOMElement domCmd;
    RETURN_IF_FAILED(domCmd.FromString(strCommand));

    const cString strCmdName = domCmd.GetAttribute("name");

    if (strCmdName.IsEqual("hello"))
    {
        const cString strGreeter = domCmd.GetAttribute("from");
        LOG_INFO(cString::Format("Received greetings from '%s'.", strGreeter.GetPtr()).GetPtr());
    }
    else if (strCmdName.IsEqual("byebye"))
    {
        const cString strBuddy = domCmd.GetAttribute("from");
        LOG_INFO(cString::Format("'%s' said bye bye.", strBuddy.GetPtr()).GetPtr());
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }
    RETURN_NOERROR;
}
