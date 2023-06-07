// (C)2007 S2 Games
// c_spellcommandercenturionarmor.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERCENTURIONARMOR_H__
#define __C_SPELLCOMMANDERCENTURIONARMOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderCenturionArmor
//=============================================================================
class CSpellCommanderCenturionArmor : public ISpellItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, CommanderCenturionArmor)

public:
    ~CSpellCommanderCenturionArmor()    {}
    CSpellCommanderCenturionArmor() :
    ISpellItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERCENTURIONARMOR_H__
