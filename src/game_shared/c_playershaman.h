// (C)2006 S2 Games
// c_playershaman.h
//
//=============================================================================
#ifndef __C_PLAYERSHAMAN_H__
#define __C_PLAYERSHAMAN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerShaman
//=============================================================================
class CPlayerShaman : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, Shaman);

public:
	~CPlayerShaman()	{}
	CPlayerShaman() :
	IPlayerEntity(GetEntityConfig())
	{}

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERSHAMAN_H__
