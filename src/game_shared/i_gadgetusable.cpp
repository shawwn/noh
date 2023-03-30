// (C)2007 S2 Games
// i_gadgetusable.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gadgetusable.h"
//=============================================================================

/*====================
  IGadgetUsable::CEntityConfig::CEntityConfig
  ====================*/
IGadgetUsable::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(UseEffectPath, _T("")),
INIT_ENTITY_CVAR(UseExperience, 50.0f)
{
}


/*====================
  IGadgetUsable::Use
  ====================*/
bool    IGadgetUsable::Use(IGameEntity *pActivator)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return false;

    IPlayerEntity *pPlayer(pActivator->GetAsPlayerEnt());
    if (pPlayer == NULL)
        return false;
    if (pPlayer->GetTeam() != m_iTeam)
        return false;
    if (m_bAccessed || m_setAccessors.find(pPlayer->GetClientID()) != m_setAccessors.end())
        return false;

    if (!UseEffect(pActivator))
        return false;

    IPlayerEntity *pOwner(Game.GetPlayerEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
    {
        m_fTotalExperience += m_pEntityConfig->GetUseExperience();
        pOwner->GiveExperience(m_pEntityConfig->GetUseExperience(), GetPosition() + GetBounds().GetMid());
    }

    if (!m_pEntityConfig->GetUseEffectPath().empty() && Game.IsServer())
    {
        CGameEvent evReload;
        evReload.SetSourceEntity(GetIndex());
        evReload.SetTargetEntity(pPlayer->GetIndex());
        evReload.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetUseEffectPath()));
        Game.AddEvent(evReload);
    }

    m_setAccessors.insert(pPlayer->GetClientID());

    if (Game.IsServer())
    {
        CBufferFixed<9> buffer;
        buffer << GAME_CMD_GADGET_ACCESSED << GetIndex() << GetSpawnTime();
        Game.SendGameData(pPlayer->GetClientID(), buffer, true);
    }

    m_auiCounter[0] = INT_SIZE(m_setAccessors.size());
    return true;
}


/*====================
  IGadgetUsable::ClientPrecache
  ====================*/
void    IGadgetUsable::ClientPrecache(CEntityConfig *pConfig)
{
    IGadgetEntity::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetUseEffectPath().empty())
        g_ResourceManager.Register(pConfig->GetUseEffectPath(), RES_EFFECT);
}


/*====================
  IGadgetUsable::ServerPrecache
  ====================*/
void    IGadgetUsable::ServerPrecache(CEntityConfig *pConfig)
{
    IGadgetEntity::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetUseEffectPath().empty())
        g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetUseEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}

