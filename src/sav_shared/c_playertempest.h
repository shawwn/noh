// (C)2006 S2 Games
// c_playertemptest.h
//
//=============================================================================
#ifndef __C_PLAYERTEMPEST_H__
#define __C_PLAYERTEMPEST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerTempest
//=============================================================================
class CPlayerTempest : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Tempest);

public:
    ~CPlayerTempest()   {}
    CPlayerTempest();

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERTEMPEST_H__
