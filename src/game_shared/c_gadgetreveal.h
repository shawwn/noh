// (C)2007 S2 Games
// c_gadgetreveal.h
//
//=============================================================================
#ifndef __C_GADGETREVEAL_H__
#define __C_GADGETREVEAL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetReveal
//=============================================================================
class CGadgetReveal : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, Reveal)

public:
	~CGadgetReveal()	{}
	CGadgetReveal() :
	IGadgetEntity(GetEntityConfig())
	{}

	bool	CanSpawn()	{ return true; }
};
//=============================================================================

#endif //__C_GADGETREVEAL_H__
