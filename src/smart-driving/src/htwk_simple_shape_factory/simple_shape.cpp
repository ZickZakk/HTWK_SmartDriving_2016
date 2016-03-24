/**
 *
 * Deno shapes outputer source.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: WNEROLF $
 * $Date: 2012-09-04 08:26:02 +0200 (Tue, 04 Sep 2012) $
 * $Revision: 15922 $
 *
 * @remarks
 *
 */
#include "stdafx.h"
#include "simple_shape.h"

#ifndef M_PI
#define M_PI 3.1415926
#endif

ADTF_FILTER_PLUGIN("Demo Shapes Filter", OID_SIMPLE_SHAPES_FILTER, cSimpleShapesFilter);


cSimpleShapesFilter::cSimpleShapesFilter(const tChar* __info) : adtf::cFilter(__info)
{
    m_hTimer = NULL;
    m_nDelay = 20000;

    // default is to send new data every 20 ms
    SetPropertyInt("Delay",  static_cast<tInt>(m_nDelay));


    // init values
    m_i32StartX = 0;
    m_i32StartY = 0;
    m_i32Width = 0;
    m_i32Height = 0;

    // create the filter properties
    SetPropertyInt("Start X", m_i32StartX);
    SetPropertyInt("Start X" NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr("Start X" NSSUBPROP_DESCRIPTION, "X Start value");

    SetPropertyInt("Start Y", m_i32StartY);
    SetPropertyInt("Start Y" NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr("Start Y" NSSUBPROP_DESCRIPTION, "Y Start value");

    SetPropertyInt("Width", m_i32Width);
    SetPropertyInt("Width" NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr("Width" NSSUBPROP_DESCRIPTION, "Width value");

    SetPropertyInt("Height", m_i32Height);
    SetPropertyInt("Height" NSSUBPROP_REQUIRED, tTrue);
    SetPropertyStr("Height" NSSUBPROP_DESCRIPTION, "Height value");

}

cSimpleShapesFilter::~cSimpleShapesFilter()
{
}

tResult cSimpleShapesFilter::Init(tInitStage eStage, __exception)
{
    // call parent first
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == StageFirst)
    {
        // create our Pins
        RETURN_IF_FAILED(m_oPinOutput.Create("output", new adtf::cMediaType(MEDIA_TYPE_SHAPES, MEDIA_SUBTYPE_SHAPES), this));
        RETURN_IF_FAILED(RegisterPin(&m_oPinOutput));
    }
    else if (eStage == StageNormal)
    {
        // read properties
        m_nDelay = GetPropertyInt("Delay");
        m_i32StartX = GetPropertyInt("Start X");
        m_i32StartY = GetPropertyInt("Start Y");
        m_i32Width = GetPropertyInt("Width");
        m_i32Height = GetPropertyInt("Height");
    }

    RETURN_NOERROR;
}

tResult cSimpleShapesFilter::Start(__exception)
{
    // give parent a chance to start
    RETURN_IF_FAILED(adtf::cFilter::Start(__exception_ptr));

    // we do require a kernel
    THROW_IF_POINTER_NULL(_kernel);

    // create our timer
    m_hTimer = _kernel->TimerCreate(m_nDelay, 0, static_cast<IRunnable*>(this));
    if (!m_hTimer)
    {
        THROW_ERROR_DESC(ERR_UNEXPECTED, "Unable to create a timer");
    }

    RETURN_NOERROR;
}

tResult cSimpleShapesFilter::Run(tInt nActivationType, const tVoid* pvUserData, tInt szUserDataSize, __exception)
{
    RETURN_IF_FAILED(cFilter::Run(nActivationType, pvUserData, szUserDataSize, __exception_ptr));

    if (IRunnable::RUN_TIMER == nActivationType)
    {
        // if its been the timer, send new data
        RETURN_IF_FAILED(OnNewData());
    }

    RETURN_NOERROR;
}

tResult cSimpleShapesFilter::Stop(__exception)
{
    //destroy the timer
    if (m_hTimer)
    {
        _kernel->TimerDestroy(m_hTimer);
        m_hTimer = NULL;
    }

    return cFilter::Stop(__exception_ptr);
}

tResult cSimpleShapesFilter::OnNewData()
{
    // the struct that we will send
    tShapeInput a;

    // send some lines
    a.clearBefore = tTrue; // clear the scene with this sample
    a.x = m_i32StartX;
    a.y = m_i32StartY;
    a.z = 0;
    a.red = 0;
    a.green = 0;
    a.blue = 0;          // BLACK
    a.alpha = 255;         // full opacity
    a.type = SHAPE_QUADS;  // we want to draw lines
    a.yaw = 0; // one full rotation each 10 seconds
    a.roll = 0;
    a.pitch = 0;

    tInt nCoord = -1;

    // quad
    a.coords[++nCoord] = 0.0;
    a.coords[++nCoord] = 0.0;
    a.coords[++nCoord] = 0.0;

    a.coords[++nCoord] = m_i32Width;
    a.coords[++nCoord] = 0.0;
    a.coords[++nCoord] = 0.0;

    a.coords[++nCoord] = m_i32Width;
    a.coords[++nCoord] = m_i32Height;
    a.coords[++nCoord] = 0.0;

    a.coords[++nCoord] = 0.0;
    a.coords[++nCoord] = m_i32Height;
    a.coords[++nCoord] = 0.0;

    // finally store the amount of coordinates
    a.numCoords = nCoord + 1;
    // and send it
    SendShape(&a);

    RETURN_NOERROR;
}

tResult cSimpleShapesFilter::SendShape(tShapeInput* pShape)
{
    // allocate a new sample
    cObjectPtr<IMediaSample> pNewSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid**) &pNewSample));
    // update its data
    RETURN_IF_FAILED(pNewSample->Update(_clock->GetStreamTime(), pShape, sizeof(tShapeInput), 0));
    // and transmit it
    return m_oPinOutput.Transmit(pNewSample);
}

