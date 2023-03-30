// (C)2006 S2 Games
// c_playerobserver.h
//
//=============================================================================
#ifndef __C_PLAYEROBSERVER_H__
#define __C_PLAYEROBSERVER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerObserver
//=============================================================================
class CPlayerObserver : public IPlayerEntity
{
private:
	DECLARE_ENT_ALLOCATOR2(Player, Observer);

public:
	~CPlayerObserver()	{}
	CPlayerObserver() :
	IPlayerEntity(GetEntityConfig())
	{}

	bool		HasAltInfo() const						{ return false; }

	virtual bool		IsObserver() const				{ return true; }
	virtual bool		IsStealthed()					{ return true; }

	CSkeleton*	AllocateSkeleton()						{ return NULL; }

	void		Move(const CClientSnapshot &snapshot)	{ MoveFly(snapshot); }

	GAME_SHARED_API virtual bool	AIShouldTarget()	{ return false; }
};
//=============================================================================

#endif //__C_PLAYEROBSERVER_H__
