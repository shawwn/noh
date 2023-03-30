// (C)2006 S2 Games
// c_skillactivateevent.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_skillactivateevent.h"
//=============================================================================

/*====================
  CSkillActivateEvent::CSkillActivateEvent
  ====================*/
CSkillActivateEvent::CSkillActivateEvent() :
m_pOwner(NULL),
m_iSlot(-1),
m_bActive(false),
m_uiActivateTime(0),
m_bActivated(false)
{
}


/*====================
  CSkillActivateEvent::Clear
  ====================*/
void	CSkillActivateEvent::Clear()
{
	m_bActive = false;
	m_pOwner = NULL;
	m_iSlot = -1;
	m_uiActivateTime = 0;
	m_bActivated = false;
}


/*====================
  CSkillActivateEvent::TryImpact
  ====================*/
bool	CSkillActivateEvent::TryImpact()
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
	ISkillItem *pSkill(NULL);
	if (pItem == NULL ||
		(pSkill = pItem->GetAsSkill()) == NULL)
	{
		Clear();
		return false;
	}
		
	pSkill->Impact();
	m_bActivated = true;
	return true;
}
