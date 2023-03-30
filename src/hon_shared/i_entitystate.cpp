// (C)2006 S2 Games
// i_entitystate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_entitystate.h"

#include "c_player.h"
#include "i_unitentity.h"
#include "c_damageevent.h"

#include "../k2/c_texture.h"
#include "../k2/c_model.h"
#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint	IEntityState::s_uiBaseType(ENTITY_BASE_TYPE_STATE);

DEFINE_ENTITY_DESC(IEntityState, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(ctx_Game, TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(ISlaveEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiInflictorIndex"), TYPE_GAMEINDEX, 0, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCreationTime"), TYPE_INT, 32, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiLifetime"), TYPE_INT, 32, 0));
}
//=============================================================================

/*====================
  IEntityState::IEntityState
  ====================*/
IEntityState::IEntityState() :
m_uiInflictorIndex(INVALID_INDEX),

m_uiCreationTime(INVALID_TIME),
m_uiLifetime(INVALID_TIME),
m_uiLastIntervalTriggerTime(INVALID_TIME),

m_uiAuraSourceUID(INVALID_INDEX),
m_uiAuraTime(INVALID_TIME),
m_uiAuraEffectType(0),
m_bExpired(false),
m_bExpireNextFrame(false),
m_uiTimeout(INVALID_TIME)
{
}


/*====================
  IEntityState::GetSnapshot
  ====================*/
void	IEntityState::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	ISlaveEntity::GetSnapshot(snapshot, uiFlags);

	snapshot.WriteGameIndex(m_uiInflictorIndex);
	snapshot.WriteField(m_uiCreationTime);
	snapshot.WriteField(m_uiLifetime);
}


/*====================
  IEntityState::ReadSnapshot
  ====================*/
bool	IEntityState::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		ISlaveEntity::ReadSnapshot(snapshot, 1);

		snapshot.ReadGameIndex(m_uiInflictorIndex);
		snapshot.ReadField(m_uiCreationTime);
		snapshot.ReadField(m_uiLifetime);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IEntityState::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  IEntityState::Baseline
  ====================*/
void	IEntityState::Baseline()
{
	ISlaveEntity::Baseline();

	m_uiInflictorIndex = INVALID_INDEX;
	m_uiCreationTime = INVALID_TIME;
	m_uiLifetime = INVALID_TIME;
}


/*====================
  IEntityState::Defer
  ====================*/
void	IEntityState::Defer(uint uiTime)
{
	uint uiExpireTime(GetExpireTime());
	if (uiExpireTime != INVALID_TIME && uiExpireTime >= Game.GetGameTime())
		m_uiLifetime = uiExpireTime - Game.GetGameTime();

	m_uiCreationTime = Game.GetGameTime() + uiTime;
	m_uiLastIntervalTriggerTime += uiTime;
}


/*====================
  IEntityState::Activated
  ====================*/
void	IEntityState::Activated()
{
}


/*====================
  IEntityState::Expired
  ====================*/
void	IEntityState::Expired()
{
	IUnitEntity *pOwner(GetOwner());

	CStateDefinition *pDef(GetDefinition<CStateDefinition>());
	if (pDef != NULL)
		pDef->ExecuteActionScript(ACTION_SCRIPT_EXPIRED, this, GetInflictor(), this, pOwner, pOwner ? pOwner->GetPosition() : V_ZERO, GetProxy(0), GetLevel());
}


/*====================
  IEntityState::Spawn
  ====================*/
void	IEntityState::Spawn()
{
	ISlaveEntity::Spawn();

	if (GetImpactInterval() > 0)
		m_uiLastIntervalTriggerTime = Game.GetGameTime();
}


/*====================
  IEntityState::ServerFrameSetup
  ====================*/
bool	IEntityState::ServerFrameSetup()
{
	IUnitEntity *pOwner(Game.GetUnitEntity(GetOwnerIndex()));
	if (pOwner == NULL)
		return false;

	if (m_bExpireNextFrame || Game.GetGameTime() > GetExpireTime())
		m_bExpired = true;

	// Auras
	if (IsActive() && pOwner->GetStatus() == ENTITY_STATUS_ACTIVE)
	{
		if (m_pDefinition != NULL)
			m_pDefinition->ApplyAuras(this, GetLevel());
	}

	return ISlaveEntity::ServerFrameSetup();
}


/*====================
  IEntityState::ServerFrameThink
  ====================*/
bool	IEntityState::ServerFrameThink()
{
	if (m_uiAuraSourceUID != INVALID_INDEX)
	{
		if (m_uiAuraTime != Game.GetGameTime())
		{
			if (m_uiLifetime != INVALID_TIME && m_uiLifetime > 0)
			{
				m_uiCreationTime = m_uiAuraTime;
				m_uiAuraSourceUID = INVALID_INDEX;
			}
		}
		else
		{
			m_uiCreationTime = INVALID_TIME;
		}
	}

	if (m_uiCreationTime == INVALID_TIME && m_uiTimeout != INVALID_TIME && m_uiTimeout < Game.GetGameTime())
	{
		m_uiTimeout = INVALID_TIME;
		m_uiCreationTime = Game.GetGameTime();
	}

	return ISlaveEntity::ServerFrameThink();
}


/*====================
  IEntityState::ServerFrameAction
  ====================*/
bool	IEntityState::ServerFrameAction()
{
	if (!IsActive())
		return true;

	// Frame action
	CStateDefinition *pDefinition(GetDefinition<CStateDefinition>(GetModifierBits()));
	if (pDefinition != NULL)
		pDefinition->ExecuteActionScript(ACTION_SCRIPT_FRAME, this, GetInflictor(), this, GetOwner(), GetOwner()->GetPosition(), GetProxy(0), GetLevel());

	// Interval action
	if (GetImpactInterval() > 0)
	{
		while (IsActive() && Game.GetGameTime() - m_uiLastIntervalTriggerTime >= GetImpactInterval())
		{
			m_uiLastIntervalTriggerTime += GetImpactInterval();

			CStateDefinition *pDefinition(GetDefinition<CStateDefinition>(GetModifierBits()));
			if (pDefinition != NULL)
				pDefinition->ExecuteActionScript(ACTION_SCRIPT_IMPACT, this, GetInflictor(), this, GetOwner(), GetOwner()->GetPosition(), GetProxy(0), GetLevel());
		}
	}

	return true;
}


/*====================
  IEntityState::OwnerDamaged
  ====================*/
bool	IEntityState::OwnerDamaged(CDamageEvent &damage)
{
	if (!IsActive())
		return true;

	if (!ISlaveEntity::OwnerDamaged(damage))
		return false;

	return !GetDispelOnDamage();
}


/*====================
  IEntityState::OwnerAction
  ====================*/
bool	IEntityState::OwnerAction()
{
	if (!IsActive())
		return true;

	if (!ISlaveEntity::OwnerAction())
		return false;

	return !GetDispelOnAction();
}


/*====================
  IEntityState::ExecuteActionScript
  ====================*/
void	IEntityState::ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target)
{
	CGameInfo *pGameInfo(Game.GetGameInfo());
	if (pGameInfo != NULL)
		pGameInfo->ExecuteActionScript(eScript, GetOwner(), this, pTarget, v3Target);

	CStateDefinition *pDef(GetDefinition<CStateDefinition>(GetModifierBits()));
	if (pDef == NULL)
		return;

	pDef->ExecuteActionScript(eScript, this, GetOwner(), this, pTarget, v3Target, GetProxy(0), GetLevel());
}


/*====================
  IEntityState::ClientPrecache
  ====================*/
void	IEntityState::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
	ISlaveEntity::ClientPrecache(pConfig, eScheme, sModifier);

	if (!pConfig)
		return;
}


/*====================
  IEntityState::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void	IEntityState::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
	ISlaveEntity::ServerPrecache(pConfig, eScheme, sModifier);

	if (!pConfig)
		return;
}


/*====================
  IEntityState::UpdateModifiers
  ====================*/
void	IEntityState::UpdateModifiers()
{
	m_vModifierKeys = m_vPersistentModifierKeys;

	uint uiModifierBits(GetModifierBits(m_vModifierKeys));
	if (m_uiActiveModifierKey != INVALID_INDEX)
		uiModifierBits |= GetModifierBit(m_uiActiveModifierKey);

	// Activate conditional modifiers
	IUnitEntity *pOwner(GetOwner());
	if (pOwner == NULL)
		return;

	// Grab base definition
	IEntityDefinition *pDefinition(GetBaseDefinition<IEntityDefinition>());
	if (pDefinition == NULL)
		return;

	const EntityModifierMap &mapModifiers(pDefinition->GetModifiers());
	for (EntityModifierMap::const_iterator cit(mapModifiers.begin()), citEnd(mapModifiers.end()); cit != citEnd; ++cit)
	{
		if (cit->second->GetExclusive())
			continue;

		const tstring &sCondition(cit->second->GetCondition());
		if (sCondition.empty())
			continue;

		tsvector vsTypes(TokenizeString(sCondition, _T(' ')));

		tsvector_cit itType(vsTypes.begin()), itTypeEnd(vsTypes.end());
		for (; itType != itTypeEnd; ++itType)
		{
			if (!itType->empty() && (*itType)[0] == _T('!'))
			{
				if (pOwner->IsTargetType(itType->substr(1), pOwner))
					break;
			}
			else
			{
				if (!pOwner->IsTargetType(*itType, pOwner))
					break;
			}
		}
		if (itType == itTypeEnd)
			uiModifierBits |= cit->first;
	}

	SetModifierBits(uiModifierBits);
}
