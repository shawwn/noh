// (C)2006 S2 Games
// c_consumablemanaminor.h
//
//=============================================================================
#ifndef __C_CONSUMABLEMANAMINOR_H__
#define __C_CONSUMABLEMANAMINOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableManaMinor
//=============================================================================
class CConsumableManaMinor : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, ManaMinor);

    static  CCvarf      s_cvarManaAmount;

public:
    ~CConsumableManaMinor() {}
    CConsumableManaMinor() :
    IConsumableItem(GetEntityConfig()) {}

    bool    ActivatePrimary(int iButtonStatus);
};
//=============================================================================

#endif //__C_CONSUMABLEMANAMINOR_H__
