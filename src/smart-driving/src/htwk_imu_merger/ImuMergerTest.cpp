//
// Created by pbachmann on 2/11/16.
//

#include "ImuMergerTest.h"
#include "../htwk_structs/tIMU.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, ImuMergerTest);

ImuMergerTest::ImuMergerTest(const tChar *__info) : logger(FILTER_NAME)
{

}

ImuMergerTest::~ImuMergerTest(void)
{

}

tResult ImuMergerTest::Init(cFilter::tInitStage eStage, ucom::IException **__exception_ptr)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateDescriptions(__exception_ptr));
        RETURN_IF_FAILED(CreateInputPins(__exception_ptr));
    }

    RETURN_NOERROR;
}

tResult ImuMergerTest::OnPinEvent(IPin *pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample *pMediaSample)
{
    RETURN_IF_POINTER_NULL(pSource);
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (IPinEventSink::PE_MediaSampleReceived != nEventCode)
    {
        RETURN_NOERROR;
    }

    if (pSource == &imuStructInput)
    {
        tIMU imu;

        cObjectPtr<IMediaSerializer> pSerializer;
        descriptionImuStruct->GetMediaSampleSerializer(&pSerializer);

        {
            __adtf_sample_read_lock_mediadescription(descriptionImuStruct, pMediaSample, pCoder);

            pCoder->Get("tYaw", (tVoid *) &imu.tYaw);
            pCoder->Get("tAccX", (tVoid *) &imu.tAccX);
            pCoder->Get("tAccY", (tVoid *) &imu.tAccY);
            pCoder->Get("tAccZ", (tVoid *) &imu.tAccZ);
            pCoder->Get("tPitch", (tVoid *) &imu.tPitch);
            pCoder->Get("tRoll", (tVoid *) &imu.tRoll);
        }

        logger.Log("Received IMU:");
        logger.Log(cString::Format("Yaw: %f", imu.tYaw).GetPtr());
        logger.Log(cString::Format("Pitch: %f", imu.tPitch).GetPtr());
        logger.Log(cString::Format("Roll: %f", imu.tRoll).GetPtr());
        logger.Log(cString::Format("AccX: %f", imu.tAccX).GetPtr());
        logger.Log(cString::Format("AccY: %f", imu.tAccY).GetPtr());
        logger.Log(cString::Format("AccZ: %f", imu.tAccZ).GetPtr());
        logger.Log("------");
    }

    RETURN_NOERROR;
}

tResult ImuMergerTest::CreateInputPins(IException **__exception_ptr)
{
    RETURN_IF_FAILED(imuStructInput.Create("imu_struct", typeImuStruct, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&imuStructInput));
    RETURN_NOERROR;
}

tResult ImuMergerTest::CreateDescriptions(IException **__exception_ptr)
{
    cObjectPtr<IMediaDescriptionManager> descManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER,
                                         (tVoid **) &descManager, __exception_ptr));

    // get IMU Struct
    tChar const *imuStructDescription = descManager->GetMediaDescription("tIMU");
    RETURN_IF_POINTER_NULL(imuStructDescription);
    typeImuStruct = new cMediaType(0, 0, 0, "tIMU", imuStructDescription,
                                   IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(typeImuStruct->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid **) &descriptionImuStruct));
    RETURN_NOERROR;
}
