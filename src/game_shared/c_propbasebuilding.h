// (C)2006 S2 Games
// c_propbasebuilding.h
//
//=============================================================================
#ifndef __C_PROPBASEBUILDING_H__
#define __C_PROPBASEBUILDING_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propentity.h"
//=============================================================================

//=============================================================================
// CPropBaseBuilding
//=============================================================================
class CPropBaseBuilding : public IPropEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Prop, BaseBuilding);

	uint			m_uiBaseUID;

public:
	~CPropBaseBuilding() {}
	CPropBaseBuilding();

	void	GameStart();
	void	WarmupStart();

	void	Link()		{}
	void	Unlink()	{}

	bool			AddToScene(const CVec4f &v4Color, int iFlags)	{ return false; }
};
//=============================================================================

#endif //__C_PROPBASEBUILDING_H__
