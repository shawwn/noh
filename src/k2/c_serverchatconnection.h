// (C)2010 S2 Games
// c_serverchatconnection.h
//
//=============================================================================
#ifndef __C_SERVERCHATCONNECTION_H__
#define __C_SERVERCHATCONNECTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_packet.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSocket;
//=============================================================================

//=============================================================================
// CServerChatConnection
//=============================================================================
#ifndef K2_CLIENT
class CServerChatConnection
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

	CHostServer*	m_pHostServer;
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

	CServerChatConnection();

	void	ReadSocket();
	bool	HandleAccept(CPacket &pkt);
	bool	HandleReject(CPacket &pkt);
	bool	HandleCreateMatch(CPacket &pkt);
	bool	HandleRosterSubstitute(CPacket &pkt);
	bool	HandleRemoteCommand(CPacket &pkt);

	void	Reconnect(uint uiTimeout, const tstring &sReason = TSNULL);

	void	Handshake();

public:
	~CServerChatConnection();
	CServerChatConnection(CHostServer *pHostServer);

	void		Connect(const wstring &sAddress, ushort unPort);
	void		Disconnect(const tstring &sReason = TSNULL);
	void		Frame();

	inline uint	GetNextReminderTime() const		{ return m_uiNextConnectReminder; }
	void		UpdateReminderTime();

	void		SendStatusUpdate();
	void		SendConnectionReminder(uint uiAccountID);
	void		ReplacePlayer(uint uiAccountID);
	void		SendAbandonMatch(bool bFailed);
	void		SendMatchStarted();
	void		SendMatchAborted(EMatchAbortedReason eReason);
};
#endif
//=============================================================================

#endif //__C_SERVERCHATCONNECTION_H__
