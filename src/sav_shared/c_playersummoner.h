// (C)2006 S2 Games
// c_playersummoner.h
//
//=============================================================================
#ifndef __C_PLAYERSUMMONER_H__
#define __C_PLAYERSUMMONER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerSummoner
//=============================================================================
class CPlayerSummoner : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Summoner);

public:
    ~CPlayerSummoner()  {}
    CPlayerSummoner() :
    IPlayerEntity(GetEntityConfig())
    {}

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERSUMMONER_H__
