// (C)2006 S2 Games
// c_spelldefile.h
//
//=============================================================================
#ifndef __C_SPELLDEFILE_H__
#define __C_SPELLDEFILE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellHeal
//=============================================================================
class CSpellDefile : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, Defile);

	static	CCvarf	s_cvarDamage;

public:
	~CSpellDefile()	{}
	CSpellDefile() :
	ISpellItem(GetEntityConfig())
	{}

	bool			TryImpact();
};
//=============================================================================

#endif //__C_SPELLDEFILE_H__
