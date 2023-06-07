// (C)2008 S2 Games
// i_petentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_petentity.h"
#include "i_entitytool.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint        IPetEntity::s_uiBaseType(ENTITY_BASE_TYPE_PET);
//=============================================================================

/*====================
  IPetEntity::Spawn
  ====================*/
void    IPetEntity::Spawn()
{
    IUnitEntity::Spawn();

    // Pet's abilities match the level of the pet
    for (uint ui(INVENTORY_START_ABILITIES); ui <= INVENTORY_END_ABILITIES; ++ui)
    {
        if (m_apInventory[ui] == nullptr)
            continue;

        m_apInventory[ui]->SetLevel(GetLevel());
    }

    SetLifetime(Game.GetGameTime(), GetLifetime());
}


/*====================
  IPetEntity::ServerFrameThink
  ====================*/
bool    IPetEntity::ServerFrameThink()
{
    // Issue default behavior (Guard Position)
    if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
        m_cBrain.AddCommand(GetDefaultBehavior(), false, m_v3Position.xy(), INVALID_INDEX, uint(-1), true);

    return IUnitEntity::ServerFrameThink();
}


/*====================
  IPetEntity::ServerFrameCleanup
  ====================*/
bool    IPetEntity::ServerFrameCleanup()
{
    // Check for an expiring corpse
    if (GetStatus() == ENTITY_STATUS_CORPSE &&
        Game.GetGameTime() >= m_uiCorpseTime &&
        GetIsPersistent())
    {
        m_uiCorpseTime = INVALID_TIME;
        SetStatus(ENTITY_STATUS_DORMANT);
    }

    return IUnitEntity::ServerFrameCleanup();
}


/*====================
  IPetEntity::UpdateModifiers

  Inherit owner modifiers every frame on persistant pets
  ====================*/
void    IPetEntity::UpdateModifiers()
{
    IUnitEntity *pOwner(GetOwner());
    if (pOwner != nullptr && GetIsPersistent())
    {
        pOwner->UpdateModifiers();
        m_vPersistentModifierKeys = pOwner->GetModifierKeys();
    }

    IUnitEntity::UpdateModifiers();
}


/*====================
  IPetEntity::Die
  ====================*/
void    IPetEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
    if (GetStatus() != ENTITY_STATUS_ACTIVE)
        return;

    IUnitEntity::Die(pAttacker, unKillingObjectID);

    if (HasUnitFlags(UNIT_FLAG_ILLUSION))
        return;

    CPlayer *pPlayerOwner(Game.GetPlayerFromClientNumber(GetOwnerClientNumber()));

    if (pAttacker != nullptr && !GetProtectedDeath())
    {
        CPlayer *pPlayerKiller(Game.GetPlayerFromClientNumber(pAttacker->GetOwnerClientNumber()));      
                
        if (IsTargetType(_T("Courier"), pAttacker))
        {
            if (pPlayerKiller != nullptr)
            {
                // It was a player who killed the courier
                if (pPlayerOwner != nullptr)
                {
                    CBufferFixed<13> buffer;
                    buffer << GAME_CMD_KILL_COURIER_MESSAGE << pPlayerKiller->GetClientNumber() << pPlayerOwner->GetClientNumber() << GetIndex();
                    Game.BroadcastGameData(buffer, true);
                }
            }
            else
            {
                CBufferFixed<10> buffer;
                
                // It was the team that killed the courier, send a the team number to display the appropriate message
                if (pAttacker->GetTeam() == TEAM_1)
                {
                    buffer << GAME_CMD_KILL_COURIER_MESSAGE2 << byte(TEAM_1) << pPlayerOwner->GetClientNumber() << GetIndex();
                }
                else if (pAttacker->GetTeam() == TEAM_2)
                {
                    buffer << GAME_CMD_KILL_COURIER_MESSAGE2 << byte(TEAM_2) << pPlayerOwner->GetClientNumber() << GetIndex();
                }
                else if (pAttacker->GetTeam() == TEAM_NEUTRAL)
                {
                    buffer << GAME_CMD_KILL_COURIER_MESSAGE2 << byte(TEAM_NEUTRAL) << pPlayerOwner->GetClientNumber() << GetIndex();
                }
                    
                Game.BroadcastGameData(buffer, true);           
            }
        }
    }
}


/*====================
  IPetEntity::SetLevel
  ====================*/
void    IPetEntity::SetLevel(uint uiLevel)
{
    if (m_uiLevel != uiLevel)
    {
        m_uiLevel = uiLevel;

        // Abilities match the level of the gadget
        for (uint ui(INVENTORY_START_ABILITIES); ui <= INVENTORY_END_ABILITIES; ++ui)
        {
            if (m_apInventory[ui] == nullptr)
                continue;

            m_apInventory[ui]->SetLevel(GetLevel());
        }
    }
}
