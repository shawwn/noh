// (C)2008 S2 Games
// c_bsentry.h
//
//=============================================================================
#ifndef __C_BSENTRY_H__
#define __C_BSENTRY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBSentry
//=============================================================================
class CBSentry : public IBehavior
{
private:

public:
    CBSentry() :
    IBehavior(EBT_SENTRY)
    {}

    virtual void        CopyFrom(const IBehavior* pBehavior);
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual bool    Validate();
    virtual void    Update();
    virtual void    BeginBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
    virtual void    CleanupFrame();
};
//=============================================================================

#endif // __C_BSENTRY_H__
