// (C)2007 S2 Games
// c_teaminfo.h
//
//=============================================================================
#ifndef __C_ENTITYGAMEINFO_H__
#define __C_ENTITYGAMEINFO_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CEntityGameInfo
//=============================================================================
class CEntityGameInfo : public IGameEntity
{
private:
	static vector<SDataField>	*s_pvFields;

	DECLARE_ENT_ALLOCATOR(Entity, GameInfo);

	uint		m_uiGamePhase;
	uint		m_uiPhaseStartTime;
	uint		m_uiPhaseDuration;
	int			m_iGameMatchID;
	bool		m_bSuddenDeathActive;

public:
	~CEntityGameInfo()	{}
	CEntityGameInfo();

	// Network
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot);
	
	GAME_SHARED_API static const vector<SDataField>&	GetTypeVector();

	void		SetGamePhase(uint uiGamePhase)				{ m_uiGamePhase = uiGamePhase; }
	void		SetPhaseStartTime(uint uiPhaseStartTime)	{ m_uiPhaseStartTime = uiPhaseStartTime; }
	void		SetPhaseDuration(uint uiPhaseDuration)		{ m_uiPhaseDuration = uiPhaseDuration; }
	void		SetGameMatchID(int iGameMatchID)			{ m_iGameMatchID = iGameMatchID; }
	void		SetSuddenDeath(bool bSuddenDeath)			{ m_bSuddenDeathActive = bSuddenDeath; }

	uint		GetGamePhase() const					{ return m_uiGamePhase & ~BIT(31); }
	uint		GetPhaseStartTime() const				{ return m_uiPhaseStartTime; }
	uint		GetPhaseDuration() const				{ return m_uiPhaseDuration; }
	int			GetGameMatchID() const					{ return m_iGameMatchID; }
	bool		GetSuddenDeath() const					{ return m_bSuddenDeathActive; }

	virtual bool		Reset()		{ return true; }
};
//=============================================================================

#endif //__C_ENTITYGAMEINFO_H__
