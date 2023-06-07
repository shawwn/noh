// (C)2007 S2 Games
// c_spellreincarnate.h
//
//=============================================================================
#ifndef __C_SPELLREINCARNATE_H__
#define __C_SPELLREINCARNATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellrevive.h"
//=============================================================================

//=============================================================================
// CSpellReincarnate
//=============================================================================
class CSpellReincarnate : public ISpellRevive
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, Reincarnate)

public:
    ~CSpellReincarnate()    {}
    CSpellReincarnate() :
    ISpellRevive(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLREINCARNATE_H__
