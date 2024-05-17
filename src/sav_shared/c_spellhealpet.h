// (C)2007 S2 Games
// c_spellhealpet.h
//
//=============================================================================
#ifndef __C_SPELLHEALPET_H__
#define __C_SPELLHEALPET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellHealPet
//=============================================================================
class CSpellHealPet : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, HealPet);

public:
    ~CSpellHealPet()    {}
    CSpellHealPet() :
    ISpellItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLHEALPET_H__