// (C)2009 S2 Games
// c_baggressivewander.h
//=============================================================================
#ifndef __C_BAGGRESSIVEWANDER_H__
#define __C_BAGGRESSIVEWANDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_bwander.h"
#include "c_battack.h"
//=============================================================================

//=============================================================================
// CBAggressiveWander
//=============================================================================
class CBAggressiveWander : public CBWander
{
private:
    CBAttack    m_Attack;
    uint        m_uiLastAggroUpdate;
    bool        m_bAttacking;

    void    UpdateAggro();
    void    Aggro(IUnitEntity *pAttacker, uint uiChaseTime);

public:
    virtual ~CBAggressiveWander()   {}
    
    CBAggressiveWander() :
    m_Attack(INVALID_INDEX),
    m_uiLastAggroUpdate(INVALID_TIME),
    m_bAttacking(false)
    {
        SetType(EBT_AGGRESSIVEWANDER);
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
};
//=============================================================================

#endif  //__C_BAGGRESSIVEWANDER_H__