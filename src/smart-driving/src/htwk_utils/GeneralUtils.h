//
// Created by pbachmann on 12/23/15.
//

#ifndef HTWK_2016_GENERALUTILS_H
#define HTWK_2016_GENERALUTILS_H

#include <map>
#include <linux/limits.h>
#include <unistd.h>
#include "opencv2/opencv.hpp"
#include <opencv2/core/cvstd.hpp>

using namespace std;
using namespace cv;


class GeneralUtils
{
    public:
        template<typename KeyType, typename LeftValue, typename RightValue>
        static map<KeyType, std::pair<LeftValue, RightValue> > IntersectMaps(const map<KeyType, LeftValue> &left,
                                                                             const map<KeyType, RightValue> &right);

        static bool Equals(float f1, float f2, float delta);

        static bool Equals(Point2f p1, Point2f p2, float delta);

        static bool Smaller(float f1, float f2, float delta);

        static bool Bigger(float f1, float f2, float delta);

        static bool Smaller(Point2f p1, Point2f p2, float delta);

        static string GetExePath();

        template<typename Type>
        static bool Contains(const std::vector<Type> &vector, const Type &value);

        static bool IsBetween(float f, float upper, float lower, float delta);
};


/**
 * Wondering about the implementation in header file?
 * See: http://stackoverflow.com/a/8752879/4936633
 */
template<typename KeyType, typename LeftValue, typename RightValue>
map<KeyType, std::pair<LeftValue, RightValue> > GeneralUtils::IntersectMaps(const map<KeyType, LeftValue> &left,
                                                                            const map<KeyType, RightValue> &right)
{
    map<KeyType, pair<LeftValue, RightValue> > result;
    typename map<KeyType, LeftValue>::const_iterator il = left.begin();
    typename map<KeyType, RightValue>::const_iterator ir = right.begin();
    while (il != left.end() && ir != right.end())
    {
        if (il->first < ir->first)
        {
            ++il;
        }
        else if (ir->first < il->first)
        {
            ++ir;
        }
        else
        {
            result.insert(make_pair(il->first, make_pair(il->second, ir->second)));
            ++il;
            ++ir;
        }
    }
    return result;
}

template<class T1, class T2, class cmp1, class cmp2>
class SimpleBimap
{
    private:
        map<T1, T2, cmp1> map1;
        map<T2, T1, cmp2> map2;

    public:
        void insert(T1 t1, T2 t2)
        {
            map1.insert(pair<T1, T2>(t1, t2));
            map2.insert(pair<T2, T1>(t2, t1));
        }

        T1 getFirst(T2 t2)
        {
            return map2[t2];
        }

        T2 getSecond(T1 t1)
        {
            return map1[t1];
        }

        bool containsFirst(T1 t1)
        {
            return map1.find(t1) != map1.end();
        }

        bool containsSecond(T2 t2)
        {
            return map2.find(t2) != map2.end();
        }

        int size()
        {
            return static_cast<int>(map1.size());
        }
};


template<typename Type>
bool GeneralUtils::Contains(const vector<Type> &vector, const Type &value)
{
    for (unsigned int i = 0; i < vector.size(); ++i)
    {
        if (value == vector[i])
        {
            return true;
        }
    }

    return false;
}

#endif //HTWK_2016_GENERALUTILS_H
