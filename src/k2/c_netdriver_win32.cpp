// (C)2004 S2 Games
// c_netdriver_win32.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <winsock2.h>
#include <ws2tcpip.h>

#include "c_netdriver.h"
#include "c_packet.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CNetDriver NetDriver;

CVAR_STRING(    net_forceIP,    "");

struct SWinsockServer
{
    WSADATA         wsaData;
    uint    nonblock;
}
g_WinsockServer;
//=============================================================================

/*====================
  CNetDriver::CNetDriver
  ====================*/
CNetDriver::CNetDriver() :
m_pAddrLocal(K2_NEW(ctx_Net,  inetAddr_t)),
m_bBlockIncoming(false),
m_bBlockOutgoing(false)
{
    if (m_pAddrLocal == NULL)
        EX_FATAL(_T("CNetDriver::CNetDriver() - Failed to allocate m_pAddrLocal"));
}


/*====================
  CNetDriver::Initialize
  ====================*/
void    CNetDriver::Initialize()
{
    try
    {
        Console << _T("Network Initialization") << newl;

        if (!net_forceIP.empty())
        {
            string sAddr(TStringToString(net_forceIP));
            unsigned long ulAddr(inet_addr(sAddr.c_str()));
            MemManager.Copy(m_pAddrLocal, (void *)&ulAddr, sizeof(inetAddr_t));
        }
        else
        {
            if (WSAStartup(0x202, &g_WinsockServer.wsaData) == SOCKET_ERROR)
                EX_ERROR(_T("WSAStartup() failed: ") + K2System.GetErrorString(WSAGetLastError()));

            char szAddr[256];
            if (gethostname(szAddr, 255) == SOCKET_ERROR)
                EX_ERROR(_T("gethostname() failed: ") + K2System.GetErrorString(WSAGetLastError()));
            Console.Net << _T("Host is: '") << szAddr << _T("'") << newl;

            int bestip(0);
            struct hostent *pHE(gethostbyname(szAddr));
            if (pHE != NULL)
            {
                for (int i(0); pHE->h_addr_list[i] != 0; ++i)
                {
                    MemManager.Copy(m_pAddrLocal, pHE->h_addr_list[i], sizeof(inetAddr_t));
                    char *szIP(inet_ntoa(*m_pAddrLocal));
                    Console.Net << _T("Local IP # ") << i + 1 << _T(": ") << szIP << newl;

                    // FIXME: this is a hack in case we're shared off the connection
                    // to the internet.  This won't work all the time so find a better way
                    if (strncmp(szIP, "192.168.", 8) != 0)
                        bestip = i;
                }
            }
            else
            {
                Console.Net << _T("Unable to find Local IP for '") << szAddr << _T("'") << newl;
            }
        }

        g_WinsockServer.nonblock = 1;
    }
    catch (CException &ex)
    {
        WSACleanup();
        ex.Process(_T("CNetDriver::Initialize() - "), NO_THROW);
    }
}


/*====================
  CNetDriver::Shutdown
  ====================*/
void    CNetDriver::Shutdown()
{
    Console << _T("Network Shutdown") << newl;
    WSACleanup();

    K2_DELETE(m_pAddrLocal);
}


/*====================
  CNetDriver::CloseConnection
  ====================*/
bool    CNetDriver::CloseConnection(dword dwSocket)
{
    return closesocket(dwSocket) == 0;
}


/*====================
  CNetDriver::NewSocket
  ====================*/
bool    CNetDriver::NewSocket(dword &dwSocket, ESocketType eType, bool bBlock)
{
    SOCKET sock;

    try
    {
        if (dwSocket)
            CloseConnection(dwSocket);

        // Make sure we use the correct type for our protocol
        switch (eType)
        {
        case K2_SOCKET_GAME:
        case K2_SOCKET_UDP:
            sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            break;

        case K2_SOCKET_TCP:
        case K2_SOCKET_TCP2:
            sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            break;

        default:
            EX_ERROR(_T("Invalid socket type"));
            break;
        }

        dwSocket = dword(sock);

        if (sock == SOCKET_INVALID)
            EX_ERROR(_T("socket() failure: ") + K2System.GetErrorString(WSAGetLastError()));

        uint uiBlock(!bBlock);
        if (ioctlsocket(dwSocket, FIONBIO, (ULONG*)&uiBlock) == SOCKET_ERROR)
            EX_ERROR(_T("ioctlsocket() failure: ") + WSAGetLastError());

        return true;
    }
    catch (CException &ex)
    {
        CloseConnection(dwSocket);
        ex.Process(_T("CNetDriver::NewSocket() - "), NO_THROW);
        return false;
    }
}


/*====================
  CNetDriver::OpenPort
  ====================*/
bool    CNetDriver::OpenPort(dword &dwSocket, word &wRequestedPort, ESocketType eType, bool bBlocking, uint uiSendBuffer, uint uiRecvBuffer)
{
    try
    {
        netAddr_t addr;
        MemManager.Set(&addr, 0, sizeof(netAddr_t));
        addr.sin_family = AF_INET;
        if (net_forceIP.empty())
        {
            addr.sin_addr.s_addr = INADDR_ANY;
        }
        else
        {
            unsigned long address;
            address = inet_addr(TStringToString(net_forceIP).c_str());
            MemManager.Copy(&addr.sin_addr, &address, sizeof(unsigned long));
        }
        addr.sin_port = htons(wRequestedPort);

        if (!NewSocket(dwSocket, eType, bBlocking))
            EX_ERROR(_T("Socket creation failed"));

        if (bind(dwSocket, (sockaddr*)&(addr), sizeof(netAddr_t)) == SOCKET_ERROR)
            EX_ERROR(_T("bind() failed: ") + K2System.GetErrorString(WSAGetLastError()));

        socklen_t namelen = sizeof(netAddr_t);
        getsockname(dwSocket, (sockaddr*)(&addr), &namelen);
        wRequestedPort = ntohs(addr.sin_port);

        if (uiSendBuffer != -1)
            setsockopt(dwSocket, SOL_SOCKET, SO_SNDBUF, (char *)&uiSendBuffer, sizeof(uiSendBuffer));
        if (uiRecvBuffer != -1)
            setsockopt(dwSocket, SOL_SOCKET, SO_RCVBUF, (char *)&uiRecvBuffer, sizeof(uiRecvBuffer));

        return true;
    }
    catch (CException &ex)
    {
        CloseConnection(dwSocket);
        ex.Process(_T("CNetDriver::OpenPort() - "), NO_THROW);
        return false;
    }
}


/*====================
  CNetDriver::CheckTCPBuffer

  returns true if we created a packet
  ====================*/
bool    CNetDriver::CheckTCPBuffer(IBuffer &cBuffer, CPacket &pkt)
{
    if (cBuffer.GetLength() < 2)
        return false;

    const char *pBuffer(cBuffer.Get());

    ushort unLength(ntohs(*(ushort *)pBuffer));

    if (unLength > cBuffer.GetLength() - 2)
        return false;

    //Console << _T("Buffered packet: ") << unLength << _T(" / ") << cBuffer.GetLength() << newl;

    pkt = CPacket(pBuffer + 2, unLength);

    if (int(cBuffer.GetLength()) - 2 - unLength > 0)
    {
        char *pResizeBuffer = K2_NEW_ARRAY(ctx_Net, char, cBuffer.GetLength() - 2 - unLength);

        MemManager.Copy(pResizeBuffer, cBuffer.Get(2 + unLength), cBuffer.GetLength() - 2 - unLength);

        cBuffer.Write(pResizeBuffer, cBuffer.GetLength() - 2 - unLength);

        SAFE_DELETE_ARRAY(pResizeBuffer);
    }
    else
        cBuffer.Clear();

    return true;
}


/*====================
  CNetDriver::ReceivePacket
  ====================*/
int     CNetDriver::ReceivePacket(ESocketType eType, dword dwSocket, CPacket &pkt, wstring &sAddrName, word &wPort, IBuffer &cTCPBuffer)
{
    pkt.Clear();

    if (CheckTCPBuffer(cTCPBuffer, pkt))
        return 1;

    netAddr_t from;
    socklen_t fromlen(sizeof(sockAddr_t));
    char cBuffer[HEADER_SIZE + MAX_PACKET_SIZE + sizeof(ushort)];

    int iBytesRead(recvfrom(dwSocket, cBuffer, MAX_PACKET_SIZE + (eType == K2_SOCKET_GAME ? HEADER_SIZE : 0) + (eType == K2_SOCKET_TCP2 ? sizeof(ushort) : 0), 0, (sockAddr_t*)&from, &fromlen));

    // Check for errors
    if (iBytesRead == SOCKET_ERROR)
    {
        int iError(WSAGetLastError());

        // Don't treat "would block" as a failure
        if (iError != WSAEWOULDBLOCK)
        {
            tstring sAddr;
            StrToTString(sAddr, inet_ntoa(from.sin_addr));
            Console.Net << L"recvfrom() failed: (" << iError << L") - " << K2System.GetErrorString(iError) << L" [" << sAddr << L"]" << newl;
            return -1;
        }

        return 0;
    }

    if (iBytesRead == 0)
        return 0;

    if (m_bBlockIncoming)
        return 0;

    sAddrName = SingleToWide(inet_ntoa(from.sin_addr));
    wPort = ntohs(from.sin_port);

    if (eType == K2_SOCKET_GAME)
    {
        int iHeaderSize(3);
        if (iBytesRead < iHeaderSize)
        {
            Console.Net << L"Bad header on packet" << newl;
            return -1;
        }

        SPacketHeader *pHeader((SPacketHeader*)cBuffer);
        if (pHeader->m_yFlags & PACKET_RELIABLE)
        {
            iHeaderSize += 4;
            if (iBytesRead < iHeaderSize)
            {
                Console.Net << L"Bad header on packet" << newl;
                return -1;
            }

            pkt = CPacket(&cBuffer[iHeaderSize], iBytesRead - iHeaderSize);
            pkt.SetHeader(pHeader->m_unConnectionID, pHeader->m_yFlags, pHeader->m_uiSequence);
        }
        else
        {
            pkt = CPacket(&cBuffer[iHeaderSize], iBytesRead - iHeaderSize);
            pkt.SetHeader(pHeader->m_unConnectionID, pHeader->m_yFlags);
        }
    }
    else if (eType == K2_SOCKET_TCP2)
    {
        ushort unLength(iBytesRead >= 2 ? ntohs(*(ushort *)cBuffer) : USHRT_MAX);

        // If this stream update contains a whole packet,
        // process it immediately, otherwise store it for later.
        // Also, if there is already some data buffered, just continue to buffer the new data
        if (cTCPBuffer.IsEmpty() && unLength != USHRT_MAX && iBytesRead - 2 >= unLength)
        {
#if 0
            Console << _T("ReceivePacket New: ") << unLength << _T(" / ") << iBytesRead - 2 << newl;
#endif

            pkt = CPacket(cBuffer + 2, unLength);
        
            // Store any remainder
            if (iBytesRead - unLength - 2 > 0)
                cTCPBuffer.Append(cBuffer + unLength + 2, iBytesRead - unLength - 2);
        }
        else
        {
#if 0
            Console << _T("ReceivePacket More: ") << iBytesRead << newl;
#endif

            cTCPBuffer.Append(cBuffer, iBytesRead);

            if (CheckTCPBuffer(cTCPBuffer, pkt))
                return 1;
        }
    }
    else
    {
        pkt = CPacket(cBuffer, iBytesRead);
    }

    return iBytesRead;
}


/*====================
  CNetDriver::GetBroadcastAddress
  ====================*/
tstring CNetDriver::GetBroadcastAddress(dword dwSocket)
{
    netAddr_t netAddr;
    MemManager.Set(&netAddr, 0, sizeof(netAddr_t));

    DWORD dwBytesWritten;
    WSAIoctl(dwSocket, SIO_GET_BROADCAST_ADDRESS, NULL, NULL, &netAddr, sizeof(netAddr_t), &dwBytesWritten, NULL, NULL);

    return StringToTString(inet_ntoa(netAddr.sin_addr));
}


/*====================
  CNetDriver::SendPacket
  ====================*/
size_t  CNetDriver::SendPacket(dword dwSocket, const void *pVoidAddr, const CPacket &packet, bool bIncludeHeader)
{
    const sockAddr_t *pSockAddr(static_cast<const sockAddr_t*>(pVoidAddr));

    CBufferStatic buffer(packet.GetLength() + (bIncludeHeader ? HEADER_SIZE : 0));

    if (bIncludeHeader)
    {
        buffer << packet.GetConnectionID() << packet.GetFlags();
        if (packet.HasFlags(PACKET_RELIABLE))
            buffer << packet.GetSequence();
        buffer.Append(packet.GetBuffer(), packet.GetUnreadLength());
    }
    else
    {
        buffer.Write(packet.GetBuffer(), packet.GetUnreadLength());
    }

    if (m_bBlockOutgoing)
        return 0;

    int ret(sendto(dwSocket, buffer.Get(), buffer.GetLength(), 0, pSockAddr, sizeof(sockAddr_t)));
    if (ret == SOCKET_ERROR)
    {
        int err(WSAGetLastError());
        if (err != WSAEWOULDBLOCK)
            Console.Net << _T("CNetDriver::SendPacket() - ") << K2System.GetErrorString(err) << newl;

        return 0;
    }

    return ret;
}


/*====================
  CNetConnection::SetSendAddr
  ====================*/
bool    CNetDriver::SetSendAddr(tstring &sAddr, word &wPort, void *&pVoidNetAddr, bool &bIsLocalConnection, bool &bIsLANConnection)
{
    try
    {
        // Clear the old NetAddr struct if it exists and allocate a new one
        if (pVoidNetAddr != NULL)
        {
            K2_DELETE((netAddr_t *)pVoidNetAddr);
            pVoidNetAddr = NULL;
        }
    
        netAddr_t *pNetAddr(K2_NEW(ctx_Net,  netAddr_t));
        if (pNetAddr == NULL)
            EX_ERROR(_T("Failed to allocate a net address structure"));
        pVoidNetAddr = pNetAddr;
        MemManager.Set(pNetAddr, 0, sizeof(netAddr_t));

        if (sAddr.empty())
            EX_ERROR(_T("No address specified"));

        // Extract port and base ip address from sAddr
        size_t zColon(sAddr.find_first_of(_T(':')));
        if (zColon != tstring::npos)
        {
            uint uiPort(AtoI(sAddr.substr(zColon + 1)));
            if (uiPort > USHRT_MAX)
                EX_ERROR(_T("Invalid port"));
            wPort = ushort(uiPort);
            sAddr = sAddr.substr(0, zColon);
        }

        if (wPort == 0)
            EX_ERROR(_T("No port specified"));

        if (IsIPAddress(sAddr))
        {
            uint uint_addr;
            uint_addr = inet_addr(TStringToString(sAddr).c_str());
            MemManager.Copy(&pNetAddr->sin_addr, &uint_addr, 4);
            pNetAddr->sin_family = AF_INET;
        }
        else
        {
            struct hostent *pHP(gethostbyname(TStringToString(sAddr).c_str()));
            if (pHP == NULL)
                EX_ERROR(_T("Failed to resolve address:") + K2System.GetErrorString(WSAGetLastError()));

            MemManager.Copy(&pNetAddr->sin_addr, pHP->h_addr, pHP->h_length);
            pNetAddr->sin_family = pHP->h_addrtype;
        }

        sAddr = StringToTString(inet_ntoa(pNetAddr->sin_addr));
        pNetAddr->sin_port = htons(wPort);

        tsvector vIPBytes;
        vIPBytes = TokenizeString(sAddr, '.');

        if (pNetAddr->sin_addr.S_un.S_un_b.s_b1 == 127)
            bIsLocalConnection = true;
        else
            bIsLocalConnection = false;

        if ((pNetAddr->sin_addr.S_un.S_un_b.s_b1 == 192 && pNetAddr->sin_addr.S_un.S_un_b.s_b2 == 168) ||
            (pNetAddr->sin_addr.S_un.S_un_b.s_b1 == 172 && pNetAddr->sin_addr.S_un.S_un_b.s_b2 >= 16 && pNetAddr->sin_addr.S_un.S_un_b.s_b2 <= 31) ||
            (pNetAddr->sin_addr.S_un.S_un_b.s_b1 == 10))
            bIsLANConnection = true;
        else
            bIsLANConnection = false;

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CNetConnection::SetSendAddr() - "), NO_THROW);
        return false;
    }
}


/*====================
  CNetDriver::FreeSendAddr
  ====================*/
void    CNetDriver::FreeSendAddr(void *&pVoidNetAddr)
{
    if (pVoidNetAddr != NULL)
    {
        K2_DELETE((netAddr_t *)pVoidNetAddr);
        pVoidNetAddr = NULL;
    }
}


/*====================
  CNetDriver::Connected
  ====================*/
bool    CNetDriver::Connected(dword dwSocket, uint uiMSecToWait)
{
    if (dwSocket == SOCKET_INVALID)
        return false;

    uint uiStartTime(K2System.Milliseconds());

    do
    {
        fd_set fdSocketSet;
        timeval timeWait;

        FD_ZERO(&fdSocketSet);
        FD_SET(dwSocket, &fdSocketSet);

        timeWait.tv_sec = 0;
        timeWait.tv_usec = 0;

        if (select(NULL, NULL, &fdSocketSet, NULL, &timeWait) > 0)
            return true;

        if (uiMSecToWait == 0)
            return false;

        K2System.Sleep(1);
    }
    while(K2System.Milliseconds() - uiStartTime < uiMSecToWait);

    return false;
}


/*====================
  CNetDriver::HasError
  ====================*/
bool    CNetDriver::HasError(dword dwSocket, uint uiMSecToWait)
{
    if (dwSocket == SOCKET_INVALID)
        return true;

    uint uiStartTime(K2System.Milliseconds());

    do
    {
        fd_set fdSocketSet;
        timeval timeWait;

        FD_ZERO(&fdSocketSet);
        FD_SET(dwSocket, &fdSocketSet);

        timeWait.tv_sec = 0;
        timeWait.tv_usec = 0;

        if (select(NULL, NULL, NULL, &fdSocketSet, &timeWait) > 0)
            return true;

        if (uiMSecToWait == 0)
            return false;

        K2System.Sleep(1);
    }
    while (K2System.Milliseconds() - uiStartTime < uiMSecToWait);

    return false;
}


/*====================
  CNetDriver::DataWaiting
  ====================*/
bool    CNetDriver::DataWaiting(dword dwSocket, IBuffer &cTCPBuffer, uint uiWaitTime)
{
    if (dwSocket == SOCKET_INVALID)
        return false;

    if (cTCPBuffer.GetLength() >= 2)
    {
        const char *pBuffer(cTCPBuffer.Get());

        ushort unLength(ntohs(*(ushort *)pBuffer));

        if (cTCPBuffer.GetLength() - 2 >= unLength)
            return true;
    }

    uint uiStartTime(K2System.Milliseconds());

    do
    {
        fd_set fdSocketSet;
        timeval timeWait;

        FD_ZERO(&fdSocketSet);
        FD_SET(dwSocket, &fdSocketSet);

        timeWait.tv_sec = 0;
        timeWait.tv_usec = 0;

        int iReturn(select(NULL, &fdSocketSet, NULL, NULL, &timeWait));

        if (iReturn != SOCKET_ERROR && iReturn > 0)
            return true;

        if (uiWaitTime == 0)
            return false;

        K2System.Sleep(1);
    }
    while(K2System.Milliseconds() - uiStartTime < uiWaitTime);

    return false;
}


/*====================
  CNetDriver::StartListening
  ====================*/
bool    CNetDriver::StartListening(dword &dwSocket, int iNumConnWaiting)
{
    return (listen(dwSocket, iNumConnWaiting) == 0 ? true : false);
}


/*====================
  CNetDriver::AcceptConnection
  ====================*/
dword   CNetDriver::AcceptConnection(dword dwFromSocket)
{
    return (dword)accept(dwFromSocket, NULL, NULL);
}


/*====================
  CNetDriver::Connect
  ====================*/
bool    CNetDriver::Connect(dword dwSocket, void *&pVoidNetAddr)
{
    sockaddr *sockTargetAddr;
    int iError;

    try
    {
        if (pVoidNetAddr == NULL)
            EX_ERROR(_T("Net Address not initialized properly."));

        sockTargetAddr = ((sockaddr *)pVoidNetAddr);

        iError = connect(dwSocket, sockTargetAddr, sizeof(sockaddr));
        
        if (iError != 0)
        {
            iError = K2System.GetLastError();
            if (iError != WSAEWOULDBLOCK)
                EX_ERROR(_T("Could not initialize connection: ") + K2System.GetErrorString(WSAGetLastError()));
        }

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CNetConnection::Connect() - "), NO_THROW);
        return false;
    }
}


/*====================
  CNetDriver::Flush
  ====================*/
void    CNetDriver::Flush(ushort unSocket)
{
    try
    {
        char cBuffer[1];
        int iBytesRead(1);
        while (iBytesRead > 0)
            iBytesRead = recv(unSocket, cBuffer, 1, 0);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CNetDriver::ReceivePacket() - "), NO_THROW);
    }
}


/*====================
  CNetDriver::SetSocketOptions
  ====================*/
void    CNetDriver::SetSocketOptions(dword dwSocket, ESocketType eType, int iOption, const string &sOptVal)
{
    if (eType == K2_SOCKET_TCP || eType == K2_SOCKET_TCP2)
        setsockopt(dwSocket, IPPROTO_TCP, iOption, sOptVal.c_str(), (int)sOptVal.length());
    else
        setsockopt(dwSocket, IPPROTO_UDP, iOption, sOptVal.c_str(), (int)sOptVal.length());
}


/*====================
  CNetDriver::AllowBroadcast
  ====================*/
void    CNetDriver::AllowBroadcast(dword dwSocket, bool bValue)
{
    setsockopt(dwSocket, SOL_SOCKET, SO_BROADCAST, (char *)&bValue, sizeof(bool));
}


/*====================
  CNetDriver::SetSendBuffer
  ====================*/
void    CNetDriver::SetSendBuffer(dword dwSocket, uint uiSendBuffer)
{
    setsockopt(dwSocket, SOL_SOCKET, SO_SNDBUF, (char *)&uiSendBuffer, sizeof(uiSendBuffer));
}


/*====================
  CNetDriver::SetRecvBuffer
  ====================*/
void    CNetDriver::SetRecvBuffer(dword dwSocket, uint uiRecvBuffer)
{
    setsockopt(dwSocket, SOL_SOCKET, SO_RCVBUF, (char *)&uiRecvBuffer, sizeof(uiRecvBuffer));
}


/*====================
  CNetDriver::SetBlockIncoming
  ====================*/
void    CNetDriver::SetBlockIncoming(bool b)
{
    m_bBlockIncoming = b;
}


/*====================
  CNetDriver::SetBlockOutgoing
  ====================*/
void    CNetDriver::SetBlockOutgoing(bool b)
{
    m_bBlockOutgoing = b;
}
