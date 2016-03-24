/**
 *
 * ADTF Demo Netio MessageBus Filter. This demo filter measures Network capacity.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: ABOEMI9 $
 * $Date: 2013-10-17 15:47:05 +0200 (Thu, 17 Oct 2013) $
 * $Revision: 41818 $
 *
 * @remarks
 *
 */
#ifndef _DEMO_NETIO_FILTER_HEADER_
#define _DEMO_NETIO_FILTER_HEADER_


#define OID_ADTF_DEMO_NETIO  "adtf.example.demo_netio"

/// Message content
#define MEASUREMESSAGE  "measure_message"

/// Measure time
#define MEASURETIME     30

class cDemoNetIOFilter : public cFilter
{
    ADTF_FILTER(OID_ADTF_DEMO_NETIO, "Demo MessageBus NetIO Filter", OBJCAT_BridgeDevice)

    private: //private members
        class cSenderThread: public cCyclicThread
        {
            protected:
                cDemoNetIOFilter*    m_pParent;
                tSize                m_szCurrentMessageSize;
                tInt                 m_nMessageCountForThisSize;
            public:
                cSenderThread();
                tResult SetParent(cDemoNetIOFilter* pParent,  tSize szBeginofMeasureSize);

            public : //overwrite CyclicFunc of cCyclicThread
                tResult CyclicFunc();
        };

        typedef struct
        {
            tUInt8       ui8Platform;
            tTimeStamp   tmClientSendTime;
            tTimeStamp   tmServerReceivedTime;
            tInt64       szSizeOfData;
            tInt32       szMessageCount; //for measuring a message loss
            tUInt64      nNumber;
        } tMeasuringMessage;

        typedef struct
        {
          tSize      szSizeOfData;

          tTimeStamp tmAll;
          tTimeStamp tmMinTimeBack;
          tTimeStamp tmMaxTimeBack;
          tSize      szAveragePerSecondBack;

          tTimeStamp tmOverallBeginTime;

          tInt       nMessages;
          tInt       nMessageCount;
        } tMeasureStatistics;

        map<tSize, tMeasureStatistics> m_mapStatistics;

        tBool                       m_bServer;
        cString                     m_strServerURL;
        cSenderThread               m_oSenderThread;

        cObjectPtr<IMessageBus>     m_pMessageBus;
        cString                     m_strMyChannelName;
        tSize                       m_nCurrentMessageSizeToMeasureFor;

       tUInt8                       m_ui8Platform;

       tSize                        m_nLastMeasuringSize;

    public: //common implementation
        cDemoNetIOFilter(const tChar* __info);
        virtual ~cDemoNetIOFilter();

    public: // overwrites cFilter
        tResult Init(tInitStage eStage, __exception = NULL);
        tResult Start(__exception = NULL);
        tResult Stop(__exception = NULL);
        tResult Shutdown(tInitStage eStage, __exception = NULL);

        tResult Run(tInt nActivationCode,
                    const tVoid* pvUserData,
                    tInt szUserDataSize,
                    ucom::IException** __exception_ptr);

    public:
        tResult SendMessage(tSize szSizeOfData, tInt nCount);
        tResult LogStatistics(tMeasureStatistics& sStatistic,
                              tTimeStamp& tmCurrentTime);

    private:
        tInt    m_nMessureTime;     // Measure time
        tInt    m_nGenerateRate;    // Generate rate in samples per second
        tInt    m_nTimerPeriod;     // Cycle time period
        tInt    m_nMaxPeriods;      // Number of test periods
        tInt    m_nCurrentPeriod;   // Current test period
};

#endif //
