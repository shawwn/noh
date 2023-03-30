// (C)2006 S2 Games
// c_spellcommanderresurrect.h
//
//=============================================================================
#ifndef _C_SPELLCOMMANDERRESURRECT_H__
#define _C_SPELLCOMMANDERRESURRECT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellrevive.h"
//=============================================================================

//=============================================================================
// CSpellCommanderResurrect
//=============================================================================
class CSpellCommanderResurrect : public ISpellRevive
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, CommanderResurrect);

public:
	~CSpellCommanderResurrect()	{}
	CSpellCommanderResurrect() :
	ISpellRevive(GetEntityConfig())
	{}
};
//=============================================================================

#endif //_C_SPELLCOMMANDERRESURRECT_H__
