// (C)2005 S2 Games
// c_entityclientinfo.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entityclientinfo.h"
#include "c_teaminfo.h"
#include "c_teamdefinition.h"

#include "../k2/c_clientconnection.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR(Entity, ClientInfo);

vector<SDataField>*	CEntityClientInfo::s_pvFields;

CVAR_FLOATF(	g_unitTradeInRefund,		0.75f,			CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_INTF(		g_statPointsPerLevel,		3,				CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_statsCostLevelMult,		0.0f,			CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_statsCostExponent,		1.0f,			CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_statsCostTotalMult,		1.0f,			CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_INTF(		g_maxPlayerLevel,			15,				CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_expLevelExponent,			0.5f,			CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_expLevelMultiplier,		100.0f,			CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_INTF(		g_maxAttributeBoost,			-1,				CVAR_TRANSMIT | CVAR_GAMECONFIG);

/*CVAR_FLOATF(	g_attributeBoostMaxHealth,		0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostMaxMana,		0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostMaxStamina,		0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostHealthRegen,	0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostManaRegen,		0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostStaminaRegen,	0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostArmor,			0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostMeleeDamage,	0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostAttackSpeed,	0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostRangedDamage,	0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);*/

CVAR_FLOATF(	g_attributeBoostEndurance,		0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostIntelligence,	0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostAgility,		0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_attributeBoostStrength,		0.05f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF(		g_maxInitialGold,				600,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_initialGoldPerSec,			0.65f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	g_initialExpPerSec,				0.80f,		CVAR_TRANSMIT | CVAR_GAMECONFIG);
//=============================================================================

/*====================
  CEntityClientInfo::CEntityClientInfo
  ====================*/
CEntityClientInfo::CEntityClientInfo() :
IGameEntity(NULL),
m_uiPlayerEntityIndex(INVALID_INDEX),

m_iTeam(-1),
m_iLastTeam(-1),
m_ySquad(INVALID_SQUAD),

m_fExperience(0.0f),
m_fCommExperience(0.0f),
m_unGold(0),
m_fInitialExperience(0.0f),
m_unInitialGold(0),

m_iLevel(1),
m_iCommLevel(1),

m_uiConnectTime(INVALID_TIME),
m_uiDisconnectTime(INVALID_TIME),
m_uiPlayTime(0),
m_uiCommPlayTime(0),
m_uiDiscTime(0),
m_uiCommDiscTime(0),
m_uiDemoTimeRemaining(INVALID_TIME),

m_uiLoadoutTime(0),
m_ySpawnQueuePosition(0)
{
	for (int i(0); i < NUM_PERSISTANT_STATS; ++i)
		m_iPersistantStats[i] = 0;

	for (int i(0); i < NUM_PLAYER_ATTRIBUTES; ++i)
		m_anAttributes[i] = 0;

	for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
		m_auiGadgets[i] = INVALID_INDEX;
}


/*====================
  CEntityClientInfo::Initialize
  ====================*/
void	CEntityClientInfo::Initialize(CClientConnection *pClientConnection)
{
	if (pClientConnection == NULL)
		return;

	m_uiConnectTime = Game.GetGameTime();
	m_iAccountID = pClientConnection->GetAccountID();
	m_iClientNumber = pClientConnection->GetClientNum();
	m_sName = pClientConnection->GetName();
	m_uiPlayerEntityIndex = INVALID_INDEX;

	m_uiLastInputTime = Game.GetGameTime();

	m_uiLoadoutTime = 0;
	m_ySpawnQueuePosition = 0;

	m_iFlags = CLIENT_INFO_WANTS_TO_BE_OFFICER;

	// Set initial experience
	float fMinExp(MsToSec(Game.GetCurrentGameLength()) * g_initialExpPerSec);
 	if (GetExperience() < fMinExp)
	{
		float fExpGiven(MIN(fMinExp - GetExperience(), fMinExp - GetInitialExperience()));
		fExpGiven = MAX(0.0f, fExpGiven);

		SetInitialExperience(GetInitialExperience() + fExpGiven);
		GiveExperience(fExpGiven);

		Console << _T("Giving user ") << pClientConnection->GetName() << _T(" ") << fExpGiven << _T(" exp, at a ratio of ") << g_initialExpPerSec << _T(" exp/second (") << Game.GetCurrentGameLength() / MS_PER_SEC << _T(" secs).") << newl;
	}

	// Set initial gold
	ushort unMinGold(MIN((ushort)(MsToSec(Game.GetCurrentGameLength()) * g_initialGoldPerSec), (ushort)g_maxInitialGold.GetUnsignedInteger()));
 	if (GetGold() < unMinGold)
	{
		ushort unGoldGiven(MIN(unMinGold - GetGold(), unMinGold - GetInitialGold()));	

		SetInitialGold(GetInitialGold() + unGoldGiven);
		GiveGold(unGoldGiven);

		Console << _T("Giving user ") << pClientConnection->GetName() << _T(" ") << unGoldGiven << _T(" gold, at a ratio of ") << g_initialGoldPerSec << _T(" gold/second (") << Game.GetCurrentGameLength() / MS_PER_SEC << _T(" secs).") << newl;
	}
}


/*====================
  CEntityClientInfo::Reset
  ====================*/
bool	CEntityClientInfo::Reset()
{
	m_uiLoadoutTime = 0;
	m_ySpawnQueuePosition = 0;

	m_fInitialExperience = 0.0f;
	m_fExperience = 0.0f;
	m_fCommExperience = 0.0f;

	m_iLevel = 1;
	m_iCommLevel = 1;

	m_unGold = 0;
	m_unInitialGold = 0;

	m_unSouls = 0;

	if (m_uiPlayerEntityIndex != INVALID_INDEX)
	{
		Game.DeleteEntity(m_uiPlayerEntityIndex);
		m_uiPlayerEntityIndex = INVALID_INDEX;
	}

	for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
		m_auiGadgets[i] = INVALID_INDEX;

	return true;
}


/*====================
  CEntityClientInfo::SetDisconnected
  ====================*/
void	CEntityClientInfo::SetDisconnected(bool b)
{
	if (b)
		m_iFlags |= CLIENT_INFO_DISCONNECTED;
	else
		m_iFlags &= ~(CLIENT_INFO_DISCONNECTED);
}

/*====================
  CEntityClientInfo::ServerFrame
  ====================*/
bool	CEntityClientInfo::ServerFrame()
{
	// Clean up gadget list
	for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
	{
		if (Game.GetGadgetEntity(m_auiGadgets[i]) == NULL)
			RemoveGadget(m_auiGadgets[i]);
	}

	if (GetLastTeam() > 0 && Game.GetGamePhase() == GAME_PHASE_ACTIVE && !HasNetFlags(ENT_NET_FLAG_QUEUED))
	{
		CEntityTeamInfo *pTeam(Game.GetTeam(GetLastTeam()));

		if (pTeam != NULL && pTeam->GetLastCommanderClientID() == m_iClientNumber)
		{
			if (!IsDisconnected() && pTeam->GetCommanderClientID() == m_iClientNumber)
				m_uiCommPlayTime += Game.GetFrameLength();
			else if (m_uiCommPlayTime > SecToMs(ICvar::GetUnsignedInteger(_T("svr_minStatsTimeComm"))))
				m_uiCommDiscTime += Game.GetFrameLength();
		}
		
		if (pTeam == NULL || pTeam->GetCommanderClientID() != m_iClientNumber)
		{
			if (!IsDisconnected() && GetTeam() != 0)
				m_uiPlayTime += Game.GetFrameLength();
			else if (m_uiPlayTime > SecToMs(ICvar::GetUnsignedInteger(_T("svr_minStatsTime"))))
				m_uiDiscTime += Game.GetFrameLength();
		}
	}

	// Spec doesn't count towards time played
	// Time spent in queue doesn't count towards time played
	// Time spent disconnected doesn't count towards time played
	if (GetDemoTimeRemaining() > Game.GetCurrentGameLength() &&
		((GetTeam() == 0 && IsDemoAccount() && Game.GetGamePhase() == GAME_PHASE_ACTIVE) ||
		(HasNetFlags(ENT_NET_FLAG_QUEUED) && Game.GetGamePhase() == GAME_PHASE_ACTIVE && GetLastTeam() > 0 && IsDemoAccount()) ||
		(IsDisconnected() && IsDemoAccount())))
		m_uiDemoTimeRemaining += Game.GetFrameLength();
	
	IPlayerEntity *pPlayer(GetPlayerEntity());

	// Dead
	if (pPlayer != NULL && (pPlayer->GetStatus() == ENTITY_STATUS_DEAD || pPlayer->GetStatus() == ENTITY_STATUS_CORPSE))
	{
		// Check for limbo transition
		if (Game.GetGameTime() >= pPlayer->GetDeathTime() || pPlayer->HasNetFlags(ENT_NET_FLAG_NO_RESURRECT))
		{
			uint uiLoadoutTime(0);
			if (pPlayer->GetDeathTime() > Game.GetGameTime() && Game.GetGamePhase() != GAME_PHASE_WARMUP)
				uiLoadoutTime = pPlayer->GetDeathTime() - Game.GetGameTime();

			if (GetTeam() > 0)
			{
				CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
				if (pTeam != NULL)
					pTeam->UpdateSpawnQueue(this);
			}

			pPlayer = Game.ChangeUnit(m_iClientNumber, Player_Observer, CHANGE_UNIT_NO_DELETE | CHANGE_UNIT_TRANSFER_PERSISTANTS | CHANGE_UNIT_REFUND_GOLD);

			if (pPlayer != NULL)
			{
				pPlayer->SetStatus(ENTITY_STATUS_DORMANT);
				pPlayer->SetNetFlags(ENT_NET_FLAG_KILLED);

				// Trigger "on entering loadout" script
				Game.RegisterTriggerParam(_T("name"), m_sName);
				Game.RegisterTriggerParam(_T("clientid"), XtoA(m_iClientNumber));
				Game.RegisterTriggerParam(_T("index"), XtoA(pPlayer->GetIndex()));
				Game.TriggerGlobalScript(_T("loadout"));
			}

			SetLoadoutTime(uiLoadoutTime);
		}
	}

	return true;
}


/*====================
  CEntityClientInfo::ClearAffiliations
  ====================*/
void	CEntityClientInfo::ClearAffiliations()
{
	bool bWasDisconnected(IsDisconnected());

	m_iVote = -1;
	m_iFlags = CLIENT_INFO_WANTS_TO_BE_OFFICER | (bWasDisconnected ? CLIENT_INFO_DISCONNECTED : 0);

	RemoveNetFlags(ENT_NET_FLAG_QUEUED | ENT_NET_FLAG_KILLED);

	m_ySpawnQueuePosition = 0;
	m_uiLoadoutTime = 0;

	SetTeam(0);
	SetSquad(INVALID_SQUAD);
}


/*====================
  CEntityClientInfo::GetAvailablePoints
  ====================*/
int		CEntityClientInfo::GetAvailablePoints() const
{
	return ((m_iLevel - 1) * g_statPointsPerLevel) - m_unAttributePointsSpent;
}


/*====================
  CEntityClientInfo::SpendPoint
  ====================*/
void	CEntityClientInfo::SpendPoint(int iStat)
{
	if (iStat <= ATTRIBUTE_NULL || iStat >= NUM_PLAYER_ATTRIBUTES)
		return;

	int iPointsRequired(GetAttributeCost(iStat));

	if (GetAvailablePoints() < iPointsRequired || (g_maxAttributeBoost.GetInteger() != -1 && m_anAttributes[iStat] >= g_maxAttributeBoost.GetInteger()))
		return;

	++m_anAttributes[iStat];
	m_unAttributePointsSpent += iPointsRequired;
}


/*====================
  CEntityClientInfo::ResetAttributes
  ====================*/
void	CEntityClientInfo::ResetAttributes()
{
	for (int iLoop(1); iLoop < NUM_PLAYER_ATTRIBUTES; iLoop++)
		m_anAttributes[iLoop] = 0;

	m_unAttributePointsSpent = 0;
}


/*====================
  CEntityClientInfo::AddGadget
  ====================*/
void	CEntityClientInfo::AddGadget(uint uiIndex)
{
	for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
	{
		if (m_auiGadgets[i] != INVALID_INDEX)
			continue;

		m_auiGadgets[i] = uiIndex;
		break;
	}
}


/*====================
  CEntityClientInfo::RemoveGadget
  ====================*/
void	CEntityClientInfo::RemoveGadget(uint uiIndex)
{
	for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
	{
		if (m_auiGadgets[i] != uiIndex)
			continue;

		for (int n(i); n < MAX_DEPLOYED_GADGETS - 1; ++n)
			m_auiGadgets[n] = m_auiGadgets[n + 1];
		m_auiGadgets[MAX_DEPLOYED_GADGETS - 1] = INVALID_INDEX;
		break;
	}
}


/*====================
  CEntityClientInfo::GetPercentNextLevel
  ====================*/
float	CEntityClientInfo::GetPercentNextLevel() const
{
	float fLastLevel(0.0f);
	float fNextLevel(0.0f);
	int i(1);
	for (; i <= g_maxPlayerLevel; ++i)
	{
		fNextLevel += pow(float(i - 1), g_expLevelExponent) * g_expLevelMultiplier;
		if (m_fExperience < fNextLevel)
			break;

		fLastLevel = fNextLevel;
	}

	if (m_fExperience >= fNextLevel)
		return 1.0f;

	float fDiff(fNextLevel - fLastLevel);

	return (m_fExperience - fLastLevel) / fDiff;
}


/*====================
  CEntityClientInfo::GiveExperience
  ====================*/
void	CEntityClientInfo::GiveExperience(float fExperience)
{
	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return;

	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));

	if (pTeam != NULL && pTeam->GetCommanderClientID() == GetClientNumber())
	{
		GiveCommExperience(fExperience);
		return;
	}

	// Apply experience
	m_fExperience += fExperience;

	// Calculate current level
	int iNewLevel(1);
	float fAccumulator(0.0f);
	for (; iNewLevel < g_maxPlayerLevel; ++iNewLevel)
	{
		fAccumulator += pow(float(iNewLevel), g_expLevelExponent) * g_expLevelMultiplier;
		if (m_fExperience <= fAccumulator)
			break;
	}

	bool bCanSpend(false);

	if (iNewLevel > m_iLevel && m_uiPlayerEntityIndex != INVALID_INDEX)
	{
		CGameEvent evLevelup;
		evLevelup.SetSourceEntity(m_uiPlayerEntityIndex);
		evLevelup.SetEffect(Game.RegisterEffect(_T("/shared/effects/levelup.effect")));
		Game.AddEvent(evLevelup);

		bCanSpend = true;
	}

	m_iLevel = iNewLevel;

	// Randomly assign stats for demo accounts
	if (IsDemoAccount())
	{
		while (GetAvailablePoints() > 0 && bCanSpend)
		{
			SpendPoint(M_Randnum(1, NUM_PLAYER_ATTRIBUTES));

			bCanSpend = false;
			int iAvailable(GetAvailablePoints());
			for (int i(1); i < NUM_PLAYER_ATTRIBUTES; i++)
			{
				if (GetAttributeCost(i) <= iAvailable && m_anAttributes[i] < g_maxAttributeBoost.GetInteger())
				{
					bCanSpend = true;
					break;
				}
			}
		}
	}
}

void	CEntityClientInfo::GiveExperience(float fExperience, const CVec3f &v3Pos)
{
	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return;

	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));

	if (pTeam != NULL && pTeam->GetCommanderClientID() == GetClientNumber())
	{
		GiveCommExperience(fExperience, v3Pos);
		return;
	}

	GiveExperience(fExperience);

	// Send reward event
	if (fExperience > 0.0f)
	{
		CBufferFixed<17> buffer;
		buffer << GAME_CMD_REWARD << byte(1) << v3Pos << ushort(ROUND(fExperience));
		Game.SendGameData(GetClientNumber(), buffer, true);
	}
}

/*====================
  CEntityClientInfo::GiveCommExperience
  ====================*/
void	CEntityClientInfo::GiveCommExperience(float fExperience)
{
	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return;

	// Apply experience
	m_fCommExperience += fExperience;

	// Calculate current level
	int iNewLevel(1);
	float fAccumulator(0.0f);
	for (; iNewLevel < g_maxPlayerLevel; ++iNewLevel)
	{
		fAccumulator += pow(float(iNewLevel), g_expLevelExponent) * g_expLevelMultiplier;
		if (m_fCommExperience <= fAccumulator)
			break;
	}

	bool bCanSpend(false);

	if (iNewLevel > m_iCommLevel && m_uiPlayerEntityIndex != INVALID_INDEX)
	{
		CGameEvent evLevelup;
		evLevelup.SetSourceEntity(m_uiPlayerEntityIndex);
		evLevelup.SetEffect(Game.RegisterEffect(_T("/shared/effects/levelup.effect")));
		Game.AddEvent(evLevelup);

		bCanSpend = true;
	}

	m_iCommLevel = iNewLevel;

	// Randomly assign stats for demo accounts
	if (IsDemoAccount())
	{
		while (GetAvailablePoints() > 0 && bCanSpend)
		{
			SpendPoint(M_Randnum(1, NUM_PLAYER_ATTRIBUTES));

			bCanSpend = false;
			int iAvailable(GetAvailablePoints());
			for (int i(1); i < NUM_PLAYER_ATTRIBUTES; i++)
			{
				if (GetAttributeCost(i) <= iAvailable && m_anAttributes[i] < g_maxAttributeBoost.GetInteger())
				{
					bCanSpend = true;
					break;
				}
			}
		}
	}

	// Record commander experience
	Game.MatchStatEvent(GetClientNumber(), COMMANDER_MATCH_EXPERIENCE, fExperience);
}

void	CEntityClientInfo::GiveCommExperience(float fExperience, const CVec3f &v3Pos)
{
	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return;

	GiveExperience(fExperience);

	// Send reward event
	if (fExperience > 0.0f)
	{
		CBufferFixed<17> buffer;
		buffer << GAME_CMD_REWARD << byte(1) << v3Pos << ushort(ROUND(fExperience));
		Game.SendGameData(GetClientNumber(), buffer, true);
	}
}

/*====================
  CEntityClientInfo::GiveGold
  ====================*/
void	CEntityClientInfo::GiveGold(ushort unGold)
{
	if (Game.GetGamePhase() == GAME_PHASE_WARMUP)
		return;

	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));

	if (pTeam != NULL && pTeam->GetCommanderClientID() == GetClientNumber())
		pTeam->GiveGold(unGold);
	else
		m_unGold += unGold;
}


/*====================
  CEntityClientInfo::GetAttributeBoost
  ====================*/
float	CEntityClientInfo::GetAttributeBoost(int iAttribute) const
{
	if (iAttribute <= ATTRIBUTE_NULL || iAttribute >= NUM_PLAYER_ATTRIBUTES)
		return 0.0f;

	IPlayerEntity *pPlayer(Game.GetPlayerEntity(m_uiPlayerEntityIndex));
	if (pPlayer == NULL || !pPlayer->GetApplyAttributes())
		return 0.0f;

	int iAttributePoints(m_anAttributes[iAttribute]);
	CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
	if (pTeam != NULL)
		iAttributePoints += pTeam->GetStatLevel(iAttribute);
		
	switch (iAttribute)
	{
	case ATTRIBUTE_ENDURANCE: return iAttributePoints * g_attributeBoostEndurance;
	case ATTRIBUTE_INTELLIGENCE: return iAttributePoints * g_attributeBoostIntelligence;
	case ATTRIBUTE_AGILITY: return iAttributePoints * g_attributeBoostAgility;
	case ATTRIBUTE_STRENGTH: return iAttributePoints * g_attributeBoostStrength;
	}

	return 0.0f;
}

/*====================
  CEntityClientInfo::GetAttributeBoostIncrease
  ====================*/
float	CEntityClientInfo::GetAttributeBoostIncrease(int iAttribute) const
{
	if (iAttribute <= ATTRIBUTE_NULL || iAttribute >= NUM_PLAYER_ATTRIBUTES)
		return 0.0f;

	switch (iAttribute)
	{
	case ATTRIBUTE_ENDURANCE: return g_attributeBoostEndurance;
	case ATTRIBUTE_INTELLIGENCE: return g_attributeBoostIntelligence;
	case ATTRIBUTE_AGILITY: return g_attributeBoostAgility;
	case ATTRIBUTE_STRENGTH: return g_attributeBoostStrength;
	}

	return 0.0f;
}


/*====================
  CEntityClientInfo::GetAttributeCost
  ====================*/
int		CEntityClientInfo::GetAttributeCost(int iStat)
{
	int iPointsRequired(1);

	iPointsRequired += (m_anAttributes[iStat] * g_statsCostLevelMult);
	iPointsRequired = pow(iPointsRequired, g_statsCostExponent);
	iPointsRequired = (iPointsRequired * g_statsCostTotalMult);

	return iPointsRequired;
}


/*====================
  CEntityClientInfo::GetPlayerEntity()
  ====================*/
IPlayerEntity*	CEntityClientInfo::GetPlayerEntity()
{
	return Game.GetPlayerEntity(m_uiPlayerEntityIndex);
}


/*====================
  CEntityClientInfo::GetPlayerEntity()
  ====================*/
uint	CEntityClientInfo::GetPlayerEntityIndex()
{
	return m_uiPlayerEntityIndex;
}


/*====================
  CEntityClientInfo::Baseline
  ====================*/
void	CEntityClientInfo::Baseline()
{
	IGameEntity::Baseline();

	m_uiPlayerEntityIndex = INVALID_INDEX;

	m_iAccountID = -1;
	m_iClientNumber = -1;
	m_sClanName = _T("");
	m_sClanRank = _T("");
	m_sName = _T("");
	m_iKarma = 0;

	m_iVote = -1;
	m_iFlags = CLIENT_INFO_WANTS_TO_BE_OFFICER;
	m_iTeam = -1;
	m_iLastTeam = -1;
	m_ySquad = INVALID_SQUAD;

	m_fExperience = 0.0f;
	m_fCommExperience = 0.0f;
	m_unGold = 0;
	m_unSouls = 0;
	m_iLevel = 1;
	m_iCommLevel = 1;
	m_unAttributePointsSpent = 0;

	m_uiPlayTime = 0;
	m_uiDiscTime = 0;
	m_uiCommPlayTime = 0;
	m_uiCommDiscTime = 0;

	m_ySpawnQueuePosition = 0;

	m_uiLoadoutTime = 0;

	m_fInitialExperience = 0.0f;
	m_unInitialGold = 0;

	for (int i(0); i < NUM_PLAYER_ATTRIBUTES; ++i)
		m_anAttributes[i] = 0;

	m_unPing = 0;

	m_iKills = 0;
	m_iDeaths = 0;
	m_iAssists = 0;
	m_fPlayerDamage = 0;
	m_iRazes = 0;
	m_fBuildingDamage = 0;
	m_fHealed = 0;
	m_iResurrects = 0;
	m_fRepaired = 0;

	for (int i(0); i < NUM_PERSISTANT_STATS; ++i)
		m_iPersistantStats[i] = 0;

	for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
		m_auiGadgets[i] = INVALID_INDEX;
}


/*====================
  CEntityClientInfo::GetSnapshot
  ====================*/
void	CEntityClientInfo::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IGameEntity::GetSnapshot(snapshot);

	snapshot.AddGameIndex(m_uiPlayerEntityIndex);

	snapshot.AddField(m_iAccountID);
	snapshot.AddField(m_iClientNumber);
	snapshot.AddField(m_sName);
	snapshot.AddField(m_sClanName);
	snapshot.AddField(m_sClanRank);
	snapshot.AddField(m_iKarma);

	snapshot.AddField(m_iVote);
	snapshot.AddField(m_iFlags);
	
	snapshot.AddField(m_iTeam);
	snapshot.AddField(m_ySquad);

	snapshot.AddField(m_uiLoadoutTime);

	snapshot.AddRound16(m_fExperience);
	snapshot.AddField(m_unGold);
	snapshot.AddField(m_unSouls);
	snapshot.AddField(m_iLevel);
	snapshot.AddField(m_unAttributePointsSpent);

	for (int i(0); i < NUM_PLAYER_ATTRIBUTES; ++i)
		snapshot.AddField(m_anAttributes[i]);

	snapshot.AddField(m_unPing);

	snapshot.AddField(m_ySpawnQueuePosition);

	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_KILLS].GetTotalInt());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_DEATHS].GetTotalInt());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_ASSISTS].GetTotalInt());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_PLAYER_DAMAGE].GetTotalFloat());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_RAZED].GetTotalInt());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_BUILDING_DAMAGE].GetTotalFloat());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_HEALED].GetTotalFloat());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_RESURRECTS].GetTotalInt());
	snapshot.AddField(m_aMatchStatRecords[PLAYER_MATCH_REPAIRED].GetTotalFloat());

	for (int i(0); i < NUM_PERSISTANT_STATS; ++i)
		snapshot.AddField(m_iPersistantStats[i]);

	for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
		snapshot.AddGameIndex(m_auiGadgets[i]);

	snapshot.AddField(m_uiDemoTimeRemaining);
}


/*====================
  CEntityClientInfo::ReadSnapshot
  ====================*/
bool	CEntityClientInfo::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		IGameEntity::ReadSnapshot(snapshot);

		snapshot.ReadNextGameIndex(m_uiPlayerEntityIndex);
		
		snapshot.ReadNextField(m_iAccountID);
		snapshot.ReadNextField(m_iClientNumber);
		snapshot.ReadNextField(m_sName);
		snapshot.ReadNextField(m_sClanName);
		snapshot.ReadNextField(m_sClanRank);
		snapshot.ReadNextField(m_iKarma);

		snapshot.ReadNextField(m_iVote);
		snapshot.ReadNextField(m_iFlags);

		snapshot.ReadNextField(m_iTeam);
		snapshot.ReadNextField(m_ySquad);

		snapshot.ReadNextField(m_uiLoadoutTime);

		snapshot.ReadNextRound16(m_fExperience);
		snapshot.ReadNextField(m_unGold);
		snapshot.ReadNextField(m_unSouls);
		snapshot.ReadNextField(m_iLevel);
		snapshot.ReadNextField(m_unAttributePointsSpent);

		for (int i(0); i < NUM_PLAYER_ATTRIBUTES; ++i)
			snapshot.ReadNextField(m_anAttributes[i]);

		snapshot.ReadNextField(m_unPing);

		snapshot.ReadNextField(m_ySpawnQueuePosition);

		snapshot.ReadNextField(m_iKills);
		snapshot.ReadNextField(m_iDeaths);
		snapshot.ReadNextField(m_iAssists);
		snapshot.ReadNextField(m_fPlayerDamage);
		snapshot.ReadNextField(m_iRazes);
		snapshot.ReadNextField(m_fBuildingDamage);
		snapshot.ReadNextField(m_fHealed);
		snapshot.ReadNextField(m_iResurrects);
		snapshot.ReadNextField(m_fRepaired);

		for (int i(0); i < NUM_PERSISTANT_STATS; ++i)
			snapshot.ReadNextField(m_iPersistantStats[i]);

		for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
			snapshot.ReadNextGameIndex(m_auiGadgets[i]);

		snapshot.ReadNextField(m_uiDemoTimeRemaining);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityClientInfo::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  CEntityClientInfo::GetTypeVector
  ====================*/
const vector<SDataField>&	CEntityClientInfo::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IGameEntity::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

		s_pvFields->push_back(SDataField(_T("m_uiPlayerEntityIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));

		s_pvFields->push_back(SDataField(_T("m_iAccountID"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_iClientNumber"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_sName"), FIELD_PUBLIC, TYPE_STRING));
		s_pvFields->push_back(SDataField(_T("m_sClanName"), FIELD_PUBLIC, TYPE_STRING));
		s_pvFields->push_back(SDataField(_T("m_sClanRank"), FIELD_PUBLIC, TYPE_STRING));
		s_pvFields->push_back(SDataField(_T("m_iKarma"), FIELD_PUBLIC, TYPE_INT));

		s_pvFields->push_back(SDataField(_T("m_iVote"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_iFlags"), FIELD_PUBLIC, TYPE_INT));

		s_pvFields->push_back(SDataField(_T("m_iTeam"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_ySquad"), FIELD_PUBLIC, TYPE_CHAR));

		s_pvFields->push_back(SDataField(_T("m_uiLoadoutTime"), FIELD_PUBLIC, TYPE_INT));

		s_pvFields->push_back(SDataField(_T("m_fExperience"), FIELD_PUBLIC, TYPE_ROUND16));
		s_pvFields->push_back(SDataField(_T("m_unGold"), FIELD_PUBLIC, TYPE_SHORT));
		s_pvFields->push_back(SDataField(_T("m_unSouls"), FIELD_PUBLIC, TYPE_SHORT));
		s_pvFields->push_back(SDataField(_T("m_iLevel"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_unAttributePointsSpent"), FIELD_PUBLIC, TYPE_SHORT));

		for (int i(0); i < NUM_PLAYER_ATTRIBUTES; ++i)
			s_pvFields->push_back(SDataField(_T("m_anAttributes[") + XtoA(i) + _T("]"), FIELD_PUBLIC, TYPE_SHORT));

		s_pvFields->push_back(SDataField(_T("m_unPing"), FIELD_PUBLIC, TYPE_SHORT));

		s_pvFields->push_back(SDataField(_T("m_ySpawnQueuePosition"), FIELD_PUBLIC, TYPE_CHAR));

		s_pvFields->push_back(SDataField(_T("m_iKills"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_iDeaths"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_iAssists"), FIELD_PUBLIC, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_fPlayerDamage"), FIELD_PRIVATE, TYPE_FLOAT));
		s_pvFields->push_back(SDataField(_T("m_iRazes"), FIELD_PRIVATE, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_fBuildingDamage"), FIELD_PRIVATE, TYPE_FLOAT));
		s_pvFields->push_back(SDataField(_T("m_fHealed"), FIELD_PRIVATE, TYPE_FLOAT));
		s_pvFields->push_back(SDataField(_T("m_iResurrects"), FIELD_PRIVATE, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_fRepaired"), FIELD_PRIVATE, TYPE_FLOAT));

		for (int i(0); i < NUM_PERSISTANT_STATS; ++i)
			s_pvFields->push_back(SDataField(_T("m_iPersistantStats[") + XtoA(i) + ("]"), FIELD_PUBLIC, TYPE_INT));

		for (int i(0); i < MAX_DEPLOYED_GADGETS; ++i)
			s_pvFields->push_back(SDataField(_T("m_auiGadgets[") + XtoA(i) + _T("]"), FIELD_PRIVATE, TYPE_GAMEINDEX));

		s_pvFields->push_back(SDataField(_T("m_uiDemoTimeRemaining"), FIELD_PRIVATE, TYPE_INT));
	}

	return *s_pvFields;
}


/*====================
  CEntityClientInfo::ChangeUnit
  ====================*/
bool	CEntityClientInfo::ChangeUnit(ushort unNewUnitID, int iFlags)
{
	IPlayerEntity *pPlayer(GetPlayerEntity());

	// Check same type
	if (pPlayer != NULL && pPlayer->GetType() == unNewUnitID)
		return false;

	// Check commander
	if ((iFlags & CHANGE_UNIT_CHECK_COMMANDER) && HasFlags(CLIENT_INFO_IS_COMMANDER))
		return false;

	// Check race
	if ((iFlags & CHANGE_UNIT_CHECK_RACE))
	{
		ICvar *pRace(g_EntityRegistry.GetGameSetting(unNewUnitID, _T("Race")));
		tstring sRace(pRace == NULL ? SNULL : pRace->GetString());
		
		CEntityTeamInfo *pTeam(Game.GetTeam(GetTeam()));
		if (!sRace.empty() && pTeam != NULL && sRace != pTeam->GetDefinition()->GetName())
			return false;
	}

	// Status
	if ((iFlags & CHANGE_UNIT_CHECK_STATUS) && pPlayer != NULL && pPlayer->GetStatus() != ENTITY_STATUS_DORMANT)
		return false;

	// Prerequisites
	if (iFlags & CHANGE_UNIT_CHECK_PREREQUISITES)
	{
		ICvar *pPrerequisite(g_EntityRegistry.GetGameSetting(unNewUnitID, _T("Prerequisite")));
		tstring sPrerequisite(pPrerequisite == NULL ? SNULL : pPrerequisite->GetString());
		if (!sPrerequisite.empty())
		{
			CEntityTeamInfo *pTeamInfo(Game.GetTeam(GetTeam()));
			if (pTeamInfo == NULL)
				EX_ERROR(_T("Invalid team: ") + XtoA(GetTeam()));

			if (!pTeamInfo->HasBuilding(sPrerequisite))
				return false;
		}
	}

	// Check cost
	if (iFlags & CHANGE_UNIT_CHECK_COST)
	{
		ICvar *pCost(g_EntityRegistry.GetGameSetting(unNewUnitID, _T("Cost")));
		int iCost(pCost == NULL ? 0 : pCost->GetUnsignedInteger());
		if ((iFlags & CHANGE_UNIT_REFUND_GOLD) && pPlayer != NULL)
			iCost = iCost - INT_CEIL(pPlayer->GetCost() * g_unitTradeInRefund);

		if (iCost > 0)
		{
			if (!SpendGold(short(iCost)))
				return false;
		}
		else
			GiveGold(short(iCost * -1));

		// Hellbourne
		// FIXME: This doesn't really belong here...
		ICvar *pIsHellbournce(g_EntityRegistry.GetGameSetting(unNewUnitID, _T("IsHellbourne")));
		if (pIsHellbournce != NULL && pIsHellbournce->GetBool())
		{
			ICvar *pSoulCost(g_EntityRegistry.GetGameSetting(unNewUnitID, _T("SoulCost")));
			if (pSoulCost != NULL && !SpendSouls(pSoulCost->GetInteger()))
				return false;
		}
	}

	// Create the new entity
	IGameEntity *pNewEntity(Game.AllocateEntity(unNewUnitID));
	IPlayerEntity *pNewPlayer(NULL);

	if (pNewEntity != NULL && pNewEntity->IsPlayer())
	{
		m_uiPlayerEntityIndex = pNewEntity->GetIndex();
		pNewPlayer = pNewEntity->GetAsPlayerEnt();
	}

	if (pNewPlayer == NULL)
	{
		Console.Err << _T("CEntityClientInfo::ChangeClass() - Allocated a non-player entity") << newl;
		SAFE_DELETE(pNewEntity);
		return false;
	}

	if (pPlayer != NULL && pPlayer->HasNetFlags(ENT_NET_FLAG_KILLED))
		pNewPlayer->SetNetFlags(ENT_NET_FLAG_KILLED);

	pNewPlayer->SetClientID(GetClientNumber());

	pNewPlayer->SetTeam(GetTeam());
	pNewPlayer->SetSquad(GetSquad());
	pNewPlayer->Spawn();

	// Transfer items from old entity to new entity.
	// Sell if the new entity can't carry items.
	if ((iFlags & CHANGE_UNIT_TRANSFER_ITEMS || iFlags & CHANGE_UNIT_TRANSFER_PERSISTANTS)&& pPlayer != NULL)
	{
		for (int i(INVENTORY_START_BACKPACK); i < INVENTORY_END_BACKPACK; ++i)
		{
			IInventoryItem *pItem(pPlayer->GetItem(i));
			if (pItem == NULL)
				continue;

			if (!pItem->IsPersistant() && !(iFlags & CHANGE_UNIT_TRANSFER_ITEMS))
				continue;

			if (!pNewPlayer->GetCanPurchase())
			{
				m_unGold += (pItem->GetCost() * pPlayer->GetAmmoCount(i));
				continue;
			}

			pNewPlayer->GiveItem(i, pItem->GetType());
			IInventoryItem *pNewItem(pNewPlayer->GetItem(i));
			if (pNewItem != NULL)
			{
				pNewItem->SetAmmo(pItem->GetAmmo());
				pNewItem->SetOwner(pNewPlayer->GetIndex());
				pNewPlayer->SetAmmo(i, pPlayer->GetAmmoCount(i));

				if (pNewItem->IsPersistant() && pItem->IsPersistant())
				{
					pNewItem->GetAsPersistant()->SetItemData(pItem->GetAsPersistant()->GetItemData());
					pNewItem->GetAsPersistant()->SetItemID(pItem->GetAsPersistant()->GetItemID());
				}

				pNewItem->ActivatePassive();
			}
		}
	}

	// Set the new entities health
	float fHealthPercent(1.0f);
	if (iFlags & CHANGE_UNIT_INHERIT_HP && pPlayer != NULL && !pPlayer->HasNetFlags(ENT_NET_FLAG_KILLED) && pPlayer->GetHealth() > 0)
		fHealthPercent = pPlayer->GetHealthPercent();
	pNewPlayer->SetHealth(pNewPlayer->GetMaxHealth() * fHealthPercent);

	if (iFlags & CHANGE_UNIT_SPAWN)
	{
		pNewPlayer->SetLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT);
		pNewPlayer->Spawn2();
		pNewPlayer->RemoveLocalFlags(ENT_LOCAL_FLAG_CHANGING_UNIT);
	}

	if (pPlayer == NULL)
		return true;

	if (iFlags & CHANGE_UNIT_INHERIT_POS)
	{
		// FIXME: This code can cause players to spawn on trees, etc.
		CVec3f v3Spawn(pPlayer->GetPosition());
		CVec3f v3Start(v3Spawn);
		STraceInfo trace;

		Game.TraceBox(trace, v3Start, v3Spawn, pNewPlayer->GetBounds(), TRACE_PLAYER_MOVEMENT, pPlayer->GetWorldIndex());

		while (trace.bStartedInSurface)
		{
			v3Start[Z] += 25.0f;
			Game.TraceBox(trace, v3Start, v3Spawn, pNewPlayer->GetBounds(), TRACE_PLAYER_MOVEMENT, pPlayer->GetWorldIndex());
		}

		pNewPlayer->SetPosition(trace.v3EndPos);
		pNewPlayer->SetAngles(pPlayer->GetAngles()[PITCH], 0.0f, pPlayer->GetAngles()[YAW]);
	}

	if (iFlags & CHANGE_UNIT_INHERIT_DAMAGE_RECORD)
	{
		map<uint, SDamageRecord> mapRecord;

		mapRecord = pPlayer->GetDamageRecordMap();
		pNewPlayer->SetDamageRecordMap(mapRecord);
	}

	// Copy scripts
	Game.CopyEntityScripts(pPlayer->GetIndex(), pNewPlayer->GetIndex());
	Game.ClearEntityScripts(pPlayer->GetIndex());

	return true;
}


/*====================
  CEntityClientInfo::WriteMatchStatBuffer
  ====================*/
void	CEntityClientInfo::WriteMatchStatBuffer(IBuffer &buffer)
{
	buffer << m_iClientNumber << m_uiPlayTime << m_uiCommPlayTime << m_fInitialExperience << byte(NUM_PLAYER_MATCH_STATS);

	for (int i(0); i < NUM_PLAYER_MATCH_STATS; ++i)
		m_aMatchStatRecords[i].WriteToBuffer(buffer, g_eMatchStatType[i]);
}


/*====================
  CEntityClientInfo::ReadMatchStatBuffer
  ====================*/
void	CEntityClientInfo::ReadMatchStatBuffer(IBuffer &buffer)
{
	m_uiPlayTime = buffer.ReadInt();
	m_uiCommPlayTime = buffer.ReadInt();
	m_fInitialExperience = buffer.ReadFloat();

	byte yNumStats(buffer.ReadByte());

	for (byte y(0); y < yNumStats; ++y)
		m_aMatchStatRecords[y].ReadFromBuffer(buffer, g_eMatchStatType[y]);
}


/*====================
  CEntityClientInfo::MatchStatEvent
  ====================*/
void	CEntityClientInfo::MatchStatEvent(EPlayerMatchStat eStat, float fValue, int iTargetClientID, ushort unInflictorType, ushort unTargetType, uint uiTime)
{
	m_aMatchStatRecords[eStat].AddEvent(fValue, iTargetClientID, unTargetType, unInflictorType, uiTime);
}

void	CEntityClientInfo::MatchStatEvent(EPlayerMatchStat eStat, int iValue, int iTargetClientID, ushort unInflictorType, ushort unTargetType, uint uiTime)
{
	m_aMatchStatRecords[eStat].AddEvent(iValue, iTargetClientID, unTargetType, unInflictorType, uiTime);
}


/*====================
  CEntityClientInfo::GameStart
  ====================*/
void	CEntityClientInfo::GameStart()
{
	m_uiPlayTime = 0;
	m_uiCommPlayTime = 0;

	m_uiDiscTime = 0;
	m_uiCommDiscTime = 0;
}


/*====================
  CEntityClientInfo::GetRemainingLoadoutTime
  ====================*/
uint	CEntityClientInfo::GetRemainingLoadoutTime() const
{
	if (m_uiLoadoutTime <= Game.GetGameTime())
		return 0;
	return m_uiLoadoutTime - Game.GetGameTime();
}


/*====================
  CEntityClientInfo::SetLoadoutTime
  ====================*/
void	CEntityClientInfo::SetLoadoutTime(uint uiDuration)
{
	m_uiLoadoutTime = Game.GetGameTime() + uiDuration;
}


/*====================
  CEntityClientInfo::SetTeam
  ====================*/
void	CEntityClientInfo::SetTeam(int iTeam)
{
	m_iTeam = iTeam;
	
	if (iTeam > 0)
		m_iLastTeam = iTeam;

	IPlayerEntity *pPlayer(GetPlayerEntity());

	if (pPlayer != NULL)
		pPlayer->SetTeam(iTeam);
}

/*====================
  CEntityClientInfo::SetSquad
  ====================*/
void	CEntityClientInfo::SetSquad(byte ySquad)
{
	m_ySquad = ySquad;

	IPlayerEntity *pPlayer(GetPlayerEntity());

	if (pPlayer != NULL)
		pPlayer->SetSquad(ySquad);
}
