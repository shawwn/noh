// (C)2006 S2 Games
// c_consumableammosatchel.h
//
//=============================================================================
#ifndef __C_CONSUMABLEAMMOSATCHEL_H__
#define __C_CONSUMABLEAMMOSATCHEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableAmmoSatchel
//=============================================================================
class CConsumableAmmoSatchel : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, AmmoSatchel);

public:
	~CConsumableAmmoSatchel() {}
	CConsumableAmmoSatchel() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEAMMOSATCHEL_H__
