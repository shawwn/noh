// (C)2004 S2 Games
// c_netdriver.h
//
//=============================================================================
#ifndef __C_NETDRIVER_H__
#define __C_NETDRIVER_H__

//=============================================================================
// Declarations
//=============================================================================
class CPacket;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef struct in_addr      inetAddr_t;
typedef struct sockaddr_in  netAddr_t;
typedef struct sockaddr     sockAddr_t;

enum ESocketType
{
    K2_SOCKET_GAME,
    K2_SOCKET_TCP,
    K2_SOCKET_UDP,
    K2_SOCKET_TCP2,

    K2_SOCKET_INVALID
};

#define SOCKET_INVALID  dword(-1)
//=============================================================================

//=============================================================================
// CNetDriver
//=============================================================================
class CNetDriver
{
private:
    inetAddr_t*     m_pAddrLocal;
    bool            m_bBlockIncoming;
    bool            m_bBlockOutgoing;

    bool    NewSocket(dword &dwSocket, ESocketType eType, bool bBlocking);
    void    AddHeader(char fullPacket[], byte flags, uint seq);

    bool    CheckTCPBuffer(IBuffer &cTCPBuffer, CPacket &pkt);

public:
    ~CNetDriver()   {}
    CNetDriver();

    void    Initialize();
    void    Shutdown();

    bool    OpenPort(dword &dwSocket, word &wRequestedPort, ESocketType eType, bool bBlocking, uint uiSendBuffer, uint uiRecvBuffer);
    bool    CloseConnection(dword dwSocket);
    int     ReceivePacket(ESocketType eType, dword dwSocket, CPacket &recv, tstring &sAddrName, word &wPort, IBuffer &cBuffer);
    size_t  SendPacket(dword dwSocket, const void *pVoidAddr, const CPacket &packet, bool bIncludeHeader);
    bool    SetSendAddr(tstring &sAddr, word &wPort, void *&pVoidNetAddr, bool &bIsLocalConnection, bool &bIsLANConnection);
    void    FreeSendAddr(void *&pVoidNetAddr);
    void    Flush(ushort unSocket);
    bool    Connect(dword dwSocket, void *&pVoidNetAddr);
    bool    Connected(dword dwSocket, uint uiMSecToWait);
    bool    DataWaiting(dword dwSocket, IBuffer &cTCPBuffer, uint uiWaitTime = 0);
    bool    HasError(dword dwSocket, uint uiMSecToWait = 0);
    dword   AcceptConnection(dword dwFromSocket);

    bool    StartListening(dword &dwSocket, int iNumConnWaiting);

    char    GetLocalIPByte(size_t z)    { assert(z < 4); return ((char*)m_pAddrLocal)[z]; } // HACK: This is not very safe

    void    SetSocketOptions(dword dwSocket, ESocketType eType, int iOption, const string &sOptVal);

    void    AllowBroadcast(dword dwSocket, bool bValue);
    tstring GetBroadcastAddress(dword dwSocket);

    void    SetSendBuffer(dword dwSocket, uint uiSendBuffer);
    void    SetRecvBuffer(dword dwSocket, uint uiRecvBuffer);

    void    SetBlockIncoming(bool b);
    void    SetBlockOutgoing(bool b);
};

extern CNetDriver NetDriver;
//=============================================================================

#endif //__C_NETDRIVER_H__
