// (C)2007 S2 Games
// c_spellcommanderfear.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERFEAR_H__
#define __C_SPELLCOMMANDERFEAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderFear
//=============================================================================
class CSpellCommanderFear : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, CommanderFear)

public:
	~CSpellCommanderFear()	{}
	CSpellCommanderFear() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERFEAR_H__
