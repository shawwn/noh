// (C)2006 S2 Games
// c_consumablespeedboost.h
//
//=============================================================================
#ifndef __C_CONSUMABLESPEEDBOOST_H__
#define __C_CONSUMABLESPEEDBOOST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableSpeedBoost
//=============================================================================
class CConsumableSpeedBoost : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, SpeedBoost);

public:
	~CConsumableSpeedBoost() {}
	CConsumableSpeedBoost() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLESPEEDBOOST_H__
