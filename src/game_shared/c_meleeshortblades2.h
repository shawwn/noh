// (C)2006 S2 Games
// c_meleeshortblades2.h
//
//=============================================================================
#ifndef __C_MELEESHORTBLADES2_H__
#define __C_MELEESHORTBLADES2_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeShortBlades2
//=============================================================================
class CMeleeShortBlades2 : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, ShortBlades2);

public:
	~CMeleeShortBlades2()	{}
	CMeleeShortBlades2() :
	IMeleeItem(GetEntityConfig())
	{
	}
};
//=============================================================================

#endif //__C_MELEESHORTBLADES2_H__
