/*
@author: ffreihube
*/
//#############################################################################
#include "Median_values_lib.h"

//#############################################################################
cMedianValuesLib::cMedianValuesLib()
{
    this->start = NULL;
    this->anker = NULL;

    this->stackSize = 0;
    this->m_fsmoothedData = 0;

    memoryCounter = 0;
}

//#############################################################################
cMedianValuesLib::~cMedianValuesLib()
{
    this->anker = this->start;

    if (this->anker != NULL)
    {
        while (this->anker->next != NULL)
        {
            this->stack = this->anker;
            this->anker = this->anker->next;
            this->stack->last->next = NULL;
            this->stack->last = NULL;

            if (stack != NULL)
            {
                delete this->stack;
                memoryCounter--;
            }
        }

        delete anker;
        memoryCounter--;
    }

}

//#############################################################################
bool cMedianValuesLib::haveEnoughSamples(const int bufferSize) const
{
    if ((this->stackSize) > bufferSize)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//#############################################################################
bool cMedianValuesLib::stackSample(float &value, const int bufferSize)
{
    if (value != 0) // Ring-Buffer
    {

        //return false;

        if (0 == this->stackSize)
        {
            this->stack = new TStack;

            this->stack->data = value;
            //this->stack->last = this->anker;
            start = stack;
            this->anker = this->stack;
            (this->stackSize)++;
        }

        if ((this->stackSize) < bufferSize && (this->stackSize) != 0)
        {
            this->stack = new TStack;

            this->stack->data = value;
            this->stack->last = this->anker;
            this->anker->next = this->stack;
            this->anker = this->stack;
            (this->stackSize)++;
        }

        if ((this->stackSize) == bufferSize)
        {
            this->stack = new TStack;

            this->stack->data = value;
            this->stack->last = this->anker;
            this->anker->next = this->stack;
            this->stack->next = this->start;
            this->start->last = this->stack;
            (this->stackSize)++;
        }

        if ((this->stackSize) > bufferSize)
        {
            this->stack = this->stack->next;
            this->stack->data = value;
        }

    }

    return true;
}

//#############################################################################
float cMedianValuesLib::getMedian(const int &frameSize)
{
    TStack *lastOfFrame = this->stack->last;

    for (int i = 0; i < (frameSize - 1); i++)
    {
        lastOfFrame = lastOfFrame->last;
    }

    if (this->m_fsmoothedData != 0) //online calc of median
    {
        this->m_fsmoothedData =
                this->m_fsmoothedData + (this->stack->data / frameSize) - (lastOfFrame->data / frameSize);
    }
    else
    {
        this->m_fsmoothedData = this->stack->data;
    }

    return (this->m_fsmoothedData);
}