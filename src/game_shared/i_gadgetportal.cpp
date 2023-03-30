// (C)2007 S2 Games
// i_gadgetportal.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gadgetportal.h"
#include "c_teaminfo.h"
//=============================================================================

/*====================
  IGadgetPortal::CEntityConfig::CEntityConfig
  ====================*/
IGadgetPortal::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(UseExperience, 5.0f)
{
}


/*====================
  IGadgetPortal::Baseline
  ====================*/
void	IGadgetPortal::Baseline()
{
	IGadgetEntity::Baseline();
	m_auiCounter[0] = 0;
}


/*====================
  IGadgetPortal::Spawn
  ====================*/
void	IGadgetPortal::Spawn()
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

		CBufferFixed<1> buffer;
		buffer << GAME_CMD_SPAWNFLAG_PLACED;

		for (ivector_cit it(vClients.begin()); it != vClients.end(); ++it)
		{
			IGameEntity *pEnt(Game.GetPlayerEntityFromClientID(*it));

			if (pEnt == NULL)
			{
				Console.Warn << _T("IGadgetPortal::Spawn() - Invalid client (") << *it << _T(") in team list!") << newl;
				continue;
			}

			if (Game.GetPlayerEntityFromClientID(*it)->GetSquad() == GetSquad())
				Game.SendGameData(*it, buffer, true);
		}
	}
}


/*====================
  IGadgetPortal::CanSpawnFrom
  ====================*/
bool	IGadgetPortal::CanSpawnFrom(IPlayerEntity *pPlayer)
{
	if (!IGadgetEntity::CanSpawnFrom(pPlayer))
		return false;

	if (m_setAccessors.find(pPlayer->GetClientID()) != m_setAccessors.end())
		return false;

	if (m_bAccessed)
		return false;

	return true;
}


/*====================
  IGadgetPortal::PlayerSpawnedFrom
  ====================*/
void	IGadgetPortal::PlayerSpawnedFrom(IPlayerEntity *pPlayer)
{
	if (pPlayer == NULL)
		return;

	m_setAccessors.insert(pPlayer->GetClientID());
	IncrementCounter(0);

	CBufferFixed<9> buffer;
	buffer << GAME_CMD_GADGET_ACCESSED << GetIndex() << GetSpawnTime();
	Game.SendGameData(pPlayer->GetClientID(), buffer, true);
}
