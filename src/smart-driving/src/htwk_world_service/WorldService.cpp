#include <WorldService.h>
#include <CarConfigReader.h>

ADTF_SERVICE_PLUGIN("World Service", OID_WORLD_SERVICE, WorldService)

WorldService::WorldService(const tChar *__info) : cService(__info), logger(SERVICE_NAME)
{
    settingsPath = NS_SETTINGS "/" HTWK_SETTINGS;
}

WorldService::~WorldService()
{
}

tResult WorldService::ServiceInit(IException **__exception_ptr)
{
    RETURN_IF_FAILED(cService::ServiceInit(__exception_ptr));

    Clear();

    if (IS_FAILED(_runtime->GetObject(OID_ADTF_NAMESPACE, IID_ADTF_NAMESPACE, (tVoid **) &myNamespace)))
    {
        THROW_ERROR_DESC(ERR_NOT_FOUND,
                         cString::Format("Demo UI Service: Can only load this service, when \"%s\" is not present!", OID_ADTF_NAMESPACE));
    }

    logger.Log(cString::Format("Try getting settings handle %s", settingsPath.c_str()).GetPtr(), false);
    tHandle hSettings = myNamespace->GetObject(settingsPath.c_str());
    if (!hSettings)
    {
        logger.Log(cString::Format("Creating settings handle %s", settingsPath.c_str()).GetPtr(), false);
        if (IS_FAILED(myNamespace->CreateObject(settingsPath.c_str(), &hSettings)))
        {
            hSettings = 0;
        }
        else
        {
            logger.Log(cString::Format("Creating property %s", CAR_CONFIG_PATH_PROPERTY).GetPtr(), false);
            myNamespace->SetPropertyStr(hSettings, NSPROP_NAMESPACE_TYPE, NSTYPE_FOLDER);
            myNamespace->SetPropertyStr(hSettings, CAR_CONFIG_PATH_PROPERTY, "/home");
        }
    }

    if (hSettings != 0)
    {
        logger.Log(cString::Format("Adding filename property to %s", CAR_CONFIG_PATH_PROPERTY).GetPtr(), false);
        myNamespace->SetPropertyBool(hSettings, CAR_CONFIG_PATH_PROPERTY NSSUBPROP_FILENAME, tTrue);
    }

    RETURN_NOERROR;
}

tResult WorldService::ServiceShutdown(IException **__exception_ptr)
{
    Clear();

    return cService::ServiceShutdown(__exception_ptr);
}

tResult WorldService::ServiceEvent(tInt eventId, tUInt32 param1, tUInt32 param2, tVoid *pvData, tInt szData, IException **__exception_ptr)
{
    logger.Log(cString::Format("Run level change %d", param1).GetPtr(), false);

    if (SE_ChangeRunLevel == eventId && _runtime->RL_Application == param1)
    {
        tHandle hSettings = myNamespace->GetObject(settingsPath.c_str());
        if (hSettings)
        {
            string carConfigPath = myNamespace->GetPropertyStr(hSettings, CAR_CONFIG_PATH_PROPERTY);
            logger.Log(cString::Format("Car config is %s", carConfigPath.c_str()).GetPtr(), false);

            CarConfigReader reader = CarConfigReader(carConfigPath);
            Push(WORLD_CARCONFIG, reader);
        }
    }

    return cService::ServiceEvent(eventId, param1, param2, pvData, szData, __exception_ptr);
}

