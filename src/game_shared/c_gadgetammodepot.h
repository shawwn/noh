// (C)2006 S2 Games
// c_gadgetammodepot.h
//
//=============================================================================
#ifndef __C_GADGETAMMODEPOT_H__
#define __C_GADGETAMMODEPOT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetusable.h"
//=============================================================================

//=============================================================================
// CGadgetAmmoDepot
//=============================================================================
class CGadgetAmmoDepot : public IGadgetUsable
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, AmmoDepot);

public:
	~CGadgetAmmoDepot()	{}
	CGadgetAmmoDepot() :
	IGadgetUsable(GetEntityConfig())
	{}

	bool	UseEffect(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_GADGETAMMODEPOT_H__
