// (C)2006 S2 Games
// c_playerrevenant.h
//
//=============================================================================
#ifndef __C_PLAYERREVENANT_H__
#define __C_PLAYERREVENANT_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"
//=============================================================================

//=============================================================================
// CPlayerRevenant
//=============================================================================
class CPlayerRevenant : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, Revenant);

public:
	~CPlayerRevenant()	{}
	CPlayerRevenant() :
	IPlayerEntity(GetEntityConfig())
	{}

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERREVENANT_H__
