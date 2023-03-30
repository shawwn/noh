// (C)2006 S2 Games
// c_playermalphas.h
//
//=============================================================================
#ifndef __C_PLAYERMALPHAS_H__
#define __C_PLAYERMALPHAS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"
//=============================================================================

//=============================================================================
// CPlayerMalphas
//=============================================================================
class CPlayerMalphas : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Malphas);

public:
    ~CPlayerMalphas()   {}
    CPlayerMalphas() :
    IPlayerEntity(GetEntityConfig())
    {}

    virtual void Spawn();

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERMALPHAS_H__
