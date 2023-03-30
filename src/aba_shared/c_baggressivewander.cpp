// (C)2009 S2 Games
// c_baggressivewander.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_baggressivewander.h"
#include "c_asMoving.h"
//=============================================================================

//=============================================================================
//=============================================================================
EXTERN_CVAR_UINT(behavior_WanderPeriod);
//=============================================================================

/*====================
  CBAggressiveWander::CopyFrom
  ====================*/
void	CBAggressiveWander::CopyFrom(const IBehavior* pBehavior)
{
	assert( GetType() == pBehavior->GetType() );
	if (GetType() != pBehavior->GetType())
		return;

	const CBAggressiveWander *pCBBehavior(static_cast<const CBAggressiveWander*>(pBehavior));

	m_Attack.CopyFrom(&pCBBehavior->m_Attack);
	m_Attack.SetBrain(m_pBrain);
	m_Attack.SetSelf(m_pSelf);
	m_uiLastAggroUpdate = pCBBehavior->m_uiLastAggroUpdate;
	m_bAttacking = pCBBehavior->m_bAttacking;

	CBWander::CopyFrom(pCBBehavior);
}

/*====================
  CBAggressiveWander::Clone
  ====================*/
IBehavior*	CBAggressiveWander::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
	IBehavior* pBehavior( K2_NEW(g_heapAI,    CBAggressiveWander)() );
	pBehavior->SetBrain(pNewBrain);
	pBehavior->SetSelf(pNewSelf);
	pBehavior->CopyFrom(this);
	return pBehavior;
}

/*====================
  CBAggressiveWander::UpdateAggro
  ====================*/
void	CBAggressiveWander::UpdateAggro()
{
	if (m_uiLastAggroUpdate != INVALID_TIME && m_uiLastAggroUpdate + BEHAVIOR_UPDATE_MS >= Game.GetGameTime())
		return;

	// No aggro updates while still in an attack
	if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
		return;

	if (!m_pSelf->IsAttackReady())
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

	if (!bOwnerHasTarget)
	{
		static uivector vEntities;
		CVec3f v3Position(pOwner != NULL ? pOwner->GetPosition() : m_pSelf->GetPosition());
		float fAggroRange(m_pSelf->GetAggroRange() + (pOwner != NULL ? pOwner->GetBounds().GetDim(X) * DIAG : m_pSelf->GetBounds().GetDim(X) * DIAG));
		CBBoxf bbRegion(CVec3f(v3Position.xy() - CVec2f(fAggroRange), -FAR_AWAY),  CVec3f(v3Position.xy() + CVec2f(fAggroRange), FAR_AWAY));

		// Fetch
		Game.GetEntitiesInRegion(vEntities, bbRegion, REGION_ACTIVE_UNIT);

		// Find the most threating enemy
		float fThreat(-FAR_AWAY);
		uint uiClosestWorldIndex(INVALID_INDEX);
		uivector_cit citEnd(vEntities.end());
		for (uivector_cit cit(vEntities.begin()); cit != citEnd; ++cit)
		{
			if (*cit == m_pSelf->GetWorldIndex())
				continue;

			IUnitEntity *pTarget(Game.GetUnitEntity(Game.GetGameIndexFromWorldIndex(*cit)));
			if (pTarget == NULL)
				continue;
			if (!m_pSelf->ShouldTarget(pTarget))
				continue;

			float fCurrent(m_pSelf->GetThreatLevel(pTarget, pTarget->GetIndex() == uiCurrentTargetIndex));

			if (fCurrent > fThreat &&
				Distance(pTarget->GetPosition().xy(), v3Position.xy()) - pTarget->GetBounds().GetDim(X) * DIAG <= fAggroRange)
			{
				fThreat = fCurrent;
				uiClosestWorldIndex = *cit;
			}
		}

		uiCurrentTargetIndex = Game.GetGameIndexFromWorldIndex(uiClosestWorldIndex);
	}

	// Start attack
	if (uiCurrentTargetIndex != INVALID_INDEX && !(m_bAttacking && m_Attack.GetTarget() == uiCurrentTargetIndex))
	{
		if (m_bAttacking)
			m_Attack.EndBehavior();

		m_bAttacking = true;

		m_Attack = CBAttack(m_pSelf->GetArmingSequence(), 1);
		m_Attack.SetBrain(m_pBrain);
		m_Attack.SetSelf(m_pSelf);
		m_Attack.SetTarget(uiCurrentTargetIndex);
		m_Attack.DisableAggroTrigger();
	}

	if (m_bAttacking)
		m_pSelf->CallForHelp(500.0f, Game.GetUnitEntity(m_Attack.GetTarget()));
}


/*====================
  CBAggressiveWander::Validate
  ====================*/
bool	CBAggressiveWander::Validate()
{
	if (m_pBrain == NULL ||
		m_pSelf == NULL ||
		GetFlags() & BSR_END ||
		(m_uiTargetIndex != INVALID_INDEX && Game.GetUnitEntity(m_uiTargetIndex) == NULL))
	{
		SetFlag(BSR_END);
		return false;
	}

	return true;
}


/*====================
  CBAggressiveWander::Update
  ====================*/
void	CBAggressiveWander::Update()
{
	CBWander::Update();
}


/*====================
  CBAggressiveWander::BeginBehavior
  ====================*/
void	CBAggressiveWander::BeginBehavior()
{
	if (m_pSelf == NULL)
	{
		Console << _T("CBAggressiveWander: Behavior started without valid information") << newl;
		return;
	}

	CBWander::BeginBehavior();

	m_uiLastAggroUpdate = INVALID_TIME;
	UpdateAggro();

	ClearFlag(BSR_NEW);
}


/*====================
  CBAggressiveWander::ThinkFrame
  ====================*/
void	CBAggressiveWander::ThinkFrame()
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
				CBWander::BeginBehavior();
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
		CBWander::ThinkFrame();
}


/*====================
  CBAggressiveWander::MovementFrame
  ====================*/
void	CBAggressiveWander::MovementFrame()
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
		CBWander::MovementFrame();
}


/*====================
  CBAggressiveWander::ActionFrame
  ====================*/
void	CBAggressiveWander::ActionFrame()
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
				CBWander::BeginBehavior();
		}
	}
}


/*====================
  CBAggressiveWander::CleanupFrame
  ====================*/
void	CBAggressiveWander::CleanupFrame()
{
}


/*====================
  CBAggressiveWander::EndBehavior
  ====================*/
void	CBAggressiveWander::EndBehavior()
{
	m_Attack.EndBehavior();
	CBWander::EndBehavior();
}


/*====================
  CBAggressiveWander::Aggro
  ====================*/
void	CBAggressiveWander::Aggro(IUnitEntity *pAttacker, uint uiChaseTime)
{
	if (m_bAttacking)
		m_Attack.EndBehavior();

	m_bAttacking = true;

	m_Attack = CBAttack(m_pSelf->GetArmingSequence(), 1);
	m_Attack.SetBrain(m_pBrain);
	m_Attack.SetSelf(m_pSelf);
	m_Attack.SetTarget(pAttacker->GetIndex());
	m_Attack.DisableAggroTrigger();
}


/*====================
  CBAggressiveWander::Damaged
  ====================*/
void	CBAggressiveWander::Damaged(IUnitEntity *pAttacker)
{
#if 0
	if (pAttacker == NULL)
		return;

	m_pSelf->CallForHelp(500.0f, pAttacker);

	Aggro(pAttacker, m_pSelf->GetGuardChaseTime());
#endif
}


/*====================
  CBAggressiveWander::Assist
  ====================*/
void	CBAggressiveWander::Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker)
{
#if 0
	if (pAttacker == NULL)
		return;

	Aggro(pAttacker, m_pSelf->GetGuardChaseTime());
#endif
}
