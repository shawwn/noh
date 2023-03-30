// (C)2006 S2 Games
// c_spellrainoffire.h
//
//=============================================================================
#ifndef __C_SPELLRAINOFFIRE_H__
#define __C_SPELLRAINOFFIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellRainOfFire
//=============================================================================
class CSpellRainOfFire : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, RainOfFire);

public:
    ~CSpellRainOfFire() {}
    CSpellRainOfFire() :
    ISpellItem(GetEntityConfig())
    {}

    bool    TryImpact();

    bool    ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_SPELLRAINOFFIRE_H__
