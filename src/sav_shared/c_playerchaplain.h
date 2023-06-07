// (C)2006 S2 Games
// c_playerchaplain.h
//
//=============================================================================
#ifndef __C_PLAYERCHAPLAIN_H__
#define __C_PLAYERCHAPLAIN_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"
//=============================================================================

//=============================================================================
// CPlayerChaplain
//=============================================================================
class CPlayerChaplain : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Chaplain);

public:
    ~CPlayerChaplain()  {}
    CPlayerChaplain();

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERCHAPLAIN_H__
