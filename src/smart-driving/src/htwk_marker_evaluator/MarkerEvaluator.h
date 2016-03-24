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

#ifndef cMarkerEvaluatorFilter_H
#define cMarkerEvaluatorFilter_H

#include "stdafx.h"
#include <Logger.h>
#include "../htwk_structs/tRoadSign.h"
#include "../htwk_structs/tReadyModule.h"


#define OID "htwk.markerEvaluator"
#define FILTER_NAME "HTWK Marker Evaluator"

#define NUM_ROADSIGNS 12
#define RANGE_DATASET 10

/*
 * The results of the marker detection filter can be visualized with this tool.
 * The input pin Road_Sign has to be connected with the output pin Road_Sign of the Marker Detection Filter.
 *
 * The GUI shows the symbol of the roadsign with the highest frequency in the incoming samples.
 * On the right side a graph is plotted which shows the frequency of all signs, i.e. Marker IDs,
 * in the incoming Media Samples from the marker detection filter.
*/
class MarkerEvaluator : public cFilter
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    private:
        Logger logger;

        cInputPin roadSignInput;
        cInputPin getReadyInput;
        cOutputPin readyOutput;
        cOutputPin markerOutput;
        cOutputPin noOvertakingOutput;

        cObjectPtr<IMediaType> boolMediaType;
        cObjectPtr<IMediaTypeDescription> boolDescription;

        cObjectPtr<IMediaType> enumMediaType;
        cObjectPtr<IMediaTypeDescription> enumDescription;

        cObjectPtr<IMediaType> roadSignMediaType;
        cObjectPtr<IMediaTypeDescription> roadSignDescription;

        /*
         * Buffer to hold the values received by the filter
         */
        std::deque<tInt> signBuffer;

    private:
        void calcData();

        unsigned long long lastUpdate;

        /* get the corresponding index of the given sign
         * @param signId the id of the sign
         */
        tInt getListIndexOfSign(tInt16 signId);

        tResult CreateDescriptions(IException **__exception_ptr);

        tResult CreateInputPins(IException **__exception_ptr);

        tResult CreateOutputPins(IException **__exception_ptr);

        tResult SendValue(cOutputPin &pin, tBool value);

        tResult SendEnum(cOutputPin &pin, tInt value);

    public:
        MarkerEvaluator(const tChar *__info);

        virtual ~MarkerEvaluator();

        virtual tResult Init(tInitStage eStage, IException **__exception_ptr);

        virtual tResult OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample);
};

#endif//cMarkerEvaluatorFilter_H
