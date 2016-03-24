/**
 *
 * ADTF Demo Netio MessageBus Filter. This demo filter measures Network capacity.
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: ELAMIHA $
 * $Date: 2014-03-27 16:40:09 +0100 (Thu, 27 Mar 2014) $
 * $Revision: 45736 $
 *
 * @remarks             This example shows how to implement a MessageBus
 *                      connection filter for processing data over network.
 *
 */
#include "stdafx.h"
#include "./demonetiofilter.h"
#include "demonetiofilter.h"

ADTF_FILTER_PLUGIN("Demo MessageBus NetIO Filter Plugin",
                    OID_ADTF_DEMO_NETIO,
                    cDemoNetIOFilter)

/**
 * Constructor.
 */
cDemoNetIOFilter::cSenderThread::cSenderThread():
        cCyclicThread(),
        m_pParent(NULL),
        m_szCurrentMessageSize(1024),
        m_nMessageCountForThisSize(0)
{
}

tResult cDemoNetIOFilter::cSenderThread::SetParent(cDemoNetIOFilter* pParent,
                                                   tSize szBeginofMeasureSize)
{
    m_szCurrentMessageSize = szBeginofMeasureSize;
    m_pParent = pParent;
    m_nMessageCountForThisSize = 0;

    RETURN_NOERROR;
}

/**
 * The thread function which will be called in a loop.
 * @return Standard Result Code.
 */
tResult cDemoNetIOFilter::cSenderThread::CyclicFunc()
{
    RETURN_IF_POINTER_NULL(m_pParent);

    if (m_pParent->m_nCurrentPeriod <= m_pParent->m_nMaxPeriods)
    {
        tResult nRes = m_pParent->SendMessage(m_szCurrentMessageSize,
                                              m_nMessageCountForThisSize);
        if (IS_FAILED(nRes))
        {
            if (nRes == ERR_TIMEOUT)
            {
                LOG_ERROR("NetIO-Sender: Could not send measuring message "
                          "to server, timeout!");
            }
            else if (nRes == ERR_END_OF_FILE)
            {
                //end of measuring for this this
                m_nMessageCountForThisSize = 0;
                // set next measuring size
                m_szCurrentMessageSize = m_szCurrentMessageSize * 2;
                RETURN_NOERROR;
            }
            else
            {
                LOG_ERROR("NetIO-Sender: Could not send measuring message to "
                          " server, something totally wrong here!");
            }
        }
        m_nMessageCountForThisSize++;
        RETURN_NOERROR;
    }
    else
    {
        //set self to terminate
        Terminate(tFalse);
        LOG_INFO("NetIO-Sender: Measuring ended!");
        RETURN_NOERROR;
    }
}

/**
 *   Constructor. The cFilter constructor needs to be called !!
 *   The SetProperty in the constructor is necessary if somebody wants to deal
 *   with the default values of the properties before init
 *
 */
cDemoNetIOFilter::cDemoNetIOFilter(const tChar* __info) : cFilter(__info)
{
    m_ui8Platform = PLATFORM_BYTEORDER;
    m_nMessureTime = MEASURETIME;

    SetPropertyInt("generate_rate", 0);
    SetPropertyStr("generate_rate" NSSUBPROP_DISPLAYNAME, "Client Generate Rate");
    SetPropertyStr("generate_rate" NSSUBPROP_DESCRIPTION,
                   "Messages per second. If 0 generate rate will be as fast as"
                   " possible. Client property.");

    SetPropertyInt("measure_time", m_nMessureTime);
    SetPropertyStr("measure_time" NSSUBPROP_DISPLAYNAME, "Client Period Time");
    SetPropertyStr("measure_time" NSSUBPROP_DESCRIPTION,
                   "Measurement time for each period in seconds. Client property.");

    SetPropertyInt("test_periods", 20);
    SetPropertyStr("test_periods" NSSUBPROP_DISPLAYNAME, "Client Test Periods");
    SetPropertyStr("test_periods" NSSUBPROP_DESCRIPTION,
                   "Every period will multiply message size by 2. Client property.");

    SetPropertyStr("server_url", "udp://localhost:5555");
    SetPropertyStr("server_url" NSSUBPROP_DISPLAYNAME, "Server URL");
    SetPropertyStr("server_url" NSSUBPROP_DESCRIPTION, "Adjust here the server "
                   "URL to measure the data capacity to");

    SetPropertyBool("is_server", tTrue);
    SetPropertyStr("is_server" NSSUBPROP_DISPLAYNAME, "Server Mode");
    SetPropertyStr("is_server" NSSUBPROP_DESCRIPTION, "The NetIO Server will "
                   "wait for a NetIO client to measure the capacity.");

    SetPropertyInt("begin_messagesize", 1);
    SetPropertyStr("begin_messagesize" NSSUBPROP_DISPLAYNAME, "Client First Message Size");
    SetPropertyStr("begin_messagesize" NSSUBPROP_DESCRIPTION,
                   "Message size at the begin in KBytes. Client property.");
}

/**
 *  Destructor. A Filter implementation needs always to have a virtual Destructor
 *              because the IFilter extends an IObject
 *
 */
cDemoNetIOFilter::~cDemoNetIOFilter()
{
}

/**
 *   The Filter Init Function.
 *    eInitStage ... StageFirst ... should be used for creating and registering Pins
 *               ... StageNormal .. should be used for reading the properties
 *                                  and initializing everything before pin
 *                                  connections are made
 *   see {@link IFilter#Init IFilter::Init}.
 *
 */

tResult cDemoNetIOFilter::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

    if (eStage == cFilter::StageFirst)
    {
    }
    else if (eStage == cFilter::StageNormal)
    {
        m_nGenerateRate = GetPropertyInt("generate_rate");
        if (0 >= m_nGenerateRate)
        {
            // Thread will run as fast as possible.
            m_nTimerPeriod = 0;
        }
        else if (0 < m_nGenerateRate)
        {
            // Calculate timer period from generate rate.
            m_nTimerPeriod = static_cast<tInt>((1000.0 * 1000.0) /
                             (static_cast<tFloat>(m_nGenerateRate)));
        }

        m_nMessureTime = GetPropertyInt("measure_time");

        if (0 > m_nMessureTime)
        {
            m_nMessureTime = MEASURETIME;
        }

        m_nMaxPeriods = GetPropertyInt("test_periods");

        if (0 > m_nMaxPeriods)
        {
            m_nMaxPeriods = 1;
        }

        m_nCurrentPeriod = 0;

        m_strServerURL = GetPropertyStr("server_url", 0, 0, "udp://localhost:5555");
        m_bServer      = GetPropertyBool("is_server", tTrue);
        RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MESSAGE_BUS,
                                             IID_ADTF_MESSAGE_BUS,
                                             (tVoid**)&m_pMessageBus,
                                             __exception_ptr));
        m_strMyChannelName = OIGetInstanceName();
        m_strMyChannelName.Append("_measure");
        LOG_INFO(cString::Format("channel name = %s", m_strMyChannelName.GetPtr()).GetPtr());

        RETURN_IF_FAILED(m_pMessageBus->CreateChannel(m_strMyChannelName.GetPtr(),
                                                      m_strServerURL.GetPtr(),
                                                      0,
                                                      __exception_ptr));
        LOG_INFO("NetIO Measuring Channel create");
        //with NULL we register for all messages coming on this channel
        RETURN_IF_FAILED(m_pMessageBus->Connect(m_strMyChannelName.GetPtr(),
                                                NULL,
                                                (IRunnable*)this,
                                                0,
                                                __exception_ptr));
    }
    RETURN_NOERROR;
}

/**
 *   The Filters Start Function. see {@link IFilter#Start IFilter::Start}.
 *
 */
tResult cDemoNetIOFilter::Start(__exception)
{
    RETURN_IF_FAILED(cFilter::Start(__exception_ptr));
    m_nLastMeasuringSize = 0;
    if (!m_bServer)
    {
        tInt nSize = GetPropertyInt("begin_messagesize", 1);
        if (nSize > 0 && nSize < 120000)
        {
            nSize = nSize * 1024;
        }
        else
        {
            nSize = 1024;
        }
        LOG_INFO("NetIO-Sender: Measuring Begins");
        m_mapStatistics.clear();
        m_oSenderThread.SetParent(this, (tSize)nSize);
        m_oSenderThread.Create();
        m_oSenderThread.SetCycleTime(m_nTimerPeriod);
        m_oSenderThread.Run();
    }
    else
    {
        LOG_INFO("NetIO-Receiver: Server is waiting for a client.");
    }
    RETURN_NOERROR;
}

/**
 *   The Filters Stop Function. see {@link IFilter#Stop IFilter::Stop}.
 *
 */
tResult cDemoNetIOFilter::Stop(__exception)
{
    m_oSenderThread.Terminate();
    m_oSenderThread.Release();
    LOG_INFO("NetIO Measuring Ends");
    return cFilter::Stop(__exception_ptr);
}

/**
 *   The Filters Shutdown Function. see {@link IFilter#Shutdown IFilter::Shutdown}.
 *
 */
tResult cDemoNetIOFilter::Shutdown(tInitStage eStage, __exception)
{
    if (eStage == StageNormal)
    {
        m_pMessageBus->Disconnect(m_strMyChannelName.GetPtr(), NULL,
                                  (IRunnable*)this, __exception_ptr);
        m_pMessageBus->DestroyChannel(m_strMyChannelName.GetPtr());
        m_pMessageBus = NULL;
    }
    return cFilter::Shutdown(eStage, __exception_ptr);
}


/**
 * Process the receiving of Messages coming from the MessageBus

 * @return  Standard result code.
 */
tResult cDemoNetIOFilter::SendMessage(tSize szSizeOfData, tInt nCount)
{
    if (szSizeOfData > sizeof(tMeasuringMessage))
    {
        tMeasureStatistics& sStatistic = m_mapStatistics[szSizeOfData];
        if (nCount == 0)
        {
            m_nCurrentPeriod++;
            if (m_nCurrentPeriod <= m_nMaxPeriods)
            {
                LOG_INFO(cString::Format("NetIO-Sender: Test period %d start: "
                    "Measuring for %d seconds "
                    "messages with size of %lld Bytes (%.0lf KB)",
                    m_nCurrentPeriod,
                    m_nMessureTime,
                    tInt64(szSizeOfData),
                    tFloat64(szSizeOfData) / 1024.0).GetPtr());
            }

            sStatistic.nMessages = 0;
            sStatistic.szAveragePerSecondBack = 0;
            sStatistic.szSizeOfData = szSizeOfData;
            sStatistic.tmMaxTimeBack = 0;
            sStatistic.tmMinTimeBack = 0;
            sStatistic.tmAll = 0;
            sStatistic.tmOverallBeginTime = _clock->GetTime();
            sStatistic.nMessageCount = 0;
        }

        cMemoryBlock oBlock(szSizeOfData);
        cMemoryBlock oBlockReply(szSizeOfData);
        // Is not important what data are in the block
        // only the first tMeasuringMessage bytes are interesting
        tMeasuringMessage* pMeasuring = (tMeasuringMessage*)oBlock.GetPtr();
        pMeasuring->ui8Platform          = m_ui8Platform;
        pMeasuring->szSizeOfData         = szSizeOfData;
        pMeasuring->tmClientSendTime     = _clock->GetTime();
        pMeasuring->tmServerReceivedTime = 0;
        pMeasuring->szMessageCount       = nCount;
        pMeasuring->nNumber              = 0xDEADBEAF;

        sStatistic.nMessageCount++;
        //5 seconds timeout
        //using synchrony sending
        tResult nRes = m_pMessageBus->Send(m_strMyChannelName.GetPtr(),
                                            MEASUREMESSAGE,
                                            oBlock.GetPtr(),
                                            (tSize)oBlock.GetSize(),
                                            oBlockReply.GetPtr(),
                                            (tSize)oBlockReply.GetSize(),
                                            NULL,
                                            5000000);
        tTimeStamp tmCurrentTime = _clock->GetTime();
        LOG_INFO(cString::Format("Send message with result = %d", nRes).GetPtr());
        if (IS_FAILED(nRes))
        {
            LOG_ERROR("NetIO-Sender: Send to server (and wait for reply-message) "
                      " failed. Loss of measure data ! Due to timeouts etc., "
                      "the Statistic-Results may be incorrect !!");
        }
        else
        {
            tTimeStamp tmTimeRange = tmCurrentTime - pMeasuring->tmClientSendTime;
            if (sStatistic.nMessages == 0)
            {
                 sStatistic.tmMaxTimeBack = tmTimeRange;
                 sStatistic.tmMinTimeBack = tmTimeRange;
            }
            else
            {
                if (tmTimeRange > sStatistic.tmMaxTimeBack)
                {
                    sStatistic.tmMaxTimeBack = tmTimeRange;
                }
                if (tmTimeRange < sStatistic.tmMinTimeBack)
                {
                    sStatistic.tmMinTimeBack = tmTimeRange;
                }

            }
            sStatistic.tmAll += tmTimeRange;
            sStatistic.nMessages++;
        }
        if ((tmCurrentTime - sStatistic.tmOverallBeginTime) >
            (m_nMessureTime * 1000000))
        {
            LogStatistics(sStatistic, tmCurrentTime);
            return ERR_END_OF_FILE;
        }
        return nRes;
    }

    RETURN_NOERROR;
}

tResult cDemoNetIOFilter::Run(tInt nActivationCode,
                              const tVoid* pvUserData,
                              tInt szUserDataSize,
                              ucom::IException** __exception_ptr)
{
    if (nActivationCode == IRunnable::RUN_MESSAGE
        && pvUserData != NULL
        && sizeof(IChannel::tMessage) == szUserDataSize)
    {
        IChannel::tMessage* pMessage = (IChannel::tMessage*) pvUserData;
        cString strMessageId("");
        if (pMessage->strMessage != NULL)
        {
            strMessageId = pMessage->strMessage;
        }
        if (strMessageId.IsEqual(MEASUREMESSAGE))
        {
            if (m_bServer)
            {
                if (pMessage->szDataReceive > sizeof(tMeasuringMessage))
                {
                    tMeasuringMessage* pMeasureMessage =
                        (tMeasuringMessage*)pMessage->pDataReceive;

                    if (pMeasureMessage->ui8Platform != m_ui8Platform)
                    {
                        pMeasureMessage->ui8Platform = m_ui8Platform;
                        cSystem::SwapEndian((tUInt64*)&pMeasureMessage->tmClientSendTime);
                        cSystem::SwapEndian((tUInt64*)&pMeasureMessage->szSizeOfData);
                        cSystem::SwapEndian((tUInt64*)&pMeasureMessage->tmServerReceivedTime);
                        cSystem::SwapEndian((tUInt64*)&pMeasureMessage->szMessageCount);
                    }

                    if (m_nLastMeasuringSize != (tSize)pMeasureMessage->szSizeOfData)
                    {
                        m_nLastMeasuringSize = (tSize)pMeasureMessage->szSizeOfData;
                        LOG_INFO(cString::Format("NetIO-Receiver: Start measuring "
                                 "capacity for size %lld bytes",
                                 tInt64(m_nLastMeasuringSize)).GetPtr());
                    }

                    LOG_INFO(cString::Format("LLLLLLAAAAAAA %X", pMeasureMessage->nNumber).GetPtr());

                    cMemoryBlock oBlock(pMessage->szDataReceive);
                    tMeasuringMessage* pRespondMeasureMessage
                        = (tMeasuringMessage*)oBlock.GetPtr();
                    pRespondMeasureMessage->szMessageCount
                        = pMeasureMessage->szMessageCount;
                    pRespondMeasureMessage->tmServerReceivedTime
                        = pMessage->tmReceiveTime;
                    pRespondMeasureMessage->tmClientSendTime
                        = pMeasureMessage->tmClientSendTime;
                    pRespondMeasureMessage->szSizeOfData
                        = pMeasureMessage->szSizeOfData;
                    tResult nRes = m_pMessageBus->Send(pMessage->strChannel,
                                                       pMessage->strMessageReply,
                                                       oBlock.GetPtr(),
                                                       (tSize)oBlock.GetSize());
                    if (IS_FAILED(nRes))
                    {
                        LOG_ERROR("NetIO-Receiver: Sending reply-message "
                                  "back to sender failed !");
                        return(nRes);
                    }
                }
                else
                {
                    LOG_ERROR("NetIO-Receiver: Something is totally wrong here");
                }
            }
            else
            {
                /*
                LOG_ERROR("this is a client, another client want connect, " \
                          "check your configuration for property 'is_server'");
                */
            }
        }
    }
    RETURN_NOERROR;
}

tResult cDemoNetIOFilter::LogStatistics(tMeasureStatistics& sStatistic,
                                        tTimeStamp& tmCurrentTime)
{
    LOG_INFO(cString::Format("NetIO-Sender: Test period %d done. Statistics "
                             "for messages with size of %lld Bytes (%.0lf KB):",
                             m_nCurrentPeriod,
                             tInt64(sStatistic.szSizeOfData),
                             tFloat64(sStatistic.szSizeOfData) / 1024.0).GetPtr());

    if (sStatistic.nMessages == 0)
    {
        LOG_INFO(cString::Format("    => ALL messages lost !!!  "
                                 "No Statistic-Results available.").GetPtr());
    }
    else if (sStatistic.nMessages != sStatistic.nMessageCount)
    {
        LOG_INFO(cString::Format("    => Messages got lost !!!  Due to "
                                 "timeouts etc., "
                                 " Statistic-Results may be incorrect!",
                                 sStatistic.nMessageCount -sStatistic.nMessages).GetPtr());
    }

    if (sStatistic.nMessages > 0)
    {
        tTimeStamp tmAverage = sStatistic.tmAll / sStatistic.nMessages;

        LOG_INFO(cString::Format("        %d of %d Messages got lost (roundtrip: "
                                 "send+reply): ",
                                 sStatistic.nMessageCount - sStatistic.nMessages,
                                 sStatistic.nMessageCount).GetPtr());

        LOG_INFO(cString::Format("        Real generate rate: %d messages per second",
                                 sStatistic.nMessageCount
                                 / ((tmCurrentTime - sStatistic.tmOverallBeginTime)
                                 / 1000000)).GetPtr());

        LOG_INFO(cString::Format("        Latency: Min / Max / Average: %lld "
                                 "/ %lld / %lld  in microseconds ",
                                 sStatistic.tmMinTimeBack,
                                 sStatistic.tmMaxTimeBack,
                                 tmAverage).GetPtr());

        tFloat64 szAveragePerSecondInKB = (tFloat64)((sStatistic.nMessages * 2)
                                          // *2, because of send+reply the message
                                          * sStatistic.szSizeOfData);
        tFloat64 szAveragePerSecondInKBit = szAveragePerSecondInKB * 8;
        tFloat64 tmRangeTime = static_cast<tFloat64>(sStatistic.tmAll) / 1000000;
        if (tmRangeTime > 0)
        {
            szAveragePerSecondInKB   = szAveragePerSecondInKB / tmRangeTime;
            szAveragePerSecondInKBit = szAveragePerSecondInKBit / tmRangeTime;
        }
        szAveragePerSecondInKBit = szAveragePerSecondInKBit / 1000;
        szAveragePerSecondInKB = szAveragePerSecondInKB / 1024;
        LOG_INFO(cString::Format("        Network usage time: %lld "
                                 "microseconds (%.03lf s)... %lld KBytes/s (%lld KBit/s)",
                                 sStatistic.tmAll,
                                 (static_cast<tFloat64>(sStatistic.tmAll))
                                 / 1000000.0,
                                 tInt64(szAveragePerSecondInKB),
                                 tInt64(szAveragePerSecondInKBit)).GetPtr());

        szAveragePerSecondInKB = (tFloat64)((sStatistic.nMessages * 2) *
                                             sStatistic.szSizeOfData);
        szAveragePerSecondInKBit = szAveragePerSecondInKB * 8;
        tmRangeTime = static_cast<tFloat64>(tmCurrentTime - sStatistic.tmOverallBeginTime)
                      / 1000000;
        if (tmRangeTime > 0)
        {
            szAveragePerSecondInKB   = szAveragePerSecondInKB / tmRangeTime;
            szAveragePerSecondInKBit = szAveragePerSecondInKBit / tmRangeTime;
        }
        szAveragePerSecondInKBit = szAveragePerSecondInKBit / 1000;
        szAveragePerSecondInKB = szAveragePerSecondInKB / 1024;
        LOG_INFO(cString::Format("        Current period time: %lld "
                                 "microseconds (%.03lf s) ... %lld KBytes/s (%lld KBit/s)",
                                 tmCurrentTime - sStatistic.tmOverallBeginTime,
                                 (static_cast<tFloat64>
                                 (tmCurrentTime - sStatistic.tmOverallBeginTime))
                                 / 1000000.0,
                                 tInt64(szAveragePerSecondInKB),
                                 tInt64(szAveragePerSecondInKBit)).GetPtr());
    }

    RETURN_NOERROR;
}


