// (C)2006 S2 Games
// c_gadgetelectriceye.h
//
//=============================================================================
#ifndef __C_GADGETELECTRICEYE_H__
#define __C_GADGETELECTRICEYE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetElectricEye
//=============================================================================
class CGadgetElectricEye : public IGadgetEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Gadget, ElectricEye);
	
	static	CCvarf		s_cvarRadius;
	static	CCvars		s_cvarSightedEffectPath;

public:
	~CGadgetElectricEye()	{}
	CGadgetElectricEye() :
	IGadgetEntity(GetEntityConfig())
	{}

	bool					ServerFrame();
};
//=============================================================================

#endif //__C_GADGETELECTRICEYE_H__
