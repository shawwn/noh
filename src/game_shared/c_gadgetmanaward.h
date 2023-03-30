// (C)2007 S2 Games
// c_gadgetmanaward.h
//
//=============================================================================
#ifndef __C_GADGETMANAWARD_H__
#define __C_GADGETMANAWARD_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetManaWard
//=============================================================================
class CGadgetManaWard : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, ManaWard)

public:
	~CGadgetManaWard()	{}
	CGadgetManaWard() :
	IGadgetEntity(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_GADGETMANAWARD_H__
