// (C)2006 S2 Games
// c_gunsniperbow.h
//
//=============================================================================
#ifndef __C_GUNSNIPERBOW_H__
#define __C_GUNSNIPERBOW_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gunitem.h"
//=============================================================================

//=============================================================================
// CGunSniperBow
//=============================================================================
class CGunSniperBow : public IGunItem
{
private:
	DECLARE_ENT_ALLOCATOR2(Gun, SniperBow);

public:
	~CGunSniperBow()	{}
	CGunSniperBow() :
	IGunItem(GetEntityConfig())
	{}

	virtual bool	ActivateSecondary(int iButtonStatus);
};
//=============================================================================

#endif //__C_GUNSNIPERBOW_H__
