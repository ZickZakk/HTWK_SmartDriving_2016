//
// Created by pbachmann on 3/4/16.
//

#ifndef HTWK_2016_READYMODULE_H
#define HTWK_2016_READYMODULE_H

#include <iostream>

struct tReadyModule
{
    enum ReadyModuleEnum
    {
        Nothing,
        Ipm,
        LaneDetection,
        MarkerDetection,
        MarkerEvaluator,
        Maneuver
    };

    static std::string ToString(const tReadyModule::ReadyModuleEnum &value)
    {
        switch (value)
        {
            case Nothing:
                return "Nothing";
            case Ipm:
                return "Ipm";
            case LaneDetection:
                return "LaneDetection";
            case MarkerDetection:
                return "MarkerDetection";
            case MarkerEvaluator:
                return "MarkerEvaluator";
            case Maneuver:
                return "Maneuver";
            default:
                return "Unknown Value";
        }
    }
};

#endif //HTWK_2016_READYMODULE_H
