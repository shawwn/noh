// (C)2007 S2 Games
// c_spellbeastofficerportal.h
//
//=============================================================================
#ifndef __C_SPELLBEASTOFFICERPORTAL_H__
#define __C_SPELLBEASTOFFICERPORTAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellBeastOfficerPortal
//=============================================================================
class CSpellBeastOfficerPortal : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, BeastOfficerPortal);

public:
	~CSpellBeastOfficerPortal() {}
	CSpellBeastOfficerPortal() : ISpellItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_SPELLBEASTOFFICERPORTAL_H__
