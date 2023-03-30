// (C)2006 S2 Games
// c_stateriposte.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateriposte.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Riposte);
//=============================================================================


/*====================
  CStateRiposte::CEntityConfig::CEntityConfig
  ====================*/
CStateRiposte::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamageReturn, 0.35f),
INIT_ENTITY_CVAR(TriggerEffectPath, _T("")),
INIT_ENTITY_CVAR(TriggerTargetEffectPath, _T("")),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f)
{
}


/*====================
  CStateRiposte::CStateRiposte
  ====================*/
CStateRiposte::CStateRiposte() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
	m_modArmor.Set(m_pEntityConfig->GetArmorAdd(), m_pEntityConfig->GetArmorMult(), 0.0f);
}


/*====================
  CStateRiposte::OwnerDamaged
  ====================*/
float	CStateRiposte::OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker)
{
	IVisualEntity *pOwner(NULL);
	IGameEntity *pEnt(Game.GetEntity(m_uiOwnerIndex));
	if (pEnt != NULL)
		pOwner = pEnt->GetAsVisualEnt();

	if (pAttacker == NULL)
		return fDamage;
	if (!pAttacker->IsPlayer() && !pAttacker->IsNpc() && !pAttacker->IsPet())
		return fDamage;
	if (pAttacker->IsCombat() && (pAttacker->GetAsCombatEnt()->GetIsHellbourne() || pAttacker->GetAsCombatEnt()->GetIsSiege()))
		return fDamage;
	if (!(iFlags & DAMAGE_FLAG_MELEE))
		return fDamage;

	pAttacker->Damage(fDamage * m_pEntityConfig->GetDamageReturn(), DAMAGE_FLAG_DIRECT, pOwner, m_unDamageID);

	// TODO: make this one event
	ResHandle hTargetEffect(Game.RegisterEffect(m_pEntityConfig->GetTriggerTargetEffectPath()));
	if (hTargetEffect != INVALID_RESOURCE)
	{
		CGameEvent evReflect;
		evReflect.SetSourceEntity(pAttacker->GetIndex());
		evReflect.SetEffect(hTargetEffect);
		Game.AddEvent(evReflect);
	}

	ResHandle hTriggerEffect(Game.RegisterEffect(m_pEntityConfig->GetTriggerEffectPath()));
	if (hTriggerEffect != INVALID_RESOURCE)
	{
		CGameEvent evReflect;
		evReflect.SetSourceEntity(pOwner->GetIndex());
		evReflect.SetEffect(hTriggerEffect);
		Game.AddEvent(evReflect);
	}

	return fDamage;
}


/*====================
  CStateRiposte::ClientPrecache
  ====================*/
void	CStateRiposte::ClientPrecache(CEntityConfig *pConfig)
{
	IEntityState::ClientPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetTriggerEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetTriggerEffectPath(), RES_EFFECT);
	if (!pConfig->GetTriggerTargetEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetTriggerTargetEffectPath(), RES_EFFECT);
}


/*====================
  CStateRiposte::ServerPrecache
  ====================*/
void	CStateRiposte::ServerPrecache(CEntityConfig *pConfig)
{
	IEntityState::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetTriggerEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetTriggerEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
	if (!pConfig->GetTriggerTargetEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetTriggerTargetEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}
