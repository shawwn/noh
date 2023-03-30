// (C)2007 S2 Games
// i_skilltoggle.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_skilltoggle.h"
//=============================================================================

/*====================
  ISkillToggle::CEntityConfig::CEntityConfig
  ====================*/
ISkillToggle::CEntityConfig::CEntityConfig(const tstring &sName) :
ISkillItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(EndAnimName, _T("")),
INIT_ENTITY_CVAR(FinishTime, 0),
INIT_ENTITY_CVAR(SelfState, _T("")),
INIT_ENTITY_CVAR(RadiusState, _T("")),
INIT_ENTITY_CVAR(TargetStatusLiving, false),
INIT_ENTITY_CVAR(TargetStatusDead, false),
INIT_ENTITY_CVAR(TargetStatusCorpse, false),
INIT_ENTITY_CVAR(TargetTeamAlly, false),
INIT_ENTITY_CVAR(TargetTeamEnemy, false),
INIT_ENTITY_CVAR(TargetTypePlayer, false),
INIT_ENTITY_CVAR(TargetTypeVehicle, false),
INIT_ENTITY_CVAR(TargetTypeBuilding, false),
INIT_ENTITY_CVAR(TargetTypeGadget, false),
INIT_ENTITY_CVAR(TargetTypeHellbourne, false),
INIT_ENTITY_CVAR(TargetTypePet, false),
INIT_ENTITY_CVAR(TargetTypeNPC, false),
INIT_ENTITY_CVAR(TargetTypeSiege, false),
INIT_ENTITY_CVAR(Radius, 0.0f),
INIT_ENTITY_CVAR(ManaCostPerSecond, 0.0f),
INIT_ENTITY_CVAR(MaxTime, 0)
{
}


/*====================
  ISkillToggle::ActiveFrame
  ====================*/
void    ISkillToggle::ActiveFrame()
{
    if (Game.IsClient())
        return;

    if (!HasNetFlags(ITEM_NET_FLAG_ACTIVE))
        return;

    ICombatEntity *pOwner(GetOwnerEnt());
    if (pOwner == NULL)
        return;

    float fManaCost(GetManaCostPerSecond() * MsToSec(Game.GetFrameLength()));
    if (!pOwner->SpendMana(fManaCost))
    {
        Deactivate();
        return;
    }

    if (m_pEntityConfig->GetMaxTime() > 0 &&
        Game.GetGameTime() - m_uiActivationTime >= m_pEntityConfig->GetMaxTime())
    {
        Deactivate();
        return;
    }

    // Apply radius state
    ushort unStateID(0);
    if (!GetRadiusState().empty())
        unStateID = EntityRegistry.LookupID(GetRadiusState());
    if (unStateID != 0)
    {
        uivector vResult;
        Game.GetEntitiesInRadius(vResult, CSphere(pOwner->GetPosition(), GetRadius()), 0);

        for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
        {
            IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
            if (pEnt == NULL)
                continue;
            if (!IsValidTarget(pEnt, true))
                continue;

            pEnt->ApplyState(unStateID, Game.GetGameTime(), 3000, pOwner->GetIndex());
        }
    }
}


/*====================
  ISkillToggle::Impact
  ====================*/
void    ISkillToggle::Impact()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    SetNetFlags(ITEM_NET_FLAG_ACTIVE);

    if (Game.IsServer())
        pOwner->SelectItem(m_ySlot);

    m_uiActivationTime = Game.GetGameTime();

    // Apply self state
    uint uiTime(Game.GetGameTime());
    uint uiLength(m_pEntityConfig->GetMaxTime() == 0 ? INVALID_TIME : m_pEntityConfig->GetMaxTime());
    if (!GetSelfState().empty())
        m_iSelfStateSlot = pOwner->ApplyState(EntityRegistry.LookupID(GetSelfState()), uiTime, uiLength);
}


/*====================
  ISkillToggle::Deactivate
  ====================*/
void    ISkillToggle::Deactivate()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    RemoveNetFlags(ITEM_NET_FLAG_ACTIVE);
    pOwner->RemoveNetFlags(ENT_NET_FLAG_CHANNELING);

//  Game.SelectItem(pOwner->GetDefaultInventorySlot());
    if (m_iSelfStateSlot != -1)
    {
        pOwner->RemoveState(m_iSelfStateSlot);
        m_iSelfStateSlot = -1;
    }
    SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());
    pOwner->SetAction(PLAYER_ACTION_SKILL, Game.GetGameTime() + GetFinishTime());
    if (!GetEndAnimName().empty())
        pOwner->StartAnimation(GetEndAnimName(), 1);
}


/*====================
  ISkillToggle::ActivatePrimary
  ====================*/
bool    ISkillToggle::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    ActiveFrame();

    if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
        return false;

    if (HasNetFlags(ITEM_NET_FLAG_ACTIVE))
    {
        Deactivate();
        return true;
    }

    // Make sure they can use it
    if (IsDisabled())
        return false;

    // Check cooldown timer
    if (!IsReady())
        return false;
    
    // Check what mode the player is in
    IInventoryItem *pItem(pOwner->GetCurrentItem());
    if (pItem != NULL)
    {
        if ((pItem->IsMelee() && !GetCanUseWithMelee()) ||
            (pItem->IsGun() && !GetCanUseWithRanged()))
            return false;
    }

    // Check to make sure the owner is idle
    if (!pOwner->IsIdle())
        return false;

    // Check mana cost
    if (!pOwner->SpendMana(GetManaCost()))
        return false;

    // Animation
    if (!GetAnimName().empty())
        pOwner->StartAnimation(GetAnimName(), 1);

    // Create an event for the player activating this
    CSkillActivateEvent &activate(pOwner->GetSkillActivateEvent());
    activate.Clear();
    activate.SetOwner(pOwner);
    activate.SetSlot(m_ySlot);
    activate.SetActivateTime(Game.GetGameTime() + GetActivationTime());

    // Set the player's new action
    int iAction(PLAYER_ACTION_SKILL);
    if (GetFreeze())
    {
        pOwner->StopAnimation(0);
        iAction |= PLAYER_ACTION_IMMOBILE;
    }
    if (GetManaCostPerSecond() > 0.0f)
        pOwner->SetNetFlags(ENT_NET_FLAG_CHANNELING);
    
    uint uiEndTime(GetDuration() > 0 ? Game.GetGameTime() + GetDuration() : INVALID_TIME);
    pOwner->SetAction(iAction, uiEndTime);
    Game.SelectItem(m_ySlot);
    return true;
}


/*====================
  ISkillToggle::ClientPrecache
  ====================*/
void    ISkillToggle::ClientPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ClientPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetSelfState().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetSelfState()));
    if (!pConfig->GetRadiusState().empty())
        EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetRadiusState()));
}


/*====================
  ISkillToggle::ServerPrecache
  ====================*/
void    ISkillToggle::ServerPrecache(CEntityConfig *pConfig)
{
    IInventoryItem::ServerPrecache(pConfig);

    if (!pConfig)
        return;

    if (!pConfig->GetSelfState().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetSelfState()));
    if (!pConfig->GetRadiusState().empty())
        EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetRadiusState()));
}

/*====================
  ISkillToggle::IsValidTarget
  ====================*/
bool    ISkillToggle::IsValidTarget(IGameEntity *pEntity, bool bImpact)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    if (!pEntity || !pEntity->IsVisual())
        return false;

    IVisualEntity *pVisual(pEntity->GetAsVisualEnt());

    bool bValidStatus(false);
    if (m_pEntityConfig->GetTargetStatusLiving() && (pVisual->GetStatus() == ENTITY_STATUS_ACTIVE || pVisual->GetStatus() == ENTITY_STATUS_SPAWNING))
        bValidStatus = true;
    else if (m_pEntityConfig->GetTargetStatusDead() && pVisual->GetStatus() == ENTITY_STATUS_DEAD && !pVisual->HasNetFlags(ENT_NET_FLAG_NO_CORPSE))
        bValidStatus = true;
    else if (m_pEntityConfig->GetTargetStatusCorpse() && pVisual->GetStatus() == ENTITY_STATUS_CORPSE)
        bValidStatus = true;

    // When the target wants an ally, if the target is an ally or even just looks like an ally, they are valid
    // This means a player can be healed while disguised by either team, for instance
    // If the target wants an enemy, a disguised player is only valid when the effect is actually applied
    // This means they can't be snapped to, but an area effect will hit them, even though they don't "light up" as a target
    bool bValidTeam(false);
    if (m_pEntityConfig->GetTargetTeamAlly() && (!pOwner->LooksLikeEnemy(pVisual) || !pOwner->IsEnemy(pVisual)))
        bValidTeam = true;
    else if (m_pEntityConfig->GetTargetTeamEnemy() && pOwner->IsEnemy(pVisual) && (bImpact ?  true : pOwner->LooksLikeEnemy(pVisual)))
        bValidTeam = true;

    IPlayerEntity *pPlayer(pVisual->GetAsPlayerEnt());

    bool bValidType(false);
    if (m_pEntityConfig->GetTargetTypePlayer() && (pPlayer != NULL && !pPlayer->GetIsVehicle() && !pPlayer->GetIsHellbourne() && !pPlayer->GetIsSiege() && !pPlayer->IsObserver()))
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypePet() && pVisual->IsPet())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeNPC() && pVisual->IsNpc())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeVehicle() && pPlayer != NULL && pPlayer->GetIsVehicle())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeBuilding() && pVisual->IsBuilding())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeGadget() && pVisual->IsGadget())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeHellbourne() && pPlayer != NULL && pPlayer->GetIsHellbourne())
        bValidType = true;
    else if (m_pEntityConfig->GetTargetTypeSiege() && pPlayer != NULL && pPlayer->GetIsSiege())
        bValidType = true;

    return bValidStatus && bValidTeam && bValidType;
}
