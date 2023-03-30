// (C)2006 S2 Games
// c_consumablehealthreplenish.h
//
//=============================================================================
#ifndef __C_CONSUMABLEHEALTHREPLENISH_H__
#define __C_CONSUMABLEHEALTHREPLENISH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableHealthReplenish
//=============================================================================
class CConsumableHealthReplenish : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, HealthReplenish);

public:
	~CConsumableHealthReplenish() {}
	CConsumableHealthReplenish() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEHEALTHREPLENISH_H__
