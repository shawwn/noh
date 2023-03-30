// (C)2007 S2 Games
// c_spellcommanderreveal.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERREVEEAL_H__
#define __C_SPELLCOMMANDERREVEEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderReveal
//=============================================================================
class CSpellCommanderReveal : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, CommanderReveal)

public:
    ~CSpellCommanderReveal()    {}
    CSpellCommanderReveal() :
    ISpellItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERREVEEAL_H__
