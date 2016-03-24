//
// Created by mwinkler on 2/16/16.
//

#ifndef HTWK_2016_COLLISIONPREVENTER_H
#define HTWK_2016_COLLISIONPREVENTER_H

#include "stdafx.h"

#include <MapChecker.h>
#include <BaseDecisionModule.h>
#include <DriveModule.h>
#include <WorldModel.h>
#include <VisionUtils.h>
#include <CarConfigReader.h>

#include <math.h>
#include <vector>

#define OID "htwk.collision.prevention"
#define FILTER_NAME "HTWK Collision Prevention"

#define COLL_PREV_CONFIG_GROUP "collisionprevention"

#define SPEED_FACTOR_PROPERTY "speedFactor"
#define FOLLOW_ZONE_PROPERTY "followZone"
#define FOLLOW_ENABLE_PROPERTY "followEnable"

using namespace cv;

class CollisionPrevention : public BaseDecisionModule
{
    ADTF_FILTER(OID, FILTER_NAME, OBJCAT_DataFilter);

    public:
        CollisionPrevention(const tChar *__info);

        virtual ~CollisionPrevention();

        tResult Init(tInitStage eStage, __exception = NULL);

        tResult Start(__exception = NULL);

    private:
        void InitializeProperties();

        void CheckMapForObstacle(tWorldModel worldModel);

        tBool CheckObstacleMapFrontFree(const Mat &map);

        void CheckMapForMovingObstacle(tWorldModel worldModel);

        tFloat32 CompareDistances(tFloat32 currentSpeed);

        void FollowReset(bool onlyValues);

        tResult OnTrigger(tFloat32 interval);

        // properties
        tFloat32 speedFactor;
        tInt followZoneLength;
        bool followEnable;

        // current values
        tBool didStopEmergency;
        tInt frameBuffer;
        tFloat32 followCalcTimeInterval;

        std::vector<tInt> distances;
        tInt distanceBuffer;

        // values
        tInt emergencyZoneLength;
};


#endif //HTWK_2016_COLLISIONPREVENTER_H
