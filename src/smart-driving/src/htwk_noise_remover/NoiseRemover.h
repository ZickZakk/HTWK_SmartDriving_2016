/**
 * @author pbachmann
 */

#ifndef _HTWK_NOISEREMOVER_H_
#define _HTWK_NOISEREMOVER_H_

#include "stdafx.h"

#include "../htwk_logger/Logger.h"
#include "opencv2/opencv.hpp"
#include "../htwk_vision/Vision.h"

using namespace cv;

#define OID_NOISE_REMOVER "htwk.noise_remover"
#define FILTER_NAME "HTWK Noise Remover"

// Properties
#define ITERATION_PROPERTY "Iterations"

class NoiseRemover : public adtf::cFilter
{
    ADTF_FILTER(OID_NOISE_REMOVER, FILTER_NAME, OBJCAT_DataFilter);

    private:
        cVideoPin videoInputPin;
        cVideoPin videoOutputPin;

        Logger logger;

        tBitmapFormat videoInputFormat;
        tBitmapFormat videoOutputFormat;

        Mat outputImage;
        Mat mask;

        //properties
        tUInt8 iterations;

    private:
        tResult ProcessInput();

        tResult TransmitOutput(const tTimeStamp &timeStamp);

        void InitializeProperties();

        tResult CreateInputPins(__exception = NULL);

        tResult CreateOutputPins(__exception = NULL);

    public:
        NoiseRemover(const tChar *__info);

        virtual ~NoiseRemover();

    public: // aadc overwrite
        tResult OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2, IMediaSample *mediaSample);

        tResult Init(tInitStage eStage, __exception = NULL);
};

#endif // _HTWK_NOISEREMOVER_H_
