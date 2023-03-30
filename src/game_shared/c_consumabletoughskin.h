// (C)2006 S2 Games
// c_consumabletoughskin.h
//
//=============================================================================
#ifndef __C_CONSUMABLETOUGHSKIN_H__
#define __C_CONSUMABLETOUGHSKIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableToughSkin
//=============================================================================
class CConsumableToughSkin : public IConsumableItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Consumable, ToughSkin);

public:
	~CConsumableToughSkin() {}
	CConsumableToughSkin() :
	IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLETOUGHSKIN_H__
