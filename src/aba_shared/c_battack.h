// (C)2007 S2 Games
// c_battack.h
//
//=============================================================================
#ifndef __C_BATTACK_H__
#define __C_BATTACK_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBAttack
//=============================================================================
class CBAttack : public IBehavior
{
private:
    // Parameters calculated during setup frame
    float   m_fDistSq;      
    CVec2f  m_v2ApproachPosition;
    float   m_fRange;
    bool    m_bAtGoal;
    bool    m_bSight;
    uint    m_uiArmingSequence;
    uint    m_uiAttacks;
    uint    m_uiMaxAttacks;
    bool    m_bAggroTrigger;

public:
    CBAttack(uint uiArmingSequence, uint uiMaxAttacks = uint(-1));

    virtual void        CopyFrom(const IBehavior* pBehavior);
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual bool    Validate();
    virtual void    Update();
    virtual void    BeginBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
    virtual void    CleanupFrame();

    virtual uint    GetAttackTarget() const     { return m_uiTargetIndex; }
    virtual void    DisableAggroTrigger()       { m_bAggroTrigger = true; }
};
//=============================================================================

#endif //__C_BATTACK_H__
