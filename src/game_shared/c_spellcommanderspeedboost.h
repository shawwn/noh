// (C)2007 S2 Games
// c_spellcommanderspeedboost.h
//
//=============================================================================
#ifndef __C_SPELLCOMMANDERSPEEDBOOST_H__
#define __C_SPELLCOMMANDERSPEEDBOOST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CSpellCommanderSpeedBoost
//=============================================================================
class CSpellCommanderSpeedBoost : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Spell, CommanderSpeedBoost)

public:
	~CSpellCommanderSpeedBoost()	{}
	CSpellCommanderSpeedBoost() :
	ISpellItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_SPELLCOMMANDERSPEEDBOOST_H__
