// (C)2008 S2 Games
// c_btouch.h
//
//=============================================================================
#ifndef __C_BTOUCH_H__
#define __C_BTOUCH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_behavior.h"
//=============================================================================

//=============================================================================
// CBTouch
//=============================================================================
class CBTouch : public IBehavior
{
private:
    // Parameters calculated during setup frame
    float   m_fDistSq;      
    CVec2f  m_v2ApproachPosition;
    float   m_fRange;

public:
    CBTouch() :
    IBehavior(EBT_TOUCH)
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

#endif // __C_BTOUCH_H__
