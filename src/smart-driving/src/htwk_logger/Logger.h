/**
 * @author pbachmann
 */

#ifndef _HTWK_LOGGER_H_
#define _HTWK_LOGGER_H_

#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include <sys/time.h>
#include <opencv2/opencv.hpp>

#include "../htwk_utils/DateUtils.h"

using namespace std;

#define PATH_PREFIX "log/"

class Logger
{
    public:
        Logger(string className);

        Logger(string className, int skip);

        Logger(const Logger &oldLogger);

        void SetSkip(int skipCount);

        ~Logger();

        void StartLog();

        void Log(string message);

        void Log(string message, bool skip);

        void EndLog();

    private:
        string fileName;
        bool isLogStarted;
        int skip;
        int sessionCount;
        cv::Ptr <ofstream> fileStream;

        void Initialize(const string &className);
};

#endif // _HTWK_LOGGER_H_
