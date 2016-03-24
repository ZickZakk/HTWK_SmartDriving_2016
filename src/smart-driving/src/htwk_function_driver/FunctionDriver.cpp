/**
 * @author pbachmann
 */

#include "FunctionDriver.h"

ADTF_FILTER_PLUGIN("Function Driver", OID_FunctionDriver, FunctionDriver);

FunctionDriver::FunctionDriver(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    speed = 2;
    sinWidth = 10;
    sinLength = 15;

    SetPropertyFloat(SPEED_PROPERTY, speed);
    SetPropertyBool(SPEED_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(SPEED_PROPERTY NSSUBPROP_DESCRIPTION, "Speed in m/s.");

    SetPropertyFloat(SINWIDTH_PROPERTY, sinWidth);
    SetPropertyBool(SINWIDTH_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(SINWIDTH_PROPERTY NSSUBPROP_DESCRIPTION, "Breite");

    SetPropertyFloat(SINLENGTH_PROPERTY, sinLength);
    SetPropertyBool(SINLENGTH_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(SINLENGTH_PROPERTY NSSUBPROP_DESCRIPTION, "Weite");
}

FunctionDriver::~FunctionDriver()
{
}

tResult FunctionDriver::ProcessInput(IMediaSample *mediaSample)
{
    RETURN_IF_POINTER_NULL(mediaSample);
    RETURN_NOERROR;
}

tResult FunctionDriver::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        // create all the pins
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        speed = tFloat32(GetPropertyFloat(SPEED_PROPERTY));
        sinWidth = tFloat32(GetPropertyFloat(SINWIDTH_PROPERTY));
        sinLength = tFloat32(GetPropertyFloat(SINLENGTH_PROPERTY));
    }

    RETURN_NOERROR;
}


tResult FunctionDriver::Start(__exception)
{
    vector<Point2f> values;
    for (int x = 0; x < 200; x = x + 20)
    {
        tFloat32 param[2] = {sinWidth, sinLength};
        tFloat y = HTWKMath::Integrate(SinusLengthIntegral, param, 0, x, 100, HTWKMath::SimpsonMethod);
        values.push_back(Point2f(x, y));
    }

    wayLengthFunction = HTWKMath::LinearRegression(values);

    logger.Log(cString::Format("Way Function: y = %f * x + %f",
                               wayLengthFunction.GetA(),
                               wayLengthFunction.GetM()).GetPtr(),
               false);

    return cFilter::Start(__exception_ptr);
}

tResult FunctionDriver::Stop(__exception)
{
    return cFilter::Stop(__exception_ptr);
}

tResult FunctionDriver::Shutdown(tInitStage eStage, __exception)
{
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult FunctionDriver::OnPinEvent(IPin *source, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    if (source == &distanceOverallPin)
    {
        {
            __adtf_sample_read_lock_mediadescription(descriptionSignal, mediaSample, inputCoder);
            inputCoder->Get("f32Value", (tVoid *) &distanceOverall);
            distanceOverall = distanceOverall * 100;
        }
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    tFloat32 x = wayLengthFunction.CalculateX(distanceOverall);

    tFloat32 y = CalculateSinus(0, x);
    double d = 1;
    tFloat32 m = CalculateSinus(0, x + d) - CalculateSinus(0, x - d);
    tFloat32 m_arc = atan(m) * (180 / M_PI);

    tFloat32 y1 = CalculateSinus(3, x);
    tFloat32 m1 = CalculateSinus(3, x + d) - CalculateSinus(3, x - d);
    tFloat32 m1_arc = atan(m1) * (180 / M_PI);

    tFloat32 steeringAngle = 90 - (m_arc - m1_arc);

    logger.StartLog();
    logger.Log(cString::Format("Distance Overall: %f", distanceOverall).GetPtr());
    logger.Log(cString::Format("x: %f", x).GetPtr());
    logger.Log(cString::Format("m: %f", m).GetPtr());
    logger.Log(cString::Format("y: %f", y).GetPtr());
    logger.Log(cString::Format("m1: %f", m1).GetPtr());
    logger.Log(cString::Format("y1: %f", y1).GetPtr());
    logger.Log(cString::Format("Steering Angle: %f", steeringAngle).GetPtr());
    logger.Log("----");

    logger.EndLog();

    SendValue(mediaSample, speedPin, speed);
    SendValue(mediaSample, steeringAnglePin, steeringAngle);

    RETURN_NOERROR;
}

void FunctionDriver::SendValue(IMediaSample *mediaSample, cOutputPin &outputPin, tFloat32 value)
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionSignal->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    cObjectPtr<IMediaSample> newMediaSample;
    AllocMediaSample((tVoid **) &newMediaSample);
    newMediaSample->AllocBuffer(nSize);
    {
        __adtf_sample_write_lock_mediadescription(descriptionSignal, newMediaSample, outputCoder);
        outputCoder->Set("f32Value", (tVoid *) &(value));
    }

    newMediaSample->SetTime(mediaSample->GetTime());
    outputPin.Transmit(newMediaSample);
}

tResult FunctionDriver::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(distanceOverallPin.Create("distance_overall", new cMediaType(0, 0, 0, "tSignalValue"),
                                               static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&distanceOverallPin));

    RETURN_NOERROR;
}

tResult FunctionDriver::CreateOutputPins(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &pDescManager, __exception_ptr));

    tChar const *strDescSignalValue = pDescManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strDescSignalValue);

    cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValue,
                                                             IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignal));

    RETURN_IF_FAILED(speedPin.Create("speed", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&speedPin));

    RETURN_IF_FAILED(steeringAnglePin.Create("steering_angle", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&steeringAnglePin));

    RETURN_NOERROR;
}

tFloat32 FunctionDriver::CalculateSinus(tFloat32 x, tFloat32 t)
{
    return sin((x + t) / sinLength) * sinWidth;
}

/**
 * @param x: point on x axis
 * @param param: [0] sinWidth, [1] sinLength
 */
tFloat32 FunctionDriver::SinusLengthIntegral(tFloat32 x, tFloat32 param[])
{
    return sqrt(1 + pow((param[0] / param[1]) * cos(x / param[1]), 2));
}

tFloat32 FunctionDriver::CalculateSinusSlope(tFloat32 x, tFloat32 t)
{
    tFloat32 h = sqrt(FLT_EPSILON);

    // numerical derivative
    // (f(x + h) - f(x - h)) / 2h
    return (CalculateSinus(x + h, t) - CalculateSinus(x - h, t)) / (2 * h);
}
