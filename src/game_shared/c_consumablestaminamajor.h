// (C)2006 S2 Games
// c_consumablestaminamajor.h
//
//=============================================================================
#ifndef __C_CONSUMABLESTAMINAMAJOR_H__
#define __C_CONSUMABLESTAMINAMAJOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableStaminaMajor
//=============================================================================
class CConsumableStaminaMajor : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, StaminaMajor);

	static	CCvarf		s_cvarStaminaAmount;

public:
	~CConsumableStaminaMajor() {}
	CConsumableStaminaMajor() :
	IConsumableItem(GetEntityConfig()) {}

	bool	ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_CONSUMABLESTAMINAMAJOR_H__
