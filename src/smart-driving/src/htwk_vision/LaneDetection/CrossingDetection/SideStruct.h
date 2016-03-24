#ifndef SIDE_STRUCT_HEADER
#define SIDE_STRUCT_HEADER

#include "functional"
#include "opencv2/opencv.hpp"

using namespace cv;

class PointAcceptor
{
    public:
        virtual bool check(Point pointToCheck, float imageSize) = 0;
};

class RightAcceptor : public PointAcceptor
{
    public:
        bool check(Point pointToCheck, float imageSize)
        {
            return pointToCheck.x > imageSize / 2.0f;
        }
};

class TopAcceptor : public PointAcceptor
{
    private:
        PointAcceptor *sideAcceptor;
        Point2f botPoint;

    public:
        bool check(Point pointToCheck, float imageSize)
        {
            return sideAcceptor->check(pointToCheck, imageSize) &&
                   abs(pointToCheck.x - botPoint.x) < 10
                   && abs(pointToCheck.y - botPoint.y) > 50;
        }

        TopAcceptor(PointAcceptor *sideAcceptor, Point2f botPoint)
        {
            TopAcceptor::sideAcceptor = sideAcceptor;
            TopAcceptor::botPoint = botPoint;
        }

};

class LeftAcceptor : public PointAcceptor
{
    public:
        bool check(Point pointToCheck, float imageSize)
        {
            return pointToCheck.x < imageSize / 2.0f;
        }
};

#endif // __STD_INCLUDES_HEADER
