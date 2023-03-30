// (C)2007 S2 Games
// c_propdestructable.h
//
//=============================================================================
#ifndef __C_PROPDYNAMIC_H__
#define __C_PROPDYNAMIC_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propentity.h"
//=============================================================================

//=============================================================================
// CPropDynamic
//=============================================================================
class CPropDynamic : public IPropEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Prop, Dynamic);

    uint                m_uiCorpseTime;

public:
    ~CPropDynamic() {}
    CPropDynamic();

    virtual bool        IsStatic() const                { return false; }

    virtual void        Spawn();
    virtual bool        ServerFrameMovement();
    virtual bool        ServerFrameCleanup();

    void                SetCorpseTime(uint uiCorpseTime)    { m_uiCorpseTime = uiCorpseTime; }
};
//=============================================================================

#endif //__C_PROPDYNAMIC_H__
