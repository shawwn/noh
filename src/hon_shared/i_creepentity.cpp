// (C)2008 S2 Games
// i_creepentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_creepentity.h"

#include "c_entitycreepspawner.h"
#include "c_lane.h"
#include "i_behavior.h"
#include "i_heroentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint                ICreepEntity::s_uiBaseType(ENTITY_BASE_TYPE_CREEP);

DEFINE_ENTITY_DESC(ICreepEntity, 1)
{
    s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
    s_cDesc.pFieldTypes->clear();
    const TypeVector &vBase(IUnitEntity::GetTypeVector());
    s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

    s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiCharges"), TYPE_INT, 6, 0));
}
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_UINTF      (creep_blockRepathTime,             100,            CVAR_GAMECONFIG);
CVAR_UINTF      (creep_blockRepathTimeExtra,        50,             CVAR_GAMECONFIG);
//=============================================================================

/*====================
  ICreepEntity::ICreepEntity
  ====================*/
ICreepEntity::ICreepEntity() :
m_uiCharges(0),
m_uiControllerUID(INVALID_INDEX),
m_v2Waypoint(FAR_AWAY, FAR_AWAY)
{
}


/*====================
  ICreepEntity::Baseline
  ====================*/
void    ICreepEntity::Baseline()
{
    IUnitEntity::Baseline();

    m_uiCharges = 0;
}


/*====================
  ICreepEntity::GetSnapshot
  ====================*/
void    ICreepEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
    // Base entity info
    IUnitEntity::GetSnapshot(snapshot, uiFlags);

    snapshot.WriteField(m_uiCharges);
}


/*====================
  ICreepEntity::ReadSnapshot
  ====================*/
bool    ICreepEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
    try
    {
        // Base entity info
        if (!IUnitEntity::ReadSnapshot(snapshot, 1))
            return false;

        snapshot.ReadField(m_uiCharges);
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("ICreepEntity::ReadSnapshot() - "), NO_THROW);
        return false;
    }
}


/*====================
  ICreepEntity::Copy
  ====================*/
void    ICreepEntity::Copy(const IGameEntity &B)
{
    IUnitEntity::Copy(B);

    const ICreepEntity *pB(B.GetAsCreep());

    if (!pB)    
        return;

    const ICreepEntity &C(*pB);

    m_uiCharges = C.m_uiCharges;
}


/*====================
  ICreepEntity::ServerFrameThink
  ====================*/
bool    ICreepEntity::ServerFrameThink()
{
    if (m_uiOwnerEntityIndex == INVALID_INDEX)
    {
        CEntityCreepSpawner *pController(Game.GetEntityFromUniqueIDAs<CEntityCreepSpawner>(m_uiControllerUID));

        if (pController != nullptr)
        {
            m_v2Waypoint = pController->GetLane().GetNextWaypoint(m_v3Position.xy(), m_v2Waypoint);

            // Issue default creep behavior (Attack Move)
            if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
            {
                m_cBrain.AddCommand(UNITCMD_ATTACKMOVE, false, m_v2Waypoint, INVALID_INDEX, uint(-1), true);
            }
            else
            {
                IBehavior *pBehavior(m_cBrain.GetCurrentBehavior());
                if (pBehavior != nullptr && pBehavior->GetGoal() != m_v2Waypoint)
                {
                    m_cBrain.GetCurrentBehavior()->SetGoal(m_v2Waypoint);
                    m_cBrain.GetCurrentBehavior()->ForceUpdate();
                }
            }
        }
    }
    else
    {
        // Issue default behavior (Guard Position)
        if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
            m_cBrain.AddCommand(UNITCMD_GUARD, false, m_v3Position.xy(), INVALID_INDEX, uint(-1), true);
    }

    return IUnitEntity::ServerFrameThink();
}


/*====================
  ICreepEntity::Spawn
  ====================*/
void    ICreepEntity::Spawn()
{
    IUnitEntity::Spawn();
}


/*====================
  ICreepEntity::GetThreatLevel
  ====================*/
float   ICreepEntity::GetThreatLevel(IUnitEntity *pOther, bool bCurrentTarget)
{
    float fThreatLevel(IUnitEntity::GetThreatLevel(pOther, bCurrentTarget));

    return fThreatLevel;
}


/*====================
  ICreepEntity::Die
  ====================*/
void    ICreepEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
    IUnitEntity::Die(pAttacker, unKillingObjectID);
}

