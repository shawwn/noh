// (C)2008 S2 Games
// c_bdropitem.h
//
//=============================================================================
#ifndef __C_BDROPITEM_H__
#define __C_BDROPITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_bmove.h"
//=============================================================================

//=============================================================================
// CBDropItem
//=============================================================================
class CBDropItem : public CBMove
{
private:
    uint    m_uiItemUID;

public:
    CBDropItem(uint uiItemUID) :
    m_uiItemUID(uiItemUID)
    {
        SetType(EBT_DROPITEM);
    }

    virtual void        CopyFrom(const IBehavior* pBehavior);
    virtual IBehavior*  Clone(CBrain* pNewBrain, IUnitEntity* pNewSelf) const;

    virtual bool    Validate();
    virtual void    MovementFrame();
    virtual void    ActionFrame();
};
//=============================================================================

#endif // __C_BDROPITEM_H__
