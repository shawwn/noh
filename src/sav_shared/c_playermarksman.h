// (C)2006 S2 Games
// c_playermarksman.h
//
//=============================================================================
#ifndef __C_PLAYERMARKSMAN_H__
#define __C_PLAYERMARKSMAN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerMarksman
//=============================================================================
class CPlayerMarksman : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Marksman);

public:
    ~CPlayerMarksman()  {}
    CPlayerMarksman();

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERMARKSMAN_H__
