/*
@author: ffreihube
*/
//#############################################################################

#include "Median_values.h"

//#############################################################################
ADTF_FILTER_PLUGIN(FILTER_NAME, OID, cMedianValues);

//#############################################################################
cMedianValues::cMedianValues(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 15)
{
    this->m_pMedianValues = NULL;

    // init value for Frame Size
    m_ui32FrameSize = 10;
    m_ui32BufferSize = 50;

    // create the filter properties
    SetPropertyInt("Frame Size", m_ui32FrameSize);
    SetPropertyInt("Frame Size" NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr("Frame Size" NSSUBPROP_DESCRIPTION, "Frame Size of the Medianfilter");

    SetPropertyInt("Buffer Size", m_ui32BufferSize);
    SetPropertyInt("Buffer Size" NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr("Buffer Size" NSSUBPROP_DESCRIPTION, "Buffersize");
}

//#############################################################################
cMedianValues::~cMedianValues(void)
{

}

tResult cMedianValues::CreateInputPins(__exception)
{
    // create the input pins
    RETURN_IF_FAILED(m_oInput.Create("input_value", new cMediaType(0, 0, 0, "tSignalValue"),
                                     static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInput));
    RETURN_NOERROR;
}

tResult cMedianValues::CreateOutputPins(__exception)
{
    //get the media description manager for this filter
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &pDescManager, __exception_ptr));


    //get description for signal value input pin
    tChar const *strDescSignalValue = pDescManager->GetMediaDescription("tSignalValue");

    // checks if exists
    RETURN_IF_POINTER_NULL(strDescSignalValue);

    //get mediatype
    cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValue,
                                                             IMediaDescription::MDF_DDL_DEFAULT_VERSION);

    //get mediatype description for data type
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &m_pDescriptionSignal));

    // create output pins
    RETURN_IF_FAILED(m_oOutput.Create("output_value", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oOutput));

    RETURN_NOERROR;
}

//#############################################################################
tResult cMedianValues::Init(tInitStage eStage, ucom::IException **__exception_ptr)
{
    switch (eStage)
    {
        case StageFirst:
        {
            // create pins
            RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
            RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));

            break;
        }

        case StageNormal:
        {
            // init the median values lib
            m_ui32FrameSize = tUInt32(GetPropertyFloat("Frame Size"));
            m_ui32BufferSize = tUInt32(GetPropertyFloat("Buffer Size"));

            this->m_pMedianValues = new cMedianValuesLib();

            break;
        }

        case StageGraphReady:
        {
            m_bIDsSignalSet = tFalse;
            break;
        }
    }

    RETURN_NOERROR;
}

//#############################################################################
tResult cMedianValues::Start(ucom::IException **__exception_ptr)
{
    return cFilter::Start(__exception_ptr);;
}

//#############################################################################
tResult cMedianValues::Stop(ucom::IException **__exception_ptr)
{
    return cFilter::Stop(__exception_ptr);;
}

//#############################################################################
tResult cMedianValues::Shutdown(tInitStage eStage, ucom::IException **__exception_ptr)
{
    if (this->m_pMedianValues != NULL)
    {
        delete this->m_pMedianValues;
        this->m_pMedianValues = NULL;
    }

    return cFilter::Shutdown(eStage, __exception_ptr);
}

//#############################################################################
tResult cMedianValues::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2,
                                  IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (nEventCode == IPinEventSink::PE_MediaSampleReceived)
    {
        if (pSource == &this->m_oInput)
        {
            RETURN_IF_FAILED(ProcessInput(pMediaSample));
        }
    }

    RETURN_NOERROR;
}

//#############################################################################
tResult cMedianValues::ProcessInput(IMediaSample *pMediaSample)
{

    //write values with zero
    tFloat32 f32Value = 0;
    tUInt32 ui32TimeStamp = 0;

    //cString* logMsg = new cString();

    {   // focus for sample read lock
        // read-out the incoming Media Sample
        __adtf_sample_read_lock_mediadescription(m_pDescriptionSignal, pMediaSample, pCoderInput);

        // get the IDs for the items in the media sample
        if (!m_bIDsSignalSet)
        {
            pCoderInput->GetID("f32Value", m_szIDSignalF32Value);
            pCoderInput->GetID("ui32ArduinoTimestamp", m_szIDSignalArduinoTimestamp);
            m_bIDsSignalSet = tTrue;
        }

        //get values from media sample
        pCoderInput->Get(m_szIDSignalF32Value, (tVoid *) &f32Value);
        pCoderInput->Get(m_szIDSignalArduinoTimestamp, (tVoid *) &ui32TimeStamp);
    }
    logger.StartLog();
    logger.Log(cString::Format("Received Value: %f", f32Value).GetPtr());

    //stack sample
    this->m_pMedianValues->stackSample(f32Value, m_ui32BufferSize);

    // create median
    if (this->m_pMedianValues->haveEnoughSamples(m_ui32BufferSize))
    {
        f32Value = this->m_pMedianValues->getMedian(m_ui32FrameSize);
        RETURN_IF_FAILED(Transmit(f32Value, pMediaSample, ui32TimeStamp));
    }
    else
    {
        RETURN_IF_FAILED(Transmit(f32Value, pMediaSample, ui32TimeStamp));
    }

    logger.Log(cString::Format("Send Value: %f", f32Value).GetPtr());
    logger.Log("----");
    logger.EndLog();

    RETURN_IF_FAILED(Transmit(f32Value, pMediaSample, ui32TimeStamp));

    RETURN_NOERROR;
}


//#############################################################################
tResult cMedianValues::Transmit(tFloat32 fValue, IMediaSample *pMediaSample, tUInt32 ui32TimeStamp)
{
    //create new media sample
    cObjectPtr<IMediaSample> pNewMediaSample;
    AllocMediaSample((tVoid **) &pNewMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    m_pDescriptionSignal->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pNewMediaSample->AllocBuffer(nSize);
    {   // focus for sample write lock
        //write date to the media sample with the coder of the descriptor
        __adtf_sample_write_lock_mediadescription(m_pDescriptionSignal, pNewMediaSample, pCoderOutput);

        // get the IDs for the items in the media sample
        if (!m_bIDsSignalSet)
        {
            pCoderOutput->GetID("f32Value", m_szIDSignalF32Value);
            pCoderOutput->GetID("ui32ArduinoTimestamp", m_szIDSignalArduinoTimestamp);
            m_bIDsSignalSet = tTrue;
        }

        // set values in new media sample
        pCoderOutput->Set(m_szIDSignalF32Value, (tVoid *) &(fValue));
        pCoderOutput->Set(m_szIDSignalArduinoTimestamp, (tVoid *) &ui32TimeStamp);
    }

    //transmit media sample over output pin
    pNewMediaSample->SetTime(pMediaSample->GetTime());
    m_oOutput.Transmit(pNewMediaSample);

    RETURN_NOERROR;
}
