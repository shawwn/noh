// (C)2007 S2 Games
// c_playermaliken.h
//
//=======================================================================
#ifndef __C_PLAYERMALIKEN_H__
#define __C_PLAYERMALIKEN_H__
 
//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerMaliken
//=============================================================================
class CPlayerMaliken : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, Maliken);

public:
	~CPlayerMaliken()	{}
	CPlayerMaliken() :
	IPlayerEntity(GetEntityConfig())
	{}

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERMALIKEN_H__
