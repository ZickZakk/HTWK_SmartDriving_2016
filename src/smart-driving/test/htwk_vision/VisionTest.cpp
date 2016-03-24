/**
 * @author pbachmann
 */

#include <VisionUtils.h>
#include "../common/HTWKTest.h"
#include "../../src/htwk_vision/LaneDetection/IPMBorder.h"

using namespace std;
using namespace cv;

class VisionTest : public TestFixture
{
    CPPUNIT_TEST_SUITE(VisionTest);
            CPPUNIT_TEST(Test_Default_Mat_Constructor);
            CPPUNIT_TEST(Test_IPMBorder);
            CPPUNIT_TEST(Test_IPMBorder_Go_Around);
//            CPPUNIT_TEST(Test_Huang_Threshold);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp()
        {
        }

        void tearDown()
        {
        }

        void Test_Default_Mat_Constructor()
        {
            Mat asd;

            if (asd.data != NULL)
            {
                CPPUNIT_FAIL("Not NULL");
            }
        }

        void Test_IPMBorder()
        {
            Point2f UpLeft(0, 0);
            Point2f UpRight(200, 0);
            Point2f DownLeft(50, 200);
            Point2f DownRight(150, 200);

            IPMBorder border;

            CPPUNIT_ASSERT_EQUAL(border.IsInitialized(), false);

            Point2f current(1, 0);

            Point2f next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, -1.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, -1.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, -1.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, -1.0f);

            border.Initialize(UpLeft, UpRight, DownLeft, DownRight);

            CPPUNIT_ASSERT_EQUAL(border.IsInitialized(), true);

            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 2.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 0.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 0.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 0.0f);

            current = Point2f(51, 200);
            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 50.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 200.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 52.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 200.0f);

            current = Point2f(0, 0);
            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 1.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 0.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 0.25f);
            CPPUNIT_ASSERT_EQUAL(next.y, 1.0f);

            current = Point2f(200, 0);
            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 199.75f);
            CPPUNIT_ASSERT_EQUAL(next.y, 1.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 199.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 0.0f);

            current = Point2f(50, 200);
            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 49.75f);
            CPPUNIT_ASSERT_EQUAL(next.y, 199.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 51.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 200.0f);

            current = Point2f(150, 200);
            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 149.0f);
            CPPUNIT_ASSERT_EQUAL(next.y, 200.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 150.25f);
            CPPUNIT_ASSERT_EQUAL(next.y, 199.0f);

            current = Point2f(25, 100);
            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 24.75f);
            CPPUNIT_ASSERT_EQUAL(next.y, 99.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 25.25f);
            CPPUNIT_ASSERT_EQUAL(next.y, 101.0f);

            current = Point2f(190, 40);
            next = border.Go(current, CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 189.75f);
            CPPUNIT_ASSERT_EQUAL(next.y, 41.0f);

            next = border.Go(current, COUNTER_CLOCKWISE);
            CPPUNIT_ASSERT_EQUAL(next.x, 190.25f);
            CPPUNIT_ASSERT_EQUAL(next.y, 39.0f);
        }

        static bool f(Point2f p1, Point2f p2)
        {
            if (p1.y < p2.y)
            {
                return 1;
            }
            else if (p1.y > p2.y)
            {
                return 0;
            }
            if (p1.x < p2.x)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }

        void Test_IPMBorder_Go_Around()
        {
            Point2f UpLeft(0, 0);
            Point2f UpRight(200, 0);
            Point2f DownLeft(50, 200);
            Point2f DownRight(150, 200);

            IPMBorder border;

            border.Initialize(UpLeft, UpRight, DownLeft, DownRight);

            Point2f current = UpLeft;

            vector<Point2f> cwPoints;

            bool found = false;

            for (int i = 0; i < 1000; i++)
            {
                current = border.Go(current, COUNTER_CLOCKWISE);
                cwPoints.push_back(current);

                if (UpLeft == current)
                {
                    found = true;
                    break;
                }
            }

            CPPUNIT_ASSERT_EQUAL(true, found);

            current = UpLeft;

            vector<Point2f> ccwPoints;

            found = false;

            for (int i = 0; i < 1000; i++)
            {
                current = border.Go(current, CLOCKWISE);
                ccwPoints.push_back(current);

                if (UpLeft == current)
                {
                    found = true;
                    break;
                }
            }

            CPPUNIT_ASSERT_EQUAL(true, found);

            std::sort(cwPoints.begin(), cwPoints.end(), VisionUtils::StandardSmallerPoint);
            std::sort(ccwPoints.begin(), ccwPoints.end(), VisionUtils::StandardSmallerPoint);

//            std::vector<Point2f>::iterator it;
//            vector<Point2f> v(cwPoints.size());
//
//            it = std::set_symmetric_difference(cwPoints.begin(), cwPoints.end(), ccwPoints.begin(), ccwPoints.end(),
//                                               v.begin(), f);
//            //  5 15 25 30 40 50  0  0  0  0
//            v.resize(it - v.begin());                      //  5 15 25 30 40 50
//
//            std::cout << "The symmetric difference has " << (v.size()) << " elements:\n";
//            for (it = v.begin(); it != v.end(); ++it)
//            {
//                std::cout << "(" << it->x << ", " << it->y << ")" << endl;
//            }
//            std::cout << '\n';

            CPPUNIT_ASSERT_EQUAL(cwPoints, ccwPoints);
        }
        
//        void Test_Huang_Threshold()
//        {
//            Mat image = imread("test.jpg", CV_LOAD_IMAGE_GRAYSCALE);
//            ThreshHolder threshHolder;
//
//            CPPUNIT_ASSERT_EQUAL(false, !image.data);
//            int thresh = threshHolder.yen(image);
//            CPPUNIT_ASSERT_EQUAL(99, thresh);
//        }
};

CPPUNIT_TEST_SUITE_REGISTRATION(VisionTest);