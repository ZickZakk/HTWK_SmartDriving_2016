/**
 *
 * ADTF external data exchange communication example
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: FOX0IY4 $
 * $Date: 2013-10-15 15:04:11 +0200 (Tue, 15 Oct 2013) $
 * $Revision: 41718 $
 *
 * @remarks This example communicates from outside ADTF over the Messagebus/Dataexchange to an ADTF.
 *          It uses the UDP and TCP communication protocol. For detailed information please have a look into
 *          ADTF-SDK Documentation  (ADTF Extreme Programmers)
 *
 */
#include "stdafx.h"
#include <additional/dx_udp_extension_intf.h>
#include <additional/dx_tcp_extension_intf.h>

//#############################################################################
/**
 *
 * Main function of the program
 *
 * @param argc [in] Argument count
 * @param argv [in] Argument pointer
 * @return  Returns a standard result code.
 *
 */
int main(int argc, const char **argv)
{
    cCommandLine oCmd(argc, argv);

    // parameter check
    if (argc != 3 && argc != 4)
    {
        LOG_INFO("Usage: programme name -ip=DEST_IP -message=MSG_NAME "
                         "[-port=PORT_NB(otherwise 5000)] [-tcp]");
        return 1;
    }

    // name of the message e.g. inport-name inside ATDF
    cString strMessage = oCmd.GetProperty("message") + "_raw";
    cString strDestIp = oCmd.GetProperty("ip");
    cString strPort = oCmd.GetProperty("port");
    tInt nPort = 5000;

    if (strPort.IsNotEmpty())
    {
        nPort = strPort.AsInt();
    }

    // my user data
    cString strData = "myProcessData";
    tInt szSize = strData.GetLength();

    if (!oCmd.GetFlag("tcp"))
    {
        // use UDP

        // filling the header with information
        cMemoryBlock oBlock(ADTF_DX_UDP_MAX_PACKAGESIZE);
        tADTFDXUDPHeaderASync *psHeader = (tADTFDXUDPHeaderASync *) oBlock.GetPtr();

        psHeader->ui32StartBytes = ADTF_DX_UDP_STARTBYTES;
        psHeader->ui32SenderUID = 1;
        psHeader->tmTime = cSystem::GetTime();
        psHeader->ui8HeaderType = ADTF_DX_UDP_HEADER_TYPE_ASYNC;
        psHeader->ui32DataSize = szSize;

        cString::Copy(psHeader->strMsgId, strMessage, -1, ADTF_DX_UDP_MAX_MESSAGEID_LENGTH + 2);
        psHeader->ui32MsgSize = szSize;

        // copy data
        tVoid *pDataArea = (tVoid *) (((tUInt8 *) psHeader) + sizeof(tADTFDXUDPHeaderASync));
        cMemoryBlock::MemCopy(pDataArea, strData.GetPtr(), psHeader->ui32DataSize);

        /*!
        * socket information
        * IP 192.168.1.151 is the Destination Address. ADTF is running there
        */

        tUInt32 ui32Address = cSocket::AddressToUInt32(strDestIp);
        LOG_INFO("Destination IP: " + strDestIp);
        cDatagramSocket oSenderPort;
        oSenderPort.Open();

        // sending the data
        for (tInt n = 0; n < 20; n++)
        {
            LOG_INFO("Try to send data. " + cString::FromInt(n));
            if (IS_FAILED(oSenderPort.Write(ui32Address,
                                            nPort,
                                            oBlock.GetPtr(),
                                            sizeof(tADTFDXUDPHeaderASync) + psHeader->ui32DataSize)))
            {
                LOG_ERROR("Unable to send data");
                return 1;
            }
        }
    }
    else
    {
        //use TCP
        cStreamSocket oSocket;
        if (IS_FAILED(oSocket.Connect(strDestIp, ADTF_DX_TCP_CHANNEL_DEFAULT_PORT)))
        {
            LOG_ERROR("Unable to connect");
            return 1;
        }

        //calculate the overall size of the message
        tSize nPayloadSize = strData.GetLength() + 1;
        tADTFDXTCPMessageHeader sHeader;
        sHeader.nMessageNameSize = strMessage.GetLength() + 1;
        sHeader.nReplyMessageNameSize = 0;
        sHeader.nSize = sizeof(tADTFDXTCPMessageHeader) + sHeader.nMessageNameSize + (tUInt32) nPayloadSize;
        // important: convert to network byte order
        cSystem::HostToNetwork(&sHeader.nSize);

        for (tInt nMessage = 0; nMessage < 20; ++nMessage)
        {
            LOG_INFO("Try to send data. " + cString::FromInt(nMessage));

            // first send the header
            if (IS_FAILED(oSocket.Write(&sHeader, sizeof(sHeader))))
            {
                LOG_ERROR("Unable to send header");
                return 1;
            }

            // then the message name
            if (IS_FAILED(oSocket.Write(strMessage.GetPtr(), sHeader.nMessageNameSize)))
            {
                LOG_ERROR("Unable to send message name");
                return 1;
            }

            // then the payload (we do not send a reply message name)
            if (IS_FAILED(oSocket.Write(strData.GetPtr(), (tInt) nPayloadSize)))
            {
                LOG_ERROR("Unable to send payload");
                return 1;
            }
        }

        oSocket.Close();
    }
    return 0;
}
