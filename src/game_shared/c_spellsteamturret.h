// (C)2007 S2 Games
// c_spellsteamturret.h
//
//=============================================================================
#ifndef __C_SPELLSTEAMTURRET_H__
#define __C_SPELLSTEAMTURRET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellSteamTurret
//=============================================================================
class CSpellSteamTurret : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, SteamTurret);

public:
	~CSpellSteamTurret()	{}
	CSpellSteamTurret() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLSTEAMTURRET_H__
