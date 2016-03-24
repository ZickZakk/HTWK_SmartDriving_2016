/**
 * @author pbachmann
 */

#include "../common/HTWKTest.h"

class ParkAssistTest : public TestFixture
{
    CPPUNIT_TEST_SUITE(ParkAssistTest);
            CPPUNIT_TEST(Test_Steer_Left_Forward);
            CPPUNIT_TEST(Test_Steer_Left_Reverse);
            CPPUNIT_TEST(Test_Steer_Right_Forward);
            CPPUNIT_TEST(Test_Steer_Right_Reverse);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        }

        void tearDown()
        {
        }

        void Test_Steer_Left_Forward()
        {
            int direction = 1;
            bool reverse = false;

            float yaw = 60;
            float expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(150.0, (double)expectedYaw);

            yaw = 150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-120.0, (double)expectedYaw);

            yaw = -150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-60.0, (double)expectedYaw);

            yaw = -60;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(30.0, (double)expectedYaw);
        }

        void Test_Steer_Left_Reverse()
        {
            int direction = 1;
            bool reverse = true;

            float yaw = 60;
            float expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-30.0, (double)expectedYaw);

            yaw = 150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(60.0, (double)expectedYaw);

            yaw = -150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(120.0, (double)expectedYaw);

            yaw = -60;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-150.0, (double)expectedYaw);
        }

        void Test_Steer_Right_Forward()
        {
            int direction = 2;
            bool reverse = false;

            float yaw = 60;
            float expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-30.0, (double)expectedYaw);

            yaw = 150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(60.0, (double)expectedYaw);

            yaw = -150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(120.0, (double)expectedYaw);

            yaw = -60;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-150.0, (double)expectedYaw);
        }

        void Test_Steer_Right_Reverse()
        {
            int direction = 2;
            bool reverse = true;

            float yaw = 60;
            float expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(150.0, (double)expectedYaw);

            yaw = 150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-120.0, (double)expectedYaw);

            yaw = -150;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(-60.0, (double)expectedYaw);

            yaw = -60;
            expectedYaw = CalculateExpectedYaw(yaw, direction, reverse);
            CPPUNIT_ASSERT_EQUAL(30.0, (double)expectedYaw);
        }

        float CalculateExpectedYaw(float yaw, int direction, bool reverse)
        {
            // direction
            // 1 steer left
            // 2 steer right

            int modify = reverse ? -1 : 1;

            float expectedYaw = 0;

            switch (direction)
            {
                case 1:
                    expectedYaw = yaw + (modify * 90);
                    break;
                case 2:
                    expectedYaw = yaw - (modify * 90);
                    break;
                default:
                    // ERROR
                    expectedYaw = yaw;
            }

            if (expectedYaw > 180)
            {
                expectedYaw = -180 + (expectedYaw - 180);
            }

            if (expectedYaw < -180)
            {
                expectedYaw = 180 + (expectedYaw + 180);
            }

            return expectedYaw;
        }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParkAssistTest);