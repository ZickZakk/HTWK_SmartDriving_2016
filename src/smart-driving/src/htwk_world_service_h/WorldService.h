#ifndef _WORLD_SERVICE_CLASS_HEADER_
#define _WORLD_SERVICE_CLASS_HEADER_

#include "stdafx.h"

#include <ucom/service.h>

#include "tWorldData.h"
#include <Logger.h>

#define SERVICE_NAME "HTWK World Service"
#define OID_WORLD_SERVICE "htwk.world_service"
#define IID_WORLD_INTERFACE "htwk.i_world_service"

#define CAR_CONFIG_PATH_PROPERTY "CarConfig"
#define HTWK_SETTINGS "HTWK"

// tBool
#define WORLD_IS_NO_PASSING_ACTIVE "isNoPassingActive"
// tInt
#define WORLD_CURRENT_ROAD_SIGN "currentRoadSign"
// tFloat32
#define WORLD_CURRENT_ROAD_SIGN_SIZE "currentRoadSignSize"
// Mat
#define WORLD_OBSTACLE_MAT "obstacleVideo"
// tLane
#define WORLD_LANE "lane"
// tManeuver
#define WORLD_CURRENT_MANEUVER "currentManeuver"
// tIMU
#define WORLD_IMU "imuStruct"
// tCarState
#define WORLD_CAR_STATE "carState"
// tFloat32
#define WORLD_DISTANCE_OVERALL "distanceOverall"
// tFloat32
#define WORLD_CAR_SPEED "currentSpeed"
// vector<tReadyModule::ReadyModuleEnum>
#define WORLD_READY_MODULES "readyModules"
// CarConfigReader
#define WORLD_CARCONFIG "carConfig"

class WorldService : public ucom::cService
{
    ADTF_SERVICE_BEGIN(OID_WORLD_SERVICE, SERVICE_NAME)
            ADTF_EXPORT_INTERFACE(IID_SERVICE, ucom::IService)
            ADTF_EXPORT_INTERFACE(IID_WORLD_INTERFACE, WorldService)
        ADTF_SERVICE_END()

    public:
        WorldService(const tChar *);

        virtual ~WorldService();

        tResult ServiceInit(IException **__exception_ptr = NULL);

        tResult ServiceShutdown(IException **__exception_ptr = NULL);

        tResult ServiceEvent(tInt eventId,
                             tUInt32 param1 = 0,
                             tUInt32 param2 = 0,
                             tVoid *pvData = NULL,
                             tInt szData = 0,
                             IException **__exception_ptr = NULL);

    private:
        Logger logger;
        std::map<string, tWorldData> map;

        cObjectPtr<INamespace> myNamespace;
        string settingsPath;

    public:
        void Clear()
        {
            map.clear();
        }

        template<typename Type>
        tResult Push(string key, Type value)
        {
            if (1 == map.count(key))
            {
                map[key].lock->LockWrite();

                Type *existingValue = static_cast<Type *>(map[key].data);
                free(existingValue);

                void *pVoid = malloc(sizeof(Type));
                map[key].data = new(pVoid) Type(value);

                map[key].lock->UnlockWrite();
            }
            else
            {
                tWorldData entry;
                entry.lock = new cReadWriteMutex();
                entry.lock->Create(100);

                void *pVoid = malloc(sizeof(Type));
                entry.data = new(pVoid) Type(value);

                map[key] = entry;
            }

            RETURN_NOERROR;
        }

        template<typename Type>
        tResult Pull(string key, Type &result)
        {
            if (1 != map.count(key))
            {
                logger.Log(cString::Format("No value for '%s'", key.c_str()).GetPtr());
                RETURN_ERROR(-1);
            }

            map[key].lock->LockRead();

            Type *existingValue = static_cast<Type *>(map[key].data);
            result = *existingValue;

            map[key].lock->UnlockRead();
            RETURN_NOERROR;
        }
};

#endif // _WORLD_SERVICE_CLASS_HEADER_
