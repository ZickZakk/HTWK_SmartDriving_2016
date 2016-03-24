/**
 * @author gjenschmischek
 */
#ifndef _T_LINE
#define _T_LINE

#include "stdafx.h"
#include "tPoint.h"
#include "tLineStatus.h"

typedef struct
{
    tPoint tStart;
    tPoint tEnd;
    tLineStatus tStatus;
    tInt tCrossingDistance;
} tLine;

#endif
