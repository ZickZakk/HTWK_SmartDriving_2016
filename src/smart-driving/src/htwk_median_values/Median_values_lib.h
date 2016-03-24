/*
@author: ffreihube
*/
//#############################################################################
#ifndef _HTWK_MEDIANVALUES_LIB_H_
#define _HTWK_MEDIANVALUES_LIB_H_

#include <stddef.h>
#include <sstream>

//#############################################################################
class cMedianValuesLib
{
    private:

        struct TStack //struct for Buffer
        {
            float data;   // Data
            TStack *last; // Link to last
            TStack *next; // Link to next
        };


        TStack *stack;
        TStack *anker;
        TStack *start;

        float m_fsmoothedData;
        int stackSize;
        int memoryCounter;

    public:
        /**
         * @brief Constructor.
         */
        cMedianValuesLib(void);

        /**
         * @brief Destructor.
         */
        ~cMedianValuesLib(void);

        /**
         * @brief Stack one sample
         *
         * @param [in] value the float value
         * @param [in] frameSize the size of the frame
         * @return true if sample was stacked
         */
        bool stackSample(float &value, const int bufferSize);

        /**
         * @brief Checks if there are enough samples to create median
         *
         * @param [in] frameSize the size of the frame
         * @return true if a median can be processed
         */
        bool haveEnoughSamples(const int bufferSize) const;

        /**
         * @brief Stack one sample
         *
         * @return median value
         */
        float getMedian(const int &frameSize);

};

#endif // _HTWK_MEDIANVALUES_LIB_H_
