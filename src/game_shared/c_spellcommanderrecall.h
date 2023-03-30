// (C)2007 S2 Games
// c_spellcommanderrecall.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERRECALL_H__
#define __C_SPELLCOMMANDERRECALL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderRecall
//=============================================================================
class CSpellCommanderRecall : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, CommanderRecall)

public:
    ~CSpellCommanderRecall()    {}
    CSpellCommanderRecall() :
    ISpellItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERRECALL_H__
