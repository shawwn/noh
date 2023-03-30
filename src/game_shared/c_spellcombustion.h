// (C)2006 S2 Games
// c_spellcombustion.h
//
//=============================================================================
#ifndef __C_SPELLCOMBUSTION_H__
#define __C_SPELLCOMBUSTION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCombustion
//=============================================================================
class CSpellCombustion : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, Combustion);

public:
	~CSpellCombustion()	{}
	CSpellCombustion() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLCOMBUSTION_H__
