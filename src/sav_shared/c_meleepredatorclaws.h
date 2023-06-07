    // (C)2007 S2 Games
// c_meleepredatorclaws.h
//
//=============================================================================
#ifndef __C_MELEEPREDATORCLAWS_H__
#define __C_MELEEPREDATORCLAWS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleePredatorClaws
//=============================================================================
class CMeleePredatorClaws : public IMeleeItem
{
private:
    DECLARE_ENT_ALLOCATOR2(Melee, PredatorClaws);

public:
    ~CMeleePredatorClaws()  {}
    CMeleePredatorClaws() :
    IMeleeItem(GetEntityConfig())
    {}
};
//=============================================================================

#endif //__C_MELEEPREDATORCLAWS_H__
