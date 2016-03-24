#include "ipm.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID_IPM, IPM)

/**
 *   Contructor. The cFilter contructor needs to be called !!
 *   The SetProperty in the constructor is necessary if somebody wants to deal with
 *   the default values of the properties before init
 *
 */
IPM::IPM(const tChar *__info) : cFilter(__info), logger(FILTER_NAME, 20)
{   // create the filter properties
    SetPropertyFloat(MAX_LINECOUNT_PROPERTY, 10);
    SetPropertyBool(MAX_LINECOUNT_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(MAX_LINECOUNT_PROPERTY NSSUBPROP_DESCRIPTION,
                   "Number of Lines detected by the Hough algorithm.");

    SetPropertyFloat(THRESHHOLD_PROPERTY, 50);
    SetPropertyBool(THRESHHOLD_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(THRESHHOLD_PROPERTY NSSUBPROP_DESCRIPTION,
                   "Threshhold used after Sobel.");

    SetPropertyFloat(INITIAL_VP_X_PROPERTY, 0);
    SetPropertyBool(INITIAL_VP_X_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(INITIAL_VP_X_PROPERTY NSSUBPROP_DESCRIPTION,
                   "Initial Vanishing Point X-Coordinate.");

    SetPropertyFloat(INITIAL_VP_Y_PROPERTY, 0);
    SetPropertyBool(INITIAL_VP_Y_PROPERTY NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(INITIAL_VP_Y_PROPERTY NSSUBPROP_DESCRIPTION,
                   "Initial Vanishing Point Y-Coordinate.");

    stabilizedVP = false;
    readySended = false;
}

/**
 *  Destructor. A Filter implementation needs always to have a virtual Destructor
 *              because the IFilter extends an IObject
 *
 */
IPM::~IPM()
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
tResult IPM::ProcessImage(IMediaSample *mediaSample)
{
    Mat workingImage;
    VisionUtils::ExtractImageFromMediaSample(mediaSample, inputFormat).copyTo(workingImage);
    RETURN_IF_POINTER_NULL(workingImage.data);

    cvtColor(workingImage, workingImage, CV_RGB2GRAY);
    Mat ipmImage = workingImage.clone();

    Mat upperIrrelevantArea(workingImage, Rect(0, 0, workingImage.cols, roi.y));
    upperIrrelevantArea = CV_RGB(0, 0, 0);
    Mat lowerIrrelevantArea(workingImage,
                            Rect(0, roi.y + roi.height, workingImage.cols, workingImage.rows - roi.y - roi.height));
    lowerIrrelevantArea = CV_RGB(0, 0, 0);

    // Calculate Vanishing Point
    Point2f vanishingPoint = vanishingPointDetector.CalculateVanishingPoint(workingImage);

    stabilizedVP = vanishingPointDetector.IsStabilized();

    Mat relevantIpm(ipmImage,
                    Rect(0, 0, workingImage.cols, roi.y + roi.height));

    outputImage = inversePerspectiveMapper.MapInversePerspectiveFromVanishingPoint(relevantIpm, vanishingPoint);

#ifndef NDEBUG
    cvtColor(workingImage, debugImage, CV_GRAY2RGB);
    circle(debugImage, vanishingPoint, 2, CV_RGB(255, 0, 0), -1);
    putText(debugImage, cString::Format("VP x: %d, y: %d", cvRound(vanishingPoint.x), cvRound(vanishingPoint.y)).GetPtr(),
            vanishingPoint + Point2f(2, 0), FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0));
#endif

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
tResult IPM::Init(tInitStage eStage, __exception)
{
    // this->logger.Log("Init...");
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(InitDescriptions(__exception_ptr));
        // create all the pins
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        maxLines = static_cast<unsigned long>(GetPropertyInt(MAX_LINECOUNT_PROPERTY));
        threshHold = GetPropertyInt(THRESHHOLD_PROPERTY);
        VanishingPointX = tFloat32(GetPropertyFloat(INITIAL_VP_X_PROPERTY));
        VanishingPointY = tFloat32(GetPropertyFloat(INITIAL_VP_Y_PROPERTY));

        vanishingPointDetector.Reset(maxLines, threshHold, VanishingPointX, VanishingPointY);
    }
    else if (eStage == StageGraphReady)
    {
        // get the image format of the input video pin
        cObjectPtr<IMediaType> type;
        RETURN_IF_FAILED(videoInputPin.GetMediaType(&type));

        cObjectPtr<IMediaTypeVideo> pTypeVideo;
        RETURN_IF_FAILED(type->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &pTypeVideo));

        inputFormat = *(pTypeVideo->GetFormat());
//        outputFormat = *(pTypeVideo->GetFormat());
        outputFormat.nWidth = 300;
        outputFormat.nHeight = 200;
        outputFormat.nBitsPerPixel = 8;
        outputFormat.nPixelFormat = cImage::PF_GREYSCALE_8;
        outputFormat.nBytesPerLine = 300;
        outputFormat.nSize = outputFormat.nBytesPerLine * 200;
        outputFormat.nPaletteSize = 0;


        ipmOutputPin.SetFormat(&outputFormat, NULL);
        videoDebugPin.SetFormat(&inputFormat, NULL);
    }

    RETURN_NOERROR;
}

/**
 *   The Filters Pin Event Implementation. see {@link IPinEventSink#OnPinEvent IPinEventSink::OnPinEvent}.
 *   Here the receiving Pin (cInputPin) will call the OnPinEvent.
 *
 */
tResult IPM::OnPinEvent(IPin *source,
                        tInt nEventCode,
                        tInt nParam1,
                        tInt nParam2,
                        IMediaSample *mediaSample)
{
    if (nEventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (source == &videoInputPin)
    {
        tFloat diffBefore = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;

        RETURN_IF_FAILED(ProcessImage(mediaSample));
        RETURN_IF_FAILED(TransmitImage(outputImage, mediaSample->GetTime(), ipmOutputPin));

        if (stabilizedVP)
        {
            RETURN_IF_FAILED(TransmitReady());
        }

#ifndef NDEBUG
        RETURN_IF_FAILED(TransmitDebug(debugImage, mediaSample->GetTime()));
#endif

        tFloat diff = (_clock->GetStreamTime() - mediaSample->GetTime()) / 1000.0;
        logger.Log(cString::Format("Before: %f, After: %f, Diff: %f", diffBefore, diff, diff - diffBefore).GetPtr());
    }
    else if (source == &roiPin)
    {
        RETURN_IF_FAILED(ProcessROI(mediaSample));
    }
    else if (source == &getReadyInput)
    {
        RETURN_IF_FAILED(ProcessGetReady(mediaSample));
    }
    else
    {
        RETURN_ERROR(ERR_NOT_SUPPORTED);
    }

    logger.EndLog();

    RETURN_NOERROR;
}


tResult IPM::CreateInputPins(IException **__exception_ptr)
{
    // create the input pins
    RETURN_IF_FAILED(videoInputPin.Create("Video_RGB_input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoInputPin));

    RETURN_IF_FAILED(roiPin.Create("ROI_Rect_input", typeRect, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&roiPin));

    RETURN_IF_FAILED(getReadyInput.Create("Get_Ready_Input", typeEnumBox, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&getReadyInput));

    RETURN_NOERROR;
}

tResult IPM::CreateOutputPins(IException **__exception_ptr)
{
    //create the video ipm output pin
    RETURN_IF_FAILED(
            ipmOutputPin.Create("Video_IPM_output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&ipmOutputPin));
    RETURN_IF_FAILED(
            videoDebugPin.Create("Debug_output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&videoDebugPin));

    //create pin for output
    RETURN_IF_FAILED(readyPin.Create("Ready_output", typeEnumBox, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&readyPin));

    RETURN_NOERROR;
}

tResult IPM::InitDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &pDescManager, __exception_ptr));

    tChar const *tRectDescription = pDescManager->GetMediaDescription("tRect");
    RETURN_IF_POINTER_NULL(tRectDescription);

    typeRect = new cMediaType(0, 0, 0, "tRect", tRectDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeRect->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionRect));


    //get description
    tChar const *enumBoxDescription = pDescManager->GetMediaDescription("tEnumBox");

    // checks if exists
    RETURN_IF_POINTER_NULL(enumBoxDescription);

    typeEnumBox = new cMediaType(0, 0, 0, "tEnumBox", enumBoxDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeEnumBox->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionEnumBox));

    RETURN_NOERROR;
}

tResult IPM::TransmitImage(const Mat image, const tTimeStamp &timeStamp, cVideoPin &pin)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    //creating new media sample for output
    cObjectPtr<IMediaSample> newMediaSample;
    RETURN_IF_FAILED(
            _runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &newMediaSample));
    RETURN_IF_FAILED(newMediaSample->AllocBuffer(outputFormat.nSize));
    newMediaSample->Update(timeStamp, image.data, outputFormat.nSize, 0);

    pin.Transmit(newMediaSample);

    RETURN_NOERROR;
}

tResult IPM::TransmitReady()
{
    if (!readyPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    if (readySended)
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid **) &pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionEnumBox->GetMediaSampleSerializer(&pSerializer);
    pMediaSample->AllocBuffer(pSerializer->GetDeserializedSize());

    tInt32 value = static_cast<tInt32>(tReadyModule::Ipm);
    //write date to the media sample with the coder of the descriptor
    {
        __adtf_sample_write_lock_mediadescription(descriptionEnumBox, pMediaSample, pCoderOutput);

        // set value from sample
        pCoderOutput->Set("tEnumValue", (tVoid *) &value);
    }

    pMediaSample->SetTime(_clock->GetStreamTime());

    //transmit media sample over output pin
    readyPin.Transmit(pMediaSample);

    readySended = true;

    RETURN_NOERROR;
}

tResult IPM::TransmitDebug(const Mat debugImage, const tTimeStamp &timeStamp)
{
    if (!videoDebugPin.IsConnected())
    {
        RETURN_NOERROR;
    }

    //creating new media sample for output
    cObjectPtr<IMediaSample> newMediaSample;
    RETURN_IF_FAILED(
            _runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &newMediaSample));
    RETURN_IF_FAILED(newMediaSample->AllocBuffer(inputFormat.nSize));
    newMediaSample->Update(timeStamp, debugImage.data, inputFormat.nSize, 0);

    videoDebugPin.Transmit(newMediaSample);

    RETURN_NOERROR;
}

tResult IPM::ProcessROI(IMediaSample *mediaSample)
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionRect->GetMediaSampleSerializer(&pSerializer);

    {
        __adtf_sample_read_lock_mediadescription(descriptionRect, mediaSample, pCoder);

        pCoder->Get("tX", (tVoid *) &roi.x);
        pCoder->Get("tY", (tVoid *) &roi.y);
        pCoder->Get("tWidth", (tVoid *) &roi.width);
        pCoder->Get("tHeight", (tVoid *) &roi.height);
    }

    logger.Log(cString::Format("ROI x: %d, y: %d, width: %d, height: %d", roi.x, roi.y, roi.width, roi.height)
                       .GetPtr());
    RETURN_NOERROR;
}

tResult IPM::ProcessGetReady(IMediaSample *mediaSample)
{
    cObjectPtr<IMediaSerializer> pSerializer;
    descriptionEnumBox->GetMediaSampleSerializer(&pSerializer);

    tReadyModule::ReadyModuleEnum module;

    {
        __adtf_sample_read_lock_mediadescription(descriptionEnumBox, mediaSample, pCoder);
        pCoder->Get("tEnumValue", (tVoid *) &module);
    }

    if (tReadyModule::Ipm == module)
    {
        logger.Log("Resetting.", false);
        vanishingPointDetector.Reset(maxLines, threshHold, VanishingPointX, VanishingPointY);
        readySended = false;
    }

    RETURN_NOERROR;
}
