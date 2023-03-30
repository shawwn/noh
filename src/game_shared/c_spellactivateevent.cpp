// (C)2006 S2 Games
// c_spellactivateevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellactivateevent.h"
//=============================================================================

/*====================
  CSpellActivateEvent::CSpellActivateEvent
  ====================*/
CSpellActivateEvent::CSpellActivateEvent() :
m_pOwner(NULL),
m_iSlot(-1),
m_bActive(false),
m_uiActivateTime(0),
m_bActivated(false),
m_bSucceeded(false)
{
}


/*====================
  CSpellActivateEvent::Clear
  ====================*/
void	CSpellActivateEvent::Clear()
{
	m_bActive = false;
	m_pOwner = NULL;
	m_iSlot = -1;
	m_uiActivateTime = 0;
	m_bActivated = false;
	m_bSucceeded = false;
}


/*====================
  CSpellActivateEvent::TryImpact
  ====================*/
bool	CSpellActivateEvent::TryImpact()
{
	if (m_pOwner == NULL)
	{
		Clear();
		return false;
	}

	if (m_bActivated)
		return false;

	if (Game.GetGameTime() < m_uiActivateTime)
		return false;

	IInventoryItem *pItem(m_pOwner->GetItem(m_iSlot));
	ISpellItem *pSpell(NULL);
	if (pItem == NULL ||
		(pSpell = pItem->GetAsSpell()) == NULL)
	{
		Clear();
		return false;
	}
		
	m_bSucceeded = pSpell->TryImpact();
	m_bActivated = true;
	return true;
}
