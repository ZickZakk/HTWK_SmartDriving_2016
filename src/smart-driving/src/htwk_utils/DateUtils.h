/**
 * @author pbachmann
 */

#ifndef _HTWK_DATEUTILS_H_
#define _HTWK_DATEUTILS_H_

#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/time.h>

using namespace std;

class DateUtils
{
    public:
        static string GetDateTime();

        static string GetTime();

        static string ReplaceString(string subject, const string &search, const string &replace);
};


#endif // _HTWK_DATEUTILS_H_
