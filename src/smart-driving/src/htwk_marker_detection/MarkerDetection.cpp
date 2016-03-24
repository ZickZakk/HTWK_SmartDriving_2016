/**
Copyright (c)
Audi Autonomous Driving Cup. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: “This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.”
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


**********************************************************************
* $Author:: spiesra $  $Date:: 2015-05-13 08:29:07#$ $Rev:: 35003   $
**********************************************************************/

#include "MarkerDetection.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, MarkerDetection)

MarkerDetection::MarkerDetection(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    UCOM_REGISTER_TIMING_SPOT(cString(OIGetInstanceName()) + "::Process::Start", m_oProcessStart);

    SetPropertyBool("Debug Output to Console", tFalse);
    SetPropertyStr("Debug Output to Console" NSSUBPROP_DESCRIPTION,
                   "If enabled additional debug information is printed to the console (Warning: decreases performance).");

    SetPropertyStr("Dictionary File For Markers", "");
    SetPropertyBool("Dictionary File For Markers" NSSUBPROP_FILENAME, tTrue);
    SetPropertyStr("Dictionary File For Markers" NSSUBPROP_FILENAME NSSUBSUBPROP_EXTENSIONFILTER, "YML Files (*.yml)");
    SetPropertyStr("Dictionary File For Markers" NSSUBPROP_DESCRIPTION,
                   "Here you have to set the dicitionary file which holds the marker ids and their content");

    SetPropertyStr("Calibration File for used Camera", "");
    SetPropertyBool("Calibration File for used Camera" NSSUBPROP_FILENAME, tTrue);
    SetPropertyStr("Calibration File for used Camera" NSSUBPROP_FILENAME NSSUBSUBPROP_EXTENSIONFILTER,
                   "YML Files (*.yml)");
    SetPropertyStr("Calibration File for used Camera" NSSUBPROP_DESCRIPTION,
                   "Here you have to set the file with calibration paraemters of the used camera");

    SetPropertyInt("Video Output Pin", 1);
    SetPropertyStr("Video Output Pin" NSSUBPROP_VALUELIST, "1@None|2@Detected Signs|");
    SetPropertyStr("Video Output Pin" NSSUBPROP_DESCRIPTION,
                   "If enabled the video stream with the highlighted markers is transmitted at the output pin (Warning: decreases performance).");

    SetPropertyFloat("Size of Markers", 0.117f);
    SetPropertyStr("Size of Markers" NSSUBPROP_DESCRIPTION, "Size (length of one side) of markers in m");

    UCOM_REGISTER_TIMING_SPOT(cString(OIGetInstanceName()) + "::Process::End", m_oProcessEnd);
}

MarkerDetection::~MarkerDetection()
{
}

tResult MarkerDetection::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
        RETURN_IF_FAILED(CreateOutputPins(__exception_ptr));
    }
    else if (eStage == StageNormal)
    {
        m_iOutputMode = GetPropertyInt("Video Output Pin");
        m_f32MarkerSize = static_cast<tFloat32>(GetPropertyFloat("Size of Markers"));
        m_bDebugModeEnabled = GetPropertyBool("Debug Output to Console");

        m_bCamaraParamsLoaded = tFalse;

        //Get path of marker configuration file
        cFilename fileConfig = GetPropertyStr("Dictionary File For Markers");

        //create absolute path for marker configuration file
        ADTF_GET_CONFIG_FILENAME(fileConfig);
        fileConfig = fileConfig.CreateAbsolutePath(".");

        //check if marker configuration file exits
        if (fileConfig.IsEmpty() || !(cFileSystem::Exists(fileConfig)))
        {
            LOG_ERROR("Dictionary File for Markers not found");
            RETURN_ERROR(ERR_INVALID_FILE);
        }
        else
        {
            //try to read the marker configuration file
            if (m_Dictionary.fromFile(string(fileConfig)) == false)
            {
                RETURN_ERROR(ERR_INVALID_FILE);
                LOG_ERROR("Dictionary File for Markers could not be read");
            }
            if (m_Dictionary.size() == 0)
            {
                RETURN_ERROR(ERR_INVALID_FILE);
                LOG_ERROR("Dictionary File does not contain valid markers or could not be read sucessfully");
            }
            //set marker configuration file to highlyreliable markers class
            if (!(HighlyReliableMarkers::loadDictionary(m_Dictionary) == tTrue))
            {
                LOG_ERROR("Dictionary File could not be read for highly reliable markers");
            }
        }

        //Get path of calibration file with camera paramters
        cFilename fileCalibration = GetPropertyStr("Calibration File for used Camera");;

        //Get path of calibration file with camera paramters
        ADTF_GET_CONFIG_FILENAME(fileCalibration);
        fileCalibration = fileCalibration.CreateAbsolutePath(".");

        //check if calibration file with camera paramters exits
        if (fileCalibration.IsEmpty() || !(cFileSystem::Exists(fileCalibration)))
        {
            LOG_ERROR("Calibration File for camera not found");
        }
        else
        {
            // read the calibration file with camera paramters exits and save to member variable
            m_TheCameraParameters.readFromXMLFile(fileCalibration.GetPtr());
            cv::FileStorage camera_data(fileCalibration.GetPtr(), cv::FileStorage::READ);
            camera_data["camera_matrix"] >> m_Intrinsics;
            camera_data["distortion_coefficients"] >> m_Distorsion;
            m_bCamaraParamsLoaded = tTrue;
        }
    }
    else if (eStage == StageGraphReady)
    {
        // get the image format of the input video pin
        cObjectPtr<IMediaType> pType;
        RETURN_IF_FAILED(m_oPinInputVideo.GetMediaType(&pType));

        cObjectPtr<IMediaTypeVideo> pTypeVideo;
        RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &pTypeVideo));

        // set the image format of the input video pin
        UpdateInputImageFormat(pTypeVideo->GetFormat());
        //TODO: why was the following line inserted?
        //m_oPinInputVideo.SetFormat(&m_sInputFormat, NULL);

        // set the image format of the output video pin
        UpdateOutputImageFormat(pTypeVideo->GetFormat());

        // set paramenter for the marker detector class
        // same parameters as in aruco sample aruco_hrm_test.cpp
        // effects of parameters has to be tested
        m_MDetector.setMakerDetectorFunction(aruco::HighlyReliableMarkers::detect);
        m_MDetector.setThresholdParams(21, 7);
        m_MDetector.setCornerRefinementMethod(aruco::MarkerDetector::LINES);
        m_MDetector.setWarpSize((m_Dictionary[0].n() + 2) * 8);
        m_MDetector.setMinMaxSize(0.005f, 0.5f);
    }
    RETURN_NOERROR;
}

tResult MarkerDetection::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    switch (nEventCode)
    {
        case IPinEventSink::PE_MediaSampleReceived:
            // a new image was received so the processing is started
            if (pSource == &m_oPinInputVideo)
            {
                UCOM_TIMING_SPOT(m_oProcessStart);
                ProcessVideo(pMediaSample);
                UCOM_TIMING_SPOT(m_oProcessEnd);
            }
            else if (pSource == &getReadyInput)
            {
                tReadyModule::ReadyModuleEnum module;

                {
                    __adtf_sample_read_lock_mediadescription(enumDescription, pMediaSample, pCoder);
                    pCoder->Get("tEnumValue", (tVoid *) &module);
                }

                if (tReadyModule::MarkerDetection == module)
                {
                    logger.Log("Resetting.", false);

                    m_TheMarkers.clear();
                    m_f32MarkerSize = 0;

                    sendRoadSignStruct(MARKER_ID_NOMATCH, m_f32MarkerSize, pMediaSample->GetTime());
                    SendEnum(readyOutput, static_cast<tInt>(tReadyModule::MarkerDetection));
                }
            }
            break;
        case IPinEventSink::PE_MediaTypeChanged:
            if (pSource == &m_oPinInputVideo)
            {
                //the input format was changed, so the imageformat has to changed in this filter also
                cObjectPtr<IMediaType> pType;
                RETURN_IF_FAILED(m_oPinInputVideo.GetMediaType(&pType));

                cObjectPtr<IMediaTypeVideo> pTypeVideo;
                RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid **) &pTypeVideo));

                UpdateInputImageFormat(m_oPinInputVideo.GetFormat());
                UpdateOutputImageFormat(m_oPinInputVideo.GetFormat());
            }
            break;
        default:
            break;
    }
    RETURN_NOERROR;
}

tResult MarkerDetection::ProcessVideo(adtf::IMediaSample *pISample)
{

    RETURN_IF_POINTER_NULL(pISample);

    //creating new media sample for output
    cObjectPtr<IMediaSample> pNewSample;
    RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid **) &pNewSample));
    RETURN_IF_FAILED(pNewSample->AllocBuffer(m_sOutputFormat.nSize));

    //creating new pointer for input data
    const tVoid *l_pSrcBuffer;
    //creating matrix for input image
    Mat TheInputImage;
    Mat TheOutputImage;
    //receiving data from input sample, and saving to TheInputImage
    if (IS_OK(pISample->Lock(&l_pSrcBuffer)))
    {
        //convert to mat
        TheInputImage = Mat(m_sInputFormat.nHeight, m_sInputFormat.nWidth, CV_8UC3, (tVoid *) l_pSrcBuffer, m_sInputFormat.nBytesPerLine);

        // take only the left half to detect any markers
        TheInputImage = TheInputImage(cv::Rect(0, 0, TheInputImage.cols / 2, TheInputImage.rows));

        // doing the detection of markers in image
        m_MDetector.detect(TheInputImage, m_TheMarkers, m_TheCameraParameters, static_cast<float>(m_f32MarkerSize));
    }

    pISample->Unlock(l_pSrcBuffer);

    // 1: nothing is drawn, 2: results are drawn so we need the have copy of the frame otherwise the orginal mediasample is modified
    if (m_iOutputMode == 2)
    {
        // do a deep copy of the image, otherwise the orginal frame is modified
        TheOutputImage = TheInputImage.clone();
    }

    unsigned int maxAreaIndex = 0;
//    for (unsigned int i = 0; i < m_TheMarkers.size(); i++)
//    {
//        if (m_TheMarkers[maxAreaIndex].getArea() > m_TheMarkers[i].getArea())
//        {
//            maxAreaIndex = i;
//        }
//        float f = (4 * m_TheMarkers[i].getArea() * M_PI) / (m_TheMarkers[i].getPerimeter() * m_TheMarkers[i].getPerimeter());
//        logger.Log(cString::Format("Formfaktor: %f", f).GetPtr(), false);
//        // markers that are too unproportional are considered false alarms and thus marked as "no sign"
//        if (f < 0.745f || f > 0.815)
//        { // regular markers have f = 0.78 -> +- 3%
//            m_TheMarkers[i].id = 99;
//            logger.Log("Fake marker", false);
//        }
//    }

    //print marker info and draw the markers in image
    for (unsigned int i = 0; i < m_TheMarkers.size(); i++)
    {

        bool fake = false;
        float f = (4 * m_TheMarkers[i].getArea() * M_PI) / (m_TheMarkers[i].getPerimeter() * m_TheMarkers[i].getPerimeter());
        logger.Log(cString::Format("Formfaktor: %f", f).GetPtr(), false);
        // markers that are too unproportional are considered false alarms and thus marked as "no sign"
        if (f < 0.745f || f > 0.815)
        { // regular markers have f = 0.78 -> +- 3%
            fake = true;
            m_TheMarkers[i].id = 99;
            logger.Log("Fake marker", false);
        }
        else
        {
            if (m_TheMarkers[maxAreaIndex].getArea() > m_TheMarkers[i].getArea())
            {
                maxAreaIndex = i;
            }
        }

        if (m_iOutputMode == 2)
        {
            // draw the marker in the image
            m_TheMarkers[i].draw(TheOutputImage, Scalar(0, 0, 255), 1);

            // draw cube in image
            //CvDrawingUtils::draw3dCube(TheInputImage,m_TheMarkers[i],m_TheCameraParameters);

            // draw 3d axis in image if the intrinsic params were loaded
            if (m_bCamaraParamsLoaded)
            {
                CvDrawingUtils::draw3dAxis(TheOutputImage, m_TheMarkers[i], m_TheCameraParameters);
            }
        }

        //logger.Log(cString::Format("Marker Size: %f; Perim: %f",
        //                           m_TheMarkers[i].getArea(), m_TheMarkers[i].getPerimeter()).GetPtr());

        // call the function to transmit a road sign sample with the detected marker
        if (!fake)
        {
            sendRoadSignStruct(static_cast<tInt16>(m_TheMarkers[maxAreaIndex].id), m_TheMarkers[maxAreaIndex].getArea(), pISample->GetTime());
        }

        // call the function to transmit a extended road sign sample with the detected marker if the
        // Tvec in the marker was correctly set
        if (m_bCamaraParamsLoaded)
        {
            //sendRoadSignStructExt(static_cast<tInt16>(m_TheMarkers[i].id), m_TheMarkers[i].getArea(),
            //                      pISample->GetTime(), m_TheMarkers[i].Tvec, m_TheMarkers[i].Rvec);
            //get camera position

            if (m_TheMarkers[i].id == 2)
            {
                cv::Mat rotationVector(m_TheMarkers[i].Rvec);;
                cv::Mat translationVector(m_TheMarkers[i].Tvec);;
                tFloat32 aSignPosition[3];
                aSignPosition[0] = 0;
                aSignPosition[1] = 0;
                aSignPosition[2] = 0;
                vector<Point3d> m_worldCoordinatesSign;
                tFloat32 halfSize = m_f32MarkerSize / 2;
                m_worldCoordinatesSign.push_back(
                        Point3d(aSignPosition[0] - halfSize, aSignPosition[1] - halfSize, aSignPosition[2]));
                m_worldCoordinatesSign.push_back(
                        Point3d(aSignPosition[0] + halfSize, aSignPosition[1] - halfSize, aSignPosition[2]));
                m_worldCoordinatesSign.push_back(
                        Point3d(aSignPosition[0] + halfSize, aSignPosition[1] + halfSize, aSignPosition[2]));
                m_worldCoordinatesSign.push_back(
                        Point3d(aSignPosition[0] - halfSize, aSignPosition[1] + halfSize, aSignPosition[2]));


                vector<Point2d> ImagePoints;
                for (int c = 0; c < 4; c++)
                {

                    //LOG_INFO(cString::Format("Imagepoint %d: %f,%f",c,m_TheMarkers[i][c].x,m_TheMarkers[i][c].y));
                    ImagePoints.push_back(Point2d(m_TheMarkers[i][c].x, m_TheMarkers[i][c].y));
                }


                cv::solvePnP(m_worldCoordinatesSign, ImagePoints, m_Intrinsics, m_Distorsion, rotationVector,
                             translationVector, true, 0);

                cv::Mat RodriguesMat;

                cv::Rodrigues(rotationVector, RodriguesMat);
                RodriguesMat = RodriguesMat.t();

                translationVector = -RodriguesMat * translationVector;

                /*cv::Rodrigues(Rodrigues.t(),cameraRotationVector);
                cv::Mat t= translationVector.t();
                cameraTranslationVector= -cameraRotationVector * t;*/

            }
        }
    }

    //update new media sample with image data if something has to be transmitted
    if (m_iOutputMode == 2)
    {
        pNewSample->Update(pISample->GetTime(), TheOutputImage.data, m_sOutputFormat.nSize, 0);
        m_oPinOutputVideo.Transmit(pNewSample);
    }

    RETURN_NOERROR;
}

tResult MarkerDetection::UpdateInputImageFormat(const tBitmapFormat *pFormat)
{
    if (pFormat != NULL)
    {
        m_sInputFormat = (*pFormat);

        LOG_INFO(adtf_util::cString::Format(
                "Marker Detection Filter: Input: Size %d x %d ; BPL %d ; Size %d , PixelFormat; %d",
                m_sInputFormat.nWidth, m_sInputFormat.nHeight, m_sInputFormat.nBytesPerLine, m_sInputFormat.nSize,
                m_sInputFormat.nPixelFormat));

    }

    RETURN_NOERROR;
}

tResult MarkerDetection::UpdateOutputImageFormat(const tBitmapFormat *pFormat)
{
    if (pFormat == NULL)
    {
        RETURN_NOERROR;
    }

    m_sOutputFormat = (*pFormat);

    // halfed output format, because image is cut in half in the process
    m_sOutputFormat.nWidth = m_sOutputFormat.nWidth / 2;
    m_sOutputFormat.nBytesPerLine = m_sOutputFormat.nBytesPerLine / 2;
    m_sOutputFormat.nSize = m_sOutputFormat.nBytesPerLine * m_sOutputFormat.nHeight;

    logger.Log(cString::Format(
            "Marker Detection Filter: Output: Size %d x %d ; BPL %d ; Size %d, PixelFormat; %d",
            m_sOutputFormat.nWidth, m_sOutputFormat.nHeight, m_sOutputFormat.nBytesPerLine, m_sOutputFormat.nSize,
            m_sOutputFormat.nPixelFormat).GetPtr(), false);

    m_oPinOutputVideo.SetFormat(&m_sOutputFormat, NULL);

    RETURN_NOERROR;
}

tResult MarkerDetection::sendRoadSignStruct(const tInt16 &i16ID, const tFloat32 &f32MarkerSize, const tTimeStamp &timeOfFrame)
{
    cObjectPtr<IMediaSample> pMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &pMediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    roadSignDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(roadSignDescription, pMediaSample, pCoder);

        pCoder->Set("i16Identifier", (tVoid *) &i16ID);
        pCoder->Set("f32Imagesize", (tVoid *) &f32MarkerSize);

        pMediaSample->SetTime(timeOfFrame);
    }

    RETURN_IF_FAILED(m_oPinRoadSign.Transmit(pMediaSample));

    RETURN_NOERROR;
}

tResult MarkerDetection::sendRoadSignStructExt(const tInt16 &i16ID, const tFloat32 &f32MarkerSize, const tTimeStamp &timeOfFrame, const Mat &Tvec,
                                               const Mat &Rvec)
{
    cObjectPtr<IMediaSample> pMediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &pMediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    roadSignExtDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();

    RETURN_IF_FAILED(pMediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(roadSignExtDescription, pMediaSample, pCoder);

        pCoder->Set("i16Identifier", (tVoid *) &i16ID);
        pCoder->Set("f32Imagesize", (tVoid *) &f32MarkerSize);
        pCoder->Set("af32TVec", (tVoid *) Tvec.data);
        pCoder->Set("af32RVec", (tVoid *) Rvec.data);

        pMediaSample->SetTime(timeOfFrame);
    }

    RETURN_IF_FAILED(m_oPinRoadSignExt.Transmit(pMediaSample));

    RETURN_NOERROR;
}

tResult MarkerDetection::SendEnum(cOutputPin &pin, tInt value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    enumDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(enumDescription, mediaSample, pCoder);
        pCoder->Set("tEnumValue", (tVoid *) &value);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult MarkerDetection::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get roadSign
    tChar const *roadSignSignalValueDescription = descManager->GetMediaDescription("tRoadSign");
    RETURN_IF_POINTER_NULL(roadSignSignalValueDescription);
    roadSignMediaType = new cMediaType(0, 0, 0, "tRoadSign", roadSignSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(roadSignMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &roadSignDescription));

    // get roadSign
    tChar const *roadSignExtSignalValueDescription = descManager->GetMediaDescription("tRoadSignExt");
    RETURN_IF_POINTER_NULL(roadSignExtSignalValueDescription);
    roadSignExtMediaType = new cMediaType(0, 0, 0, "tRoadSignExt", roadSignExtSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(roadSignExtMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &roadSignExtDescription));

    // get roadSign
    tChar const *strDescEnum = descManager->GetMediaDescription("tEnumBox");
    RETURN_IF_POINTER_NULL(strDescEnum);
    enumMediaType = new cMediaType(0, 0, 0, "tEnumBox", strDescEnum, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(enumMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &enumDescription));

    RETURN_NOERROR;
}

tResult MarkerDetection::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(m_oPinOutputVideo.Create("Video_RGB_output", IPin::PD_Output, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oPinOutputVideo));

    RETURN_IF_FAILED(m_oPinRoadSign.Create("RoadSignAnalysis_Output", roadSignMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&m_oPinRoadSign));

    RETURN_IF_FAILED(m_oPinRoadSignExt.Create("RoadSignExtAnalysis_Output", roadSignExtMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&m_oPinRoadSignExt));

    RETURN_IF_FAILED(readyOutput.Create("Ready_Ouput", enumMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&readyOutput));

    RETURN_NOERROR;
}

tResult MarkerDetection::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(m_oPinInputVideo.Create("Video_RGB_input", IPin::PD_Input, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&m_oPinInputVideo));

    RETURN_IF_FAILED(getReadyInput.Create("Reset_Input", enumMediaType, this));
    RETURN_IF_FAILED(RegisterPin(&getReadyInput));

    RETURN_NOERROR;
}
