// (C)2007 S2 Games
// c_petcommandattack.h
//
//=============================================================================
#ifndef __C_PETCOMMANDATTACK_H__
#define __C_PETCOMMANDATTACK_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_spellitem.h"
//=============================================================================

//=============================================================================
// CPetCommandAttack
//=============================================================================
class CPetCommandAttack : public ISpellItem
{
private:
	DECLARE_ENT_ALLOCATOR2(PetCommand, Attack);

public:
	~CPetCommandAttack()	{}
	CPetCommandAttack() : ISpellItem(GetEntityConfig())	{}

	virtual void	ImpactEntity(uint uiTargetIndex, CGameEvent &evImpact, bool bCheckTarget = true);
};
//=============================================================================

#endif //__C_PETCOMMANDATTACK_H__
