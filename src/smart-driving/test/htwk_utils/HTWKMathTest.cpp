/**
 * @author pbachmann
 */

#include <vector>
#include <PolynomialFitter.h>

#include "../common/HTWKTest.h"

using namespace std;

class HTWKMathTest : public TestFixture
{
    CPPUNIT_TEST_SUITE(HTWKMathTest);
            CPPUNIT_TEST(Test_Integrate_Simpson_Sinus);
            CPPUNIT_TEST(Test_Integrate_Simpson_Sinus_Large);
            CPPUNIT_TEST(Test_Integrate_Simpson_Complex);
            CPPUNIT_TEST(Test_LinearRegression);
            CPPUNIT_TEST(Test_PolynomialFit);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        }

        void tearDown()
        {
        }

        void Test_Integrate_Simpson_Sinus()
        {
            tFloat32 result = HTWKMath::Integrate(Sinus, 0.0, 1.0, 10, HTWKMath::SimpsonMethod);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.4597, result, 0.001);
        }

        void Test_Integrate_Simpson_Sinus_Large()
        {
            tFloat32 result = HTWKMath::Integrate(Sinus, 0.0, 100.0, 75, HTWKMath::SimpsonMethod);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1376, result, 0.001);
        }

        void Test_Integrate_Simpson_Complex()
        {
            tFloat32 result = HTWKMath::Integrate(Complex, 0.0, 100.0, 75, HTWKMath::SimpsonMethod);

            CPPUNIT_ASSERT_DOUBLES_EQUAL(121.512, result, 0.001);
        }

        /**
         * Testing this sample
         * https://de.wikipedia.org/wiki/Methode_der_kleinsten_Quadrate#Beispiel_mit_einer_Ausgleichsgeraden
         */
        void Test_LinearRegression()
        {
            vector<Point2f> values;
            values.push_back(Point2f(208, 21.6));
            values.push_back(Point2f(152, 15.5));
            values.push_back(Point2f(113, 10.4));
            values.push_back(Point2f(227, 31.0));
            values.push_back(Point2f(137, 13.0));
            values.push_back(Point2f(238, 32.4));
            values.push_back(Point2f(178, 19.0));
            values.push_back(Point2f(104, 10.4));
            values.push_back(Point2f(191, 19.0));
            values.push_back(Point2f(130, 11.8));

            LinearFunction result = HTWKMath::LinearRegression(values);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(-8.6451, result.GetM(), 0.001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(0.1612, result.GetA(), 0.001);
        }

        /**
         * Testing this sample
         * http://rosettacode.org/wiki/Polynomial_regression
         */
        void Test_PolynomialFit()
        {
            vector<Point3f> values;
            values.push_back(Point3f(0, 1, 0.1));
            values.push_back(Point3f(1, 6, 0.1));
            values.push_back(Point3f(2, 17, 0.1));
            values.push_back(Point3f(3, 34, 0.1));
            values.push_back(Point3f(4, 57, 0.1));
            values.push_back(Point3f(5, 86, 0.1));
            values.push_back(Point3f(6, 121, 0.1));
            values.push_back(Point3f(7, 162, 0.1));
            values.push_back(Point3f(8, 209, 0.1));
            values.push_back(Point3f(9, 262, 0.1));
            values.push_back(Point3f(10, 321, 0.1));

            PolynomialFitterData data;
            data.Degrees = 2;
            data.Points = values;
            double parameter[3] = {0.0, 10.0, 20.0};
            data.Parameter.assign(&parameter[0], &parameter[2]);

            Mat result = PolynomialFitter::PolynomialFit(data).GetM();

//            cout << result.at<float>(0, 0) << " | " << result.at<float>(1, 0) << " | " << result.at<float>(2, 0) << endl;

            CPPUNIT_ASSERT_DOUBLES_EQUAL(1, result.at<float>(0, 0), 0.001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(2, result.at<float>(1, 0), 0.001);
            CPPUNIT_ASSERT_DOUBLES_EQUAL(3, result.at<float>(2, 0), 0.001);
        }

        static tFloat32 Sinus(tFloat32 x)
        {
            return sin(x);
        }

        static tFloat32 Complex(tFloat32 x)
        {
            return sqrt(1 + pow(cos(x), 2));
        }
};

CPPUNIT_TEST_SUITE_REGISTRATION(HTWKMathTest);