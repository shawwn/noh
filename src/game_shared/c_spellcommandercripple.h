// (C)2007 S2 Games
// c_spellcommandercripple.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERCRIPPLE_H__
#define __C_SPELLCOMMANDERCRIPPLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderCripple
//=============================================================================
class CSpellCommanderCripple : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, CommanderCripple)

public:
	~CSpellCommanderCripple()	{}
	CSpellCommanderCripple() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERCRIPPLE_H__
