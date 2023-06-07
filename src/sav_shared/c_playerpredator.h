// (C)2006 S2 Games
// c_playerpredator.h
//
//=============================================================================
#ifndef __C_PLAYERPREDATOR_H__
#define __C_PLAYERPREDATOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerPredator
//=============================================================================
class CPlayerPredator : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Predator);

public:
    ~CPlayerPredator()  {}
    CPlayerPredator() :
    IPlayerEntity(GetEntityConfig())
    {}

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERPREDATOR_H__
