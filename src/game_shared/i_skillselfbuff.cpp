// (C)2006 S2 Games
// i_skillselfbuff.h
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_skillselfbuff.h"
//=============================================================================

/*====================
  ISkillSelfBuff::CEntityConfig::CEntityConfig
  ====================*/
ISkillSelfBuff::CEntityConfig::CEntityConfig(const tstring &sName) :
ISkillItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(SelfState, _T("")),
INIT_ENTITY_CVAR(SelfStateDuration, 0)
{
}


/*====================
  ISkillSelfBuff::Impact
  ====================*/
void    ISkillSelfBuff::Impact()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    uint uiTime(Game.GetGameTime());

    if (!GetSelfState().empty())
        pOwner->ApplyState(EntityRegistry.LookupID(GetSelfState()), uiTime, GetSelfStateDuration());
}


/*====================
  ISkillSelfBuff::ClientPrecache
  ====================*/
void    ISkillSelfBuff::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;
    
    if (!pConfig->GetSelfState().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetSelfState()));
}


/*====================
  ISkillSelfBuff::ServerPrecache
  ====================*/
void    ISkillSelfBuff::ServerPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetSelfState().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetSelfState()));
}
