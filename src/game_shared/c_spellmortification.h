// (C)2006 S2 Games
// c_spellmortification.h
//
//=============================================================================
#ifndef __C_SPELLMORTIFICATION_H__
#define __C_SPELLMORTIFICATION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellMortification
//=============================================================================
class CSpellMortification : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, Mortification);

public:
	~CSpellMortification()	{}
	CSpellMortification() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLMORTIFICATION_H__
