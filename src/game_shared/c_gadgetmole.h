// (C)2007 S2 Games
// c_gadgetmole.h
//
//=============================================================================
#ifndef __C_GADGETMOLE_H__
#define __C_GADGETMOLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetMole
//=============================================================================
class CGadgetMole : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, Mole)

	static CCvarf	s_cvarRepairRadius;
	static CCvarf	s_cvarRepairRate;
	
	float	m_fRepairAccumulator;

public:
	~CGadgetMole()	{}
	CGadgetMole();

	bool	ServerFrame();
	void	Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
};
//=============================================================================

#endif //__C_GADGETMOLE_H__
