//
// Created by pbachmann on 3/12/16.
//

/**
 * @author pbachmann
 */

#include <iostream>
#include <unistd.h>
#include <CarConfigReader.h>

#include "../common/HTWKTest.h"

using namespace std;

class CarConfigReaderTest : public TestFixture
{
    CPPUNIT_TEST_SUITE(CarConfigReaderTest);
            CPPUNIT_TEST(Test_Constructor);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        }

        void tearDown()
        {
        }

        void Test_Constructor()
        {
            CarConfigReader reader = CarConfigReader("resources/config.xml");

            tInt val1;
            reader.Pull<tInt>("section1", "key1", val1);
            CPPUNIT_ASSERT_EQUAL(1, val1);

            tFloat32 val2;
            reader.Pull<tFloat32>("section1", "key2", val2);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, val2, 0.1);

            tBool val3;
            reader.Pull<tBool>("section1", "key3", val3);
            CPPUNIT_ASSERT_EQUAL(tFalse, val3);
        }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CarConfigReaderTest);


