// (C)2007 S2 Games
// c_spellcommanderdispel.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERDISPEL_H__
#define __C_SPELLCOMMANDERDISPEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderDispel
//=============================================================================
class CSpellCommanderDispel : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, CommanderDispel)

public:
    ~CSpellCommanderDispel()    {}
    CSpellCommanderDispel() :
    ISpellItem(GetEntityConfig())
    {}

    virtual bool    ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);
};
//=============================================================================

#endif //__C_SPELLCOMMANDERDISPEL_H__
