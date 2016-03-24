/*
@author: ffreihube
*/
//#############################################################################

#include "Movement_analyzer.h"

//#############################################################################
ADTF_FILTER_PLUGIN(FILTER_NAME, OID, cMovementAnalyzer);

//#############################################################################
cMovementAnalyzer::cMovementAnalyzer(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 5)
{
    this->calcOffsetAccX = new CalcOffset();
    this->calcOffsetAccY = new CalcOffset();
    this->movementCalculator = new MovementCalculator();

    newDistance = 0;
}

//#############################################################################
cMovementAnalyzer::~cMovementAnalyzer(void)
{

}

tResult cMovementAnalyzer::CreateInputPins(__exception)
{
    // create the input pins
    RETURN_IF_FAILED(m_oInputSpeed.Create("car_speed", new cMediaType(0, 0, 0, "tSignalValue"),
                                     static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputSpeed));

    RETURN_IF_FAILED(m_oInputAccX.Create("AccX", new cMediaType(0, 0, 0, "tSignalValue"),
                                          static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputAccX));

    RETURN_IF_FAILED(m_oInputAccY.Create("AccY", new cMediaType(0, 0, 0, "tSignalValue"),
                                          static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputAccY));

    RETURN_IF_FAILED(m_oInputDistance.Create("distance_overall", new cMediaType(0, 0, 0, "tSignalValue"),
                                         static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oInputDistance));


    RETURN_NOERROR;
}

tResult cMovementAnalyzer::CreateOutputPins(__exception)
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
    RETURN_IF_FAILED(m_oOutputCalcSpeed.Create("calc_car_speed", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oOutputCalcSpeed));

    RETURN_IF_FAILED(m_oOutputCalcAccX.Create("calc_AccX", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oOutputCalcAccX));

    RETURN_IF_FAILED(m_oOutputCalcAccY.Create("calc_AccY", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oOutputCalcAccY));

    RETURN_IF_FAILED(m_oOutputCalcDistance.Create("calc_distance_overall", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&m_oOutputCalcDistance));

    RETURN_NOERROR;
}

//#############################################################################
tResult cMovementAnalyzer::Init(tInitStage eStage, ucom::IException **__exception_ptr)
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
tResult cMovementAnalyzer::Start(ucom::IException **__exception_ptr)
{
    return cFilter::Start(__exception_ptr);;
}

//#############################################################################
tResult cMovementAnalyzer::Stop(ucom::IException **__exception_ptr)
{
    return cFilter::Stop(__exception_ptr);;
}

//#############################################################################
tResult cMovementAnalyzer::Shutdown(tInitStage eStage, ucom::IException **__exception_ptr)
{
    if (this->calcOffsetAccX != NULL)
    {
        delete this->calcOffsetAccX;
        this->calcOffsetAccX = NULL;
    }

    if (this->calcOffsetAccY != NULL)
    {
        delete this->calcOffsetAccY;
        this->calcOffsetAccY = NULL;
    }

    if (this->movementCalculator != NULL)
    {
        delete this->movementCalculator;
        this->movementCalculator = NULL;
    }

    return cFilter::Shutdown(eStage, __exception_ptr);
}

//#############################################################################
tResult cMovementAnalyzer::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2,
                                  IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (nEventCode == IPinEventSink::PE_MediaSampleReceived)
    {
        if (pSource != NULL)
        {
            RETURN_IF_FAILED(ProcessInput(pMediaSample, pSource));
        }
    }

    RETURN_NOERROR;
}

//#############################################################################
tResult cMovementAnalyzer::ProcessInput(IMediaSample *pMediaSample, IPin *pSource)
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

    if (pSource == &this->m_oInputSpeed)
    {
        this->movementCalculator->setMeasSpeed(f32Value, ui32TimeStamp);

        if (movementCalculator->newMovement())
        {
            tFloat32 calcSpeed = movementCalculator->calculateSpeedKalman();
            Transmit(calcSpeed, pMediaSample, ui32TimeStamp, &this->m_oInputSpeed);
        }

    }

    if (pSource == &this->m_oInputAccX)
    {
        RETURN_IF_FAILED(f32Value = adjustAcc(f32Value, calcOffsetAccX));
        this->movementCalculator->setX(f32Value, ui32TimeStamp);

        if (movementCalculator->newMovement())
        {
            newDistance = movementCalculator->calculateDistance();
            Transmit(newDistance, pMediaSample, ui32TimeStamp, &this->m_oInputDistance);
            Transmit(f32Value, pMediaSample, ui32TimeStamp, &this->m_oInputAccX);
        }
    }

    if (pSource == &this->m_oInputAccY)
    {
        RETURN_IF_FAILED(f32Value = adjustAcc(f32Value, calcOffsetAccY));
        this->movementCalculator->setY(f32Value, ui32TimeStamp);

        if (movementCalculator->newMovement())
        {
            newDistance = movementCalculator->calculateDistance();
            Transmit(newDistance, pMediaSample, ui32TimeStamp, &this->m_oInputDistance);
            Transmit(f32Value, pMediaSample, ui32TimeStamp, &this->m_oInputAccY);
        }
    }

    if (pSource == &this->m_oInputDistance)
    {

    }

    //RETURN_IF_FAILED(Transmit(f32Value, pMediaSample, ui32TimeStamp, pSource));

    RETURN_NOERROR;
}


//#############################################################################
tResult cMovementAnalyzer::Transmit(tFloat32 fValue, IMediaSample *pMediaSample, tUInt32 ui32TimeStamp, IPin *pSource)
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

    if (pSource == &this->m_oInputSpeed)
    {
        m_oOutputCalcSpeed.Transmit(pNewMediaSample);
    }

    if (pSource == &this->m_oInputAccX)
    {
        m_oOutputCalcAccX.Transmit(pNewMediaSample);
    }

    if (pSource == &this->m_oInputAccY)
    {
        m_oOutputCalcAccY.Transmit(pNewMediaSample);
    }

    if (pSource == &this->m_oInputDistance)
    {
        m_oOutputCalcDistance.Transmit(pNewMediaSample);
    }

    RETURN_NOERROR;
}

tFloat32 cMovementAnalyzer::adjustAcc(tFloat32 f32Value, CalcOffset *pOffset)
{
    if(pOffset->haveEnoughSamples()) {
        return (f32Value - pOffset->calculate());
    } else {
        pOffset->stackSample(f32Value);
        return f32Value;
    }
}
