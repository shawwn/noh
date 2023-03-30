// (C)2009 S2 Games
// c_bassist.h
//
// Hybrid of Guard and Follow prioritizing following
//=============================================================================
#ifndef __C_BASSIST_H__
#define __C_BASSIST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
#include "c_bfollow.h"
#include "c_battack.h"
#include "c_bguard.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CBAssist
//=============================================================================
class CBAssist : public CBFollow
{
private:
    CBAttack    m_Attack;
    uint        m_uiLastAggroUpdate;
    bool        m_bAttacking;
    float       m_fAssistRange;

    void    UpdateAggro();
    void    Aggro(IUnitEntity *pAttacker, uint uiChaseTime);

public:
    virtual ~CBAssist() {}
    
    CBAssist() :
    m_Attack(INVALID_INDEX),
    m_uiLastAggroUpdate(INVALID_TIME),
    m_bAttacking(false)
    {
        SetType(EBT_ASSIST);
    }

    virtual void        CopyFrom(const IBehavior* pBehavior);
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual bool    Validate();
    virtual void    Update();
    virtual void    BeginBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
    virtual void    CleanupFrame();
    virtual void    EndBehavior();

    virtual void    Damaged(IUnitEntity *pAttacker);
    virtual void    Assist(IUnitEntity *pAlly, IUnitEntity *pAttacker);
    virtual uint    GetAttackTarget() const     { return m_bAttacking ? m_Attack.GetAttackTarget() : INVALID_INDEX; }

    virtual void    SetValue0(float fValue)     { m_fAssistRange = fValue; }
};
//=============================================================================

#endif //__C_BASSIST_H__
