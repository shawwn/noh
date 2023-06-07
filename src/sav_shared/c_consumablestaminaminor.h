// (C)2006 S2 Games
// c_consumablestaminaminor.h
//
//=============================================================================
#ifndef __C_CONSUMABLESTAMINAMINOR_H__
#define __C_CONSUMABLESTAMINAMINOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableStaminaMinor
//=============================================================================
class CConsumableStaminaMinor : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, StaminaMinor);

    static  CCvarf      s_cvarStaminaAmount;

public:
    ~CConsumableStaminaMinor() {}
    CConsumableStaminaMinor() :
    IConsumableItem(GetEntityConfig()) {}

    bool    ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_CONSUMABLESTAMINAMINOR_H__
