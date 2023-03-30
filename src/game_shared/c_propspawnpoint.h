// (C)2006 S2 Games
// c_propspawnpoint.h
//
//=============================================================================
#ifndef __C_PROPSPAWNPOINT_H__
#define __C_PROPSPAWNPOINT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_propentity.h"
//=============================================================================

//=============================================================================
// CPropSpawnPoint
//=============================================================================
class CPropSpawnPoint : public IPropEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Prop, SpawnPoint)

public:
	~CPropSpawnPoint()	{}
	CPropSpawnPoint() :
	IPropEntity(GetEntityConfig())
	{}

	GAME_SHARED_API void	Spawn();

	bool					AddToScene(const CVec4f &v4Color, int iFlags)	{ return false; }
	void					Link()		{}
	void					Unlink()	{}
};
//=============================================================================

#endif //__C_PROPSPAWNPOINT_H__
