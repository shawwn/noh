// (C)2006 S2 Games
// c_gadgetfirerain.h
//
//=============================================================================
#ifndef __C_GADGETFIRERAIN_H__
#define __C_GADGETFIRERAIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetFireRain
//=============================================================================
class CGadgetFireRain : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, FireRain);

public:
	~CGadgetFireRain()	{}
	CGadgetFireRain() :
	IGadgetEntity(GetEntityConfig())
	{}

	GAME_SHARED_API virtual bool	AIShouldTarget()			{ return false; }
};
//=============================================================================

#endif //__C_GADGETFIRERAIN_H__
