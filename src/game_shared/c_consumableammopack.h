// (C)2006 S2 Games
// c_consumableammopack.h
//
//=============================================================================
#ifndef __C_CONSUMABLEAMMOPACK_H__
#define __C_CONSUMABLEAMMOPACK_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableAmmoPack
//=============================================================================
class CConsumableAmmoPack : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, AmmoPack);

public:
    ~CConsumableAmmoPack() {}
    CConsumableAmmoPack() :
    IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEAMMOPACK_H__
