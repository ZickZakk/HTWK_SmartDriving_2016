//
// Created by pbachmann on 3/4/16.
//

#ifndef HTWK_2016_TCARSTATE_H
#define HTWK_2016_TCARSTATE_H

#include <iostream>

struct tCarState
{
    enum CarStateEnum
    {
        Startup = 0,
        GetReady = 1,
        Ready = 2,
        Running = 3,
        Complete = 4,
        Error = 5,
    };

    static std::string ToString(const tCarState::CarStateEnum &value)
    {
        switch (value)
        {
            case tCarState::Startup:
                return "Startup";
            case tCarState::GetReady:
                return "GetReady";
            case tCarState::Ready:
                return "Ready";
            case tCarState::Running:
                return "Running";
            case tCarState::Complete:
                return "Complete";
            case tCarState::Error:
                return "Error";
            default:
                return "Unknown Value";
        }
    }
};
#endif //HTWK_2016_TCARSTATE_H
