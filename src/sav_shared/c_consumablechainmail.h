// (C)2006 S2 Games
// c_consumablechainmail.h
//
//=============================================================================
#ifndef __C_CONSUMABLECHAINMAIL_H__
#define __C_CONSUMABLECHAINMAIL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumableChainmail
//=============================================================================
class CConsumableChainmail : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, Chainmail);

public:
    ~CConsumableChainmail() {}
    CConsumableChainmail() :
    IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLECHAINMAIL_H__
