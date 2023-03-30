// (C)2006 S2 Games
// c_playerbehemoth.h
//
//=============================================================================
#ifndef __C_PLAYERBEHEMOTH_H__
#define __C_PLAYERBEHEMOTH_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerBehemoth
//=============================================================================
class CPlayerBehemoth : public IPlayerEntity
{
private:
    DECLARE_ENT_ALLOCATOR2(Player, Behemoth);

public:
    ~CPlayerBehemoth()  {}
    CPlayerBehemoth() :
    IPlayerEntity(GetEntityConfig())
    {}

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }
};
//=============================================================================

#endif //__C_PLAYERBEHEMOTH_H__
