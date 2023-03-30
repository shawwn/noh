// (C)2007 S2 Games
// c_gadgethealingward.h
//
//=============================================================================
#ifndef __C_GADGETHEALINGWARD_H__
#define __C_GADGETHEALINGWARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetHealingWard
//=============================================================================
class CGadgetHealingWard : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, HealingWard)

public:
	~CGadgetHealingWard()	{}
	CGadgetHealingWard() :
	IGadgetEntity(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_GADGETHEALINGWARD_H__
