// (C)2007 S2 Games
// c_battackmove.h
//
//=============================================================================
#ifndef __C_BATTACKMOVE_H__
#define __C_BATTACKMOVE_H__

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
//=============================================================================

//=============================================================================
// CBAttackMove
//=============================================================================
class CBAttackMove : public CBMove
{
private:
    CBAttack    m_Attack;
    bool        m_bAttacking;
    uint        m_uiLastAggroUpdate;
    uint        m_uiPrimaryTarget;
    uint        m_uiPrimaryTargetTime;
    
    uint        m_uiDelayTarget;
    uint        m_uiDelayTime;
    uint        m_uiDelayDuration;
    bool        m_bDelayReaggroBlock;

    uint        m_uiLastAttackSequence;

    void    UpdateAggro();

public:
    ~CBAttackMove() {}
    CBAttackMove() :
    m_Attack(INVALID_INDEX),
    m_bAttacking(false),
    m_uiLastAggroUpdate(INVALID_TIME),
    m_uiPrimaryTarget(INVALID_INDEX),
    m_uiPrimaryTargetTime(INVALID_TIME),
    m_uiDelayTarget(INVALID_INDEX),
    m_uiDelayTime(INVALID_TIME),
    m_uiDelayDuration(0),
    m_bDelayReaggroBlock(false),
    m_uiLastAttackSequence(-1)
    {
        SetType(EBT_ATTACKMOVE);
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
    virtual bool    IsTraveling() const     { return false; }
    virtual uint    GetAttackTarget() const { return m_bAttacking ? m_Attack.GetAttackTarget() : INVALID_INDEX; }
};
//=============================================================================

#endif //__C_BATTACKMOVE_H__
