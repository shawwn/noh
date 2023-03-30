// (C)2006 S2 Games
// c_playerbatteringram.h
//
//=============================================================================
#ifndef __C_PLAYERBATTERINGRAM_H__
#define __C_PLAYERBATTERINGRAM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"
//=============================================================================

//=============================================================================
// CPlayerBatteringRam
//=============================================================================
class CPlayerBatteringRam : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, BatteringRam);

public:
	~CPlayerBatteringRam()	{}
	CPlayerBatteringRam();

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERBATTERINGRAM_H__
