// (C)2008 S2 Games
// c_gamelog.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_gamelog.h"

#include "../aba_shared/c_player.h"
#include "../aba_shared/i_heroentity.h"
#include "../aba_shared/i_entityitem.h"
#include "../aba_shared/i_entityability.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define STAT_LABEL(name)		(_T(" ") _T(#name) _T(":"))
#define WRITE_STAT(name, value)	m_hLogFile << STAT_LABEL(name) << (value)
//=============================================================================

/*====================
  CGameLog::Open
  ====================*/
void	CGameLog::Open(uint uiMatchID)
{
	if (m_hLogFile.IsOpen())
		m_hLogFile.Close();

	tstring sLogFilename;
	if (uiMatchID == -1)
		sLogFilename = FileManager.GetNextFileIncrement(5, _T("~/logs/game_"), _T("log"));
	else
		sLogFilename = _T("~/logs/M") + XtoA(uiMatchID) + _T(".log");

	m_hLogFile.Open(sLogFilename, FILE_WRITE | FILE_TEXT);
}


/*====================
  CGameLog::WriteInfo
  ====================*/
void	CGameLog::WriteInfo(EGameLogEvent eEvent, const tstring &sTagA, const tstring &sValueA, const tstring &sTagB, const tstring &sValueB)
{
	if (!m_hLogFile.IsOpen())
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	if (!sTagA.empty() && !sValueA.empty())
		m_hLogFile << SPACE << sTagA << _T(":") << QuoteStr(sValueA);
	if (!sTagB.empty() && !sValueB.empty())
		m_hLogFile << SPACE << sTagB << _T(":") << QuoteStr(sValueB);
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteStatus
  ====================*/
void	CGameLog::WriteStatus(EGameLogEvent eEvent, const tstring &sTag, const tstring &sValue)
{
	if (!m_hLogFile.IsOpen())
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	uint uiMatchTime(Game.GetMatchTime());
	if (uiMatchTime > 0)
		WRITE_STAT(time, uiMatchTime);
	if (!sTag.empty() && !sValue.empty())
		m_hLogFile << SPACE << sTag << _T(":") << QuoteStr(sValue);
	m_hLogFile << newl;
}


/*====================
  CGameLog::WritePlayer
  ====================*/
void	CGameLog::WritePlayer(EGameLogEvent eEvent, CPlayer *pPlayer, const tstring &sParamA, const tstring &sParamB)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pPlayer == NULL || pPlayer->GetClientNumber() == -1)
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	if (Game.GetMatchTime() > 0)
		WRITE_STAT(time, Game.GetMatchTime());
	WRITE_STAT(player, pPlayer->GetClientNumber());
	switch (eEvent)
	{
	case GAME_LOG_PLAYER_CONNECT:
		WRITE_STAT(name, QuoteStr(pPlayer->GetTrueName()));
		if (pPlayer->GetAccountID() >= 0)
		{
			WRITE_STAT(id, pPlayer->GetAccountID());
			WRITE_STAT(psr, pPlayer->GetRank());
		}
		break;

	case GAME_LOG_PLAYER_DISCONNECT:
		if (!sParamA.empty())
			WRITE_STAT(reason, QuoteStr(sParamA));
		break;

	case GAME_LOG_PLAYER_TIMEDOUT:
		if (!sParamA.empty())
			WRITE_STAT(reason, QuoteStr(sParamA));
		break;
		
	case GAME_LOG_PLAYER_KICKED:
		if (!sParamA.empty())
			WRITE_STAT(reason, QuoteStr(sParamA));
		break;

	case GAME_LOG_PLAYER_KICKED_AFK:
		break;

	case GAME_LOG_PLAYER_KICKED_VOTE:
		break;

	case GAME_LOG_PLAYER_TERMINATED:
		break;

	case GAME_LOG_PLAYER_TEAM_CHANGE:
		WRITE_STAT(team, pPlayer->GetTeam());
		break;

	case GAME_LOG_PLAYER_CHAT:
		if (!sParamA.empty() && !sParamB.empty())
		{
			WRITE_STAT(target, QuoteStr(sParamA));
			WRITE_STAT(msg, QuoteStr(sParamB));
		}
		break;

	case GAME_LOG_PLAYER_ACTIONS:
		WRITE_STAT(count, pPlayer->GetActionCount());
		WRITE_STAT(period, pPlayer->GetActionCountPeriod());
		WRITE_STAT(team, pPlayer->GetTeam());
		break;

	case GAME_LOG_PLAYER_SELECT:
		if (pPlayer->HasSelectedHero())
			WRITE_STAT(hero, QuoteStr(EntityRegistry.LookupName(pPlayer->GetSelectedHero())));
		break;

	case GAME_LOG_PLAYER_RANDOM:
		if (pPlayer->HasSelectedHero())
			WRITE_STAT(hero, QuoteStr(EntityRegistry.LookupName(pPlayer->GetSelectedHero())));
		break;

	case GAME_LOG_PLAYER_REPICK:
		if (!sParamA.empty())
			WRITE_STAT(hero, QuoteStr(sParamA));
		break;

	case GAME_LOG_PLAYER_BAN:
		if (!sParamA.empty())
			WRITE_STAT(hero, QuoteStr(sParamA));
		break;

	case GAME_LOG_PLAYER_SWAP:
		if (!sParamA.empty())
			WRITE_STAT(oldhero, QuoteStr(sParamA));
		if (pPlayer->HasSelectedHero())
			WRITE_STAT(newhero, QuoteStr(EntityRegistry.LookupName(pPlayer->GetSelectedHero())));
		break;

	case GAME_LOG_PLAYER_BUYBACK:
		if (!sParamA.empty())
			WRITE_STAT(cost, sParamA);
		WRITE_STAT(team, pPlayer->GetTeam());
		break;

	case GAME_LOG_PLAYER_CALL_VOTE:
		if (!sParamA.empty())
			WRITE_STAT(type, sParamA);
		break;

	case GAME_LOG_PLAYER_VOTE:
		if (!sParamA.empty())
			WRITE_STAT(type, sParamA);
			WRITE_STAT(vote, sParamB);
		break;
	}

	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteKill
  ====================*/
void	CGameLog::WriteKill(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, ivector *pAssists)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pTarget == NULL)
		return;

	if (pTarget->IsHero() && pTarget->GetOwnerClientNumber() != -1)
	{
		m_hLogFile << GetGameLogEventName(GAME_LOG_HERO_DEATH);
		WRITE_STAT(time, Game.GetMatchTime());
		WRITE_STAT(x, INT_FLOOR(pTarget->GetPosition().x));
		WRITE_STAT(y, INT_FLOOR(pTarget->GetPosition().y));
		WRITE_STAT(z, INT_FLOOR(pTarget->GetPosition().z));
		WRITE_STAT(player, pTarget->GetOwnerClientNumber());
		WRITE_STAT(team, pTarget->GetTeam());
		if (pAttacker != NULL)
		{
			WRITE_STAT(attacker, QuoteStr(pAttacker->GetTypeName()));
			if (pAttacker->GetOwnerClientNumber() != -1)
				WRITE_STAT(owner, pAttacker->GetOwnerClientNumber());
		}
		if (pInflictor != NULL)
			WRITE_STAT(inflictor, QuoteStr(pInflictor->GetTypeName()));
		m_hLogFile << newl;
	}

	if (pAttacker == NULL)
		return;

	m_hLogFile << GetGameLogEventName(GAME_LOG_KILL);
	WRITE_STAT(time, Game.GetMatchTime());
	WRITE_STAT(x, INT_FLOOR(pTarget->GetPosition().x));
	WRITE_STAT(y, INT_FLOOR(pTarget->GetPosition().y));
	WRITE_STAT(z, INT_FLOOR(pTarget->GetPosition().z));
	if (pAttacker->GetOwnerClientNumber() != -1)
		WRITE_STAT(player, pAttacker->GetOwnerClientNumber());
	WRITE_STAT(team, pAttacker->GetTeam());
	WRITE_STAT(target, QuoteStr(pTarget->GetTypeName()));
	WRITE_STAT(attacker, QuoteStr(pAttacker->GetTypeName()));
	if (pTarget->GetOwnerClientNumber() != -1)
		WRITE_STAT(owner, pTarget->GetOwnerClientNumber());
	if (pInflictor != NULL)
		WRITE_STAT(inflictor, QuoteStr(pInflictor->GetTypeName()));
	if (pAssists != NULL)
	{
		tstring sAssists;
		for (ivector_it it(pAssists->begin()); it != pAssists->end(); ++it)
		{
			if (!sAssists.empty())
				sAssists += _T(",");
			sAssists += XtoA(*it);
		}
		if (!sAssists.empty())
			WRITE_STAT(assists, sAssists);
	}
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteAssist
  ====================*/
void	CGameLog::WriteAssist(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, CPlayer *pPlayer)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pPlayer == NULL || pPlayer->GetClientNumber() == -1)
		return;

	m_hLogFile << GetGameLogEventName(GAME_LOG_HERO_ASSIST);
	m_hLogFile << SPACE << _T("time:") << Game.GetMatchTime();
	if (pTarget != NULL)
	{
		m_hLogFile << SPACE << _T("x:") << INT_FLOOR(pTarget->GetPosition().x);
		m_hLogFile << SPACE << _T("y:") << INT_FLOOR(pTarget->GetPosition().y);
		m_hLogFile << SPACE << _T("z:") << INT_FLOOR(pTarget->GetPosition().z);
	}
	m_hLogFile << SPACE << _T("player:") << pPlayer->GetClientNumber();
	m_hLogFile << SPACE << _T("team:") << pPlayer->GetTeam();
	if (pTarget != NULL)
	{
		m_hLogFile << SPACE << _T("target:") << QuoteStr(pTarget->GetTypeName());
		m_hLogFile << SPACE << _T("owner:") << pTarget->GetOwnerClientNumber();
	}
	if (pAttacker != NULL)
		m_hLogFile << SPACE << _T("attacker:") << QuoteStr(pAttacker->GetTypeName());
	if (pInflictor != NULL)
		m_hLogFile << SPACE << _T("inflictor:") << QuoteStr(pInflictor->GetTypeName());
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteDamage
  ====================*/
void	CGameLog::WriteDamage(IUnitEntity *pTarget, int iPlayer, ushort unAttackerType, ushort unInflictorType, float fDamage)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pTarget == NULL || iPlayer == -1)
		return;

	CPlayer *pPlayer(Game.GetPlayer(iPlayer));

	m_hLogFile << GetGameLogEventName(GAME_LOG_DAMAGE);
	m_hLogFile << SPACE << _T("player:") << iPlayer;
	if (pPlayer != NULL)
		m_hLogFile << SPACE << _T("team:") << pPlayer->GetTeam();
	if (unAttackerType != INVALID_ENT_TYPE)
		m_hLogFile << SPACE << _T("attacker:") << QuoteStr(EntityRegistry.LookupName(unAttackerType));
	if (unInflictorType != INVALID_ENT_TYPE)
		m_hLogFile << SPACE << _T("inflictor:") << QuoteStr(EntityRegistry.LookupName(unInflictorType));
	m_hLogFile << SPACE << _T("target:") << QuoteStr(pTarget->GetTypeName());
	m_hLogFile << SPACE << _T("damage:") << XtoA(fDamage, 0, 0, 2);
	if (pTarget->GetOwnerClientNumber() != -1)
		m_hLogFile << SPACE << _T("owner:") << pTarget->GetOwnerClientNumber();
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteItem
  ====================*/
void	CGameLog::WriteItem(EGameLogEvent eEvent, IEntityItem *pItem, IUnitEntity *pTarget)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pItem == NULL)
		return;
	
	IUnitEntity *pOwner(pItem->GetOwner());
	if (eEvent == GAME_LOG_ITEM_TRANSFER)
	{
		pOwner = pTarget;
		pTarget = pItem->GetOwner();
	}

	if (pOwner == NULL || pOwner->GetOwnerClientNumber() == -1)
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	m_hLogFile << SPACE << _T("time:") << Game.GetMatchTime();
	m_hLogFile << SPACE << _T("x:") << INT_FLOOR(pOwner->GetPosition().x);
	m_hLogFile << SPACE << _T("y:") << INT_FLOOR(pOwner->GetPosition().y);
	m_hLogFile << SPACE << _T("z:") << INT_FLOOR(pOwner->GetPosition().z);
	m_hLogFile << SPACE << _T("player:") << pOwner->GetOwnerClientNumber();
	m_hLogFile << SPACE << _T("team:") << pOwner->GetTeam();
	m_hLogFile << SPACE << _T("item:") << QuoteStr(pItem->GetTypeName());
	if (eEvent == GAME_LOG_ITEM_PURCHASE)
		m_hLogFile << SPACE << _T("cost:") << pItem->GetCost();
	if (eEvent == GAME_LOG_ITEM_SELL)
		m_hLogFile << SPACE << _T("value:") << pItem->GetValue();
	if (eEvent == GAME_LOG_ITEM_TRANSFER && pTarget != NULL)
	{
		m_hLogFile << SPACE << _T("source:") << QuoteStr(pOwner->GetTypeName());
		m_hLogFile << SPACE << _T("target:") << QuoteStr(pTarget->GetTypeName());
		if (pTarget->GetOwnerClientNumber() != -1)
			m_hLogFile << SPACE << _T("owner:") << pTarget->GetOwnerClientNumber();
	}
	if (eEvent == GAME_LOG_ITEM_ACTIVATE && pTarget != NULL)
	{
		m_hLogFile << SPACE << _T("target:") << QuoteStr(pTarget->GetTypeName());
		if (pTarget->GetOwnerClientNumber() != -1)
			m_hLogFile << SPACE << _T("owner:") << pTarget->GetOwnerClientNumber();
	}
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteDeny
  ====================*/
void	CGameLog::WriteDeny(IUnitEntity *pTarget, IUnitEntity *pAttacker, IGameEntity *pInflictor, float fExperience, ushort unGold)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pTarget == NULL || pAttacker == NULL || pAttacker->GetOwnerClientNumber() == -1)
		return;

	if (pTarget->IsCreep())
		m_hLogFile << GetGameLogEventName(GAME_LOG_CREEP_DENY);
	else if (pTarget->IsHero())
		m_hLogFile << GetGameLogEventName(GAME_LOG_HERO_DENY);
	else if (pTarget->IsBuilding())
		m_hLogFile << GetGameLogEventName(GAME_LOG_BUILDING_DENY);
	else
		return;

	WRITE_STAT(time, Game.GetMatchTime());
	WRITE_STAT(x, INT_FLOOR(pTarget->GetPosition().x));
	WRITE_STAT(y, INT_FLOOR(pTarget->GetPosition().y));
	WRITE_STAT(z, INT_FLOOR(pTarget->GetPosition().z));
	WRITE_STAT(player, pAttacker->GetOwnerClientNumber());
	WRITE_STAT(team, pAttacker->GetTeam());
	if (pInflictor != NULL)
		WRITE_STAT(inflictor, QuoteStr(pInflictor->GetTypeName()));
	WRITE_STAT(name, QuoteStr(pTarget->GetTypeName()));
	if (fExperience >= 0.01f)
		WRITE_STAT(experience, XtoA(fExperience, 0, 0, 2));
	if (unGold > 0)
		WRITE_STAT(gold, unGold);
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteExperience
  ====================*/
void	CGameLog::WriteExperience(EGameLogEvent eEvent, IUnitEntity *pTarget, IUnitEntity *pSource, float fExperience)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pTarget == NULL || pTarget->GetOwnerClientNumber() == -1)
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	m_hLogFile << SPACE << _T("time:") << Game.GetMatchTime();
	m_hLogFile << SPACE << _T("x:") << INT_FLOOR(pTarget->GetPosition().x);
	m_hLogFile << SPACE << _T("y:") << INT_FLOOR(pTarget->GetPosition().y);
	m_hLogFile << SPACE << _T("z:") << INT_FLOOR(pTarget->GetPosition().z);
	m_hLogFile << SPACE << _T("player:") << pTarget->GetOwnerClientNumber();
	WRITE_STAT(team, pTarget->GetTeam());
	m_hLogFile << SPACE << _T("experience:") << XtoA(fExperience, 0, 0, 2);
	if (pSource != NULL)
	{
		m_hLogFile << SPACE << _T("source:") << QuoteStr(pSource->GetTypeName());

		if (pSource->GetOwnerClientNumber() != -1)
			m_hLogFile << SPACE << _T("owner:") << pSource->GetOwnerClientNumber();
	}

	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteGold
  ====================*/
void	CGameLog::WriteGold(EGameLogEvent eEvent, CPlayer *pPlayer, IUnitEntity *pSource, ushort unGold)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pPlayer == NULL || pPlayer->GetClientNumber() == -1 || unGold == 0)
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	WRITE_STAT(time, Game.GetMatchTime());
	if (pSource != NULL)
	{
		WRITE_STAT(x, INT_FLOOR(pSource->GetPosition().x));
		WRITE_STAT(y, INT_FLOOR(pSource->GetPosition().y));
		WRITE_STAT(z, INT_FLOOR(pSource->GetPosition().z));
	}
	WRITE_STAT(player, pPlayer->GetClientNumber());
	WRITE_STAT(team, pPlayer->GetTeam());
	if (pSource != NULL)
	{
		WRITE_STAT(source, QuoteStr(pSource->GetTypeName()));
		if (pSource->GetOwnerClientNumber() != -1)
			WRITE_STAT(owner, pSource->GetOwnerClientNumber());
	}
	WRITE_STAT(gold, unGold);
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteAbility
  ====================*/
void	CGameLog::WriteAbility(EGameLogEvent eEvent, IEntityAbility *pAbility, IUnitEntity *pTarget)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pAbility == NULL)
		return;

	IUnitEntity *pOwner(pAbility->GetOwner());
	if (pOwner == NULL || pOwner->GetOwnerClientNumber() == -1)
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	m_hLogFile << SPACE << _T("time:") << Game.GetMatchTime();
	m_hLogFile << SPACE << _T("x:") << INT_FLOOR(pOwner->GetPosition().x);
	m_hLogFile << SPACE << _T("y:") << INT_FLOOR(pOwner->GetPosition().y);
	m_hLogFile << SPACE << _T("z:") << INT_FLOOR(pOwner->GetPosition().z);
	m_hLogFile << SPACE << _T("player:") << pOwner->GetOwnerClientNumber();
	WRITE_STAT(team, pOwner->GetTeam());
	m_hLogFile << SPACE << _T("name:") << QuoteStr(pAbility->GetTypeName());
	m_hLogFile << SPACE << _T("level:") << pAbility->GetLevel();
	m_hLogFile << SPACE << _T("slot:") << pAbility->GetSlot();
	if (eEvent == GAME_LOG_ABILITY_ACTIVATE && pTarget != NULL)
	{
		m_hLogFile << SPACE << _T("target:") << QuoteStr(pTarget->GetTypeName());
		if (pTarget->GetOwnerClientNumber() != -1)
			m_hLogFile << SPACE << _T("owner:") << pTarget->GetOwnerClientNumber();
	}
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteHero
  ====================*/
void	CGameLog::WriteHero(EGameLogEvent eEvent, IHeroEntity *pHero, const tstring &sParamA)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pHero == NULL || pHero->GetOwnerClientNumber() == -1)
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	m_hLogFile << SPACE << _T("time:") << Game.GetMatchTime();
	m_hLogFile << SPACE << _T("x:") << INT_FLOOR(pHero->GetPosition().x);
	m_hLogFile << SPACE << _T("y:") << INT_FLOOR(pHero->GetPosition().y);
	m_hLogFile << SPACE << _T("z:") << INT_FLOOR(pHero->GetPosition().z);
	m_hLogFile << SPACE << _T("player:") << pHero->GetOwnerClientNumber();
	WRITE_STAT(team, pHero->GetTeam());
	switch (eEvent)
	{
	case GAME_LOG_HERO_RESPAWN:
		m_hLogFile << SPACE << _T("duration:") << (Game.GetGameTime() - pHero->GetDeathTimeStamp());
		break;

	case GAME_LOG_HERO_LEVEL:
		m_hLogFile << SPACE << _T("level:") << pHero->GetLevel();
		break;

	case GAME_LOG_HERO_POWERUP:
		m_hLogFile << SPACE << _T("type:") << QuoteStr(sParamA);
		break;

	case GAME_LOG_HERO_SUICIDE:
		if (!sParamA.empty())
			m_hLogFile << SPACE << _T("inflicter:") << QuoteStr(sParamA);
		break;
	}
	m_hLogFile << newl;
}


/*====================
  CGameLog::WriteAward
  ====================*/
void	CGameLog::WriteAward(EGameLogEvent eEvent, IUnitEntity *pAttacker, IUnitEntity *pTarget)
{
	if (!m_hLogFile.IsOpen())
		return;

	if (pAttacker == NULL || pAttacker->GetOwnerClientNumber() == -1)
		return;

	m_hLogFile << GetGameLogEventName(eEvent);
	m_hLogFile << SPACE << _T("time:") << Game.GetMatchTime();
	m_hLogFile << SPACE << _T("x:") << INT_FLOOR(pAttacker->GetPosition().x);
	m_hLogFile << SPACE << _T("y:") << INT_FLOOR(pAttacker->GetPosition().y);
	m_hLogFile << SPACE << _T("z:") << INT_FLOOR(pAttacker->GetPosition().z);
	m_hLogFile << SPACE << _T("player:") << pAttacker->GetOwnerClientNumber();
	WRITE_STAT(team, pAttacker->GetTeam());

	CPlayer *pOwner(pAttacker->GetOwnerPlayer());
	if (pOwner != NULL)
	{
		if (eEvent == GAME_LOG_AWARD_KILL_STREAK)
			m_hLogFile << SPACE << _T("count:") << pOwner->GetKillStreak();
		else if (eEvent == GAME_LOG_AWARD_MULTI_KILL)
			m_hLogFile << SPACE << _T("count:") << pOwner->GetMultiKill();
		else if (eEvent == GAME_LOG_AWARD_KILL_STREAK_BREAK && pTarget != NULL && pTarget->GetOwnerPlayer() != NULL)
			m_hLogFile << SPACE << _T("count:") << pTarget->GetOwnerPlayer()->GetKillStreak();
	}

	if (pTarget != NULL)
	{
		m_hLogFile << SPACE << _T("name:") << QuoteStr(pTarget->GetTypeName());
		if (pTarget->GetOwnerClientNumber() != -1)
			m_hLogFile << SPACE << _T("owner:") << pTarget->GetOwnerClientNumber();
	}
	m_hLogFile << newl;
}
