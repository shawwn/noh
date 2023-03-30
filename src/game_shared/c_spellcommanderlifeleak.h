// (C)2007 S2 Games
// c_spellcommanderlifeleak.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERLIFELEAK_H__
#define __C_SPELLCOMMANDERLIFELEAK_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderLifeLeak
//=============================================================================
class CSpellCommanderLifeLeak : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, CommanderLifeLeak)

public:
    ~CSpellCommanderLifeLeak()  {}
    CSpellCommanderLifeLeak() :
    ISpellItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERLIFELEAK_H__
