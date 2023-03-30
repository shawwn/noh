// (C)2007 S2 Games
// c_spellsummonworker.h
//
//=============================================================================
#ifndef __C_SPELLSUMMONWORKER_H__
#define __C_SPELLSUMMONWORKER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellsummon.h"
//=============================================================================

//=============================================================================
// CSpellSummonWorker
//=============================================================================
class CSpellSummonWorker : public ISpellSummon
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, SummonWorker)

public:
	~CSpellSummonWorker()	{}
	CSpellSummonWorker() :
	ISpellSummon(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLSUMMONWORKER_H__
