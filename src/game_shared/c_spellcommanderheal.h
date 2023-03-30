// (C)2007 S2 Games
// c_spellcommanderheal.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERHEAL_H__
#define __C_SPELLCOMMANDERHEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderHeal
//=============================================================================
class CSpellCommanderHeal : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, CommanderHeal)

public:
    ~CSpellCommanderHeal()  {}
    CSpellCommanderHeal() :
    ISpellItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERHEAL_H__
