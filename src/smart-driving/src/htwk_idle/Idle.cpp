//
// Created by pbachmann on 2/12/16.
//

#include "Idle.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, Idle)

Idle::Idle(const tChar *__info) : BaseDecisionModule(__info, FILTER_NAME, DM_IDLE)
{
    logger.SetSkip(100);
}

Idle::~Idle()
{
}

tResult Idle::Init(cFilter::tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Init(eStage, __exception_ptr));

    RETURN_NOERROR;
}

tResult Idle::OnTrigger(tFloat32 interval)
{
    tCarState::CarStateEnum state;

    if (IS_FAILED(worldService->Pull<tCarState::CarStateEnum>(WORLD_CAR_STATE, state)))
    {
        logger.Log("No car state available.", false);

        TakeControl();
        RETURN_NOERROR;
    }

    switch (state)
    {
        case tCarState::Running:
            ResetDriveInstructions(sourceModule);
            break;
        default:
            TakeControl();
            break;
    }

    RETURN_NOERROR;
}

void Idle::TakeControl()
{
    steeringControlTaken = true;
    speed = 0;
    speedControlTaken = true;
    curveRadius = INT_MAX;
    curveAngle = 0;

    turnSignalLeftEnabled = false;
    turnSignalRightEnabled = false;
    hazardLightsEnabled = false;
    headLightEnabled = true;
    brakeLightEnabled = true;
}
