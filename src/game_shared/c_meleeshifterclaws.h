// (C)2007 S2 Games
// c_meleeshifterclaws.h
//
//=============================================================================
#ifndef __C_MELEESHIFTERCLAWS_H__
#define __C_MELEESHIFTERCLAWS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeShifterClaws
//=============================================================================
class CMeleeShifterClaws : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, ShifterClaws);

public:
	~CMeleeShifterClaws()	{}
	CMeleeShifterClaws() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEESHIFTERCLAWS_H__
