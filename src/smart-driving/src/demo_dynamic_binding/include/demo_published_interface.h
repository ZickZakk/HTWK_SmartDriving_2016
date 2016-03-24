/**
 *
 * ADTF Demo Server Filter.
 *    This is only for demo a Filter application.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: ABOEMI9 $
 * $Date: 2013-08-27 10:02:18 +0200 (Di, 27 Aug 2013) $
 * $Revision: 40286 $
 *
 * @remarks
 *
 */
#ifndef _DEMO_DYNAMIC_PUBLISHED_INTERFACE_HEADER_
#define _DEMO_DYNAMIC_PUBLISHED_INTERFACE_HEADER_


#define IID_DEMO_PUBLISHED_INTERFACE "adtf.demoiid_published_interface"

class IDemoPublishedInterface : public ucom::IObject
{
    public:
        //Data to publish via Pin
        //IMPORTANT: For this we use ONLY plain old C-Style Structure definition
        // do not implement a CTOR or DTOR !!
#pragma pack (push, 4)
        typedef struct
        {
            tInt8 i8Value;
            tUInt32 ui32Value;
        } tPublishedDataStruct;

        //Internal state information to publish via Interface
        //IMPORTANT: For this we use ONLY plain old C-Style Structure definition
        // do not implement a CTOR or DTOR !!
        typedef struct
        {
            tInt32 i32ActivateCount;
            tUInt8 ui8ServerState;
            tInt64 i64ProcessCallCounter;
            tInt64 i64TransmitCounter;
        } tDemoServerState;
#pragma pack (pop)

    public:
        virtual tResult GetServerName(tChar *strBuffer, const tInt32 i32BufferSize) = 0;

        virtual tResult GetInternalState(tDemoServerState *pState, const tInt32 i32ServerStateSize) = 0;

        virtual tResult ActivateOutputs(const tUInt8 &ui8Activate) = 0;
};

#endif // _DEMO_DYNAMIC_PUBLISHED_INTERFACE_HEADER_

