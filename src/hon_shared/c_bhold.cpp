// (C)2008 S2 Games
// c_bhold.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_bhold.h"

#include "c_brain.h"
//=============================================================================

/*====================
  CBHold::CopyFrom
  ====================*/
void    CBHold::CopyFrom(const IBehavior* pBehavior)
{
    assert( GetType() == pBehavior->GetType() );
    if (GetType() != pBehavior->GetType())
        return;

    const CBHold *pCBBehavior(static_cast<const CBHold*>(pBehavior));

    m_uiFlags = pCBBehavior->m_uiFlags;

    IBehavior::CopyFrom(pCBBehavior);
}

/*====================
  CBHold::Clone
  ====================*/
IBehavior*  CBHold::Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const
{
    IBehavior* pBehavior( K2_NEW(ctx_Game,    CBHold)(m_uiFlags) );
    pBehavior->SetBrain(pNewBrain);
    pBehavior->SetSelf(pNewSelf);
    pBehavior->CopyFrom(this);
    return pBehavior;
}

/*====================
  CBHold::Validate
  ====================*/
bool    CBHold::Validate()
{
    if (!IBehavior::Validate())
    {
        SetFlag(BSR_END);
        return false;
    }

    return true;
}


/*====================
  CBHold::BeginBehavior
  ====================*/
void    CBHold::BeginBehavior()
{
    if (m_pSelf == NULL)
    {
        Console << _T("CBHold: Behavior started without valid information") << newl;
        return;
    }

    if (m_uiFlags & HOLD_FLAGS_CANCEL_ANIMATIONS)
        m_pBrain->EndActionStates(2);
    else
        m_pBrain->EndActionStates(1);

    ClearFlag(BSR_NEW);
}


/*====================
  CBHold::ThinkFrame
  ====================*/
void    CBHold::ThinkFrame()
{
    m_pSelf->Interrupt(UNIT_ACTION_STOP);

    m_pBrain->EndActionStates(0);

    m_pBrain->SetMoving(false);
}


/*====================
  CBHold::MovementFrame
  ====================*/
void    CBHold::MovementFrame()
{
    m_pSelf->Interrupt(UNIT_ACTION_STOP);

    if (m_pSelf->GetCanRotate() && !m_pSelf->IsImmobilized(false, true) && !m_pSelf->IsStunned())
    {
        float fDeltaTime(MsToSec(Game.GetFrameLength()));
        CVec3f v3Angles(m_pSelf->GetAngles());
        float fGoalYaw(m_pSelf->GetAttentionAngles()[YAW]);
        float fYawDelta(M_ChangeAngle(m_pSelf->GetTurnRate() * fDeltaTime, v3Angles[YAW], fGoalYaw) - v3Angles[YAW]);

        v3Angles[YAW] += fYawDelta;

        m_pSelf->SetAngles(v3Angles);
    }

    m_pBrain->EndActionStates(0);
}


/*====================
  CBHold::ActionFrame
  ====================*/
void    CBHold::ActionFrame()
{
    m_pSelf->Interrupt(UNIT_ACTION_STOP);

    m_pBrain->EndActionStates(0);

    if (m_pBrain->GetCurrentBehavior() == this && m_pBrain->GetBehaviorsPending() > 0)
        SetFlag(BSR_END);
}


/*====================
  CBHold::CleanupFrame
  ====================*/
void    CBHold::CleanupFrame()
{
}
