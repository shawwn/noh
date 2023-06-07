// (C)2007 S2 Games
// c_spellmanafountain.h
//
//=============================================================================
#ifndef __C_SPELLMANAFOUNTAIN_H__
#define __C_SPELLMANAFOUNTAIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellManaFountain
//=============================================================================
class CSpellManaFountain : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, ManaFountain)

public:
    ~CSpellManaFountain() {}
    CSpellManaFountain() :
    ISpellItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SPELLMANAFOUNTAIN_H__
