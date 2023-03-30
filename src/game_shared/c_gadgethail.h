// (C)2007 S2 Games
// c_gadgethail.h
//
//=============================================================================
#ifndef __C_GADGETHAIL_H__
#define __C_GADGETHAIL_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetHail
//=============================================================================
class CGadgetHail : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, Hail)

	static CCvarf	s_cvarRadius;
	static CCvarf	s_cvarDamagePerSecond;

public:
	~CGadgetHail()	{}
	CGadgetHail() :
	IGadgetEntity(GetEntityConfig())
	{}

	bool	ServerFrame();

	GAME_SHARED_API virtual bool	AIShouldTarget()			{ return false; }
};
//=============================================================================

#endif //__C_GADGETHAIL_H__
