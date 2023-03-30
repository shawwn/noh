// (C)2006 S2 Games
// c_meleeshortblades.h
//
//=============================================================================
#ifndef __C_MELEESHORTBLADES_H__
#define __C_MELEESHORTBLADES_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeShortBlades
//=============================================================================
class CMeleeShortBlades : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, ShortBlades);

public:
	~CMeleeShortBlades()	{}
	CMeleeShortBlades() :
	IMeleeItem(GetEntityConfig())
	{
	}
};
//=============================================================================

#endif //__C_MELEESHORTBLADES_H__
