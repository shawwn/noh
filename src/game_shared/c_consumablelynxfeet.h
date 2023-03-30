// (C)2006 S2 Games
// c_consumablelynxfeet.h
//
//=============================================================================
#ifndef __C_CONSUMABLELYNXFEET_H__
#define __C_CONSUMABLELYNXFEET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableLynxFeet
//=============================================================================
class CConsumableLynxFeet : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, LynxFeet);

public:
    ~CConsumableLynxFeet() {}
    CConsumableLynxFeet() :
    IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLELYNXFEET_H__
