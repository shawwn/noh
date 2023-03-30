// (C)2005 S2 Games
// c_svent_player.h
//
//=============================================================================
#ifndef __C_SVENT_PLAYER_H__
#define __C_SVENT_PLAYER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_serverentity.h"
//=============================================================================

//=============================================================================
// CSvEnt_Player
//=============================================================================
class CSvEnt_Player : public CServerEntity
{
private:
    int     m_iClientNum;

    CSvEnt_Player();

public:
    ~CSvEnt_Player();
    CSvEnt_Player(int iClientNum);

    int     GetClientNum() const    { return m_iClientNum; }

    void    GetUpdateData(CBufferDynamic &buffer) const {}
};
//=============================================================================

#endif //__C_SVENT_PLAYER_H__
