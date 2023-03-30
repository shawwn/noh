// (C)2006 S2 Games
// c_playersavage.h
//
//=============================================================================
#ifndef __C_PLAYERSAVAGE_H__
#define __C_PLAYERSAVAGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerSavage
//=============================================================================
class CPlayerSavage : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, Savage);

public:
	~CPlayerSavage()	{}
	CPlayerSavage();

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERSAVAGE_H__
