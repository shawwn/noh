// (C)2010 S2 Games
// c_managerchatconnection.h
//
//=============================================================================
#ifndef __C_MANAGERCHATCONNECTION_H__
#define __C_MANAGERCHATCONNECTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_packet.h"

//=============================================================================
// Declarations
//=============================================================================
class CSocket;
//=============================================================================

//=============================================================================
// CManagerChatConnection
//=============================================================================
#ifndef K2_CLIENT
class CManagerChatConnection
{
private:
	enum EState
	{
		STATE_IDLE,
		STATE_DISCONNECTED,
		STATE_CONNECTING,
		STATE_AUTHENTICATING,
		STATE_CONNECTED
	};

	CSocket*		m_pSocket;
	CPacket			m_pktSend;

	wstring			m_sAddress;
	ushort			m_unPort;

	EState			m_eState;
	uint			m_uiReconnectTime;
	uint			m_uiTimeout;
	uint			m_uiLastReceiveTime;
	bool			m_bSentPing;

	uint			m_uiNextConnectReminder;

	void	ReadSocket();
	bool	HandleAccept(CPacket &pkt);
	bool	HandleReject(CPacket &pkt);

	void	Reconnect(uint uiTimeout);

	void	Handshake();

public:
	~CManagerChatConnection();
	CManagerChatConnection();
};
#endif //K2_CLIENT
//=============================================================================

#endif //__C_MANAGERCHATCONNECTION_H__
