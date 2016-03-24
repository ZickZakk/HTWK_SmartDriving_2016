//
// Created by pbachmann on 12/11/15.
//

#include "MovementCalculator.h"

MovementCalculator::MovementCalculator() : logger ("MovementCalculator")
{
    accX = 0;
    accY = 0;
    measSpeed = 0;

    xUsed = true;
    yUsed = true;

    this->tx = 0;
    this->ty = 0;

    lastSpeed = 0;
    lastDistance = 0;
    kalmanSpeed = 0;

    lastSpeedsCounter =0;

    for(int i = 0; i < 5; i++)
    {
        lastSpeeds[i] = 0;
    }

    deltatx = 0;
    deltaty = 0;
    deltats = 0;

    counter = 0;

    {
        KF = new KalmanFilter(2,2,0);
        KF->measurementMatrix = (Mat_<float>(2,2) << 1, 0, 0, 1);
        KF->measurementNoiseCov = (Mat_<float>(2, 2) << 0.001, 0, 0, 0.5);
        setIdentity(KF->processNoiseCov, Scalar::all(1e-3));
        setIdentity(KF->errorCovPost, Scalar::all(2));
        setIdentity(KF->errorCovPre, Scalar::all(2));

        KF->statePre = Scalar::all(0);

        state = Mat::zeros(2, 1, CV_32F);

    }
}

MovementCalculator::~MovementCalculator()
{
    if(KF != NULL)
        delete KF;
}


bool MovementCalculator::newMovement()
{
    counter++;

    return !xUsed && !yUsed && (counter > 15);
}

void MovementCalculator::setX(float &accX, uint32_t &tx)
{
    if(tx != this->tx)
    {
        deltatx = getDeltaT(this->tx, tx);
        this->accX = accX;
        this->tx = tx;
        xUsed = false;
    }
}

void MovementCalculator::setY(float &accY, uint32_t &ty)
{
    if (ty != this->ty)
    {
        deltaty = getDeltaT(this->ty, ty);
        this->accY = accY;
        this->ty = ty;
        yUsed = false;
    }
}

float MovementCalculator::calculateDistance()
{

    lastDistance += 0.5 * kalmanSpeed * ((deltaty + deltats)/2);

    //logger.StartLog();

    stringstream output_distance;
    output_distance << lastDistance;
    //logger.Log("Distance: " + output_distance.str());

    //logger.EndLog();

    return lastDistance;
}

float MovementCalculator::getDeltaT(uint32_t t_old, uint32_t t_new)
{
    return (((float) t_new) - ((float) t_old)) / 1000000;
}

float MovementCalculator::calculateSpeedKalman()
{

    logger.StartLog();

    if ((measSpeed < 0.001) && (measSpeed > -0.001))
    {
        state.setTo(Scalar::all(0));
        KF->errorCovPost.setTo(Scalar::all(0));
    } else {
        Mat_<float> measurement(2, 1);
        measurement.setTo(Scalar(0));
        measurement = (Mat_<float>(2, 1) << measSpeed, accY);
        KF->transitionMatrix = (Mat_<float>(2, 2) << 1, ((deltats + deltaty) / 2), 0, 1);

        KF->predict();
        state = KF->correct(measurement);
    }

    stringstream meas_speed;
    stringstream meas_acc;
    stringstream convert_speed;
    stringstream convert_error;
    meas_speed << measSpeed;
    meas_acc << accY;
    convert_speed << state.at<float>(0);
    convert_error << KF->errorCovPost.at<float>(0);

    logger.Log("MeasSpeed: " + meas_speed.str());
    logger.Log("MeasAcc: " + meas_acc.str());
    logger.Log("calcSpeed: " + convert_speed.str());
    logger.Log("error: " + convert_error.str());
    logger.EndLog();

    kalmanSpeed = state.at<float>(0);
    return kalmanSpeed;
}


void MovementCalculator::setMeasSpeed(float &measSpeed, uint32_t &ts)
{
    //if (ts != this->ts)
    {
        deltats = getDeltaT(this->ts, ts);
        this->measSpeed = measSpeed;
        this->ts = ts;
    }
}
