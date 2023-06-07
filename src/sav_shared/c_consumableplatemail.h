// (C)2006 S2 Games
// c_consumableplatemail.h
//
//=============================================================================
#ifndef __C_CONSUMABLEPLATEMAIL_H__
#define __C_CONSUMABLEPLATEMAIL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_consumableitem.h"
//=============================================================================

//=============================================================================
// CConsumablePlatemail
//=============================================================================
class CConsumablePlatemail : public IConsumableItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Consumable, Platemail);

public:
    ~CConsumablePlatemail() {}
    CConsumablePlatemail() :
    IConsumableItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_CONSUMABLEPLATEMAIL_H__
