// (C)2007 S2 Games
// c_statesacrifice.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statesacrifice.h"

#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Sacrifice)

CVAR_UINTF( g_sacrificeDeathTime,   5000,   CVAR_GAMECONFIG);
//=============================================================================


/*====================
  CStateSacrifice::CEntityConfig::CEntityConfig
  ====================*/
CStateSacrifice::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(BlastRadius, 300.0f),
INIT_ENTITY_CVAR(BlastDamage, 1000.0f),
INIT_ENTITY_CVAR(ExpiredEffectPath, _T("")),
INIT_ENTITY_CVAR(SpeedMult, 1.0f),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0),
INIT_ENTITY_CVAR(StaminaRegenMult, 0)
{
}


/*====================
  CStateSacrifice::CStateSacrifice
  ====================*/
CStateSacrifice::CStateSacrifice() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
    m_modArmor.Set(m_pEntityConfig->GetArmorMult(), m_pEntityConfig->GetArmorAdd(), 0.0f);
    m_modStaminaRegen.SetMult(m_pEntityConfig->GetStaminaRegenMult());
}


/*====================
  CStateSacrifice::Activated
  ====================*/
void    CStateSacrifice::Activated()
{
    IEntityState::Activated();

    ICombatEntity *pOwner(Game.GetCombatEntity(m_uiOwnerIndex));
    if (pOwner == NULL)
        return;
}


/*====================
  CStateSacrifice::Expired
  ====================*/
void    CStateSacrifice::Expired()
{
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner == NULL)
        return;
    if (pOwner->GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    uivector vTargets;
    Game.GetEntitiesInRadius(vTargets, CSphere(pOwner->GetPosition(), m_pEntityConfig->GetBlastRadius()), 0);
    for (uivector_it it(vTargets.begin()); it != vTargets.end(); ++it)
    {
        IGameEntity *pTarget(Game.GetEntityFromWorldIndex(*it));
        if (pTarget == NULL)
            continue;

        pTarget->Damage(m_pEntityConfig->GetBlastDamage(), DAMAGE_FLAG_SIEGE | DAMAGE_FLAG_SPLASH | DAMAGE_FLAG_EXPLOSIVE, pOwner, m_unDamageID);
    }

    CGameEvent evExplode;
    evExplode.SetSourcePosition(pOwner->GetPosition());
    evExplode.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetExpiredEffectPath()));
    Game.AddEvent(evExplode);

    pOwner->Kill(pOwner, m_unDamageID);
    pOwner->SetNetFlags(ENT_NET_FLAG_NO_CORPSE);
    pOwner->Unlink();
    if (pOwner->GetAsPlayerEnt() != NULL)
        pOwner->GetAsPlayerEnt()->SetDeathTime(Game.GetGameTime() + g_sacrificeDeathTime);
}


/*====================
  CStateSacrifice::ClientPrecache
  ====================*/
void    CStateSacrifice::ClientPrecache(CEntityConfig *pConfig)
{
    IEntityState::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetExpiredEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetExpiredEffectPath(), RES_EFFECT);
}


/*====================
  CStateSacrifice::ServerPrecache
  ====================*/
void    CStateSacrifice::ServerPrecache(CEntityConfig *pConfig)
{
    IEntityState::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetExpiredEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetExpiredEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}
