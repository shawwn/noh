// (C)2006 S2 Games
// c_consumablehealthshrine.h
//
//=============================================================================
#ifndef __C_CONSUMABLEHEALTHSHRINE_H__
#define __C_CONSUMABLEHEALTHSHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableHealthShrine
//=============================================================================
class CConsumableHealthShrine : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, HealthShrine);

public:
	~CConsumableHealthShrine() {}
	CConsumableHealthShrine() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEHEALTHSHRINE_H__
