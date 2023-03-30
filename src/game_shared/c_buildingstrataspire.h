// (C)2007 S2 Games
// c_buildingstrataspire.h
//
//=============================================================================
#ifndef __C_BUILDINGSTRATASPIRE_H__
#define __C_BUILDINGSTRATASPIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_attackbuildingentity.h"
//=============================================================================

//=============================================================================
// CBuildingStrataSpire
//=============================================================================
class CBuildingStrataSpire : public IAttackBuildingEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Building, StrataSpire)

public:
	~CBuildingStrataSpire()	{}
	CBuildingStrataSpire() :
	IAttackBuildingEntity(GetEntityConfig())
	{}
};
//=============================================================================

#endif //__C_BUILDINGSTRATASPIRE_H__
