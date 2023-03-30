// (C)2006 S2 Games
// c_spelleruption.h
//
//=============================================================================
#ifndef __C_SPELLERUPTION_H__
#define __C_SPELLERUPTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellHeal
//=============================================================================
class CSpellEruption : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, Eruption);

    static  CCvarf  s_cvarDamage;
    static  CCvarui s_cvarStunLength;

public:
    ~CSpellEruption()   {}
    CSpellEruption() :
    ISpellItem(GetEntityConfig())
    {}

    bool            TryImpact();
};
//=============================================================================

#endif //__C_SPELLERUPTION_H__
