// (C)2006 S2 Games
// c_siegefirebombardment.h
//
//=============================================================================
#ifndef __C_SIEGEFIREBOMBARDMENT_H__
#define __C_SIEGEFIREBOMBARDMENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_siegeitem.h"
//=============================================================================

//=============================================================================
// CSiegeFireBombardment
//=============================================================================
class CSiegeFireBombardment : public ISiegeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Siege, FireBombardment);

public:
    ~CSiegeFireBombardment()    {}
    CSiegeFireBombardment() :
    ISiegeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SIEGEFIREBOMBARDMENT_H__
