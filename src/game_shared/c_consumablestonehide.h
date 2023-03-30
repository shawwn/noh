// (C)2006 S2 Games
// c_consumablestonehide.h
//
//=============================================================================
#ifndef __C_CONSUMABLESTONEHIDE_H__
#define __C_CONSUMABLESTONEHIDE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableStoneHide
//=============================================================================
class CConsumableStoneHide : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, StoneHide);

public:
	~CConsumableStoneHide() {}
	CConsumableStoneHide() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLESTONEHIDE_H__
