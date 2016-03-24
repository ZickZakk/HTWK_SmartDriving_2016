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

/*
 * Modified by HTWK Smart-Driving.
 */

#include "MarkerEvaluator.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, MarkerEvaluator);

MarkerEvaluator::MarkerEvaluator(const tChar *__info) : cFilter(__info), logger(FILTER_NAME)
{
    lastUpdate = 0;
}

MarkerEvaluator::~MarkerEvaluator()
{
}

tResult MarkerEvaluator::Init(tInitStage eStage, __exception)
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
    }
    else if (eStage == StageGraphReady)
    {
    }

    RETURN_NOERROR;
}

tResult MarkerEvaluator::OnPinEvent(IPin *sourcePin, tInt eventCode, tInt nParam1, tInt nParam2, IMediaSample *mediaSample)
{
    if (eventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (sourcePin == &roadSignInput)
    {
        struct timeval tv;

        gettimeofday(&tv, NULL);

        unsigned long long msnew =
                (unsigned long long) (tv.tv_sec) * 1000 +
                (unsigned long long) (tv.tv_usec) / 1000;

        if (lastUpdate + 3000 < msnew)
        {
            signBuffer.clear();
        }
        lastUpdate = msnew;

        tInt16 signId = 0;
        tFloat32 signSize = 0;

        {
            __adtf_sample_read_lock_mediadescription(roadSignDescription, mediaSample, pCoderInput);

            pCoderInput->Get("i16Identifier", (tVoid *) &signId);
            pCoderInput->Get("f32Imagesize", (tVoid *) &signSize);
        }

        signBuffer.push_back(signId);
        if (signBuffer.size() > RANGE_DATASET)
        {
            calcData();
        }
    }
    else if (sourcePin == &getReadyInput)
    {
        tReadyModule::ReadyModuleEnum module;

        {
            __adtf_sample_read_lock_mediadescription(enumDescription, mediaSample, pCoder);
            pCoder->Get("tEnumValue", (tVoid *) &module);
        }

        if (tReadyModule::MarkerEvaluator == module)
        {
            logger.Log("Resetting.", false);

            signBuffer.clear();

            SendEnum(markerOutput, static_cast<tInt>(tRoadSign::NO_MATCH));
            SendValue(noOvertakingOutput, false);

            SendEnum(readyOutput, static_cast<tInt>(tReadyModule::MarkerEvaluator));
        }
    }

    logger.EndLog();

    RETURN_NOERROR;
}

void MarkerEvaluator::calcData()
{
    tInt16 data;

    short int num_list[NUM_ROADSIGNS] = {0};

    //increase all roadsign by 1
    for (int i = 0; i < RANGE_DATASET; i++)
    {
        data = signBuffer.front();
        signBuffer.pop_front();

        num_list[getListIndexOfSign(data)]++;

    }

    int tmp = 0, tmpIndex = 0;
    //get the max value
    for (int i = 0; i < NUM_ROADSIGNS; i++)
    {
        if (num_list[i] > tmp)
        {
            tmp = num_list[i];
            tmpIndex = i;
        }
    }

    logger.Log(cString::Format("Sign detected: %d", tmpIndex).GetPtr());
    SendEnum(markerOutput, tmpIndex);

    // TODO reset no-overtaking only on specific signs
    tBool isNoOvertaking = tRoadSign::NO_OVERTAKING == tmpIndex;
    SendValue(noOvertakingOutput, isNoOvertaking);
}

tResult MarkerEvaluator::SendValue(cOutputPin &pin, tBool value)
{
    if (!pin.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> mediaSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid **) &mediaSample));

    cObjectPtr<IMediaSerializer> pSerializer;
    boolDescription->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    RETURN_IF_FAILED(mediaSample->AllocBuffer(nSize));

    {
        __adtf_sample_write_lock_mediadescription(boolDescription, mediaSample, pCoder);
        pCoder->Set("bValue", (tVoid *) &value);
    }

    mediaSample->SetTime(_clock->GetStreamTime());
    RETURN_IF_FAILED(pin.Transmit(mediaSample));

    RETURN_NOERROR;
}

tResult MarkerEvaluator::SendEnum(cOutputPin &pin, tInt value)
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

tInt MarkerEvaluator::getListIndexOfSign(tInt16 signId)
{
    switch (signId)
    {
        case MARKER_ID_GIVEWAY:
            return tRoadSign::GIVE_WAY;
        case MARKER_ID_HAVEWAY:
            return tRoadSign::HAVE_WAY;
        case MARKER_ID_STOPANDGIVEWAY:
            return tRoadSign::STOP_AND_GIVE_WAY;
        case MARKER_ID_PARKINGAREA:
            return tRoadSign::PARKING_AREA;
        case MARKER_ID_AHEADONLY:
            return tRoadSign::AHEAD_ONLY;
        case MARKER_ID_UNMARKEDINTERSECTION:
            return tRoadSign::UNMARKED_INTERSECTION;
        case MARKER_ID_PEDESTRIANCROSSING:
            return tRoadSign::PEDASTRIAN_CROSSING;
        case MARKER_ID_ROUNDABOUT:
            return tRoadSign::ROUNDABOUT;
        case MARKER_ID_NOOVERTAKING:
            return tRoadSign::NO_OVERTAKING;
        case MARKER_ID_NOENTRYVEHICULARTRAFFIC:
            return tRoadSign::NO_ENTRY_VIHICULAR_TRAFFIC;
        case MARKER_ID_ONEWAYSTREET:
            return tRoadSign::ONEWAY_STREET;
        default:
            return tRoadSign::NO_MATCH;
    }
}

tResult MarkerEvaluator::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get bool
    tChar const *boolSignalValueDescription = descManager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(boolSignalValueDescription);
    boolMediaType = new cMediaType(0, 0, 0, "tBoolSignalValue", boolSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(boolMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &boolDescription));

    // get enum
    tChar const *enumSignalValueDescription = descManager->GetMediaDescription("tEnumBox");
    RETURN_IF_POINTER_NULL(enumSignalValueDescription);
    enumMediaType = new cMediaType(0, 0, 0, "tEnumBox", enumSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(enumMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &enumDescription));

    // get roadSign
    tChar const *roadSignSignalValueDescription = descManager->GetMediaDescription("tRoadSign");
    RETURN_IF_POINTER_NULL(roadSignSignalValueDescription);
    roadSignMediaType = new cMediaType(0, 0, 0, "tRoadSign", roadSignSignalValueDescription, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(roadSignMediaType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &roadSignDescription));

    RETURN_NOERROR;
}

tResult MarkerEvaluator::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(roadSignInput.Create("RoadSignAnalysis_Input", roadSignMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&roadSignInput));

    RETURN_IF_FAILED(getReadyInput.Create("Reset_Input", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&getReadyInput));

    RETURN_NOERROR;
}

tResult MarkerEvaluator::CreateOutputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(markerOutput.Create("Marker_Output", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&markerOutput));

    RETURN_IF_FAILED(noOvertakingOutput.Create("NoOvertaking_Output", boolMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&noOvertakingOutput));

    RETURN_IF_FAILED(readyOutput.Create("Ready_Output", enumMediaType, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&readyOutput));

    RETURN_NOERROR;
}
