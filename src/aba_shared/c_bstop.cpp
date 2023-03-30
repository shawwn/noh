// (C)2008 S2 Games
// c_bstop.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bstop.h"

#include "c_brain.h"
//=============================================================================

/*====================
  CBStop::CopyFrom
  ====================*/
void	CBStop::CopyFrom(const IBehavior* pBehavior)
{
	assert( GetType() == pBehavior->GetType() );
	if (GetType() != pBehavior->GetType())
		return;

	const CBStop *pCBBehavior(static_cast<const CBStop*>(pBehavior));

	IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBStop::Clone
  ====================*/
IBehavior*	CBStop::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
	IBehavior* pBehavior( K2_NEW(g_heapAI,    CBStop)() );
	pBehavior->SetBrain(pNewBrain);
	pBehavior->SetSelf(pNewSelf);
	pBehavior->CopyFrom(this);
	return pBehavior;
}

/*====================
  CBStop::Validate
  ====================*/
bool	CBStop::Validate()
{
	if (m_pBrain == NULL || m_pSelf == NULL || GetFlags() & BSR_END)
	{
		SetFlag(BSR_END);
		return false;
	}

	return true;
}


/*====================
  CBStop::BeginBehavior
  ====================*/
void	CBStop::BeginBehavior()
{
	if (m_pSelf == NULL)
	{
		Console << _T("CBStop: Behavior started without valid information") << newl;
		return;
	}

	ClearFlag(BSR_NEW);
}


/*====================
  CBStop::ThinkFrame
  ====================*/
void	CBStop::ThinkFrame()
{
	m_pSelf->Interrupt(UNIT_ACTION_STOP);

	if (m_pBrain->EndActionStates(2) == 0)
		SetFlag(BSR_END);

	m_pBrain->SetMoving(false);
}


/*====================
  CBStop::MovementFrame
  ====================*/
void	CBStop::MovementFrame()
{
	m_pSelf->Interrupt(UNIT_ACTION_STOP);

	if (m_pBrain->EndActionStates(2) == 0)
		SetFlag(BSR_END);
}


/*====================
  CBStop::ActionFrame
  ====================*/
void	CBStop::ActionFrame()
{
	m_pSelf->Interrupt(UNIT_ACTION_STOP);

	if (m_pBrain->EndActionStates(2) == 0)
		SetFlag(BSR_END);
}


/*====================
  CBStop::CleanupFrame
  ====================*/
void	CBStop::CleanupFrame()
{
}
