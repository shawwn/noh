// (C)2006 S2 Games
// c_spellactivateevent.h
//
//=============================================================================
#ifndef __C_SPELLACTIVATEEVENT_H__
#define __C_SPELLACTIVATEEVENT_H__

//=============================================================================
// Declarations
//=============================================================================
class ISpellItem;
//=============================================================================

// FIXME: This can probably be merged with CSkillActivateEvent

//=============================================================================
// CSpellActivateEvent
//=============================================================================
class CSpellActivateEvent
{
private:
	ICombatEntity*	m_pOwner;
	int				m_iSlot;

	bool			m_bActive;
	uint			m_uiActivateTime;
	bool			m_bActivated;
	bool			m_bSucceeded;

public:
	~CSpellActivateEvent()	{}
	CSpellActivateEvent();

	void	Clear();

	void	SetActive()								{ m_bActive = true; }
	void	SetInactive()							{ m_bActive = false; }
	bool	IsActive() const						{ return m_bActive; }

	void	SetOwner(ICombatEntity *pOwner)			{ m_pOwner = pOwner; }
	void	SetSlot(int iSlot)						{ m_iSlot = iSlot; }
	void	SetActivateTime(uint uiTime)			{ m_uiActivateTime = uiTime; }

	bool	GetActivated() const					{ return m_bActivated; }
	bool	GetSucceeded() const					{ return m_bSucceeded; }

	bool	TryImpact();
};
//=============================================================================

#endif //__C_SPELLACTIVATEEVENT_H__
