// (C)2007 S2 Games
// c_spellsummonimp.h
//
//=============================================================================
#ifndef __C_SPELLSUMMONIMP_H__
#define __C_SPELLSUMMONIMP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellsummon.h"
//=============================================================================

//=============================================================================
// CSpellSummonImp
//=============================================================================
class CSpellSummonImp : public ISpellSummon
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, SummonImp);

public:
    ~CSpellSummonImp()  {}
    CSpellSummonImp() :
    ISpellSummon(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SPELLSUMMONIMP_H__
