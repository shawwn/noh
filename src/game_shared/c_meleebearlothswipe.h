// (C)2007 S2 Games
// c_meleebearlothswipe.h
//
//=============================================================================
#ifndef __C_MELEEBEARLOTHSWIPE_H__
#define __C_MELEEBEARLOTHSWIPE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_meleeitem.h"
//=============================================================================

//=============================================================================
// CMeleeBearlothSwipe
//=============================================================================
class CMeleeBearlothSwipe : public IMeleeItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Melee, BearlothSwipe);

public:
	~CMeleeBearlothSwipe()	{}
	CMeleeBearlothSwipe() :
	IMeleeItem(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_MELEEBEARLOTHSWIPE_H__
