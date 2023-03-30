	// (C)2007 S2 Games
// c_meleepredatorclaws2.h
//
//=============================================================================
#ifndef __C_MELEEPREDATORCLAWS2_H__
#define __C_MELEEPREDATORCLAWS2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleePredatorClaws2
//=============================================================================
class CMeleePredatorClaws2 : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, PredatorClaws2);

public:
	~CMeleePredatorClaws2()	{}
	CMeleePredatorClaws2() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEEPREDATORCLAWS2_H__
