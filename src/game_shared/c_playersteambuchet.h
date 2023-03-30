// (C)2006 S2 Games
// c_playersteambuchet.h
//
//=============================================================================
#ifndef __C_PLAYERSTEAMBUCHET_H__
#define __C_PLAYERSTEAMBUCHET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"
//=============================================================================

//=============================================================================
// CPlayerSteambuchet
//=============================================================================
class CPlayerSteambuchet : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, Steambuchet);

public:
	~CPlayerSteambuchet()	{}
	CPlayerSteambuchet();

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERSTEAMBUCHET_H__
