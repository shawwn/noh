// (C)2006 S2 Games
// c_playermalphas.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_playermalphas.h"
#include "c_teaminfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Player, Malphas);
//=============================================================================

/*====================
  CPlayerMalphas::Spawn
  ====================*/
void	CPlayerMalphas::Spawn()
{
	IPlayerEntity::Spawn();

	if (Game.IsServer())
	{
		if (!Game.GetTeam(GetTeam())->GetPlayedMalphasSound())
		{
			CBufferFixed<2> buffer;
			buffer << GAME_CMD_MALPHAS_SPAWN;
			Game.BroadcastGameData(buffer, true);
			Game.GetTeam(GetTeam())->SetPlayedMalphasSound(true);
		}
	}
}
