//
// Created by pbachmann on 11/19/15.
//

#ifndef HTWK_2016_HTWK_ULTRA_GRID_H
#define HTWK_2016_HTWK_ULTRA_GRID_H

#include <stddef.h>
#include <math.h>

#include "opencv2/opencv.hpp"

#include "../htwk_logger/Logger.h"
#include "../htwk_structs/ObstacleGrid.h"

using namespace cv;

#define ULTRASONIC_FRONT_LEFT 0
#define ULTRASONIC_FRONT_CENTER_LEFT 1
#define ULTRASONIC_FRONT_CENTER 2
#define ULTRASONIC_FRONT_CENTER_RIGHT 3
#define ULTRASONIC_FRONT_RIGHT 4
#define ULTRASONIC_SIDE_RIGHT 5
#define ULTRASONIC_SIDE_LEFT 6
#define ULTRASONIC_REAR_LEFT 7
#define ULTRASONIC_REAR_CENTER 8
#define ULTRASONIC_REAR_RIGHT 9

#define ULTRASONIC_FRONT_CENTER_FAR_MIN 255
#define ULTRASONIC_FRONT_CENTER_FAR_MAX 300
#define ULTRASONIC_FRONT_CENTER_MID_MIN 212
#define ULTRASONIC_FRONT_CENTER_MID_MAX 312
#define ULTRASONIC_FRONT_CENTER_NEAR_MIN 208
#define ULTRASONIC_FRONT_CENTER_NEAR_MAX 305

#define ULTRASONIC_FRONT_CENTER_LEFT_FAR_MIN 225
#define ULTRASONIC_FRONT_CENTER_LEFT_FAR_MAX 250
#define ULTRASONIC_FRONT_CENTER_LEFT_MID_MIN 213
#define ULTRASONIC_FRONT_CENTER_LEFT_MID_MAX 250
#define ULTRASONIC_FRONT_CENTER_LEFT_NEAR_MIN 190
#define ULTRASONIC_FRONT_CENTER_LEFT_NEAR_MAX 255

#define ULTRASONIC_FRONT_CENTER_RIGHT_FAR_MIN 300
#define ULTRASONIC_FRONT_CENTER_RIGHT_FAR_MAX 320
#define ULTRASONIC_FRONT_CENTER_RIGHT_MID_MIN 285
#define ULTRASONIC_FRONT_CENTER_RIGHT_MID_MAX 328
#define ULTRASONIC_FRONT_CENTER_RIGHT_NEAR_MIN 270
#define ULTRASONIC_FRONT_CENTER_RIGHT_NEAR_MAX 330

#define ULTRASONIC_FRONT_RIGHT_FAR_MIN 320
#define ULTRASONIC_FRONT_RIGHT_FAR_MAX 360
#define ULTRASONIC_FRONT_RIGHT_MID_MIN 320
#define ULTRASONIC_FRONT_RIGHT_MID_MAX 360
#define ULTRASONIC_FRONT_RIGHT_NEAR_MIN 300
#define ULTRASONIC_FRONT_RIGHT_NEAR_MAX 360

#define ULTRASONIC_FRONT_LEFT_FAR_MIN 185
#define ULTRASONIC_FRONT_LEFT_FAR_MAX 215
#define ULTRASONIC_FRONT_LEFT_MID_MIN 175
#define ULTRASONIC_FRONT_LEFT_MID_MAX 215
#define ULTRASONIC_FRONT_LEFT_NEAR_MIN 175
#define ULTRASONIC_FRONT_LEFT_NEAR_MAX 225

#define ULTRASONIC_REAR_CENTER_FAR_MIN 80
#define ULTRASONIC_REAR_CENTER_FAR_MAX 120
#define ULTRASONIC_REAR_CENTER_MID_MIN 70
#define ULTRASONIC_REAR_CENTER_MID_MAX 105
#define ULTRASONIC_REAR_CENTER_NEAR_MIN 55
#define ULTRASONIC_REAR_CENTER_NEAR_MAX 120

#define ULTRASONIC_REAR_LEFT_FAR_MIN 135
#define ULTRASONIC_REAR_LEFT_FAR_MAX 180
#define ULTRASONIC_REAR_LEFT_MID_MIN 135
#define ULTRASONIC_REAR_LEFT_MID_MAX 180
#define ULTRASONIC_REAR_LEFT_NEAR_MIN 135
#define ULTRASONIC_REAR_LEFT_NEAR_MAX 180

#define ULTRASONIC_REAR_RIGHT_FAR_MIN 0
#define ULTRASONIC_REAR_RIGHT_FAR_MAX 35
#define ULTRASONIC_REAR_RIGHT_MID_MIN 0
#define ULTRASONIC_REAR_RIGHT_MID_MAX 25
#define ULTRASONIC_REAR_RIGHT_NEAR_MIN 0
#define ULTRASONIC_REAR_RIGHT_NEAR_MAX 60

#define ULTRASONIC_SIDE_LEFT_FAR_MIN 130
#define ULTRASONIC_SIDE_LEFT_FAR_MAX 185
#define ULTRASONIC_SIDE_LEFT_MID_MIN 130
#define ULTRASONIC_SIDE_LEFT_MID_MAX 185
#define ULTRASONIC_SIDE_LEFT_NEAR_MIN 130
#define ULTRASONIC_SIDE_LEFT_NEAR_MAX 185

#define ULTRASONIC_SIDE_RIGHT_FAR_MIN 310
#define ULTRASONIC_SIDE_RIGHT_FAR_MAX 5
#define ULTRASONIC_SIDE_RIGHT_MID_MIN 310
#define ULTRASONIC_SIDE_RIGHT_MID_MAX 5
#define ULTRASONIC_SIDE_RIGHT_NEAR_MIN 310
#define ULTRASONIC_SIDE_RIGHT_NEAR_MAX 5

#define PI 3.14159265
#define GREY_CLEAR_OFFSET 0

#define POL_MAP_MAX 125

//#############################################################################
class cUltraGrid
{
    private:
        Mat *obstacleGrid;
        Mat image;
        uint8_t sensorMap[10];
        Logger logger;

        int minAngFar[10];
        int maxAngFar[10];
        int minAngMid[10];
        int maxAngMid[10];
        int minAngNear[10];
        int maxAngNear[10];
        int senPosX[10];
        int senPosY[10];

    public:
        /**
         * @brief Constructor.
         */
        cUltraGrid(void);

        /**
         * @brief Destructor.
         */
        ~cUltraGrid(void);

        bool write2Grid(float &value, int sensorCode);

        Mat printGrid();

        Mat printMax(int sensorCode);

        void printSensor(int minAngFar, int maxAngFar, int minAngMid, int maxAngMid, int minAngNear, int maxAngNear, int senPosX, int senPosY,
                         Scalar color, uint8_t sensorValue);

        void printCar(Scalar color);

        Point getMaxPoint(int ang, int sensorX, int sensorY);

        Point getValuePoint(int value, int ang, int sensorX, int sensorY);

        void printWhiteRoom(int minAngFar, int maxAngFar, int senPosX, int senPosY, uint8_t sensorValue);

        Point2i getZeroSensorPoint(int senPosX, int senPosY) const;
};

#endif //HTWK_2016_HTWK_ULTRA_GRID_H
