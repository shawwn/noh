// (C)2006 S2 Games
// c_consumablemanacrystal.h
//
//=============================================================================
#ifndef __C_CONSUMABLEMANACRYSTAL_H__
#define __C_CONSUMABLEMANACRYSTAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CConsumableManaCrystal
//=============================================================================
class CConsumableManaCrystal : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, ManaCrystal);

public:
	~CConsumableManaCrystal() {}
	CConsumableManaCrystal() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEMANACRYSTAL_H__
