/**
 *
 * Deno shapes outputer source.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: belkera $
 * $Date: 2011-07-01 08:26:22 +0200 (Fri, 01 Jul 2011) $
 * $Revision: 7968 $
 *
 * @remarks
 *
 */
#ifndef _SIMPLE_SHAPES_FILTER_HEADER_
#define _SIMPLE_SHAPES_FILTER_HEADER_

#define OID_SIMPLE_SHAPES_FILTER "htwk.smart_driving.htwk_simple_shape_factory"

class cSimpleShapesFilter : public adtf::cFilter
{
    ADTF_FILTER(OID_SIMPLE_SHAPES_FILTER, "Simple Shapes Filter", adtf::OBJCAT_Tool);

    private:
        cOutputPin              m_oPinOutput; // output pin for signal data
        tHandle                 m_hTimer;
        tTimeStamp              m_nDelay;
        tInt32                  m_i32StartX;
        tInt32                  m_i32StartY;
        tInt32                  m_i32Width;
        tInt32                  m_i32Height;


    public: // construction
        cSimpleShapesFilter(const tChar* __info);
        virtual ~cSimpleShapesFilter();

    public: // overrides cBaseFilter
        tResult Init(tInitStage eStage, __exception);
        tResult Start(__exception);
        tResult Stop(__exception);

    public: // overrides IRunnable implementation
        tResult Run(tInt nActivationType, const tVoid* pvUserData, tInt szUserDataSize, __exception);
        
    protected:
        tResult OnNewData();
        tResult SendShape(tShapeInput* pShape);
};

//*************************************************************************************************
#endif // _RANDOM_GENERATOR_FILTER_HEADER_
