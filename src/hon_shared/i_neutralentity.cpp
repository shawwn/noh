// (C)2008 S2 Games
// i_neutralentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_neutralentity.h"

#include "i_behavior.h"
#include "c_entityneutralcampcontroller.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint                INeutralEntity::s_uiBaseType(ENTITY_BASE_TYPE_NEUTRAL);

DEFINE_ENTITY_DESC(INeutralEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IUnitEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================

/*====================
  INeutralEntity::INeutralEntity
  ====================*/
INeutralEntity::INeutralEntity() :
m_v3SpawnPosition(0.0f, 0.0f, 0.0f),
m_uiSpawnControllerUID(INVALID_INDEX)
{
}


/*====================
  INeutralEntity::Baseline
  ====================*/
void    INeutralEntity::Baseline()
{
    IUnitEntity::Baseline();
}


/*====================
  INeutralEntity::GetSnapshot
  ====================*/
void    INeutralEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    // Base entity info
    IUnitEntity::GetSnapshot(snapshot, uiFlags);
}


/*====================
  INeutralEntity::ReadSnapshot
  ====================*/
bool    INeutralEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        // Base entity info
        if (!IUnitEntity::ReadSnapshot(snapshot, 1))
            return false;

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("INeutralEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  INeutralEntity::Copy
  ====================*/
void    INeutralEntity::Copy(const IGameEntity &B)
{
    IUnitEntity::Copy(B);

    const INeutralEntity *pB(B.GetAsNeutral());

    if (!pB)    
        return;

    //const INeutralEntity &C(*pB);
}


/*====================
  INeutralEntity::ServerFrameThink
  ====================*/
bool    INeutralEntity::ServerFrameThink()
{
    // If no commands are pending, issue default behavior
    if (m_uiOwnerEntityIndex == INVALID_INDEX)
    {
        // Execute NPC "think" script
        Action(ACTION_SCRIPT_THINK, this, nullptr);

        if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
            m_cBrain.AddCommand(UNITCMD_GUARD, false, m_v3SpawnPosition.xy(), INVALID_INDEX, uint(-1), true);
    }
    else
    {
        if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
            m_cBrain.AddCommand(UNITCMD_GUARD, false, m_v3Position.xy(), INVALID_INDEX, uint(-1), true);
    }

    return IUnitEntity::ServerFrameThink();
}


/*====================
  INeutralEntity::Spawn
  ====================*/
void    INeutralEntity::Spawn()
{
    IUnitEntity::Spawn();

    m_v3SpawnPosition = m_v3Position;
}


/*====================
  INeutralEntity::Die
  ====================*/
void    INeutralEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
    if (m_uiSpawnControllerUID != INVALID_INDEX)
    {
        CEntityNeutralCampController *pSpawnController(Game.GetEntityFromUniqueIDAs<CEntityNeutralCampController>(m_uiSpawnControllerUID));
        if (pSpawnController != nullptr)
            pSpawnController->CreepDied(GetUniqueID());
    }

    IUnitEntity::Die(pAttacker, unKillingObjectID);

    if (pAttacker != nullptr && IsTargetType(_T("Kongor"), pAttacker))
    {
        CPlayer *pPlayerKiller(Game.GetPlayerFromClientNumber(pAttacker->GetOwnerClientNumber()));
        if (pPlayerKiller != nullptr)
        {
            CBufferFixed<11> buffer;
            buffer << GAME_CMD_KILL_KONGOR_MESSAGE << pPlayerKiller->GetClientNumber() << pAttacker->GetTeam() << ushort(GetGoldBountyTeam());
            Game.BroadcastGameData(buffer, true);
        }
        else if (pAttacker->GetTeam() == 1 || pAttacker->GetTeam() == 2)
        {
            CBufferFixed<7> buffer;
            buffer << GAME_CMD_TEAM_KILL_KONGOR_MESSAGE << pAttacker->GetTeam() << ushort(GetGoldBountyConsolation());
            Game.BroadcastGameData(buffer, true);
        }
    }
}

/*====================
  INeutralEntity::OnTakeControl
  ====================*/
void    INeutralEntity::OnTakeControl(IUnitEntity *pOwner)
{
    if (m_uiSpawnControllerUID != INVALID_INDEX)
    {
        CEntityNeutralCampController *pSpawnController(Game.GetEntityFromUniqueIDAs<CEntityNeutralCampController>(m_uiSpawnControllerUID));
        if (pSpawnController != nullptr)
            pSpawnController->CreepDied(GetUniqueID());
    }
}
