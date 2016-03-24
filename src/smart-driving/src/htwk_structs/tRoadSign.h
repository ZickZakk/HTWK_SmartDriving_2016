/**
Copyright (c)
Audi Autonomous Driving Cup. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: �This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.�
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS �AS IS� AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


**********************************************************************
* $Author:: spiesra $  $Date:: 2015-05-13 08:29:07#$ $Rev:: 35003   $
**********************************************************************/

#ifndef HTWK_ROAD_SIGN_H
#define HTWK_ROAD_SIGN_H

#define MARKER_ID_UNMARKEDINTERSECTION 0
#define MARKER_ID_STOPANDGIVEWAY 1
#define MARKER_ID_PARKINGAREA 2
#define MARKER_ID_HAVEWAY 3
#define MARKER_ID_AHEADONLY 4
#define MARKER_ID_GIVEWAY 5
#define MARKER_ID_PEDESTRIANCROSSING 6
#define MARKER_ID_ROUNDABOUT 7
#define MARKER_ID_NOOVERTAKING 8
#define MARKER_ID_NOENTRYVEHICULARTRAFFIC 9
#define MARKER_ID_ONEWAYSTREET 11
#define MARKER_ID_NOMATCH 99

struct tRoadSign
{
    enum RoadSignEnum
    {
        NO_MATCH,
        GIVE_WAY, // Vorfahrt gewaehren
        HAVE_WAY, // Vorfahrt
        STOP_AND_GIVE_WAY, // Stop
        PARKING_AREA,
        AHEAD_ONLY, // nur gerade aus
        UNMARKED_INTERSECTION, // rechts for links?
        PEDASTRIAN_CROSSING, // Fusgaenger ueberweg
        ROUNDABOUT,
        NO_OVERTAKING,
        NO_ENTRY_VIHICULAR_TRAFFIC, // einfahrt verboten (Einbahnstrasse)
        ONEWAY_STREET // Einbahnstrasse
    };

    static std::string ToString(const tRoadSign::RoadSignEnum &value)
    {
        switch (value)
        {
            case NO_MATCH:
                return "NO_MATCH";
            case GIVE_WAY:
                return "GIVE_WAY";
            case HAVE_WAY:
                return "HAVE_WAY";
            case STOP_AND_GIVE_WAY:
                return "STOP_AND_GIVE_WAY";
            case PARKING_AREA:
                return "PARKING_AREA";
            case AHEAD_ONLY:
                return "AHEAD_ONLY";
            case UNMARKED_INTERSECTION:
                return "UNMARKED_INTERSECTION";
            case PEDASTRIAN_CROSSING:
                return "PEDASTRIAN_CROSSING";
            case ROUNDABOUT:
                return "ROUNDABOUT";
            case NO_OVERTAKING:
                return "NO_OVERTAKING";
            case NO_ENTRY_VIHICULAR_TRAFFIC:
                return "NO_ENTRY_VIHICULAR_TRAFFIC";
            case ONEWAY_STREET:
                return "ONEWAY_STREET";
            default:
                return "Unknown Value";
        }
    }
};

#endif
