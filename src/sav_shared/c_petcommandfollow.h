// (C)2007 S2 Games
// c_petcommandfollow.h
//
//=============================================================================
#ifndef __C_PETCOMMANDFOLLOW_H__
#define __C_PETCOMMANDFOLLOW_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CPetCommandFollow
//=============================================================================
class CPetCommandFollow : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(PetCommand, Follow);

public:
    ~CPetCommandFollow()    {}
    CPetCommandFollow() : ISpellItem(GetEntityConfig()) {}

    virtual void    ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);
};
//=============================================================================

#endif //__C_PETCOMMANDFOLLOW_H__
