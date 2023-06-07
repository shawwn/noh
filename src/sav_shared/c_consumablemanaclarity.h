// (C)2006 S2 Games
// c_consumablemanaclarity.h
//
//=============================================================================
#ifndef __C_CONSUMABLEMANACLARITY_H__
#define __C_CONSUMABLEMANACLARITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableManaClarity
//=============================================================================
class CConsumableManaClarity : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, ManaClarity);

public:
    ~CConsumableManaClarity() {}
    CConsumableManaClarity() :
    IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEMANACLARITY_H__
