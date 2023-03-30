// (C)2009 S2 Games
// c_auradefinition.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_auradefinition.h"
#include "i_gadgetentity.h"
//=============================================================================

/*====================
  CAuraDefinition::CAuraDefinition
  ====================*/
CAuraDefinition::CAuraDefinition(
	const tstring &sStateName,
	const tstring &sGadgetName,
	const tstring &sRadius,
	const tstring &sDuration,
	const tstring &sTargetScheme,
	const tstring &sEffectType,
	const tstring &sIgnoreInvulnerable,
	const tstring &sCondition,
	const tstring &sReflexiveStateName,
	const tstring &sPropagateCondition,
	const tstring &sStack,
	bool bNoTooltip) :
m_bPrecaching(false),

m_vStateName(TokenizeString(sStateName, _T(','))),
m_vGadgetName(TokenizeString(sGadgetName, _T(','))),
m_vCondition(TokenizeString(sCondition, _T(','))),
m_vReflexiveStateName(TokenizeString(sReflexiveStateName, _T(','))),
m_vPropagateCondition(TokenizeString(sPropagateCondition, _T(','))),

m_bValid(true),
m_unStateID(INVALID_ENT_TYPE),
m_unGadgetID(INVALID_ENT_TYPE),
m_uiDuration(INVALID_TIME),
m_uiTargetScheme(INVALID_TARGET_SCHEME),
m_uiEffectType(0),
m_bIgnoreInvulnerable(false),
m_pGadgetOwner(NULL),
m_unReflexiveStateID(INVALID_ENT_TYPE),
m_bStack(false),
m_bNoTooltip(bNoTooltip)
{
	if (sRadius.empty())
	{
		m_vRadius.push_back(0.0f);
	}
	else
	{
		tsvector vRadius(TokenizeString(sRadius, _T(',')));
		for (tsvector_it it(vRadius.begin()); it != vRadius.end(); ++it)
			m_vRadius.push_back(AtoF(*it));
	}

	if (sDuration.empty())
	{
		m_vDuration.push_back(INVALID_TIME);
	}
	else
	{
		tsvector vDuration(TokenizeString(sDuration, _T(',')));
		for (tsvector_it it(vDuration.begin()); it != vDuration.end(); ++it)
			m_vDuration.push_back(AtoI(*it));
	}

	if (sTargetScheme.empty())
	{
		m_vTargetScheme.push_back(INVALID_TARGET_SCHEME);
	}
	else
	{
		tsvector vTargetScheme(TokenizeString(sTargetScheme, _T(',')));
		for (tsvector_it it(vTargetScheme.begin()); it != vTargetScheme.end(); ++it)
			m_vTargetScheme.push_back(Game.LookupTargetScheme(*it));
	}

	if (sEffectType.empty())
	{
		m_vEffectType.push_back(0);
	}
	else
	{
		tsvector vEffectType(TokenizeString(sEffectType, _T(',')));
		for (tsvector_it it(vEffectType.begin()); it != vEffectType.end(); ++it)
			m_vEffectType.push_back(Game.LookupEffectType(*it));
	}

	if (sIgnoreInvulnerable.empty())
	{
		m_vIgnoreInvulnerable.push_back(false);
	}
	else
	{
		tsvector vIgnoreInvulnerable(TokenizeString(sIgnoreInvulnerable, _T(',')));
		for (tsvector_it it(vIgnoreInvulnerable.begin()); it != vIgnoreInvulnerable.end(); ++it)
			m_vIgnoreInvulnerable.push_back(AtoB(*it));
	}

	if (sStack.empty())
	{
		m_vStack.push_back(false);
	}
	else
	{
		tsvector vStack(TokenizeString(sStack, _T(',')));
		for (tsvector_it it(vStack.begin()); it != vStack.end(); ++it)
			m_vStack.push_back(AtoB(*it));
	}

	if (m_vStateName.empty() && m_vGadgetName.empty())
		m_bValid = false;
}


/*====================
  CAuraDefinition::CanApply
  ====================*/
bool	CAuraDefinition::CanApply(IUnitEntity *pSource, IUnitEntity *pTarget) const
{
	if (pTarget == NULL)
		return false;

	if (!Game.IsValidTarget(m_uiTargetScheme, m_uiEffectType, pSource, pTarget, m_bIgnoreInvulnerable))
		return false;

	if (m_sCondition.empty())
		return true;

	return EvaluateConditionalString(m_sCondition, NULL, NULL, pSource, pTarget, NULL);
}


/*====================
  CAuraDefinition::CanPropagate
  ====================*/
bool	CAuraDefinition::CanPropagate(IUnitEntity *pSource) const
{
	if (m_sPropagateCondition.empty())
		return true;

	return EvaluateConditionalString(m_sPropagateCondition, NULL, NULL, pSource, pSource, NULL);
}


/*====================
  CAuraDefinition::ApplyState
  ====================*/
bool	CAuraDefinition::ApplyState(IGameEntity *pSource, IUnitEntity *pTarget, uint uiLevel, IGameEntity *pSpawner) const
{
	if (m_unStateID == INVALID_ENT_TYPE)
		return false;

	IEntityState *pState(pTarget->ApplyState(m_unStateID, uiLevel, INVALID_TIME, m_uiDuration, pSource->GetIndex(), INVALID_INDEX, m_bStack ? STATE_STACK_NOSELF : STATE_STACK_NONE, pSpawner ? pSpawner->GetUniqueID() : INVALID_INDEX));
	if (pState == NULL)
		return false;

	pState->SetAuraSource(pSource->GetUniqueID());
	pState->SetAuraTime(Game.GetGameTime());
	pState->SetAuraTargetScheme(m_uiTargetScheme);
	pState->SetAuraEffectType(m_uiEffectType);
	return true;
}


/*====================
  CAuraDefinition::BindGadget
  ====================*/
bool	CAuraDefinition::BindGadget(IGameEntity *pSource, IUnitEntity *pTarget, uint uiLevel, IGameEntity *pSpawner) const
{
	if (m_unGadgetID == INVALID_ENT_TYPE)
		return false;

	IGadgetEntity *pGadget(pTarget->AttachGadget(m_unGadgetID, uiLevel, INVALID_TIME, INVALID_TIME, m_pGadgetOwner));
	if (pGadget == NULL)
		return false;

	pGadget->SetAuraSource(pSource->GetUniqueID());
	pGadget->SetAuraTime(Game.GetGameTime());
	return true;
}


/*====================
  CAuraDefinition::ApplyReflexiveState
  ====================*/
bool	CAuraDefinition::ApplyReflexiveState(IGameEntity *pSource, IUnitEntity *pTarget, uint uiLevel, IGameEntity *pSpawner) const
{
	if (m_unReflexiveStateID == INVALID_ENT_TYPE)
		return false;

	IEntityState *pState(pTarget->ApplyState(m_unReflexiveStateID, uiLevel, INVALID_TIME, m_uiDuration, pSource->GetIndex(), INVALID_INDEX, STATE_STACK_NONE, pSpawner ? pSpawner->GetUniqueID() : INVALID_INDEX));
	if (pState == NULL)
		return false;

	pState->SetAuraSource(pSource->GetUniqueID());
	pState->SetAuraTime(Game.GetGameTime());
	return true;
}


/*====================
  CAuraDefinition::FetchWorkingValues
  ====================*/
void	CAuraDefinition::FetchWorkingValues(IGameEntity *pSource, uint uiLevel)
{
	const tstring &sStateName(GetStateName(uiLevel));
	m_unStateID = sStateName.empty() ? INVALID_ENT_TYPE : EntityRegistry.LookupID(sStateName);

	const tstring &sGadgetName(GetGadgetName(uiLevel));
	m_unGadgetID = sGadgetName.empty() ? INVALID_ENT_TYPE : EntityRegistry.LookupID(sGadgetName);

	m_uiDuration = GetDuration(uiLevel);
	m_uiTargetScheme = GetTargetScheme(uiLevel);
	m_uiEffectType = GetEffectType(uiLevel);
	m_bIgnoreInvulnerable = GetIgnoreInvulnerable(uiLevel);
	m_sCondition = GetCondition(uiLevel);
	m_sPropagateCondition = GetPropagateCondition(uiLevel);
	m_bStack = GetStack(uiLevel);

	const tstring &sReflexiveStateName(GetReflexiveStateName(uiLevel));
	m_unReflexiveStateID = sReflexiveStateName.empty() ? INVALID_ENT_TYPE : EntityRegistry.LookupID(sReflexiveStateName);

	if (m_unGadgetID == INVALID_ENT_TYPE || pSource == NULL)
		m_pGadgetOwner = NULL;
	else
		m_pGadgetOwner = pSource->IsState() ? pSource->GetAsState()->GetInflictor() : pSource->GetMasterOwner();
}


/*====================
  CAuraDefinition::Precache
  ====================*/
void	CAuraDefinition::Precache(EPrecacheScheme eScheme)
{
	PRECACHE_GUARD
		for (tsvector_it it(m_vStateName.begin()); it != m_vStateName.end(); ++it)
			Game.Precache(*it, eScheme);
		for (tsvector_it it(m_vGadgetName.begin()); it != m_vGadgetName.end(); ++it)
			Game.Precache(*it, eScheme);
		for (tsvector_it it(m_vReflexiveStateName.begin()); it != m_vReflexiveStateName.end(); ++it)
			Game.Precache(*it, eScheme);
	PRECACHE_GUARD_END
}


/*====================
  CAuraDefinition::GetPrecacheList
  ====================*/
void	CAuraDefinition::GetPrecacheList(EPrecacheScheme eScheme, HeroPrecacheList &deqPrecache)
{
	for (tsvector_it it(m_vStateName.begin()); it != m_vStateName.end(); ++it)
		Game.GetPrecacheList(*it, eScheme, deqPrecache);
	for (tsvector_it it(m_vGadgetName.begin()); it != m_vGadgetName.end(); ++it)
		Game.GetPrecacheList(*it, eScheme, deqPrecache);
	for (tsvector_it it(m_vReflexiveStateName.begin()); it != m_vReflexiveStateName.end(); ++it)
		Game.GetPrecacheList(*it, eScheme, deqPrecache);
}
