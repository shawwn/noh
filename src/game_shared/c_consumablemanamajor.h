// (C)2006 S2 Games
// c_consumablemanamajor.h
//
//=============================================================================
#ifndef __C_CONSUMABLEMANAMAJOR_H__
#define __C_CONSUMABLEMANAMAJOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableManaMajor
//=============================================================================
class CConsumableManaMajor : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, ManaMajor);

	static	CCvarf		s_cvarManaAmount;

public:
	~CConsumableManaMajor() {}
	CConsumableManaMajor() :
	IConsumableItem(GetEntityConfig()) {}

	bool	ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_CONSUMABLEMANAMAJOR_H__
