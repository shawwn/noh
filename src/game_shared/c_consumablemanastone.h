// (C)2006 S2 Games
// c_consumablemanastone.h
//
//=============================================================================
#ifndef __C_CONSUMABLEMANASTONE_H__
#define __C_CONSUMABLEMANASTONE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CConsumableManaStone
//=============================================================================
class CConsumableManaStone : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, ManaStone);

public:
	~CConsumableManaStone() {}
	CConsumableManaStone() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEMANASTONE_H__
