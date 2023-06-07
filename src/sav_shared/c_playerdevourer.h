// (C)2007 S2 Games
// c_playerdevourer.h
//
//=============================================================================
#ifndef __C_PLAYERDEVOURER_H__
#define __C_PLAYERDEVOURER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerDevourer
//=============================================================================
class CPlayerDevourer : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Devourer);

public:
    ~CPlayerDevourer()  {}
    CPlayerDevourer() :
    IPlayerEntity(GetEntityConfig())
    {}

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERDEVOURER_H__
