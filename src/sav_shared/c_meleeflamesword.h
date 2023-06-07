// (C)2006 S2 Games
// c_meleeflamesword.h
//
//=============================================================================
#ifndef __C_MELEEFLAMESWORD_H__
#define __C_MELEEFLAMESWORD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeFlameSword
//=============================================================================
class CMeleeFlameSword : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, FlameSword);

public:
    ~CMeleeFlameSword()         {}
    CMeleeFlameSword() :
    IMeleeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_MELEEFLAMESWORD_H__
