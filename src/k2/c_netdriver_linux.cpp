// (C)2004 S2 Games
// c_netdriver_linux.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef linux
#include <linux/sockios.h>
#endif
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "c_netdriver.h"
#include "c_packet.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CNetDriver NetDriver;

CVAR_STRING(	net_forceIP,			"");
CVAR_STRING(	net_broadcastInterface,	"");

string g_sBroadcastInterface;

//=============================================================================


/*====================
  CNetDriver::CNetDriver
  ====================*/
CNetDriver::CNetDriver() :
m_pAddrLocal(K2_NEW(g_heapNet, inetAddr_t)),
m_bBlockIncoming(false),
m_bBlockOutgoing(false)
{
	if (m_pAddrLocal == NULL)
		EX_FATAL(_T("CNetDriver::CNetDriver() - Failed to allocate m_pAddrLocal"));
}


/*====================
  CNetDriver::Initialize
  ====================*/
void	CNetDriver::Initialize()
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
			bool bFoundInternetIP(false);
			bool bFoundLocalIP(false);
			struct ifaddrs *ifastart, *ifa;
			if (getifaddrs(&ifastart) == -1)
				EX_ERROR(_T("getifaddrs() failed: ") + K2System.GetErrorString(errno));

			int i(0);
			for (ifa = ifastart; ifa; ifa = ifa->ifa_next)
			{
				if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
					continue;
				struct sockaddr_in *addr = (sockaddr_in*)ifa->ifa_addr;
				char *szIP(inet_ntoa(addr->sin_addr));
				Console.Net << _T("Local IP # ") << XtoA(++i) << _T(" ") << ParenStr(StringToTString(string(ifa->ifa_name))) << _T(": ") << StringToTString(string(szIP)) << newl;
				if (strncmp(szIP, "127.", 4) == 0 && i > 1)
					continue;
				
				if (!bFoundInternetIP)
					MemManager.Copy(m_pAddrLocal, &addr->sin_addr, sizeof(inetAddr_t));
				
				if (!bFoundLocalIP)
					g_sBroadcastInterface = ifa->ifa_name;
				
				// prefer an internet address over private network address
				if ((strncmp(szIP, "192.168.", 8) != 0) // private 192.168.0.0/16
					&& !((strncmp(szIP, "172.", 4) == 0) // private 172.16.0.0/12
					&& ((szIP[4] > '1') || (szIP[4] == '1' && szIP[5] >= '6')))
					&& (strncmp(szIP, "10.", 3) != 0) // private 10.0.0.0/8
					&& (strncmp(szIP, "127.", 4) != 0)) // loopback
					bFoundInternetIP = true;
				
				// use first local ip address for broadcast (for LAN server discovery)
				if ((strncmp(szIP, "192.168.", 8) == 0) // private 192.168.0.0/16
					|| ((strncmp(szIP, "172.", 4) == 0) // private 172.16.0.0/12
					&& ((szIP[4] > '1') || (szIP[4] == '1' && szIP[5] >= '6')))
					|| (strncmp(szIP, "10.", 3) == 0)) // private 10.0.0.0/8
					bFoundLocalIP = true;
			}
			freeifaddrs(ifastart);
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("CNetDriver::Initialize() - "), NO_THROW);
	}
}


/*====================
  CNetDriver::Shutdown
  ====================*/
void	CNetDriver::Shutdown()
{
	Console << _T("Network Shutdown") << newl;
}


/*====================
  CNetDriver::CloseConnection
  ====================*/
bool	CNetDriver::CloseConnection(dword dwSocket)
{
	return (close(dwSocket) == 0);
}


/*====================
  CNetDriver::NewSocket
  ====================*/
bool	CNetDriver::NewSocket(dword &dwSocket, ESocketType eType, bool bBlock)
{
    int sock;

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

		if (sock == -1)
			EX_ERROR(_T("socket() failure: ") + K2System.GetErrorString(errno));

		if (!bBlock)
		{
			long flags;
			if ((flags = fcntl(dwSocket, F_GETFL, 0)) == -1)
				flags = 0;

			if (fcntl(dwSocket, F_SETFL, flags | O_NONBLOCK) == -1)
				EX_ERROR(_T("fcntl() failure: ") + K2System.GetErrorString(errno));
		}

		// set sockets to close on exec
		long flags;
		if ((flags = fcntl(dwSocket, F_GETFD, 0)) == -1)
			flags = 0;

		if (fcntl(dwSocket, F_SETFD, flags | FD_CLOEXEC) == -1)
			EX_ERROR(_T("fcntl() failure: ") + K2System.GetErrorString(errno));

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
bool	CNetDriver::OpenPort(dword &dwSocket, word &wRequestedPort, ESocketType eType, bool bBlocking, uint uiSendBuffer, uint uiRecvBuffer)
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
			in_addr_t address;
			address = inet_addr(TStringToString(net_forceIP).c_str());
			MemManager.Copy(&addr.sin_addr, &address, sizeof(in_addr_t));
		}
		addr.sin_port = htons(wRequestedPort);

		if (!NewSocket(dwSocket, eType, bBlocking))
			EX_ERROR(_T("Socket creation failed"));

		if (bind(dwSocket, (sockaddr*)&(addr), sizeof(netAddr_t)) == -1)
			EX_ERROR(_T("bind() failed: ") + K2System.GetErrorString(errno));

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
bool	CNetDriver::CheckTCPBuffer(IBuffer &cBuffer, CPacket &pkt)
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
		char *pResizeBuffer = K2_NEW_ARRAY(global, char, cBuffer.GetLength() - 2 - unLength);

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
int		CNetDriver::ReceivePacket(ESocketType eType, dword dwSocket, CPacket &pkt, tstring &sAddrName, word &wPort, IBuffer &cTCPBuffer)
{
	pkt.Clear();
	
	if (CheckTCPBuffer(cTCPBuffer, pkt))
		return 1;

	netAddr_t from;
	socklen_t fromlen(sizeof(sockAddr_t));
	char cBuffer[HEADER_SIZE + MAX_PACKET_SIZE + sizeof(ushort)];
	
	int iBytesRead(recvfrom(dwSocket, cBuffer, MAX_PACKET_SIZE + (eType == K2_SOCKET_GAME ? HEADER_SIZE : 0) + (eType == K2_SOCKET_TCP2 ? sizeof(ushort) : 0), 0, (sockAddr_t*)&from, &fromlen));
	
	// Check for errors
	if (iBytesRead < 0)
	{
		if (errno != EAGAIN)
		{
			tstring sAddr;
			StrToTString(sAddr, inet_ntoa(from.sin_addr));
			Console.Net << _T("recvfrom() failed: (") << errno << _T(") - ") << K2System.GetErrorString(errno) << _T(" [") << sAddr << _T("]") << newl;
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
			Console.Net << _T("Bad header on packet") << newl;
			return -1;
		}

		SPacketHeader *pHeader((SPacketHeader*)cBuffer);
		if (pHeader->m_yFlags & PACKET_RELIABLE)
		{
			iHeaderSize += 4;
			if (iBytesRead < iHeaderSize)
			{
				Console.Net << _T("Bad header on packet") << newl;
				return -1;
			}

			pkt = CPacket(&cBuffer[iHeaderSize], iBytesRead - iHeaderSize);
			pkt.SetHeader(LittleShort(pHeader->m_unConnectionID), pHeader->m_yFlags, LittleInt(pHeader->m_uiSequence));
		}
		else
		{
			pkt = CPacket(&cBuffer[iHeaderSize], iBytesRead - iHeaderSize);
			pkt.SetHeader(LittleShort(pHeader->m_unConnectionID), pHeader->m_yFlags);
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
tstring	CNetDriver::GetBroadcastAddress(dword dwSocket)
{
	tstring sAddr(_T("255.255.255.255"));
	struct ifreq ifr;
	netAddr_t* pNetAddr;

	// linux/OS X only broadcasts over 1 interface.  The interface with the first local ip (192.168.0.0/16, 172.16.0.0/12, 10.0.0.0/8)
	//   found is used for broadcast packets. The user can override this selection and broadcast on another by setting net_broadcastInterface
	if (net_broadcastInterface.empty())
		strncpy(ifr.ifr_name, g_sBroadcastInterface.c_str(), IFNAMSIZ);
	else
		strncpy(ifr.ifr_name, TStringToString(net_broadcastInterface).c_str(), IFNAMSIZ);

	if (ioctl(dwSocket, SIOCGIFBRDADDR, &ifr) == -1)
		return sAddr;

	pNetAddr = (netAddr_t*)&ifr.ifr_broadaddr;
	sAddr = StringToTString(inet_ntoa(pNetAddr->sin_addr));

	return sAddr;
}


/*====================
  CNetDriver::SendPacket
  ====================*/
size_t	CNetDriver::SendPacket(dword dwSocket, const void *pVoidAddr, const CPacket &packet, bool bIncludeHeader)
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
	if (ret == -1)
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			Console.Net << _T("CNetDriver::SendPacket() - ") << K2System.GetErrorString(errno) << newl;

		return 0;
	}

	return ret;
}


/*====================
  CNetConnection::SetSendAddr
  ====================*/
bool	CNetDriver::SetSendAddr(tstring &sAddr, word &wPort, void *&pVoidNetAddr, bool &bIsLocalConnection, bool &bIsLANConnection)
{
	try
	{
		// Clear the old NetAddr struct if it exists and allocate a new one
		if (pVoidNetAddr != NULL)
			K2_DELETE((netAddr_t*)pVoidNetAddr);
		netAddr_t *pNetAddr(K2_NEW(g_heapNet,  netAddr_t));
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
			{
				tstring sError;
				switch (h_errno)
				{
					case HOST_NOT_FOUND:	sError = _T("Unknown Host"); break;
					case TRY_AGAIN:			sError = _T("Host name lookup failure"); break;
					case NO_RECOVERY:		sError = _T("Unknown server error"); break;
					case NO_DATA:			sError = _T("No address associated with name"); break;
					default:				sError = K2System.GetErrorString(h_errno);
				}
				EX_ERROR(_T("Failed to resolve address: ") + sError);
			}

			MemManager.Copy(&pNetAddr->sin_addr, pHP->h_addr, pHP->h_length);
			pNetAddr->sin_family = pHP->h_addrtype;
		}

		sAddr = StringToTString(inet_ntoa(pNetAddr->sin_addr));
		pNetAddr->sin_port = htons(wPort);

		tsvector vIPBytes;
		vIPBytes = TokenizeString(sAddr, '.');

		if (sAddr == _T("127.0.0.1") || sAddr == _T("localhost"))
			bIsLocalConnection = true;
		else
			bIsLocalConnection = false;

		if ((vIPBytes[0] == _T("192") && vIPBytes[1] == _T("168")) ||
			(vIPBytes[0] == _T("172") && AtoI(vIPBytes[1]) >= 16 && AtoI(vIPBytes[1]) <= 30) ||
			(vIPBytes[0] == _T("10")))
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
void	CNetDriver::FreeSendAddr(void *&pVoidNetAddr)
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
bool	CNetDriver::Connected(dword dwSocket, uint uiMSecToWait)
{
	if (dwSocket == SOCKET_INVALID)
		return false;
	
	fd_set fdSocketSet;
	timeval timeWait;

	FD_ZERO(&fdSocketSet);
	FD_SET(dwSocket, &fdSocketSet);

	timeWait.tv_sec = uint(uiMSecToWait / 1000);
	timeWait.tv_usec = (uiMSecToWait % 1000) * 1000;

#ifdef linux
	// unconnected sockets are still writable...
	if ((select(dwSocket+1, NULL, &fdSocketSet, NULL, &timeWait) < 0)
		|| (send(dwSocket, NULL, 0, MSG_NOSIGNAL) < 0))
		return false;
	else
		return true;
#else
	return (select(dwSocket+1, NULL, &fdSocketSet, NULL, &timeWait) > 0 ? true : false);
#endif
}


/*====================
  CNetDriver::HasError
  ====================*/
bool	CNetDriver::HasError(dword dwSocket, uint uiMSecToWait)
{
	if (dwSocket == SOCKET_INVALID)
		return true;
	
	fd_set fdSocketSet;
	timeval timeWait;

	FD_ZERO(&fdSocketSet);
	FD_SET(dwSocket, &fdSocketSet);

	timeWait.tv_sec = uint(uiMSecToWait / 1000);
	timeWait.tv_usec = (uiMSecToWait % 1000) * 1000;

	return (select(dwSocket+1, NULL, NULL, &fdSocketSet, &timeWait) > 0 ? true : false);
}


/*====================
  CNetDriver::DataWaiting
  ====================*/
bool	CNetDriver::DataWaiting(dword dwSocket, IBuffer &cTCPBuffer, uint uiMSecToWait)
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
	
	fd_set fdSocketSet;
	timeval timeWait;

	FD_ZERO(&fdSocketSet);
	FD_SET(dwSocket, &fdSocketSet);

	timeWait.tv_sec = uint(uiMSecToWait / 1000);
	timeWait.tv_usec = (uiMSecToWait % 1000) * 1000;

	return (select(dwSocket+1, &fdSocketSet, NULL, NULL, &timeWait) > 0 ? true : false);
}


/*====================
  CNetDriver::StartListening
  ====================*/
bool	CNetDriver::StartListening(dword &dwSocket, int iNumConnWaiting)
{
	return (listen(dwSocket, iNumConnWaiting) == 0 ? true : false);
}


/*====================
  CNetDriver::AcceptConnection
  ====================*/
dword	CNetDriver::AcceptConnection(dword dwFromSocket)
{
	return (dword)accept(dwFromSocket, NULL, NULL);
}


/*====================
  CNetDriver::Connect
  ====================*/
bool	CNetDriver::Connect(dword dwSocket, void *&pVoidNetAddr)
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
			if (errno != EINPROGRESS)
				EX_ERROR(_T("Could not initialize connection: ") + K2System.GetErrorString(errno));
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
void	CNetDriver::Flush(ushort unSocket)
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
void	CNetDriver::SetSocketOptions(dword dwSocket, ESocketType eType, int iOption, const string &sOptVal)
{
	if (eType == K2_SOCKET_TCP || eType == K2_SOCKET_TCP2)
		setsockopt(dwSocket, IPPROTO_TCP, iOption, sOptVal.c_str(), (socklen_t)sOptVal.length());
	else
		setsockopt(dwSocket, IPPROTO_UDP, iOption, sOptVal.c_str(), (socklen_t)sOptVal.length());
}


/*====================
  CNetDriver::AllowBroadcast
  ====================*/
void	CNetDriver::AllowBroadcast(dword dwSocket, bool bValue)
{
	int iValue(bValue);
	setsockopt(dwSocket, SOL_SOCKET, SO_BROADCAST, (char *)&iValue, sizeof(iValue));
}


/*====================
  CNetDriver::SetSendBuffer
  ====================*/
void	CNetDriver::SetSendBuffer(dword dwSocket, uint uiSendBuffer)
{
	setsockopt(dwSocket, SOL_SOCKET, SO_SNDBUF, (char *)&uiSendBuffer, sizeof(uiSendBuffer));
}


/*====================
  CNetDriver::SetRecvBuffer
  ====================*/
void	CNetDriver::SetRecvBuffer(dword dwSocket, uint uiRecvBuffer)
{
	setsockopt(dwSocket, SOL_SOCKET, SO_RCVBUF, (char *)&uiRecvBuffer, sizeof(uiRecvBuffer));
}

/*====================
  CNetDriver::SetBlockIncoming
  ====================*/
void	CNetDriver::SetBlockIncoming(bool b)
{
	m_bBlockIncoming = b;
}


/*====================
  CNetDriver::SetBlockOutgoing
  ====================*/
void	CNetDriver::SetBlockOutgoing(bool b)
{
	m_bBlockOutgoing = b;
}
