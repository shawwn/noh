// (C)2006 S2 Games
// c_playershapeshifter.h
//
//=============================================================================
#ifndef __C_PLAYERSHAPESHIFTER_H__
#define __C_PLAYERSHAPESHIFTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerShapeShifter
//=============================================================================
class CPlayerShapeShifter : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, ShapeShifter);

public:
	~CPlayerShapeShifter()	{}
	CPlayerShapeShifter() :
	IPlayerEntity(GetEntityConfig())
	{}

	void	Move(const CClientSnapshot &snapshot)	{ MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERSHAPESHIFTER_H__
