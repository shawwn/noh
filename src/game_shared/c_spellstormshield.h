// (C)2007 S2 Games
// c_spellstormshield.h
//
//=============================================================================
#ifndef __C_SPELLSTORMSHIELD_H__
#define __C_SPELLSTORMSHIELD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellStormShield
//=============================================================================
class CSpellStormShield : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, StormShield)

public:
    ~CSpellStormShield()    {}
    CSpellStormShield() :
    ISpellItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLSTORMSHIELD_H__
