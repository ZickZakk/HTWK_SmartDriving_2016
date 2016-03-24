//
// Created by pbachmann on 2/20/16.
//

#ifndef HTWK_2016_WORLDMODELL_H
#define HTWK_2016_WORLDMODELL_H

#include <tCarState.h>
#include <tManeuver.h>
#include <tIMU.h>
#include <tLane.h>
#include <tRoadSign.h>

typedef struct
{
    tCarState::CarStateEnum CarState;
    tManeuver::ManeuverEnum Maneuver;
    tIMU Imu;
    Mat ObstacleMap;
    tFloat32 DistanceOverall;
    tFloat32 Speed;
    tBool IsNoPassing;
    tRoadSign::RoadSignEnum RoadSign;
    tFloat32 RoadSignSize;
    tLane Lane;
    tFloat32 CurrentSpeed;
    tFloat32 Interval;
    vector<tReadyModule::ReadyModuleEnum> ReadyModules;
} tWorldModel;

#endif //HTWK_2016_WORLDMODELL_H
