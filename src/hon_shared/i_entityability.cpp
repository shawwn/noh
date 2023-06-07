// (C)2008 S2 Games
// i_entityability.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_entityability.h"
#include "i_heroentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint                IEntityAbility::s_uiBaseType(ENTITY_BASE_TYPE_ABILITY);

DEFINE_ENTITY_DESC(IEntityAbility, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    const TypeVector &vBase(IEntityTool::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================

/*====================
  IEntityAbility::IEntityAbility
  ====================*/
IEntityAbility::IEntityAbility()
{
}


/*====================
  IEntityAbility::Baseline
  ====================*/
void    IEntityAbility::Baseline()
{
    IEntityTool::Baseline();
}


/*====================
  IEntityAbility::GetSnapshot
  ====================*/
void    IEntityAbility::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    IEntityTool::GetSnapshot(snapshot, uiFlags);
}


/*====================
  IEntityAbility::ReadSnapshot
  ====================*/
bool    IEntityAbility::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        IEntityTool::ReadSnapshot(snapshot, 1);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("IEntityAbility::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  IEntityAbility::LevelUp
  ====================*/
void    IEntityAbility::LevelUp()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner == nullptr)
        return;

    if (!CanLevelUp())
        return;

    ++m_uiLevel;

    if (m_uiLevel == 1)
        ExecuteActionScript(ACTION_SCRIPT_LEARN, pOwner, pOwner->GetPosition());
    else
        ExecuteActionScript(ACTION_SCRIPT_UPGRADE, pOwner, pOwner->GetPosition());

    // Send a upgrade event to all players can that can control this unit
    CBufferFixed<4> buffer;
    buffer << GAME_CMD_INVENTORY_UPGRADE_EVENT << ushort(pOwner->GetIndex()) << m_ySlot;

    const PlayerMap &mapPlayers(Game.GetPlayerMap());
    for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
    {
        if (pOwner->CanReceiveOrdersFrom(itPlayer->first))
            Game.SendGameData(itPlayer->first, buffer, false);
    }

    Game.LogAbility(GAME_LOG_ABILITY_UPGRADE, this);

    if (pOwner != nullptr)
        pOwner->UpdateInventory();

    for (uiset_it it(m_setPersistentPetUID.begin()); it != m_setPersistentPetUID.end(); ++it)
    {
        IUnitEntity *pPet(Game.GetUnitEntity(Game.GetGameIndexFromUniqueID(*it)));
        if (pPet == nullptr)
            continue;
        if (pPet->GetStatus() != ENTITY_STATUS_ACTIVE)
            continue;

        pPet->SetLevel(GetLevel());
        pPet->UpdateInventory();
        pPet->SetModifierBits(GetModifierBits());
    }
}


/*====================
  IEntityAbility::CanLevelUp
  ====================*/
bool    IEntityAbility::CanLevelUp() const
{
    if (GetLevel() >= GetMaxLevel())
        return false;

    IUnitEntity *pOwner(GetOwner());
    if (pOwner == nullptr)
        return false;
    IHeroEntity *pHero(pOwner->GetAsHero());
    if (pHero == nullptr)
        return false;

    if (pHero->GetAvailablePoints() < 1)
        return false;

    if (GetRequiredLevelSize() == 0)
        return false;
    if (pHero->GetLevel() < GetRequiredLevel(MIN(GetRequiredLevelSize() - 1, GetLevel())))
        return false;

    return true;
}


/*====================
  IEntityAbility::IsDisabled
  ====================*/
bool    IEntityAbility::IsDisabled() const
{
    if (GetActionType() != TOOL_ACTION_PASSIVE)
    {
        IUnitEntity *pOwner(GetOwner());
        if (pOwner != nullptr && pOwner->IsSilenced() && !GetNoSilence())
            return true;
    }

    return IEntityTool::IsDisabled();
}


/*====================
  IEntityAbility::CanOrder
  ====================*/
bool    IEntityAbility::CanOrder()
{
    if (GetLevel() < 1 && GetMaxLevel() > 0)
        return false;

    return IEntityTool::CanOrder();
}


/*====================
  IEntityAbility::CanActivate
  ====================*/
bool    IEntityAbility::CanActivate()
{
    if (GetLevel() < 1 && GetMaxLevel() > 0)
        return false;

    return IEntityTool::CanActivate();
}


/*====================
  IEntityAbility::GetEffect

  Passive ability effects only play when the ability
  is leveled up
  ====================*/
ResHandle   IEntityAbility::GetEffect()
{
    if (GetLevel() < 1)
        return INVALID_RESOURCE;
    
    return IEntityTool::GetEffect();
}
