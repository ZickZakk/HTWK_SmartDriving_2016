//
// Created by pbachmann on 2/17/16.
//

#include "Magic.h"

ADTF_FILTER_PLUGIN(FILTER_NAME, OID, Magic)

Magic::Magic(const tChar *__info) : BaseDecisionModule(__info, FILTER_NAME, DM_COLL_PREV)
{
    InitializeProperties();

    state = 0;
    distanceOverall = 0;
    distanceSinceLastAction = 0;
    sourceModule = DM_MAGIC;

    lastYaw = 0;
    yawGoal = 0;
}

Magic::~Magic()
{
}

void Magic::InitializeProperties()
{
    speed = 1.2;

    SetPropertyFloat(SPEED_FACTOR_PROPERTY, speedFactor);
    SetPropertyBool(SPEED_FACTOR_PROPERTY
    NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr(SPEED_FACTOR_PROPERTY
    NSSUBPROP_DESCRIPTION, "Speed");
    SetPropertyFloat(SPEED_FACTOR_PROPERTY
    NSSUBPROP_MIN, 0);
    SetPropertyFloat(SPEED_FACTOR_PROPERTY
    NSSUBPROP_MAX, 10);

}

tResult  Magic::CreateInputPins()
{
    RETURN_IF_FAILED(distancePin.Create("DistanceInput", typeSignalValue, static_cast<IPinEventSink *> (this)));
    RETURN_IF_FAILED(RegisterPin(&distancePin));

    RETURN_IF_FAILED(imuPin.Create("IMUInput", typeImu, static_cast<IPinEventSink *>(this)));
    RETURN_IF_FAILED(RegisterPin(&imuPin));

    RETURN_NOERROR;
}

tResult  Magic::Init(tInitStage eStage, IException **__exception_ptr)
{
    RETURN_IF_FAILED(BaseDecisionModule::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        RETURN_IF_FAILED(CreateInputPins());
    }
    else if (eStage == StageNormal)
    {
        speed = tFloat32(GetPropertyFloat(SPEED_FACTOR_PROPERTY));

        //logger.Log(cString::Format("Turn speed: %f", turnSpeed).GetPtr(), false);
    }
    else if (eStage == StageGraphReady)
    {

    }

    RETURN_NOERROR;
}

tResult  Magic::OnPinEvent(IPin *sourcePin, tInt eventCode, tInt param1, tInt param2,
                                         IMediaSample *mediaSample)
{
    if (eventCode != IPinEventSink::PE_MediaSampleReceived)
    {
        RETURN_NOERROR;
    }

    RETURN_IF_POINTER_NULL(mediaSample);

    logger.StartLog();

    if (sourcePin == &distancePin)
    {
        {
            lastDistanceOverall = distanceOverall;
            __adtf_sample_read_lock_mediadescription(descriptionSignalValue, mediaSample, inputCoder);
            inputCoder->Get("f32Value", (tVoid *) &distanceOverall);

            calcDistance();

            drive();
        }
    }
    else if (sourcePin == &imuPin)
    {
        {
            lastYaw = yaw;
            __adtf_sample_read_lock_mediadescription(descriptionImu, mediaSample, inputCoder);
            inputCoder->Get("tYaw", (tVoid *) &yaw);
        }

    }

    SendDriveInstructions();

    logger.EndLog();

    RETURN_NOERROR;
}

void Magic::drive()
{
    logger.StartLog();
    logger.Log(cString::Format("State: %d", state).GetPtr());
    logger.Log(cString::Format("DistanceSineLastAction: %f", distanceSinceLastAction).GetPtr());
    logger.Log(cString::Format("yaw: %f", yaw).GetPtr());
    logger.Log(cString::Format("yawGoal: %f", yawGoal).GetPtr());
    logger.EndLog();

    if(state == 0) //grade Start
    {
        drive(1.0, RIGHT_TURN_MODE);
        return;
    }

    if(state == 1) //1. Abbiegen
    {
        turn(RIGHT_TURN_MODE);
        return;
    }

    if (state == 2) // Grade Rechts hinter
    {
        drive(1.0, LEFT_TURN_MODE);
        return;
    }

    if (state == 3) // 2. Abbiegen
    {
        turn(LEFT_TURN_MODE);
        return;
    }

    if (state == 4) // Kurze Grade
    {
        drive(1.0, LEFT_TURN_MODE);
        return;
    }

    if (state == 5) // 3. Abbiegen
    {
        turn(LEFT_TURN_MODE);
        return;
    }

    if (state == 6) //Anfang Treppenstufe
    {
        drive(1.0, RIGHT_TURN_MODE);
        return;
    }

    if (state == 7) // 4. Abbiegen
    {
        turn(RIGHT_TURN_MODE);
        return;
    }

    if (state == 8) //Mitte Treppenstufe
    {
        drive(1.0, LEFT_TURN_MODE);
        return;
    }

    if (state == 9) // 5. Abbiegen
    {
        turn(LEFT_TURN_MODE);
        return;
    }

    if (state == 10) // Ende Treppenstufe
    {
        drive(1.0, RIGHT_TURN_MODE);
        return;
    }

    if (state == 11) // 6. Abbiegen
    {
        turn(RIGHT_TURN_MODE);
        return;
    }

    if (state == 12) // Ende
    {
        //then be happy
    }


}

tBool Magic::WaitForGyro()
{
    return fabsf(lastYaw - yaw) > 0.01;
}

tFloat32 Magic::CalculateExpectedYaw(tFloat32 yaw, tInt direction, tBool reverse, tFloat32 angle)
{
    // direction
    // 1 steer left
    // 2 steer right

    tInt modify = reverse ? -1 : 1;

    tFloat32 expectedYaw = 0;

    switch (direction)
    {
        case LEFT_TURN_MODE:
            expectedYaw = yaw + (modify * angle);
            break;
        case RIGHT_TURN_MODE:
            expectedYaw = yaw - (modify * angle);
            break;
        default:
            // ERROR
            expectedYaw = yaw;
    }

    if (expectedYaw > 180)
    {
        expectedYaw = -180 + (expectedYaw - 180);
    }

    if (expectedYaw < -180)
    {
        expectedYaw = 180 + (expectedYaw + 180);
    }

    return expectedYaw;
}

void Magic::turn(int turnMode)
{
    tFloat32 leftTurnRadius = 1.2;
    tFloat32 rightTurnRadius = 1;

    if (turnMode == LEFT_TURN_MODE && !turnFinished)
    {
        curveRadius = -leftTurnRadius;
        steeringControlTaken = true;
        speedControlTaken = true;
        turnSignalLeftEnabled = true;

        if (yaw > yawGoal)
        {
            if (WaitForGyro())
            {
                speed = 0;
                curveRadius = INT_MAX;

                SendDriveInstructions();
                return;
            }

            maneuverCompleted = tTrue;
            turnFinished = tTrue;
            SendDriveInstructions();
            return;
        }

        SendDriveInstructions();
        return;
    }

    if (turnMode == RIGHT_TURN_MODE && !turnFinished)
    {
        curveRadius = rightTurnRadius;
        steeringControlTaken = true;
        speedControlTaken = true;
        turnSignalRightEnabled = true;

        if (yaw < yawGoal)
        {
           if (WaitForGyro())
            {
                speed = 0;
                curveRadius = INT_MAX;

                SendDriveInstructions();
                return;
            }

            maneuverCompleted = tTrue;
            turnFinished = tTrue;
            SendDriveInstructions();
            return;
        }

        SendDriveInstructions();
        return;
    }

    if (turnFinished)
    {
        curveRadius = INT_MAX;
        turnSignalLeftEnabled = false;
        turnSignalRightEnabled = false;
        maneuverCompleted = false;
        SendDriveInstructions();
        state++;
        distanceSinceLastAction = 0;
        ResetDriveInstructions(DM_MAGIC);
        turnFinished = tFalse;
        return;
    }
}

void Magic::drive(tFloat distance, tInt32 modeNextTurn)
{
    speedControlTaken = tFalse;
    steeringControlTaken = tFalse;

    yawGoal = CalculateExpectedYaw(yaw, modeNextTurn, tFalse, 90);

    if (distanceSinceLastAction > distance)
    {
        state++;
        distanceSinceLastAction = 0;
        SendDriveInstructions();
        ResetDriveInstructions(DM_MAGIC);
    } else {
        SendDriveInstructions();
    }

    return;
}

void Magic::calcDistance()
{
    distanceSinceLastAction += distanceOverall - lastDistanceOverall;
}
