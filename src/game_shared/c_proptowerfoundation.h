// (C)2006 S2 Games
// c_proptowerfoundation.h
//
//=============================================================================
#ifndef __C_PROPTOWERFOUNDATION_H__
#define __C_PROPTOWERFOUNDATION_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propfoundation.h"
//=============================================================================

//=============================================================================
// CPropTowerFoundation
//=============================================================================
class CPropTowerFoundation : public IPropFoundation
{
private:
	DECLARE_ENT_ALLOCATOR(Prop, TowerFoundation);

public:
	~CPropTowerFoundation()	{}
	CPropTowerFoundation()	{}
};
//=============================================================================

#endif //__C_PROPTOWERFOUNDATION_H__
