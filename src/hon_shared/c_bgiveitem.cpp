// (C)2008 S2 Games
// c_bgiveitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bgiveitem.h"

#include "c_asMoving.h"
#include "c_asAttacking.h"
#include "c_brain.h"
#include "i_unitentity.h"
#include "i_entityitem.h"
//=============================================================================

/*====================
  CBGiveItem::CopyFrom
  ====================*/
void    CBGiveItem::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBGiveItem *pCBBehavior(static_cast<const CBGiveItem*>(pBehavior));

    m_uiItemUID = pCBBehavior->m_uiItemUID;
    m_fDistSq = pCBBehavior->m_fDistSq;     
    m_v2ApproachPosition = pCBBehavior->m_v2ApproachPosition;
    m_fRange = pCBBehavior->m_fRange;

    IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBGiveItem::Clone
  ====================*/
IBehavior*  CBGiveItem::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBGiveItem)(m_uiItemUID) );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBGiveItem::Validate
  ====================*/
bool    CBGiveItem::Validate()
{
    if (!IBehavior::Validate())
    {
        SetFlag(BSR_END);
        return false;
    }

    // Check if we still have this item
    IEntityItem *pItem(nullptr);
    for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
    {
        IEntityItem *pCurItem(m_pSelf->GetItem(i));
        if (pCurItem && pCurItem->GetUniqueID() == m_uiItemUID)
        {
            pItem = pCurItem;
            break;
        }
    }
    if (pItem == nullptr)
    {
        SetFlag(BSR_END);
        return false;
    }

    // Check whether the target can accept the item
    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (!m_pSelf->CanGiveItem(pItem, pTarget))
    {
        SetFlag(BSR_END);
        return false;
    }


    return true;
}


/*====================
  CBGiveItem::Update
  ====================*/
void    CBGiveItem::Update()
{
    if (m_pSelf->IsStunned() || m_pSelf->IsImmobilized(true, true))
        return;

    // Check for path changes
    m_uiLastUpdate = Game.GetGameTime();
    m_v2UpdatedGoal = m_v2ApproachPosition;
    FindPathToUpdatedGoal();
}


/*====================
  CBGiveItem::BeginBehavior
  ====================*/
void    CBGiveItem::BeginBehavior()
{
    if (m_pSelf == nullptr)
    {
        Console << _T("CBGiveItem: Behavior started without valid information") << newl;
        return;
    }

    m_pBrain->EndActionStates(1);

    m_fDistSq = FAR_AWAY;
    m_v2ApproachPosition = V2_ZERO;
    m_fRange = 0.0f;

    m_uiLastUpdate = INVALID_TIME;
    ClearFlag(BSR_NEW);
}


/*====================
  CBGiveItem::ThinkFrame
  ====================*/
void    CBGiveItem::ThinkFrame()
{
    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (pTarget == nullptr)
        return;

    // Check for a target that has become invalid
    if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
        !m_pSelf->CanSee(pTarget))
    {
        SetFlag(BSR_END);
        return;
    }

    // if out of range we'll need to chase target
    CVec3f v3Position(m_pSelf->GetPosition());
    CVec3f v3TargetPosition(pTarget->GetPosition());
    
    m_fDistSq = DistanceSq(v3Position.xy(), v3TargetPosition.xy());
    m_v2ApproachPosition = pTarget->GetApproachPosition(m_pSelf->GetPosition(), m_pSelf->GetBounds()).xy();
    m_fRange = m_pSelf->GetBounds().GetDim(X) * DIAG + g_transferRange + pTarget->GetBounds().GetDim(X) * DIAG;

    if (m_fDistSq <= SQR(m_fRange))
    {
        m_pBrain->SetMoving(false);
        return;
    }

    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));
    
    // Not ready yet
    if (pActiveState != pGoalState)
    {
        m_pBrain->SetMoving(false);
        return;
    }
    
    // Moving unless blocked
    m_pBrain->SetMoving(m_uiLastUpdate == INVALID_TIME || !static_cast<CASMoving *>(pGoalState)->IsBlocked());
}


/*====================
  CBGiveItem::MovementFrame
  ====================*/
void    CBGiveItem::MovementFrame()
{
    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (pTarget == nullptr)
        return;

    // Check for a target that has become invalid
    if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
        !m_pSelf->CanSee(pTarget))
    {
        SetFlag(BSR_END);
        return;
    }

    if (m_fDistSq <= SQR(m_fRange))
        return;

    IActionState *pActiveState(m_pBrain->AttemptActionState(ASID_MOVING, 0));
    IActionState *pGoalState(m_pBrain->GetActionState(ASID_MOVING));

    // Not ready yet
    if (pActiveState != pGoalState)
        return;

    // Perform updates to path when moving and the target has strayed
    float fNewDistSq(DistanceSq(m_v2UpdatedGoal, m_v2ApproachPosition));
    if (((m_uiLastUpdate == INVALID_TIME || fNewDistSq > SQR(PATH_RECALC_DISTANCE)) && !static_cast<CASMoving *>(pGoalState)->GetBlocked()) ||
        static_cast<CASMoving *>(pGoalState)->ShouldTryUnblock())
        Update();

    // Set move state params
    CASMoving *pMovingState(static_cast<CASMoving*>(pGoalState));
    
    CVec3f v3Angles(m_pSelf->GetAngles());

    CVec2f v2Movement(V2_ZERO);
    float fYawDelta(0.0f);
    float fGoalYaw(v3Angles[YAW]);
    bool bAtGoal(false);

    GetMovement(v2Movement, fYawDelta, bAtGoal, fGoalYaw);

    m_pSelf->SetAttentionYaw(fGoalYaw);

    if (bAtGoal)
    {
        // if we're still out of range head straight for the target
        CVec2f v2Position(m_pSelf->GetPosition().xy());
        CVec2f v2TargetPosition(pTarget->GetPosition().xy());
        float fDistSq(DistanceSq(v2Position, v2TargetPosition));
        float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + g_transferRange + pTarget->GetBounds().GetDim(X) * DIAG);

        if (fDistSq > SQR(fRange))
        {
            float fDeltaTime(MsToSec(Game.GetFrameLength()));
            CVec2f v2DirectionOfMovement(Normalize(v2TargetPosition - v2Position));

            // Turn the unit to face the direction of movement as we progress
            float fGoalYaw(M_GetYawFromForwardVec2(v2DirectionOfMovement));

            fYawDelta = M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW];

            v2Movement = v2DirectionOfMovement * m_pSelf->GetMoveSpeed() * fDeltaTime;
        }
    }

    pMovingState->SetMovement(v2Movement, fYawDelta, m_bDirectPathing);
}


/*====================
  CBGiveItem::ActionFrame
  ====================*/
void    CBGiveItem::ActionFrame()
{
    // No dropping items while attacking or activating
    if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
        if (!m_pBrain->GetActionState(ASID_ATTACKING)->EndState(1))
            return;

    if (m_pBrain->GetActionState(ASID_CASTING)->GetFlags() & ASR_ACTIVE)
        if (!m_pBrain->GetActionState(ASID_CASTING)->EndState(1))
            return;

    IUnitEntity *pTarget(Game.GetUnitEntity(m_uiTargetIndex));
    if (pTarget == nullptr)
        return;

    // Check for a target that has become invalid
    if (pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
        !m_pSelf->CanSee(pTarget))
    {
        SetFlag(BSR_END);
        return;
    }

    // if out of range chase target
    CVec3f v3Position(m_pSelf->GetPosition());
    CVec3f v3TargetPosition(pTarget->GetPosition());
    float fDistSq(DistanceSq(v3Position.xy(), v3TargetPosition.xy()));
    float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + g_transferRange + pTarget->GetBounds().GetDim(X) * DIAG);

    // check if were in range during either setup frame or action frame
    if (m_fDistSq > SQR(m_fRange) && fDistSq > SQR(fRange))
        return;

    for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
    {
        if (m_pSelf->GetItem(i) && m_pSelf->GetItem(i)->GetUniqueID() == m_uiItemUID)
        {
            ushort unType(m_pSelf->GetItem(i)->GetIndex());
            int iSlot(pTarget->TransferItem(GetIssuedClientNumber(), static_cast<IEntityItem *>(m_pSelf->GetItem(i))));

            if (iSlot != -1)
            {
                Game.LogItem(GAME_LOG_ITEM_TRANSFER, pTarget->GetItem(iSlot), m_pSelf);

                if (pTarget->GetOwnerClientNumber() != -1)
                {
                    CBufferFixed<6> cBuffer;
                    cBuffer << GAME_CMD_PICKUP_ITEM << ushort(pTarget->GetIndex()) << byte(iSlot) << unType;
                    Game.SendGameData(pTarget->GetOwnerClientNumber(), cBuffer, false);
                }
            }
        }
    }

    // Set unit attention angle
    CVec3f v3VecToTarget(v3TargetPosition - m_pSelf->GetPosition());
    float fAngleToTarget((M_GetYawFromForwardVec2(v3VecToTarget.xy())));
    m_pSelf->SetAttentionYaw(fAngleToTarget);

    SetFlag(BSR_END);
}


/*====================
  CBGiveItem::CleanupFrame
  ====================*/
void    CBGiveItem::CleanupFrame()
{
}
