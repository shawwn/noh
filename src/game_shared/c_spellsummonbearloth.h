// (C)2007 S2 Games
// c_spellsummonbearloth.h
//
//=============================================================================
#ifndef __C_SPELLSUMMONBEARLOTH_H__
#define __C_SPELLSUMMONBEARLOTH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellsummon.h"
//=============================================================================

//=============================================================================
// CSpellSummonBearloth
//=============================================================================
class CSpellSummonBearloth : public ISpellSummon
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, SummonBearloth);

public:
	~CSpellSummonBearloth()	{}
	CSpellSummonBearloth() :
	ISpellSummon(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLSUMMONBEARLOTH_H__
