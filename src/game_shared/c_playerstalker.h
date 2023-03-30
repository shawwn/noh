// (C)2007 S2 Games
// c_playerstalker.h
//
//=============================================================================
#ifndef __C_PLAYERSTALKER_H__
#define __C_PLAYERSTALKER_H__
 
//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerStalker
//=============================================================================
class CPlayerStalker : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, Stalker);

public:
	~CPlayerStalker()	{}
	CPlayerStalker() :
	IPlayerEntity(GetEntityConfig())
	{}

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERSTALKER_H__
