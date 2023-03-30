// (C)2007 S2 Games
// c_serverstatstracker.h
//
//=============================================================================
#ifndef __C_SERVERSTATSTRACKER_H__
#define __C_SERVERSTATSTRACKER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_gameserver.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define StatsTracker (*CServerStatsTracker::GetInstance())
//=============================================================================

//=============================================================================
// CServerStatsTracker
//=============================================================================
class CServerStatsTracker
{
	SINGLETON_DEF(CServerStatsTracker)

private:
	int			m_iServerID;
	int			m_iMatchID;
	tstring		m_sSalt;

	uint		m_uiMatchLength;
	uint		m_uiLastUpdate;

	bool		m_bMatchInProgress;

	iset		m_setRatingSubmitted;

	tstring		GenerateStatsString(CPlayer *pClient, int iTeamHighSF);

public:
	~CServerStatsTracker();

	void	StartMatch(const tstring &sLogin, const tstring &sPass, bool bClanMatch, const tstring &sMap, int iPort, PlayerMap &mapClients);
	void	EndMatch();

	void	SubmitAllData();
	void	SubmitStats(PlayerMap &mapClients, const tstring &sMapName);
	void	WriteStatsToFile(PlayerMap &mapClients, const tstring &sMapName);

	int		GetMatchID()				{ return m_iMatchID; }

	void	Frame();

	bool	HasMatchStarted()			{ return m_bMatchInProgress; }

	void	RetrieveStats(int iAccountID);

	void	SubmitRating(int iAccountID, int iRating, const tstring &sComment);
};
//=============================================================================

#endif //__C_SERVERSTATSTRACKER_H__
