/**
 * @author pbachmann
 */

#include "DateUtils.h"

string DateUtils::GetDateTime()
{
    time_t now = time(0);
    char buffer[80];

    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %T", localtime(&now));

    return buffer;
}

string DateUtils::GetTime()
{
    timeval curTime;
    gettimeofday(&curTime, NULL);
    int milli = curTime.tv_usec / 1000;

    char buffer[80];
    strftime(buffer, 80, "%H:%M:%S", localtime(&curTime.tv_sec));

    char currentTime[84] = "";
    sprintf(currentTime, "%s.%d", buffer, milli);

    return currentTime;
}

string DateUtils::ReplaceString(string subject, const string &search, const string &replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}