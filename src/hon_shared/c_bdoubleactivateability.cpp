// (C)2010 S2 Games
// c_bdoubleactivateability.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bdoubleactivateability.h"

#include "c_brain.h"
#include "i_unitentity.h"
#include "i_entitytool.h"
//=============================================================================

/*====================
  CBDoubleActivateAbility::CopyFrom
  ====================*/
void    CBDoubleActivateAbility::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBDoubleActivateAbility *pCBBehavior(static_cast<const CBDoubleActivateAbility*>(pBehavior));

    m_iInventorySlot = pCBBehavior->m_iInventorySlot;
    m_pAbility = pCBBehavior->m_pAbility;

    IBehavior::CopyFrom(pBehavior);
}


/*====================
  CBDoubleActivateAbility::Clone
  ====================*/
IBehavior*  CBDoubleActivateAbility::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBDoubleActivateAbility)( m_iInventorySlot ) );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBDoubleActivateAbility::Validate
  ====================*/
bool    CBDoubleActivateAbility::Validate()
{
    if (!IBehavior::Validate())
    {
        SetFlag(BSR_END);
        return false;
    }

    if (m_pSelf->IsIllusion())
    {
        SetFlag(BSR_END);
        return false;
    }

    if (m_pSelf->HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK) &&
        m_iInventorySlot >= INVENTORY_START_BACKPACK &&
        m_iInventorySlot <= INVENTORY_END_BACKPACK)
    {
        SetFlag(BSR_END);
        return false;
    }

    m_pAbility = m_pSelf->GetTool(m_iInventorySlot);
    if (m_pAbility == NULL)
    {
        SetFlag(BSR_END);
        return false;
    }

    switch (m_pAbility->GetActionType())
    {
    case TOOL_ACTION_TARGET_POSITION:
    case TOOL_ACTION_TARGET_ENTITY:
        break;
    default:
        SetFlag(BSR_END);
        return false;
    }

    if (!m_pAbility->GetDoubleActivate())
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  CBDoubleActivateAbility::Update
  ====================*/
void    CBDoubleActivateAbility::Update()
{
}


/*====================
  CBDoubleActivateAbility::BeginBehavior
  ====================*/
void    CBDoubleActivateAbility::BeginBehavior()
{
    if (m_pSelf == NULL || m_pAbility == NULL)
    {
        Console << _T("CBDoubleActivateAbility: Behavior started without valid information") << newl;
        return;
    }

    m_uiLastUpdate = INVALID_TIME;
    ClearFlag(BSR_NEW);
}


/*====================
  CBDoubleActivateAbility::ThinkFrame
  ====================*/
void    CBDoubleActivateAbility::ThinkFrame()
{
    if (~GetFlags() & BSR_END)
    {
        if (m_pAbility->HasActionScript(ACTION_SCRIPT_DOUBLE_ACTIVATE))
        {
            // if there is an <ondoubleactivate> callback, then execute it.
            m_pAbility->ExecuteActionScript(ACTION_SCRIPT_DOUBLE_ACTIVATE, m_pSelf, m_pSelf->GetPosition());
        }
        else
        {
            // otherwise, execute default doubleactivate behavior.
            switch (m_pAbility->GetActionType())
            {
            default: break;
            case TOOL_ACTION_TARGET_POSITION:
                {
                    uint uiAllySpawnScheme(Game.LookupTargetScheme(_T("ally_well")));
                    assert(uiAllySpawnScheme != INVALID_TARGET_SCHEME);
                    if (uiAllySpawnScheme != INVALID_TARGET_SCHEME)
                    {
                        uint uiSpawnIdx(INVALID_INDEX);
                        CVec2f v2SpawnPos(V2_ZERO);

                        const UnitList &lUnits(Game.GetUnitList());
                        for (UnitList_cit itEntity(lUnits.begin()), itEntityEnd(lUnits.end()); itEntity != itEntityEnd; ++itEntity)
                        {
                            if (!Game.IsValidTarget(uiAllySpawnScheme, 0, m_pSelf, *itEntity, true))
                                continue;

                            IUnitEntity* pUnit(*itEntity);
                            uiSpawnIdx = pUnit->GetIndex();
                            v2SpawnPos = pUnit->GetPosition().xy();
                            break;
                        }

                        if (uiSpawnIdx != INVALID_INDEX)
                        {
                            SUnitCommand cmd;
                            cmd.eCommandID = UNITCMD_ABILITY;
                            cmd.uiIndex = INVALID_INDEX;
                            cmd.v2Dest = v2SpawnPos;
                            cmd.uiParam = m_iInventorySlot;
                            cmd.yQueue = QUEUE_FRONT;
                            m_pSelf->PlayerCommand(cmd);
                        }
                    }
                }
                break;
            case TOOL_ACTION_TARGET_ENTITY:
                {
                    // for target_entity abilities, target our hero.
                    SUnitCommand cmd;
                    cmd.eCommandID = UNITCMD_ABILITY;
                    cmd.uiIndex = m_pSelf->GetIndex();
                    cmd.v2Dest = m_pSelf->GetPosition().xy();
                    cmd.uiParam = m_iInventorySlot;
                    cmd.yQueue = QUEUE_FRONT;
                    m_pSelf->PlayerCommand(cmd);
                }
                break;
            }
        }
        SetFlag(BSR_END);
    }
}


/*====================
  CBDoubleActivateAbility::MovementFrame
  ====================*/
void    CBDoubleActivateAbility::MovementFrame()
{
}


/*====================
  CBDoubleActivateAbility::ActionFrame
  ====================*/
void    CBDoubleActivateAbility::ActionFrame()
{
}


/*====================
  CBDoubleActivateAbility::CleanupFrame
  ====================*/
void    CBDoubleActivateAbility::CleanupFrame()
{
}


/*====================
  CBDoubleActivateAbility::EndBehavior
  ====================*/
void    CBDoubleActivateAbility::EndBehavior()
{
    IBehavior::EndBehavior();
}
