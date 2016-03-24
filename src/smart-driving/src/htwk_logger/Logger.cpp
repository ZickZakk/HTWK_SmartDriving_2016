/**
 * @author pbachmann
 */

#include "Logger.h"

Logger::Logger(string className)
{
//#ifndef NDEBUG
    Initialize(className);
    skip = 0;
//#endif
}

Logger::Logger(string className, int skip)
{
//#ifndef NDEBUG
    Initialize(className);
    Logger::skip = skip;
//#endif
}

void Logger::Initialize(const string &className)
{
//#ifndef NDEBUG
    isLogStarted = false;
    sessionCount = 0;

    fileName = PATH_PREFIX + className + "_log.txt";
    fileName = DateUtils::ReplaceString(fileName, " ", "_");
    replace(fileName.begin(), fileName.end(), ' ', '_');
    mkdir(PATH_PREFIX, 0777);

    ofstream *stream = new ofstream();

    fileStream = stream;

    fileStream->open(fileName.c_str(), ios_base::app);
    *fileStream << "\n";
    *fileStream << "#####################################################\n";
    *fileStream << "#### " + DateUtils::GetDateTime() + "\n";
    *fileStream << "#####################################################\n";
//#endif
}

Logger::~Logger()
{
//#ifndef NDEBUG
    *fileStream << "\n";
    *fileStream << "I've benn killed!";
    fileStream->close();
//#endif
}

void Logger::StartLog()
{
//#ifndef NDEBUG
    sessionCount++;
    if (sessionCount <= skip)
    {
        return;
    }

    isLogStarted = true;
//#endif
};

void Logger::EndLog()
{
//#ifndef NDEBUG
    isLogStarted = false;
//#endif
};

void Logger::Log(string message)
{
//#ifndef NDEBUG
    Log(message, true);
//#endif
}

void Logger::Log(string message, bool skip)
{
//#ifndef NDEBUG
    // skip if log is not started, log every message if skip == 0
    if (skip && !isLogStarted && 0 != Logger::skip)
    {
        return;
    }

    if (!fileStream->is_open())
    {
        return;
    }

    *fileStream << DateUtils::GetTime() + ": " + message + "\n";
    fileStream->flush();

    // don't reset on !skip messages
    if (skip)
    {
        sessionCount = 0;
    }
//#endif
}

Logger::Logger(const Logger &oldLogger)
{
//#ifndef NDEBUG
    fileName = oldLogger.fileName;
    isLogStarted = oldLogger.isLogStarted;
    skip = oldLogger.skip;
    sessionCount = oldLogger.sessionCount;

    ofstream *stream = new ofstream();
    fileStream = stream;
    fileStream->open(fileName.c_str(), ios_base::app);
//#endif
}

void Logger::SetSkip(int skipCount)
{
//#ifndef NDEBUG
    skip = skipCount;
//#endif
}
