// (C)2007 S2 Games
// c_bmove.h
//
//=============================================================================
#ifndef __C_BMOVE_H__
#define __C_BMOVE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBMove
//=============================================================================
class CBMove : public IBehavior
{
private:

public:
    CBMove() :
    IBehavior(EBT_MOVE)
    { }

    virtual void        CopyFrom(const IBehavior* pBehavior);
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual bool    Validate();
    virtual void    Update();
    virtual void    BeginBehavior();
    virtual void    EndBehavior();
    virtual void    ThinkFrame();
    virtual void    MovementFrame();
    virtual void    ActionFrame() {}
    virtual void    CleanupFrame();
    virtual void    Moved();

    virtual bool    IsTraveling() const     { return true; }
};
//=============================================================================

#endif // __C_BMOVE_H__
