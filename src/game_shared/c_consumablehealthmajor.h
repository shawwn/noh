// (C)2006 S2 Games
// c_consumablehealthmajor.h
//
//=============================================================================
#ifndef __C_CONSUMABLEHEALTHMAJOR_H__
#define __C_CONSUMABLEHEALTHMAJOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableHealthMajor
//=============================================================================
class CConsumableHealthMajor : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, HealthMajor);

	static	CCvarf		s_cvarHealthAmount;

public:
	~CConsumableHealthMajor() {}
	CConsumableHealthMajor() :
	IConsumableItem(GetEntityConfig()) {}

	bool	ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_CONSUMABLEHEALTHMAJOR_H__
