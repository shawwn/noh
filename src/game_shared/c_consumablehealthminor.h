// (C)2006 S2 Games
// c_consumablehealthminor.h
//
//=============================================================================
#ifndef __C_CONSUMABLEHEALTHMINOR_H__
#define __C_CONSUMABLEHEALTHMINOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableHealthMinor
//=============================================================================
class CConsumableHealthMinor : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, HealthMinor);

	static	CCvarf		s_cvarHealthAmount;

public:
	~CConsumableHealthMinor() {}
	CConsumableHealthMinor() :
	IConsumableItem(GetEntityConfig()) {}

	bool	ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_CONSUMABLEHEALTHMINOR_H__
