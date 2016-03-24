/**
 * @author pbachmann
 */

#include <iostream>
#include <unistd.h>

#include "../common/HTWKTest.h"

#include "../../src/htwk_logger/Logger.h"

using namespace std;

class LoggerTest : public TestFixture
{
    CPPUNIT_TEST_SUITE(LoggerTest);
            CPPUNIT_TEST(Test_Logger);
            CPPUNIT_TEST(Test_Logger_With_Skip);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        }

        void tearDown()
        {
            remove("log/LoggerTest_log.txt");
            rmdir("log");
        }

        void Test_Logger()
        {
            Logger logger("LoggerTest");

            // Write messages
            logger.Log("MyMessage#1");
            logger.Log("MyMessage#2");

            // Check existence of messages
            ifstream t("log/LoggerTest_log.txt");
            stringstream buffer;
            buffer << t.rdbuf();
            string fileContents = buffer.str();

            bool found = fileContents.find("MyMessage#1") != string::npos;
            CPPUNIT_ASSERT_EQUAL(true, found);

            found = fileContents.find("MyMessage#2") != string::npos;
            CPPUNIT_ASSERT_EQUAL(true, found);
        }

        void Test_Logger_With_Skip()
        {
            Logger logger("LoggerTest", 2);

            // Write messages
            logger.StartLog();
            logger.Log("MyMessage#1");
            logger.EndLog();

            logger.StartLog();
            logger.Log("MyMessage#2");
            logger.EndLog();

            logger.StartLog();
            logger.Log("MyMessage#3");
            logger.EndLog();

            logger.StartLog();
            logger.Log("MyMessage#4");
            logger.EndLog();

            logger.StartLog();
            logger.Log("MyMessage#5");
            logger.EndLog();

            logger.StartLog();
            logger.Log("MyMessage#6");
            logger.EndLog();

            // Check existence of messages
            ifstream t("log/LoggerTest_log.txt");
            stringstream buffer;
            buffer << t.rdbuf();
            string fileContents = buffer.str();

            CPPUNIT_ASSERT_EQUAL(false, StringUtils::StringContains(fileContents, "MyMessage#1"));
            CPPUNIT_ASSERT_EQUAL(false, StringUtils::StringContains(fileContents, "MyMessage#2"));
            CPPUNIT_ASSERT_EQUAL(true, StringUtils::StringContains(fileContents, "MyMessage#3"));
            CPPUNIT_ASSERT_EQUAL(false, StringUtils::StringContains(fileContents, "MyMessage#4"));
            CPPUNIT_ASSERT_EQUAL(false, StringUtils::StringContains(fileContents, "MyMessage#5"));
            CPPUNIT_ASSERT_EQUAL(true, StringUtils::StringContains(fileContents, "MyMessage#6"));
        }
};

CPPUNIT_TEST_SUITE_REGISTRATION(LoggerTest);


