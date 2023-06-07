// (C)2007 S2 Games
// i_officerspawnflag.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_officerspawnflag.h"
#include "c_teaminfo.h"
//=============================================================================

/*====================
  IOfficerSpawnFlag::Spawn
  ====================*/
void    IOfficerSpawnFlag::Spawn()
{
    byte ySquad(GetSquad());
    IGadgetEntity::Spawn();
    SetSquad(ySquad);

    if (Game.IsServer())
    {
        CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
        if (pTeam == NULL)
        {
            Console.Warn << _T("Spawn flag has an invalid team") << newl;
            return;
        }

        pTeam->AddSquadObject(GetSquad(), GetIndex());

        ivector vClients(Game.GetTeam(GetTeam())->GetClientList());

        CBufferFixed<4> buffer;
        buffer << GAME_CMD_SPAWNFLAG_PLACED;

        for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
        {
            IGameEntity *pEnt(Game.GetPlayerEntityFromClientID(*it));

            if (pEnt == NULL)
            {
                Console.Warn << _T("IOfficerSpawnFlag::Spawn() - Invalid client (") << *it << _T(") in team list!") << newl;
                continue;
            }

            if (Game.GetPlayerEntityFromClientID(*it)->GetSquad() == GetSquad())
                Game.SendGameData(*it, buffer, true);
        }
    }
}
