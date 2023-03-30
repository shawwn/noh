// (C)2007 S2 Games
// c_playerpossessedlegionnaire.h
//
//=============================================================================
#ifndef __C_PLAYERPOSSESSEDLEGIONNAIRE_H__
#define __C_PLAYERPOSSESSEDLEGIONNAIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerPossessedLegionnaire
//=============================================================================
class CPlayerPossessedLegionnaire : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, PossessedLegionnaire);

public:
    ~CPlayerPossessedLegionnaire()  {}
    CPlayerPossessedLegionnaire() :
    IPlayerEntity(GetEntityConfig())
    {}

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERPOSSESSEDLEGIONNAIRE_H__
