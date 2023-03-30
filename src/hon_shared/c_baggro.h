// (C)2009 S2 Games
// c_baggro.h
//
//=============================================================================
#ifndef __C_BAGGRO_H__
#define __C_BAGGRO_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
#include "c_bhold.h"
#include "c_battack.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CBAggro
//=============================================================================
class CBAggro : public CBHold
{
private:
    CBAttack    m_Attack;
    bool        m_bAttacking;
    uint        m_uiNextAggroUpdate;
    uint        m_uiPrimaryTarget;
    uint        m_uiPrimaryTargetTime;

    void    UpdateAggro();

public:
    ~CBAggro()  {}
    CBAggro() :
    CBHold(HOLD_FLAGS_NONE),
    m_Attack(INVALID_INDEX),
    m_bAttacking(false),
    m_uiNextAggroUpdate(INVALID_TIME),
    m_uiPrimaryTarget(INVALID_INDEX),
    m_uiPrimaryTargetTime(INVALID_TIME)
    {
        SetType(EBT_AGGRO);
    }

    virtual void        CopyFrom(const IBehavior* pBehavior);
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual void    BeginBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
    virtual void    CleanupFrame();
    virtual void    EndBehavior();

    virtual PoolHandle  GetPath() const     { return m_bAttacking ? m_Attack.GetPath() : IBehavior::GetPath(); }

    virtual void    Aggro(IUnitEntity *pTarget, uint uiDuration, uint uiDelay, bool bReaggroBlock);

    virtual bool    IsIdle() const          { return !m_bAttacking; }
    virtual uint    GetAttackTarget() const { return m_bAttacking ? m_Attack.GetAttackTarget() : INVALID_INDEX; }
};
//=============================================================================

#endif //__C_BAGGRO_H__
