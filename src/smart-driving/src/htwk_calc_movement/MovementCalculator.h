
#ifndef HTWK_2016_MOVEMENTCALCULATOR_H
#define HTWK_2016_MOVEMENTCALCULATOR_H

#include <math.h>
#include "../htwk_logger/Logger.h"
#include <string.h>
#include "opencv2/opencv.hpp"
#include <sstream>
#include <stdint.h>


using namespace std;
using namespace cv;

class MovementCalculator
{
    private:
        bool xUsed;
        bool yUsed;

        float lastSpeed;
        float lastDistance;

        Logger logger;

        int counter;

        KalmanFilter *KF;


    public:
        MovementCalculator();

        ~MovementCalculator();

        float calculateDistance();

        bool newMovement();

        void setX(float &accX, uint32_t &tx);
        void setY(float &accY, uint32_t &ty);
        void setMeasSpeed(float &measSpeed, uint32_t &tx);


        float accX;
        float accY;
        float measSpeed;
        float kalmanSpeed;
        Mat state;

        uint32_t tx;
        uint32_t ty;
        uint32_t ts;

        float deltatx;
        float deltaty;
        float deltats;

        float lastSpeeds[5];
        int lastSpeedsCounter;

        float getDeltaT(uint32_t t_old, uint32_t t_new);

        float calculateSpeedKalman();
};


#endif //HTWK_2016_MOVEMENT_H
