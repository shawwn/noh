// (C)2007 S2 Games
// c_meleespirithammer2.h
//
//=============================================================================
#ifndef __C_MELEESPIRITHAMMER2_H__
#define __C_MELEESPIRITHAMMER2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeSpiritHammer2
//=============================================================================
class CMeleeSpiritHammer2 : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, SpiritHammer2);

public:
    ~CMeleeSpiritHammer2()  {}
    CMeleeSpiritHammer2() :
    IMeleeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_MELEESPIRITHAMMER2_H__
