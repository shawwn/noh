// (C)2006 S2 Games
// c_spellresurrect.h
//
//=============================================================================
#ifndef _C_SPELLRESURRECT_H__
#define _C_SPELLRESURRECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellrevive.h"
//=============================================================================

//=============================================================================
// CSpellResurrect
//=============================================================================
class CSpellResurrect : public ISpellRevive
{
private:
    DECLARE_ENT_ALLOCATOR2(Spell, Resurrect);

public:
    ~CSpellResurrect()  {}
    CSpellResurrect() :
    ISpellRevive(GetEntityConfig())
    {}
};
//=============================================================================

#endif //_C_SPELLRESURRECT_H__
