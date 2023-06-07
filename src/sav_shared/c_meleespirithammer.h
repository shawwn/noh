// (C)2007 S2 Games
// c_meleespirithammer.h
//
//=============================================================================
#ifndef __C_MELEESPIRITHAMMER_H__
#define __C_MELEESPIRITHAMMER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeSpiritHammer
//=============================================================================
class CMeleeSpiritHammer : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, SpiritHammer);

public:
    ~CMeleeSpiritHammer()   {}
    CMeleeSpiritHammer() :
    IMeleeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_MELEESPIRITHAMMER_H__
