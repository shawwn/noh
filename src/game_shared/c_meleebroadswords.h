// (C)2006 S2 Games
// c_meleebroadswords.h
//
//=============================================================================
#ifndef __C_MELEEBROADSWORDS_H__
#define __C_MELEEBROADSWORDS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeBroadswords
//=============================================================================
class CMeleeBroadswords : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, Broadswords);

public:
    ~CMeleeBroadswords()    {}
    CMeleeBroadswords() :
    IMeleeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_MELEEBROADSWORDS_H__
