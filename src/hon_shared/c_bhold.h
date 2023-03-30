// (C)2008 S2 Games
// c_bhold.h
//
//=============================================================================
#ifndef __C_BHOLD_H__
#define __C_BHOLD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
#define HOLD_FLAGS_NONE                     0
#define HOLD_FLAGS_CANCEL_ANIMATIONS        BIT(1)
//=============================================================================

//=============================================================================
// CBHold
//=============================================================================
class CBHold : public IBehavior
{
private:
    uint m_uiFlags;

public:
    CBHold(uint uiFlags) :
        IBehavior(EBT_HOLD),
        m_uiFlags(uiFlags)
    {}

    virtual void        CopyFrom(const IBehavior* pBehavior);
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual bool    Validate();
    virtual void    Update() {}
    virtual void    BeginBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
    virtual void    CleanupFrame();
};
//=============================================================================

#endif // __C_BHOLD_H__
