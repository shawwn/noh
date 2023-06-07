// (C)2008 S2 Games
// c_bdropitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "c_bdropitem.h"

#include "i_unitentity.h"
#include "c_entitychest.h"
#include "i_entityitem.h"
#include "c_asMoving.h"
#include "c_brain.h"
#include "../k2/c_path.h"
//=============================================================================

/*====================
  CBDropItem::CopyFrom
  ====================*/
void    CBDropItem::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBDropItem *pCBBehavior(static_cast<const CBDropItem*>(pBehavior));
    m_uiItemUID = pCBBehavior->m_uiItemUID;

    CBMove::CopyFrom(pCBBehavior);
}

/*====================
  CBDropItem::Clone
  ====================*/
IBehavior*  CBDropItem::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBDropItem)(m_uiItemUID) );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}


/*====================
  CBDropItem::Validate
  ====================*/
bool    CBDropItem::Validate()
{
    // DJH: Technically, this should be CBMove::Validate(), but it wasn't in here before I converted this
    //      to use shared Validate() functions and I'm not sure what effect it'll have with that change.
    //      CBDropItem should probably be changed to inheret from IBehavior and have a member CBMove (like CBAttackMove).
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

    // Check if we still have this item
    bool bFound(false);
    int iSlot(-1);
    for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
    {
        IEntityItem *pItem(m_pSelf->GetItem(i));
        if (pItem == nullptr)
            continue;

        if (pItem->GetNoDrop())
            continue;

        if (pItem->GetUniqueID() == m_uiItemUID)
        {
            iSlot = i;
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        SetFlag(BSR_END);
        return false;
    }

    if (m_pSelf->HasUnitFlags(UNIT_FLAG_LOCKED_BACKPACK))
    {
        SetFlag(BSR_END);
        return false;
    }
    
    return true;
}


/*====================
  CBDropItem::MovementFrame
  ====================*/
void    CBDropItem::MovementFrame()
{
    CBMove::MovementFrame();
    
    // Movement does not end when the goal is reached
    ClearFlag(BSR_END);
}


/*====================
  CBDropItem::ActionFrame
  ====================*/
void    CBDropItem::ActionFrame()
{
    // No dropping items while attacking or activating
    if (m_pBrain->GetActionState(ASID_ATTACKING)->GetFlags() & ASR_ACTIVE)
        if (!m_pBrain->GetActionState(ASID_ATTACKING)->EndState(1))
            return;

    if (m_pBrain->GetActionState(ASID_CASTING)->GetFlags() & ASR_ACTIVE)
        if (!m_pBrain->GetActionState(ASID_CASTING)->EndState(1))
            return;

    float fDistSq(DistanceSq(m_pSelf->GetPosition().xy(), m_v2UpdatedGoal));
    float fRange(m_pSelf->GetBounds().GetDim(X) * DIAG + 32.0f);

#if 0 // Only try to reach the destination once
    if (GetFlags() & BSR_SUCCESS)
        SetFlag(BSR_END);
#endif

    if (fDistSq > SQR(fRange))
        return;

    // If in range, drop the item
    for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
    {
        IEntityItem *pItem(m_pSelf->GetItem(i));
        if (pItem == nullptr)
            continue;
        if (!pItem->CanDrop() || pItem->HasFlag(ENTITY_TOOL_FLAG_LOCKED))
            continue;

        if (pItem->GetUniqueID() == m_uiItemUID)
        {
            // drop the item.
            Game.LogItem(GAME_LOG_ITEM_DROP, pItem);
            CVec3f v3DropPos(CVec3f(m_v2UpdatedGoal, Game.GetTerrainHeight(m_v2UpdatedGoal.x, m_v2UpdatedGoal.y)));
            pItem->Drop(v3DropPos, false);
        }
    }

    SetFlag(BSR_END);
}
