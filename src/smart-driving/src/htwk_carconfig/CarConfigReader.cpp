//
// Created by mwinkler on 3/9/16.
//

#include "CarConfigReader.h"

CarConfigReader::CarConfigReader() : logger(CAR_CONFIG_LOG_NAME)
{
}

CarConfigReader::CarConfigReader(string fileName) : logger(CAR_CONFIG_LOG_NAME)
{
    LoadCarConfig(fileName);
}

CarConfigReader::~CarConfigReader()
{
}

tResult CarConfigReader::LoadCarConfig(string fileName)
{
    // Get path of configuration file
    cFilename configFile;
    configFile = fileName.c_str();

    // check if given property is not empty
    if (configFile.IsEmpty())
    {
        logger.Log(cString::Format("File '%s' not found", fileName.c_str()).GetPtr());
        RETURN_ERROR(ERR_INVALID_FILE);
    }

    //create path from path
    //ADTF_GET_CONFIG_FILENAME(configFile);
    //configFile = configFile.CreateAbsolutePath(".");

    //Load file, parse configuration
    if (cFileSystem::Exists(configFile))
    {
        cDOM oDOM;
        oDOM.Load(configFile);

        cDOMElementList groups = oDOM.GetRoot().GetChildren();

        for (cDOMElementList::iterator group = groups.begin(); group != groups.end(); ++group)
        {
            string groupName = group->GetName().GetPtr();

            logger.Log(cString::Format("Processing %s", groupName.c_str()).GetPtr());

            configurations[groupName] = map<string, void *>();

            cDOMElementList values = group->GetChildren();
            for (cDOMElementList::iterator secIt = values.begin(); secIt != values.end(); ++secIt)
            {
                cDOMElement pConfigElement = *secIt;
                AddElement(&pConfigElement, configurations[groupName]);
            }
        }
    }
    else
    {
        logger.Log("File not found or could not be read");
        RETURN_ERROR(ERR_INVALID_FILE);
    }

    RETURN_NOERROR;
}

void CarConfigReader::AddElement(cDOMElement *element, map<string, void *> &config)
{
    string key = element->GetName().GetPtr();

    if (1 != element->GetAttributes().count("type"))
    {
        logger.Log(cString::Format("Type attribute missing for %s", key.c_str()).GetPtr());
        return;
    }

    string type = element->GetAttributes()["type"].GetPtr();

    if (type == "int")
    {
        tInt value = element->GetData().AsInt();
        logger.Log(cString::Format("%s: %d", key.c_str(), value).GetPtr());

        void *pVoid = malloc(sizeof(tInt));
        config[key] = new(pVoid) int(value);
    }
    else if (type == "float")
    {
        tFloat32 value = tFloat32(element->GetData().AsFloat64());
        logger.Log(cString::Format("%s: %f", key.c_str(), value).GetPtr());

        void *pVoid = malloc(sizeof(tFloat32));
        config[key] = new(pVoid) tFloat32(value);
    }
    else if (type == "bool")
    {
        tBool value = tBool(element->GetData().AsBool());
        logger.Log(cString::Format("%s: %d", key.c_str(), value).GetPtr());

        void *pVoid = malloc(sizeof(tBool));
        config[key] = new(pVoid) tBool(value);
    }
}
