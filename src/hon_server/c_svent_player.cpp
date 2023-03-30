// (C)2005 S2 Games
// c_svent_player.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_svent_player.h"
//=============================================================================

/*====================
  CSvEnt_Player::~CSvEnt_Player
  ====================*/
CSvEnt_Player::~CSvEnt_Player()
{
}


/*====================
  CSvEnt_Player::CSvEnt_Player
  ====================*/
CSvEnt_Player::CSvEnt_Player(int iClientNum) :
CServerEntity(ENT_CLIENT),
m_iClientNum(iClientNum)
{
}
