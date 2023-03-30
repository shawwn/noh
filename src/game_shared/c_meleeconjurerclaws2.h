// (C)2007 S2 Games
// c_meleeconjurerclaws2.h
//
//=============================================================================
#ifndef __C_MELEECONJURERCLAWS2_H__
#define __C_MELEECONJURERCLAWS2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeConjurerClaws2
//=============================================================================
class CMeleeConjurerClaws2 : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, ConjurerClaws2);

public:
    ~CMeleeConjurerClaws2() {}
    CMeleeConjurerClaws2() :
    IMeleeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_MELEECONJURERCLAWS2_H__
