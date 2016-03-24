/**
 * @author pbachmann
 */

#include "../common/HTWKTest.h"

using namespace std;

class LinearFunctionTest : public TestFixture
{
    CPPUNIT_TEST_SUITE(LinearFunctionTest);
            CPPUNIT_TEST(Test_LinearFunction);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        }

        void tearDown()
        {
        }

        void Test_LinearFunction()
        {
            const int a = 1;
            const int m = 2;

            LinearFunction function = LinearFunction(a, m);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(a, function.GetA(), 0);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(m, function.GetM(), 0);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2, function.CalculateX(4), 0);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(4, function.CalculateY(2), 0);
        }
};

CPPUNIT_TEST_SUITE_REGISTRATION(LinearFunctionTest);
