// (C)2006 S2 Games
// c_siegestonebombardment.h
//
//=============================================================================
#ifndef __C_SIEGESTONEBOMBARDMENT_H__
#define __C_SIEGESTONEBOMBARDMENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_siegeitem.h"
//=============================================================================

//=============================================================================
// CSiegeStoneBombardment
//=============================================================================
class CSiegeStoneBombardment : public ISiegeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Siege, StoneBombardment);

public:
    ~CSiegeStoneBombardment()   {}
    CSiegeStoneBombardment() :
    ISiegeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_SIEGESTONEBOMBARDMENT_H__
