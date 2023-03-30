// (C)2007 S2 Games
// c_spellentangletrap.h
//
//=============================================================================
#ifndef __C_SPELLENTANGLETRAP_H__
#define __C_SPELLENTANGLETRAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellEntangleTrap
//=============================================================================
class CSpellEntangleTrap : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, EntangleTrap);

public:
	~CSpellEntangleTrap()	{}
	CSpellEntangleTrap() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLENTANGLETRAP_H__
