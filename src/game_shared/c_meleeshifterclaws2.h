// (C)2007 S2 Games
// c_meleeshifterclaws2.h
//
//=============================================================================
#ifndef __C_MELEESHIFTERCLAWS2_H__
#define __C_MELEESHIFTERCLAWS2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeShifterClaws2
//=============================================================================
class CMeleeShifterClaws2 : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, ShifterClaws2);

public:
	~CMeleeShifterClaws2()	{}
	CMeleeShifterClaws2() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEESHIFTERCLAWS2_H__
