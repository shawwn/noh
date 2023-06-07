// (C)2007 S2 Games
// c_spellhailstorm.h
//
//=============================================================================
#ifndef __C_SPELLHAILSTORM_H__
#define __C_SPELLHAILSTORM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellHailStorm
//=============================================================================
class CSpellHailStorm : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, HailStorm)

public:
    ~CSpellHailStorm()  {}
    CSpellHailStorm() :
    ISpellItem(GetEntityConfig())
    {}

    bool    TryImpact();
};
//=============================================================================

#endif //__C_SPELLHAILSTORM_H__
