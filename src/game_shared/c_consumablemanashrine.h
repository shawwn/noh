// (C)2006 S2 Games
// c_consumablemanashrine.h
//
//=============================================================================
#ifndef __C_CONSUMABLEMANASHRINE_H__
#define __C_CONSUMABLEMANASHRINE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableManaShrine
//=============================================================================
class CConsumableManaShrine : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, ManaShrine);

public:
    ~CConsumableManaShrine() {}
    CConsumableManaShrine() :
    IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEMANASHRINE_H__
