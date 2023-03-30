// (C)2007 S2 Games
// c_gadgetmeteor.h
//
//=============================================================================
#ifndef __C_GADGETMETEOR_H__
#define __C_GADGETMETEOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetMeteor
//=============================================================================
class CGadgetMeteor : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, Meteor);

	static CCvarui	s_cvarImpactTime;
	static CCvarf	s_cvarImpactRadius;
	static CCvarf	s_cvarImpactDamage;

public:
	~CGadgetMeteor()	{}
	CGadgetMeteor() :
	IGadgetEntity(GetEntityConfig())
	{}

	bool	ServerFrame();

	GAME_SHARED_API virtual bool	AIShouldTarget()			{ return false; }
};
//=============================================================================

#endif //__C_GADGETMETEOR_H__
