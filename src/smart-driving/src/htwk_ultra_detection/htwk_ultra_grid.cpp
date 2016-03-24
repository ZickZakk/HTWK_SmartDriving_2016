/*
@author: ffreihube
*/
//#############################################################################
#include "htwk_ultra_grid.h"


//#############################################################################
cUltraGrid::cUltraGrid() : logger("Grid")
{
    obstacleGrid = new Mat(400, 400, CV_8UC1, Scalar::all(255));
    image = *obstacleGrid;

    for (int i = 0; i < 10; i++)
    {
        sensorMap[i] = 50;
    }

    for (int sc = 0; sc < 10; sc++)
    {

        switch (sc)
        {
            case ULTRASONIC_FRONT_LEFT:
                minAngFar[sc] = ULTRASONIC_FRONT_LEFT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_FRONT_LEFT_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_FRONT_LEFT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_FRONT_LEFT_MID_MAX;
                minAngNear[sc] = ULTRASONIC_FRONT_LEFT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_FRONT_LEFT_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_FRONT_LEFT_POS_X;
                senPosY[sc] = ULTRASONIC_FRONT_LEFT_POS_Y;
                break;
            case ULTRASONIC_FRONT_CENTER_LEFT:
                minAngFar[sc] = ULTRASONIC_FRONT_CENTER_LEFT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_FRONT_CENTER_LEFT_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_FRONT_CENTER_LEFT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_FRONT_CENTER_LEFT_MID_MAX;
                minAngNear[sc] = ULTRASONIC_FRONT_CENTER_LEFT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_FRONT_CENTER_LEFT_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_FRONT_CENTER_LEFT_POS_X;
                senPosY[sc] = ULTRASONIC_FRONT_CENTER_LEFT_POS_Y;
                break;
            case ULTRASONIC_FRONT_CENTER:
                minAngFar[sc] = ULTRASONIC_FRONT_CENTER_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_FRONT_CENTER_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_FRONT_CENTER_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_FRONT_CENTER_MID_MAX;
                minAngNear[sc] = ULTRASONIC_FRONT_CENTER_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_FRONT_CENTER_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_FRONT_CENTER_POS_X;
                senPosY[sc] = ULTRASONIC_FRONT_CENTER_POS_Y;
                break;
            case ULTRASONIC_FRONT_CENTER_RIGHT:
                minAngFar[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_MID_MAX;
                minAngNear[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_POS_X;
                senPosY[sc] = ULTRASONIC_FRONT_CENTER_RIGHT_POS_Y;
                break;
            case ULTRASONIC_FRONT_RIGHT:
                minAngFar[sc] = ULTRASONIC_FRONT_RIGHT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_FRONT_RIGHT_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_FRONT_RIGHT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_FRONT_RIGHT_MID_MAX;
                minAngNear[sc] = ULTRASONIC_FRONT_RIGHT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_FRONT_RIGHT_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_FRONT_RIGHT_POS_X;
                senPosY[sc] = ULTRASONIC_FRONT_RIGHT_POS_Y;
                break;
            case ULTRASONIC_SIDE_RIGHT:
                minAngFar[sc] = ULTRASONIC_SIDE_RIGHT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_SIDE_RIGHT_FAR_MAX + 360;
                minAngMid[sc] = ULTRASONIC_SIDE_RIGHT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_SIDE_RIGHT_MID_MAX + 360;
                minAngNear[sc] = ULTRASONIC_SIDE_RIGHT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_SIDE_RIGHT_NEAR_MAX + 360;
                senPosX[sc] = ULTRASONIC_SIDE_RIGHT_POS_X;
                senPosY[sc] = ULTRASONIC_SIDE_RIGHT_POS_Y;
                break;
            case ULTRASONIC_SIDE_LEFT:
                minAngFar[sc] = ULTRASONIC_SIDE_LEFT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_SIDE_LEFT_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_SIDE_LEFT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_SIDE_LEFT_MID_MAX;
                minAngNear[sc] = ULTRASONIC_SIDE_LEFT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_SIDE_LEFT_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_SIDE_LEFT_POS_X;
                senPosY[sc] = ULTRASONIC_SIDE_LEFT_POS_Y;
                break;
            case ULTRASONIC_REAR_LEFT:
                minAngFar[sc] = ULTRASONIC_REAR_LEFT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_REAR_LEFT_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_REAR_LEFT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_REAR_LEFT_MID_MAX;
                minAngNear[sc] = ULTRASONIC_REAR_LEFT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_REAR_LEFT_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_REAR_LEFT_POS_X;
                senPosY[sc] = ULTRASONIC_REAR_LEFT_POS_Y;
                break;
            case ULTRASONIC_REAR_CENTER:
                minAngFar[sc] = ULTRASONIC_REAR_CENTER_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_REAR_CENTER_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_REAR_CENTER_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_REAR_CENTER_MID_MAX;
                minAngNear[sc] = ULTRASONIC_REAR_CENTER_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_REAR_CENTER_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_REAR_CENTER_POS_X;
                senPosY[sc] = ULTRASONIC_REAR_CENTER_POS_Y;
                break;
            case ULTRASONIC_REAR_RIGHT:
                minAngFar[sc] = ULTRASONIC_REAR_RIGHT_FAR_MIN;
                maxAngFar[sc] = ULTRASONIC_REAR_RIGHT_FAR_MAX;
                minAngMid[sc] = ULTRASONIC_REAR_RIGHT_MID_MIN;
                maxAngMid[sc] = ULTRASONIC_REAR_RIGHT_MID_MAX;
                minAngNear[sc] = ULTRASONIC_REAR_RIGHT_NEAR_MIN;
                maxAngNear[sc] = ULTRASONIC_REAR_RIGHT_NEAR_MAX;
                senPosX[sc] = ULTRASONIC_REAR_RIGHT_POS_X;
                senPosY[sc] = ULTRASONIC_REAR_RIGHT_POS_Y;
                break;
            default:
                break;
        }
    }


}

//#############################################################################
cUltraGrid::~cUltraGrid()
{

    delete obstacleGrid;
    obstacleGrid = NULL;

}

bool cUltraGrid::write2Grid(float &value, int sensorCode)
{
    if (value != 0)
    {
        value = value * 100;

        switch (sensorCode)
        {
            case ULTRASONIC_FRONT_LEFT:
                sensorMap[ULTRASONIC_FRONT_LEFT] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_FRONT_CENTER_LEFT:
                sensorMap[ULTRASONIC_FRONT_CENTER_LEFT] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_FRONT_CENTER:
                sensorMap[ULTRASONIC_FRONT_CENTER] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_FRONT_CENTER_RIGHT:
                sensorMap[ULTRASONIC_FRONT_CENTER_RIGHT] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_FRONT_RIGHT:
                sensorMap[ULTRASONIC_FRONT_RIGHT] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_SIDE_RIGHT:
                sensorMap[ULTRASONIC_SIDE_RIGHT] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_SIDE_LEFT:
                sensorMap[ULTRASONIC_SIDE_LEFT] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_REAR_LEFT:
                sensorMap[ULTRASONIC_REAR_LEFT] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_REAR_CENTER:
                sensorMap[ULTRASONIC_REAR_CENTER] = (uint8_t) int(value / 2.5);
                break;
            case ULTRASONIC_REAR_RIGHT:
                sensorMap[ULTRASONIC_REAR_RIGHT] = (uint8_t) int(value / 2.5);
                break;
            default:
                break;
        }
    }

    return true;
}

Mat cUltraGrid::printGrid()
{

    delete obstacleGrid;
    obstacleGrid = NULL;

    obstacleGrid = new Mat(400, 400, CV_8UC1, Scalar::all(0));
    image = *obstacleGrid;
/*
    for (int sc = 0; sc < 10; sc++)
    {
        printSensor(minAngFar[sc] + 3, maxAngFar[sc] - 3, minAngMid[sc], maxAngMid[sc], minAngNear[sc], maxAngNear[sc], senPosX[sc], senPosY[sc], Scalar(255),
                    POL_MAP_MAX);
    }*/

    for (int sc = 9; sc >= 0; sc--)
    {
        printSensor(minAngFar[sc] - 2, maxAngFar[sc] + 2, minAngMid[sc], maxAngMid[sc], minAngNear[sc], maxAngNear[sc], senPosX[sc], senPosY[sc],
                    Scalar(0),
                    sensorMap[sc]);
    }

    printCar(Scalar(0));

    return image;
}

void cUltraGrid::printCar(Scalar color)
{
    Point rook_points_car[1][4];

    rook_points_car[0][0] = Point(185, 200 - 10); // + 10 & + 5  for irrelevant blind point right
    rook_points_car[0][1] = Point(215 + 5, 200 - 10); // in front or beneath of the car
    rook_points_car[0][3] = Point(185, 260);
    rook_points_car[0][2] = Point(215 + 5, 260);

    const Point *ppt_car[1] = {rook_points_car[0]};
    int npt_car[] = {4};

    fillPoly(image, ppt_car, npt_car, 1, color, 4);
}

void cUltraGrid::printSensor(int minAngFar, int maxAngFar, int minAngMid, int maxAngMid, int minAngNear, int maxAngNear, int senPosX, int senPosY,
                             Scalar color, uint8_t sensorValue)
{

    printWhiteRoom(minAngFar, maxAngFar, senPosX, senPosY, sensorValue);

    int points = int(ceil(maxAngFar / 2) - trunc(minAngFar / 2));

    Point rook_points_black[1][points + 5];
    int counterWhite = 0;

    for (int i = int(trunc(minAngFar / 2)); i < int(ceil(maxAngFar / 2)); i++)
        {
            rook_points_black[0][counterWhite] = Point(
                    int(sensorValue * 2.5 * cos(i * 2 * (PI / 180)) + 200 + senPosX),
                    int(sensorValue * 2.5 * sin(i * 2 * (PI / 180)) + 200 + senPosY));

            counterWhite++;
        }

    if ((sensorValue * 2.5) >= 100)
        {
            rook_points_black[0][points + 2] = getZeroSensorPoint(senPosX, senPosY); //Zero
            rook_points_black[0][points + 4] = getValuePoint(100, minAngMid, senPosX, senPosY);
            rook_points_black[0][points] = getValuePoint(100, maxAngMid, senPosX, senPosY);
            rook_points_black[0][points + 1] = getValuePoint(50, maxAngNear, senPosX, senPosY);
            rook_points_black[0][points + 3] = getValuePoint(50, minAngNear, senPosX, senPosY);
        }

    if ((sensorValue * 2.5) >= 50)
        {
            rook_points_black[0][points + 2] = getZeroSensorPoint(senPosX, senPosY);
            rook_points_black[0][points + 4] = getValuePoint(50, minAngNear, senPosX, senPosY);
            rook_points_black[0][points] = getValuePoint(50, maxAngNear, senPosX, senPosY);
            rook_points_black[0][points + 1] = getValuePoint(50, maxAngNear, senPosX, senPosY);
            rook_points_black[0][points + 3] = getValuePoint(50, minAngNear, senPosX, senPosY);
        }

    if ((sensorValue * 2.5) < 50)
        {
            rook_points_black[0][points + 2] = getZeroSensorPoint(senPosX, senPosY);
            rook_points_black[0][points + 4] = getZeroSensorPoint(senPosX, senPosY);
            rook_points_black[0][points] = getZeroSensorPoint(senPosX, senPosY);
            rook_points_black[0][points + 1] = getZeroSensorPoint(senPosX, senPosY);
            rook_points_black[0][points + 3] = getZeroSensorPoint(senPosX, senPosY);
        }

    const Point *ppt2[1] = {rook_points_black[0]};

    int npt2[] = {points + 5};

    fillPoly(image, ppt2, npt2, 1, color, 4);
}

Point2i cUltraGrid::getZeroSensorPoint(int senPosX, int senPosY) const
{ return Point(200 + senPosX, 200 + senPosY); }

void cUltraGrid::printWhiteRoom(int minAngFar, int maxAngFar, int senPosX, int senPosY, uint8_t sensorValue)
{
    Point rook_points_white[1][4];
    rook_points_white[0][0] = getMaxPoint(minAngFar, senPosX, senPosY);
    rook_points_white[0][1] = getMaxPoint(maxAngFar, senPosX, senPosY);
    rook_points_white[0][3] = getValuePoint(int(sensorValue * 2.5), minAngFar, senPosX, senPosY);
    rook_points_white[0][2] = getValuePoint(int(sensorValue * 2.5), maxAngFar, senPosX, senPosY);

    const Point *pptWhite[1] = {rook_points_white[0]};
    int nptWhite[] = {4};

    fillPoly(image, pptWhite, nptWhite, 1, Scalar(255), 4);
}

Mat cUltraGrid::printMax(int sensorCode)
{
    sensorMap[sensorCode] = POL_MAP_MAX;
    return image;
}

Point cUltraGrid::getMaxPoint(int ang, int sensorX, int sensorY)
{
    return Point(int(POL_MAP_MAX * 2.5 * cos(ang * (PI / 180)) + 200 + sensorX),
                 int(POL_MAP_MAX * 2.5 * sin(ang * (PI / 180)) + 200 + sensorY));
}

Point cUltraGrid::getValuePoint(int value, int ang, int sensorX, int sensorY)
{
    return Point(int(value * cos(trunc(ang) * (PI / 180)) + 200 + sensorX),
                 int(value * sin(trunc(ang) * (PI / 180)) + 200 + sensorY));
}







