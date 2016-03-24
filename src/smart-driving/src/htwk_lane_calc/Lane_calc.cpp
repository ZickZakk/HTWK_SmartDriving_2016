#include "Lane_calc.h"

ADTF_FILTER_PLUGIN("Lane Calculation", OID_LaneCalculation, LaneCalculation)

/**
 *   Contructor. The cFilter contructor needs to be called !!
 *   The SetProperty in the constructor is necessary if somebody wants to deal with
 *   the default values of the properties before init
 *
 */
LaneCalculation::LaneCalculation(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{
    // init value for scale factor
    reactionDistance = 1.0;
    calculationInterval = 0.02; // 20 ms
    scanTolerance = 2;

    // create the filter properties
    SetPropertyFloat(REACTION_DISTANCE, reactionDistance);
    SetPropertyFloat(REACTION_DISTANCE NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(REACTION_DISTANCE NSSUBPROP_DESCRIPTION,
                   "Standard distance for reaction to objects");

    SetPropertyFloat(CALCULATION_INTERVAL, calculationInterval);
    SetPropertyFloat(CALCULATION_INTERVAL NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(CALCULATION_INTERVAL NSSUBPROP_DESCRIPTION,
                   "Standard time between anglecalculation in s");

    SetPropertyFloat(SCAN_TOLERANCE, scanTolerance);
    SetPropertyFloat(SCAN_TOLERANCE NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(SCAN_TOLERANCE NSSUBPROP_DESCRIPTION,
                   "Number of pixeles in scan block that need to be different for the filter to take action");

    currentSpeed = 0.0;
    straightFrontDistance = 0.0;
    rightFrontDistance = 0.0;
    leftFrontDistance = 0.0;
}

/**
 *  Destructor. A Filter implementation needs always to have a virtual Destructor
 *              because the IFilter extends an IObject
 *
 */
LaneCalculation::~LaneCalculation()
{
}

/**
 * Processes on mediasample. The method only shows an example, how to deal with
 * a mediasample that is received, to get the specific data.
 * Further it shows, how to create a new mediasample in the system pool, to set the
 * data and to transmit it over a pin.
 *
 * @param   mediaSample   [in]  The received sample.
 *
 * @return  Standard result code.
 */
tResult LaneCalculation::ProcessInput(IMediaSample *mediaSample)
{
    RETURN_IF_POINTER_NULL(mediaSample);

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
tResult LaneCalculation::Init(tInitStage eStage, __exception)
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
        reactionDistance = tFloat32(GetPropertyFloat(REACTION_DISTANCE));
        calculationInterval = tFloat32(GetPropertyFloat(CALCULATION_INTERVAL));
        scanTolerance = tFloat32(GetPropertyFloat(SCAN_TOLERANCE));

    }
    else if (eStage == StageGraphReady)
    {
        // Start the timer for successive lane_calc calls
    }

    RETURN_NOERROR;
}

/**
 *   The Filters Start Function. see {@link IFilter#Start IFilter::Start}.
 *
 */
tResult LaneCalculation::Start(__exception)
{
    //nothing to do
    return cFilter::Start(__exception_ptr);
}

/**
 *   The Filters Stop Function. see {@link IFilter#Stop IFilter::Stop}.
 *
 */
tResult LaneCalculation::Stop(__exception)
{
    //nothing to do
    return cFilter::Stop(__exception_ptr);
}

/**
 *   The Filters Shutdown Function. see {@link IFilter#Shutdown IFilter::Shutdown}.
 *
 */
tResult LaneCalculation::Shutdown(tInitStage eStage, __exception)
{
    //nothing to do
    return cFilter::Shutdown(eStage, __exception_ptr);
}


/**
 *   The Filters Pin Event Implementation. see {@link IPinEventSink#OnPinEvent IPinEventSink::OnPinEvent}.
 *   Here the receiving Pin (cInputPin) will call the OnPinEvent.
 *
 */
tResult LaneCalculation::OnPinEvent(IPin *source,
                                     tInt nEventCode,
                                     tInt nParam1,
                                     tInt nParam2,
                                     IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    logger.StartLog();

    RETURN_NOERROR;
}


tResult LaneCalculation::CreateInputPins(IException **__exception_ptr)
{
    // create the input pins
    RETURN_IF_FAILED(currentSpeedPin.Create("current_speed", new cMediaType(0, 0, 0, "tSignalValue"),
                                            static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&currentSpeedPin));
/*
    RETURN_IF_FAILED(frontStraightDistancePin.Create("front_straight_distance", new cMediaType(0, 0, 0, "tSignalValue"),
                                                     static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&frontStraightDistancePin));

    RETURN_IF_FAILED(frontLeftDistancePin.Create("front_left_distance", new cMediaType(0, 0, 0, "tSignalValue"),
                                                 static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&frontLeftDistancePin));

    RETURN_IF_FAILED(frontRightDistancePin.Create("front_right_distance", new cMediaType(0, 0, 0, "tSignalValue"),
                                                  static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&frontRightDistancePin));
*/
    RETURN_NOERROR;
}

tResult LaneCalculation::CreateOutputPins(IException **__exception_ptr)
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
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionSignal));

    // create output pins
    RETURN_IF_FAILED(setSteeringAngle.Create("steering_angle", pTypeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&setSteeringAngle));

    RETURN_NOERROR;
}
