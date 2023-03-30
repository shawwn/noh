// (C)2008 S2 Games
// c_bguard.h
//
//=============================================================================
#ifndef __C_BGUARD_H__
#define __C_BGUARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
#include "c_bmove.h"
#include "c_battack.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EGuardState
{
    GUARD_HOLDING = 0,
    GUARD_CHASING,
    GUARD_RETURNING
};
//=============================================================================

//=============================================================================
// CBGuard
//=============================================================================
class CBGuard : public CBMove
{
private:
    CBAttack    m_Attack;
    bool        m_bAttacking;
    uint        m_uiLastAggroUpdate;

    EGuardState m_eGuardState;
    uint        m_uiGuardStateEndTime;

    void    UpdateAggro();
    void    Aggro(IUnitEntity *pAttacker, uint uiChaseTime);

public:
    ~CBGuard()  {}
    CBGuard() :
    m_Attack(INVALID_INDEX),
    m_bAttacking(false),
    m_uiLastAggroUpdate(INVALID_TIME)
    {
        SetType(EBT_GUARD);
    }

    virtual void        CopyFrom(const IBehavior *pBehavior);
    virtual IBehavior*  Clone(CBrain *pNewBrain, IUnitEntity *pNewSelf) const;

    virtual void    BeginBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
    virtual void    CleanupFrame();
    virtual void    EndBehavior();

    virtual void    Damaged(IUnitEntity *pAttacker);
    virtual void    Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker);

    virtual bool    IsIdle() const          { return m_eGuardState == GUARD_HOLDING; }
    virtual bool    IsTraveling() const     { return m_eGuardState == GUARD_RETURNING; }
    virtual uint    GetAttackTarget() const { return m_bAttacking ? m_Attack.GetAttackTarget() : INVALID_INDEX; }
};
//=============================================================================

#endif // __C_BGUARD_H__
