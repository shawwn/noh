// (C)2009 S2 Games
// c_battackfollow.h
//
//=============================================================================
#ifndef __C_BATTACKFOLLOW_H__
#define __C_BATTACKFOLLOW_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
#include "c_bfollow.h"
#include "c_battack.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CBAttackFollow
//=============================================================================
class CBAttackFollow : public CBFollow
{
private:
    CBAttack    m_Attack;
    bool        m_bAttacking;
    uint        m_uiLastAggroUpdate;
    uint        m_uiPrimaryTarget;
    uint        m_uiPrimaryTargetTime;

    void    UpdateAggro();

public:
    ~CBAttackFollow()   {}
    CBAttackFollow() :
    m_Attack(INVALID_INDEX),
    m_bAttacking(false),
    m_uiLastAggroUpdate(INVALID_TIME),
    m_uiPrimaryTarget(INVALID_INDEX),
    m_uiPrimaryTargetTime(INVALID_TIME)
    {
        SetType(EBT_ATTACKFOLLOW);
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

#endif //__C_BATTACKFOLLOW_H__
