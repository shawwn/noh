// (C)2009 S2 Games
// c_baggressivewander.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bassist.h"
#include "c_asMoving.h"
//=============================================================================

/*====================
  CBAssist::CopyFrom
  ====================*/
void	CBAssist::CopyFrom(const IBehavior* pBehavior)
{
	assert( GetType() == pBehavior->GetType() );
	if (GetType() != pBehavior->GetType())
		return;

	const CBAssist *pCBBehavior(static_cast<const CBAssist*>(pBehavior));

	m_Attack.CopyFrom(&pCBBehavior->m_Attack);
	m_Attack.SetBrain(m_pBrain);
	m_Attack.SetSelf(m_pSelf);
	m_bAttacking = pCBBehavior->m_bAttacking;
	m_uiLastAggroUpdate = pCBBehavior->m_uiLastAggroUpdate;
	m_fAssistRange = pCBBehavior->m_fAssistRange;

	CBFollow::CopyFrom(pCBBehavior);
}

/*====================
  CBAssist::Clone
  ====================*/
IBehavior*	CBAssist::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
	IBehavior* pBehavior( K2_NEW(ctx_Game,    CBAssist)() );
	pBehavior->SetBrain(pNewBrain);
	pBehavior->SetSelf(pNewSelf);
	pBehavior->CopyFrom(this);
	return pBehavior;
}

/*====================
  CBAssist::UpdateAggro
  ====================*/
void	CBAssist::UpdateAggro()
{
	// No aggro updates while still in an attack
	if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
		return;

	// Update the status of the attackmove behavior
	m_uiLastAggroUpdate = Game.GetGameTime();

	uint uiCurrentTargetIndex(m_bAttacking ? m_Attack.GetTarget() : INVALID_INDEX);
	bool bOwnerHasTarget(false);

	IUnitEntity *pOwner(m_pSelf->GetOwner());
	if (pOwner != NULL)
	{
		IUnitEntity *pTarget(Game.GetUnitEntity(pOwner->GetTargetIndex()));
		if (pTarget != NULL)
		{
			uiCurrentTargetIndex = pTarget->GetIndex();
			bOwnerHasTarget = true;
		}
	}

	if (!bOwnerHasTarget && m_bAttacking)
		uiCurrentTargetIndex = INVALID_INDEX;

	if (m_bAttacking && uiCurrentTargetIndex != INVALID_INDEX)
	{
		IUnitEntity *pAttackTarget(Game.GetUnitEntity(uiCurrentTargetIndex));
		if (pAttackTarget != NULL)
		{
			float fRange(Distance(m_pSelf->GetPosition().xy(), pAttackTarget->GetPosition().xy()) - pAttackTarget->GetBounds().GetDim(X) * DIAG);
			float fAssistRange(m_fAssistRange + m_pSelf->GetBounds().GetDim(X) * DIAG);
			if (m_fAssistRange != 0.0f && fRange > fAssistRange)
				uiCurrentTargetIndex = INVALID_INDEX;
		}
	}

	if (uiCurrentTargetIndex == INVALID_INDEX)
	{
		m_Attack.EndBehavior();
		m_bAttacking = false;
		m_uiLastAggroUpdate = INVALID_TIME;
	}
	// Start attack
	else if (uiCurrentTargetIndex != INVALID_INDEX && !(m_bAttacking && m_Attack.GetTarget() == uiCurrentTargetIndex))
	{
		if (m_bAttacking)
			m_Attack.EndBehavior();

		m_bAttacking = true;

		m_Attack.Init(m_pBrain, m_pSelf, uiCurrentTargetIndex);
		m_Attack.DisableAggroTrigger();
	}
}


/*====================
  CBAssist::Validate
  ====================*/
bool	CBAssist::Validate()
{
	if (!CBFollow::Validate())
	{
		SetFlag(BSR_END);
		return false;
	}

	return true;
}


/*====================
  CBAssist::Update
  ====================*/
void	CBAssist::Update()
{
	CBFollow::Update();
}


/*====================
  CBAssist::BeginBehavior
  ====================*/
void	CBAssist::BeginBehavior()
{
	if (m_pSelf == NULL)
	{
		Console << _T("CBAssist: Behavior started without valid information") << newl;
		return;
	}

	CBFollow::BeginBehavior();

	m_uiLastAggroUpdate = INVALID_TIME;
	UpdateAggro();

	ClearFlag(BSR_NEW);
}


/*====================
  CBAssist::ThinkFrame
  ====================*/
void	CBAssist::ThinkFrame()
{
	UpdateAggro();

	if (m_bAttacking)
	{
		if (m_Attack.Validate())
		{
			if (m_Attack.GetFlags() & BSR_NEW)
				m_Attack.BeginBehavior();

			if (~m_Attack.GetFlags() & BSR_NEW)
				m_Attack.ThinkFrame();
		}

		if (m_Attack.GetFlags() & BSR_END)
		{
			m_Attack.EndBehavior();
			m_bAttacking = false;
			m_uiLastAggroUpdate = INVALID_TIME;

			UpdateAggro();

			// Repath towards destination if we didn't find a new target
			if (!m_bAttacking)
			{
				CBFollow::BeginBehavior();
			}
			else
			{
				if (m_Attack.Validate())
				{
					if (m_Attack.GetFlags() & BSR_NEW)
						m_Attack.BeginBehavior();

					m_Attack.ThinkFrame();
				}
			}
		}
	}

	if (!m_bAttacking)
		CBFollow::ThinkFrame();
}


/*====================
  CBAssist::MovementFrame
  ====================*/
void	CBAssist::MovementFrame()
{
	if (m_bAttacking)
	{
		if (m_Attack.Validate())
		{
			if (~m_Attack.GetFlags() & BSR_NEW)
				m_Attack.MovementFrame();
		}
	}
	else
		CBFollow::MovementFrame();
}


/*====================
  CBAssist::ActionFrame
  ====================*/
void	CBAssist::ActionFrame()
{
	UpdateAggro();

	if (m_bAttacking)
	{
		if (m_Attack.Validate())
		{
			if (~m_Attack.GetFlags() & BSR_NEW)
				m_Attack.ActionFrame();
		}

		if (m_Attack.GetFlags() & BSR_END)
		{
			m_Attack.EndBehavior();
			m_bAttacking = false;
			m_uiLastAggroUpdate = INVALID_TIME;

			UpdateAggro();

			// Repath towards destination if we didn't find a new target
			if (!m_bAttacking)
				CBFollow::BeginBehavior();
		}
	}
}


/*====================
  CBAssist::CleanupFrame
  ====================*/
void	CBAssist::CleanupFrame()
{
}


/*====================
  CBAssist::EndBehavior
  ====================*/
void	CBAssist::EndBehavior()
{
	m_Attack.EndBehavior();
	CBFollow::EndBehavior();
}


/*====================
  CBAssist::Aggro
  ====================*/
void	CBAssist::Aggro(IUnitEntity *pAttacker, uint uiChaseTime)
{
	if (m_bAttacking)
		m_Attack.EndBehavior();

	m_bAttacking = true;

	m_Attack.Init(m_pBrain, m_pSelf, pAttacker->GetIndex(), 1);
	m_Attack.DisableAggroTrigger();
}


/*====================
  CBAssist::Damaged
  ====================*/
void	CBAssist::Damaged(IUnitEntity *pAttacker)
{
#if 0
	if (pAttacker == NULL)
		return;

	m_pSelf->CallForHelp(500.0f, pAttacker);

	Aggro(pAttacker, m_pSelf->GetGuardChaseTime());
#endif
}


/*====================
  CBAssist::Assist
  ====================*/
void	CBAssist::Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker)
{
#if 0
	if (pAttacker == NULL)
		return;

	Aggro(pAttacker, m_pSelf->GetGuardChaseTime());
#endif
}
