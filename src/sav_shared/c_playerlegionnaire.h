// (C)2006 S2 Games
// c_playerlegionnaire.h
//
//=============================================================================
#ifndef __C_PLAYELEGIONNAIRE_H__
#define __C_PLAYELEGIONNAIRE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerLegionnaire
//=============================================================================
class CPlayerLegionnaire : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Legionnaire);

public:
    ~CPlayerLegionnaire()   {}
    CPlayerLegionnaire();

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYELEGIONNAIRE_H__
