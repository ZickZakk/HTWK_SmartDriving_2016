//
// Created by pbachmann on 3/11/16.
//

#ifndef HTWK_2016_CARCONFIGREADER_H
#define HTWK_2016_CARCONFIGREADER_H

#include "stdafx.h"

#include <Logger.h>

#define CAR_CONFIG_LOG_NAME "HTWK Carconfig"

class CarConfigReader
{
    private:
        Logger logger;
        map<string, map<string, void *> > configurations;

        tResult LoadCarConfig(string fileName);

        void AddElement(cDOMElement *element, map<string, void *> &config);

    public:
        CarConfigReader();

        CarConfigReader(string fileName);

        virtual ~CarConfigReader();

        template<typename Type>
        tBool Pull(string group, string key, Type &result)
        {
            if (1 != configurations.count(group))
            {
                logger.Log(cString::Format("No value for '%s'", group.c_str()).GetPtr());
                return false;
            }

            if (1 != configurations[group].count(key))
            {
                logger.Log(cString::Format("No value for '%s'", key.c_str()).GetPtr());
                return false;
            }

            Type *existingValue = static_cast<Type *>(configurations[group][key]);
            result = *existingValue;

            return true;
        }
};


#endif //HTWK_2016_CARCONFIGREADER_H
