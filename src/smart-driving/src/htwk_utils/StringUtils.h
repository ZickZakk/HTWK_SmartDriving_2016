/**
 * @author pbachmann
 */

#ifndef _HTWK_STRINGUTILS_H_
#define _HTWK_STRINGUTILS_H_

#include <iostream>
#include <unistd.h>

using namespace std;

class StringUtils
{
    public:
        static bool StringContains(string text, string find);
};


#endif // _HTWK_STRINGUTILS_H_
