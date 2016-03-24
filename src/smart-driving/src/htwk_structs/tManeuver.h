#ifndef _MANEUVER_ENUM
#define _MANEUVER_ENUM

#include <iostream>

struct tManeuver
{
    enum ManeuverEnum
    {
        M_CROSSING_STRAIGHT = 100,
        M_CROSSING_LEFT = 101,
        M_CROSSING_RIGHT = 102,
        M_PARK_CROSS = 103,
        M_PARK_PARALLEL = 104,
        M_PULL_OUT_LEFT = 105,
        M_PULL_OUT_RIGHT = 106,
        M_UNKNOWN = -100
    };

    static std::string ToString(const tManeuver::ManeuverEnum &value)
    {
        switch (value)
        {
            case tManeuver::M_CROSSING_STRAIGHT:
                return "M_CROSSING_STRAIGHT";
            case tManeuver::M_CROSSING_LEFT:
                return "M_CROSSING_LEFT";
            case tManeuver::M_CROSSING_RIGHT:
                return "M_CROSSING_RIGHT";
            case tManeuver::M_PARK_CROSS:
                return "M_PARK_CROSS";
            case tManeuver::M_PARK_PARALLEL:
                return "M_PARK_PARALLEL";
            case tManeuver::M_PULL_OUT_LEFT:
                return "M_PULL_OUT_LEFT";
            case tManeuver::M_PULL_OUT_RIGHT:
                return "M_PULL_OUT_RIGHT";
            case tManeuver::M_UNKNOWN:
                return "M_UNKNOWN";
            default:
                return "Unknown Value";
        }
    }
};

#endif
