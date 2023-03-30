// (C)2006 S2 Games
// c_meleemalikenblade.h
//
//=============================================================================
#ifndef __C_MELEEMALIKENBLADE_H__
#define __C_MELEEMALIKENBLADE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeMalikenBlade
//=============================================================================
class CMeleeMalikenBlade : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, MalikenBlade);

public:
    ~CMeleeMalikenBlade()   {}
    CMeleeMalikenBlade() :  IMeleeItem(GetEntityConfig()) {}
};
//=============================================================================

#endif //__C_MELEEMALIKENBLADE_H__
