// (C)2008 S2 Games
// i_heroentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_heroentity.h"

#include "c_player.h"
#include "c_teaminfo.h"
#include "i_entityability.h"
#include "i_behavior.h"
#include "c_entitycreepspawner.h"

#include "../k2/c_voicemanager.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_sceneentity.h"
#include "../k2/i_emitter.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
uint		IHeroEntity::s_uiBaseType(ENTITY_BASE_TYPE_HERO);
ResHandle	IHeroEntity::s_hMinimapIcon0(INVALID_RESOURCE);
ResHandle	IHeroEntity::s_hMinimapIcon1(INVALID_RESOURCE);
ResHandle	IHeroEntity::s_hMinimapTalkingIcon(INVALID_RESOURCE);
ResHandle	IHeroEntity::s_hLevelupEffect(INVALID_RESOURCE);
ResHandle	IHeroEntity::s_hHeroIndicator(INVALID_RESOURCE);

CVAR_FLOATF(	hero_hpPerStr,					19.0f,									CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	hero_hpRegenPerStr,				0.05f,									CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	hero_mpPerInt,					13.0f,									CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	hero_mpRegenPerInt,				0.04f,									CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	hero_armorPerAgi,				0.14f,									CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_FLOATF(	hero_attackSpeedPerAgi,			0.01f,									CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF(		hero_respawnTimePerLevel,		4000,									CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldLossPerLevel,			30,										CVAR_GAMECONFIG);
CVAR_UINTF(		hero_buyBackCost,				100,									CVAR_GAMECONFIG);
CVAR_UINTF(		hero_buyBackCostPerLevel,		50,										CVAR_GAMECONFIG);

CVAR_UINTF(		hero_goldBounty,				200,									CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldBountyPerLevel,		5,										CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldBountyPerStreak,		50,										CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldBountyMinStreak,		3,										CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldBountyMaxStreak,		10,										CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldBountyFirstBlood,		200,									CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldBountyRadiusBase,		30,										CVAR_GAMECONFIG);
CVAR_UINTF(		hero_goldBountyRadiusPerLevel,	5,										CVAR_GAMECONFIG);
CVAR_FLOATF(	hero_expUnsharedBountyPerLevel,	12.0f,									CVAR_GAMECONFIG);
CVAR_UINTF(		hero_blockRepathTime,			100,									CVAR_GAMECONFIG);
CVAR_UINTF(		hero_blockRepathTimeExtra,		250,									CVAR_GAMECONFIG);

CVAR_FLOATF(	g_heroMapIconSize,				0.075f,									CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF(	g_heroMapIcon0,					"/shared/icons/minimap_hero0.tga",		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF(	g_heroMapIcon1,					"/shared/icons/minimap_hero1.tga",		CVAR_TRANSMIT | CVAR_GAMECONFIG);
CVAR_STRINGF(	g_heroTalkingMapIcon,			"/shared/icons/minimap_talking.tga",	CVAR_TRANSMIT | CVAR_GAMECONFIG);

CVAR_UINTF(		g_assistTimeThreshold,			20000,									CVAR_GAMECONFIG);
CVAR_FLOATF(	g_assistDamageThreshold,		0.2f,									CVAR_GAMECONFIG);

CVAR_STRINGF(	g_heroIndicatorPath,			"",										CVAR_TRANSMIT | CVAR_GAMECONFIG); 

CVAR_UINTF(		g_heroAnnounceAttackTime,		30000,									CVAR_GAMECONFIG); 

DEFINE_ENTITY_DESC(IHeroEntity, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(g_heapTypeVector,   TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(IUnitEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());

	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_fExperience"), TYPE_FLOOR16, 15, 0));
	s_cDesc.pFieldTypes->push_back(SDataField(_T("m_uiRespawnTime"), TYPE_INT, 32, 0));
}
//=============================================================================

/*====================
  IHeroEntity::~IHeroEntity
  ====================*/
IHeroEntity::~IHeroEntity()
{
}


/*====================
  IHeroEntity::IHeroEntity
  ====================*/
IHeroEntity::IHeroEntity() :
m_fExperience(0.0f),
m_uiRespawnTime(INVALID_TIME),
m_uiDeathTimeStamp(INVALID_TIME),
m_fRespawnTimeMultiplier(1.0f),
m_fGoldLossMultiplier(1.0f),
m_iRespawnTimeBonus(0),
m_iGoldLossBonus(0),
m_fRespawnHealthMultiplier(1.0f),
m_fRespawnManaMultiplier(0.6667f),
m_bRespawnPositionSet(false),
m_v3RespawnPosition(V3_ZERO),
m_uiLastAttackAnnounce(INVALID_TIME),
m_uiAIControllerUID(INVALID_INDEX),
m_v2Waypoint(FAR_AWAY, FAR_AWAY),
m_uiFinalExpEarnedTime(0)
{
}


/*====================
  IHeroEntity::Baseline
  ====================*/
void	IHeroEntity::Baseline()
{
	IUnitEntity::Baseline();

	m_fExperience = 0.0f;
	m_uiRespawnTime = INVALID_TIME;
	m_uiFinalExpEarnedTime = 0;
}


/*====================
  IHeroEntity::GetSnapshot
  ====================*/
void	IHeroEntity::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	// Base entity info
	IUnitEntity::GetSnapshot(snapshot, uiFlags);

	snapshot.WriteFloor16(m_fExperience);
	snapshot.WriteField(m_uiRespawnTime);
}


/*====================
  IHeroEntity::ReadSnapshot
  ====================*/
bool	IHeroEntity::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		// Base entity info
		if (!IUnitEntity::ReadSnapshot(snapshot, 1))
			return false;

		snapshot.ReadFloat16(m_fExperience);
		snapshot.ReadField(m_uiRespawnTime);
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IHeroEntity::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  IHeroEntity::Copy
  ====================*/
void	IHeroEntity::Copy(const IGameEntity &B)
{
	IUnitEntity::Copy(B);

	const IHeroEntity *pB(B.GetAsHero());

	if (pB == NULL)	
		return;

	const IHeroEntity &C(*pB);
	m_fExperience = C.m_fExperience;
	m_uiRespawnTime = C.m_uiRespawnTime;
}


/*====================
  IHeroEntity::ClientPrecache
  ====================*/
void	IHeroEntity::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)
{
	IUnitEntity::ClientPrecache(pConfig, eScheme);

	s_hMinimapIcon0 = Game.RegisterIcon(g_heroMapIcon0);
	s_hMinimapIcon1 = Game.RegisterIcon(g_heroMapIcon1);
	s_hMinimapTalkingIcon = Game.RegisterIcon(g_heroTalkingMapIcon);
	s_hLevelupEffect = Game.RegisterEffect(g_heroLevelupEffectPath);
	s_hHeroIndicator = g_ResourceManager.Register(g_heroIndicatorPath, RES_MATERIAL);

	EntityRegistry.ClientPrecache(EntityRegistry.LookupID(_T("State_Taunted")), eScheme);
	EntityRegistry.ClientPrecache(EntityRegistry.LookupID(_T("State_Taunting")), eScheme);
}


/*====================
  IHeroEntity::ServerPrecache
  ====================*/
void	IHeroEntity::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme)
{
	IUnitEntity::ServerPrecache(pConfig, eScheme);

	s_hLevelupEffect = Game.RegisterEffect(g_heroLevelupEffectPath);

	EntityRegistry.ServerPrecache(EntityRegistry.LookupID(_T("State_Taunted")), eScheme);
	EntityRegistry.ServerPrecache(EntityRegistry.LookupID(_T("State_Taunting")), eScheme);
}


/*====================
  IHeroEntity::ServerFrameSetup
  ====================*/
bool	IHeroEntity::ServerFrameSetup()
{
	return IUnitEntity::ServerFrameSetup();
}


/*====================
  IHeroEntity::ServerFrameThink
  ====================*/
bool	IHeroEntity::ServerFrameThink()
{
	if (m_uiOwnerEntityIndex == INVALID_INDEX && m_uiAIControllerUID != INVALID_INDEX)
	{
		CEntityCreepSpawner *pController(Game.GetEntityFromUniqueIDAs<CEntityCreepSpawner>(m_uiAIControllerUID));
		if (pController != NULL)
		{
			m_v2Waypoint = pController->GetLane().GetNextWaypoint(m_v3Position.xy(), m_v2Waypoint);

			// Issue default creep behavior (Attack Move)
			if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
			{
				m_cBrain.AddCommand(UNITCMD_ATTACKMOVE, false, m_v2Waypoint, INVALID_INDEX, uint(-1), true);
			}
			else
			{
				IBehavior *pBehavior(m_cBrain.GetCurrentBehavior());
				if (pBehavior != NULL && pBehavior->GetGoal() != m_v2Waypoint)
				{
					m_cBrain.GetCurrentBehavior()->SetGoal(m_v2Waypoint);
					m_cBrain.GetCurrentBehavior()->ForceUpdate();
				}
			}
		}
	}
	else
	{
		// Issue default hero behavior (Guard Position)
		if (m_cBrain.IsEmpty() && m_cBrain.GetCommandsPending() == 0)
			m_cBrain.AddCommand(GetDefaultBehavior(), false, m_v3Position.xy(), INVALID_INDEX, uint(-1), true);
	}

	return IUnitEntity::ServerFrameThink();
}


/*====================
  IHeroEntity::Respawn
  ====================*/
void	IHeroEntity::Respawn()
{
	Game.LogHero(GAME_LOG_HERO_RESPAWN, this);

	m_uiDeathTimeStamp = INVALID_TIME;

	m_uiRespawnTime = INVALID_TIME;
	m_uiLastHeroAttackTime = INVALID_TIME;

	if (m_bRespawnPositionSet)
		SetPosition(m_v3RespawnPosition);
	else
	{
		CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
		if (pTeam != NULL)
			pTeam->SetHeroSpawnPosition(this);
	}

	SetStatus(ENTITY_STATUS_ACTIVE);
	SetHealth(GetMaxHealth() * m_fRespawnHealthMultiplier);
	SetMana(GetMaxMana() * m_fRespawnManaMultiplier);
	m_fInflictDamageMultiplier = 1.0f;
	m_fReceiveDamageMultiplier = 1.0f;
	m_uiCorpseTime = INVALID_TIME;
	m_uiCorpseFadeTime = INVALID_TIME;
	m_uiDeathTime = INVALID_TIME;
	m_uiDeathFlags = 0;
	m_fExperienceBountyMultiplier = 1.0f;
	m_fGoldBountyMultiplier = 1.0f;
	m_fGoldLossMultiplier = 1.0f;
	m_fRespawnTimeMultiplier = 1.0f;
	m_iGoldLossBonus = 0;
	m_iRespawnTimeBonus = 0;
	m_fRespawnHealthMultiplier = 1.0f;
	m_fRespawnHealthMultiplier = 0.6667f;
	m_bRespawnPositionSet = false;
	m_v3RespawnPosition = V3_ZERO;
	m_v2Waypoint = CVec2f(FAR_AWAY, FAR_AWAY);
	IncDisjointSequence();
	IncArmingSequence();
	IncNoInterpolateSequence();
	IncResetSequence();

	StartAnimation(GetIdleAnim(), -1);
	
	if (GetAnimIndex(_T("respawn_1")) != -1)
		SetAnim(0, _T("respawn_1"));
	else
		SetAnim(0, GetIdleAnim(), 1.0f, 0);
	
	m_cBrain.Init();
	Link();

	if (GetRespawnEffect() != INVALID_RESOURCE)
	{
		CGameEvent ev;
		ev.SetSourceEntity(GetIndex());
		ev.SetEffect(GetRespawnEffect());
		Game.AddEvent(ev);
	}

	// Respawn action
	CHeroDefinition *pDefinition(GetDefinition<CHeroDefinition>());
	if (pDefinition != NULL)
		pDefinition->ExecuteActionScript(ACTION_SCRIPT_RESPAWN, this, GetOwner(), this, this, GetPosition(), NULL, GetLevel());

	Game.UnitRespawned(m_uiIndex);
	Game.UpdateUnitVisibility(this);
}


/*====================
  IHeroEntity::ServerFrameMovement
  ====================*/
bool	IHeroEntity::ServerFrameMovement()
{
	IUnitEntity::ServerFrameMovement();

	if (HasUnitFlags(UNIT_FLAG_TERMINATED))
		return true;

	if (GetStatus() != ENTITY_STATUS_ACTIVE)
	{
		CPlayer *pOwner(GetOwnerPlayer());
		if (pOwner != NULL)
			pOwner->SetLastInteractionTime(Game.GetGameTime());
	}

	// Set respawn time after death completes
	if ((GetStatus() == ENTITY_STATUS_CORPSE || GetStatus() == ENTITY_STATUS_DORMANT) && m_uiRespawnTime == INVALID_TIME && !HasUnitFlags(UNIT_FLAG_ILLUSION))
	{
		if (Game.HasGameOptions(GAME_OPTION_NO_RESPAWN_TIMER))
			m_uiRespawnTime = Game.GetGameTime();
		else
			m_uiRespawnTime = Game.GetGameTime() + MAX(INT_ROUND(GetRespawnDuration() * m_fRespawnTimeMultiplier) - m_iRespawnTimeBonus, 0);
	}

	// Check for respawning
	if (GetStatus() != ENTITY_STATUS_ACTIVE && (m_uiRespawnTime != INVALID_TIME && Game.GetGameTime() >= m_uiRespawnTime))
	{
		m_uiRespawnTime = INVALID_TIME;
		Respawn();
		return true;
	}

	return true;
}


/*====================
  IHeroEntity::ServerFrameCleanup
  ====================*/
bool	IHeroEntity::ServerFrameCleanup()
{
	IUnitEntity::ServerFrameCleanup();

	return true;
}


/*====================
  IHeroEntity::Spawn
  ====================*/
void	IHeroEntity::Spawn()
{	
	m_uiRespawnTime = INVALID_TIME;

	Game.Precache(m_unType, PRECACHE_ALL);

	IUnitEntity::Spawn();
}


/*====================
  IHeroEntity::Die
  ====================*/
void	IHeroEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return;
	
	CPlayer *pPlayerOwner(GetOwnerPlayer());

	uint uiDelay(1000);

	if (pAttacker != NULL && pPlayerOwner != NULL && !HasUnitFlags(UNIT_FLAG_ILLUSION))
	{
		ushort uType(EntityRegistry.LookupID(_T("State_Taunted")));

		bool bHumilation(false);
		bool bSmackdown(false);
		for (int i(INVENTORY_START_STATES); i < INVENTORY_END_STATES && (!bHumilation || !bSmackdown); i++)
		{
			if (pAttacker->GetState(i) != NULL && pAttacker->GetState(i)->GetType() == uType && pAttacker->GetState(i)->GetInflictor() == this)
				bHumilation = true;

			if (GetState(i) != NULL && GetState(i)->GetType() == uType && GetState(i)->GetInflictor() == pAttacker)
				bSmackdown = true;
		}

		// Taunt messages
		if (bSmackdown)
		{
			CBufferFixed<1> buffer;
			buffer << GAME_CMD_SMACKDOWN_MESSAGE;
			Game.BroadcastGameData(buffer, true, -1, uiDelay);

			uiDelay += 1000;
		}
		else if (bHumilation)
		{
			CBufferFixed<1> buffer;
			buffer << GAME_CMD_HUMILIATION_MESSAGE;
			Game.BroadcastGameData(buffer, true, -1, uiDelay);

			uiDelay += 1000;
		}

		if (bHumilation || bSmackdown)
		{
			CBufferFixed<5> buffer;
			buffer << GAME_CMD_TAUNT_KILLED_SOUND << pAttacker->GetIndex();

			Game.SendGameData(pPlayerOwner->GetClientNumber(), buffer, true, uiDelay);
			buffer.Rewind();
			Game.SendGameData(pAttacker->GetOwnerClientNumber(), buffer, true, uiDelay);
		}

#if 0
		if (bThisTaunted)
		{
			// Remove all other applied taunt states
			const UnitList &units(Game.GetUnitList());
			for (UnitList_cit it(units.begin()); it != units.end(); it++)
			{
				if (!(*it)->IsHero())
					continue;

				for (int i(INVENTORY_START_STATES); i < INVENTORY_END_STATES && (!bAttackerTaunted || !bThisTaunted); i++)
				{
					if ((*it)->GetState(i) != NULL && (*it)->GetState(i)->GetType() == uType && (*it)->GetState(i)->GetInflictor() == pAttacker)
						(*it)->RemoveState(i);		
				}
			}
		}
#endif
	}

	IUnitEntity::Die(pAttacker, unKillingObjectID);

	if (HasUnitFlags(UNIT_FLAG_ILLUSION))
		return;

	if (GetProtectedDeath())
		return;

	if (pPlayerOwner != NULL)
	{
		ushort unGoldLoss(MAX(INT_ROUND(GetLevel() * hero_goldLossPerLevel * m_fGoldLossMultiplier) - m_iGoldLossBonus, 0));

		pPlayerOwner->TakeGold(unGoldLoss);
		pPlayerOwner->AdjustStat(PLAYER_STAT_DEATHS, 1);
		Game.LogGold(GAME_LOG_GOLD_LOST, pPlayerOwner, pAttacker, unGoldLoss);

		if (pAttacker != this)
		{
			if (pPlayerOwner->GetKillStreak() > 2)
				Game.LogAward(GAME_LOG_AWARD_KILL_STREAK_BREAK, pAttacker, this);
			pPlayerOwner->ResetKillStreak();
			
			pPlayerOwner->AddDeathStreak();
			if (pPlayerOwner->GetDeathStreak() > 2)
			{
#if 0
				CBufferFixed<1> buffer;
				buffer << GAME_CMD_FEEDER_MESSAGE;
				Game.BroadcastGameData(buffer, true, -1, uiDelay);

				uiDelay += 1000;
#endif
			}
		}
	}

	// Check for a team wipe
	CTeamInfo *pTeam(Game.GetTeam(GetTeam()));
	if (pTeam != NULL && pTeam->GetNumActiveClients() > 1 && pTeam->AreAllDead() && !pTeam->GetAllKilled())
	{
		pTeam->SetAllKilled(true);

		CBufferFixed<5> buffer;
		buffer << GAME_CMD_TEAM_WIPE_MESSAGE << pTeam->GetTeamID();
		Game.BroadcastGameData(buffer, true, -1, 2000 + uiDelay);
	}

	if (pAttacker == this)
		Game.LogHero(GAME_LOG_HERO_SUICIDE, this, EntityRegistry.LookupName(unKillingObjectID));

	m_uiDeathTimeStamp = Game.GetGameTime();
}


/*====================
  IHeroEntity::KillReward
  ====================*/
void	IHeroEntity::KillReward(IUnitEntity *pKiller, CPlayer *pPlayerKiller)
{
	ivector vAssistPlayers;
	AssistsReward(pKiller, pPlayerKiller, vAssistPlayers);

	if (pKiller != this)
		Game.LogKill(this, pKiller, NULL, &vAssistPlayers);

	// Kill message
	ushort unGoldBounty(GetGoldBounty());
	CPlayer *pPlayerOwner(GetOwnerPlayer());

	if (pPlayerKiller != NULL)
	{
		if (pPlayerOwner != NULL)
		{
			if (pKiller == this)
			{
				CBufferFixed<5> buffer;
				buffer << GAME_CMD_SUICIDE_KILL_MESSAGE << pPlayerOwner->GetClientNumber();
				Game.BroadcastGameData(buffer, true);
			}
			else if (pPlayerKiller->GetTeam() == GetTeam())
			{
				// Deny
				CBufferFixed<9> buffer;
				buffer << GAME_CMD_HERO_DENY_MESSAGE << pPlayerKiller->GetClientNumber() << pPlayerOwner->GetClientNumber();
				Game.BroadcastGameData(buffer, true);
				
			}
			else if (pPlayerOwner->GetKillStreak() > 2)
			{
				// Streak end message
				CBufferFixed<12> buffer;
				buffer << GAME_CMD_END_STREAK_MESSAGE << pPlayerKiller->GetClientNumber() << pPlayerOwner->GetClientNumber() << pPlayerOwner->GetKillStreak() << unGoldBounty;
				Game.BroadcastGameData(buffer, true);
			}
			else
			{
				CBufferFixed<28> buffer;
				buffer << GAME_CMD_KILL_MESSAGE << pPlayerKiller->GetClientNumber() << pPlayerOwner->GetClientNumber() << unGoldBounty << byte(vAssistPlayers.size());
				for (ivector_it itAssist(vAssistPlayers.begin()), itEnd(vAssistPlayers.end()); itAssist != itEnd; ++itAssist)
					buffer << *itAssist;

				Game.BroadcastGameData(buffer, true);
			}
		}

		FirstBloodReward(pKiller, pPlayerKiller);

		if (pPlayerOwner == NULL || (pPlayerOwner->GetTeam() != pPlayerKiller->GetTeam()))
		{
			pPlayerKiller->AdjustStat(PLAYER_STAT_HERO_KILLS, 1);
			pPlayerKiller->RewardKill();
		}
	}
	else if (pKiller != NULL)
	{
		if (pPlayerOwner != NULL)
		{
			if (pKiller->GetTeam() == 1 || pKiller->GetTeam() == 2)
			{
				CBufferFixed<11> buffer;
				buffer << GAME_CMD_TEAM_KILL_MESSAGE << pKiller->GetTeam() << pPlayerOwner->GetClientNumber() << unGoldBounty;
				Game.BroadcastGameData(buffer, true);
			}
			else if (pKiller->IsTargetType(_T("Kongor"), this))
			{
				CBufferFixed<5> buffer;
				buffer << GAME_CMD_KONGOR_KILL_MESSAGE << pPlayerOwner->GetClientNumber();
				Game.BroadcastGameData(buffer, true);
			}
			else if (pKiller->IsNeutral())
			{
				CBufferFixed<5> buffer;
				buffer << GAME_CMD_NEUTRAL_KILL_MESSAGE << pPlayerOwner->GetClientNumber();
				Game.BroadcastGameData(buffer, true);
			}
			else
			{
				CBufferFixed<5> buffer;
				buffer << GAME_CMD_UNKNOWN_KILL_MESSAGE << pPlayerOwner->GetClientNumber();
				Game.BroadcastGameData(buffer, true);
			}
		}
	}
	else if (pPlayerOwner != NULL)
	{
		CBufferFixed<5> buffer;
		buffer << GAME_CMD_UNKNOWN_KILL_MESSAGE << pPlayerOwner->GetClientNumber();
		Game.BroadcastGameData(buffer, true);
	}

	IUnitEntity::KillReward(pKiller, pPlayerKiller);
}


/*====================
  IHeroEntity::AssistsReward
  ====================*/
void	IHeroEntity::AssistsReward(IUnitEntity *&pKiller, CPlayer *&pPlayerKiller, ivector &vAssistPlayers)
{
	if (pKiller != NULL && pKiller->GetTeam() == GetTeam())
		return;

	float fTotalRecentDamage(0.0f);

	// Check damage logs for potential assists
	uint uiAssisterUID(INVALID_INDEX);
	map<CPlayer*, float> mapAssistPlayers;
	for (LogDamageVector_it it(m_vLogDamageTrackers.begin()); it != m_vLogDamageTrackers.end(); ++it)
	{
		SDamageTrackerLog &logDamage(*it);
		CPlayer *pPlayer(Game.GetPlayer(logDamage.iPlayerOwner));
		if (pPlayer == NULL || pPlayer->GetTeam() == GetTeam() || pPlayer == pPlayerKiller)
			continue;

		if (Game.GetGameTime() - logDamage.uiTimeStamp > g_assistTimeThreshold)
			continue;

		fTotalRecentDamage += logDamage.fDamage;

		mapAssistPlayers[pPlayer] += logDamage.fDamage;
		uiAssisterUID = logDamage.uiAttackerUID;
	}

	// Check debuff states for potential assists
	for (int iSlot(INVENTORY_START_STATES); iSlot <= INVENTORY_END_STATES; ++iSlot)
	{
		IEntityState *pState(GetState(iSlot));
		if (pState == NULL)
			continue;

		IUnitEntity *pInflictor(pState->GetInflictor());
		if (pInflictor == NULL)
			continue;

		CPlayer *pPlayer(pInflictor->GetOwnerPlayer());
		if (pPlayer == NULL || pPlayer->GetTeam() == GetTeam() || pPlayer == pPlayerKiller)
			continue;

		if (Game.IsAssist(pState->GetEffectType()))
		{
			mapAssistPlayers[pPlayer] = -1.0f;
			uiAssisterUID = pInflictor->GetUniqueID();
		}
	}

	// Build final assist list
	vector<CPlayer*> vAssists;
	for (map<CPlayer*, float>::iterator it(mapAssistPlayers.begin()); it != mapAssistPlayers.end(); ++it)
	{
		if (it->second < 0.0f || it->second >= g_assistDamageThreshold * fTotalRecentDamage)
			vAssists.push_back(it->first);
	}

	// A kill with no valid killer and a single assist goes to the assister
	if (pPlayerKiller == NULL && vAssists.size() == 1)
	{
		pKiller = Game.GetUnitFromUniqueID(uiAssisterUID);
		pPlayerKiller = vAssists.back();
		vAssists.clear();
	}

	// Award assists
	for (vector<CPlayer*>::iterator it(vAssists.begin()); it != vAssists.end(); ++it)
	{
		(*it)->AdjustStat(PLAYER_STAT_ASSISTS, 1);
		vAssistPlayers.push_back((*it)->GetClientNumber());
		Game.LogAssist(this, NULL, NULL, *it);
	}
}


/*====================
  IHeroEntity::FirstBloodReward
  ====================*/
void	IHeroEntity::FirstBloodReward(IUnitEntity *pKiller, CPlayer *pPlayerKiller)
{
	if (Game.HasFlags(GAME_FLAG_FIRST_BLOOD))
		return;
	
	if (pKiller == NULL || pPlayerKiller == NULL)
		return;

	if (pPlayerKiller->GetTeam() == GetTeam())
		return;

	IHeroEntity *pKillerHero(pPlayerKiller->GetHero());
	
	pPlayerKiller->GiveGold(hero_goldBountyFirstBlood, pKillerHero);

	CBufferFixed<7> buffer;
	buffer << GAME_CMD_FIRST_BLOOD_MESSAGE << pPlayerKiller->GetClientNumber() << ushort(hero_goldBountyFirstBlood);
	Game.BroadcastGameData(buffer, true);

	Game.SetFlags(GAME_FLAG_FIRST_BLOOD);
	Game.LogAward(GAME_LOG_AWARD_FIRST_BLOOD, pKiller, this);
}


/*====================
  IHeroEntity::GetMapIconColor
  ====================*/
CVec4f	IHeroEntity::GetMapIconColor(CPlayer *pLocalPlayer) const
{
	CPlayer *pPlayer(Game.GetPlayerFromClientNumber(GetOwnerClientNumber()));
	if (pPlayer == NULL)
		return IUnitEntity::GetMapIconColor(pLocalPlayer);

	return pPlayer->GetColor();
}


/*====================
  IHeroEntity::GetTeamColor
  ====================*/
CVec4f	IHeroEntity::GetTeamColor(CPlayer *pLocalPlayer) const
{
	if (Game.UsePlayerColors())
	{
		CPlayer *pPlayer(Game.GetPlayerFromClientNumber(GetOwnerClientNumber()));
		if (pPlayer == NULL)
			return IUnitEntity::GetTeamColor(pLocalPlayer);

		return pPlayer->GetColor();
	}
	else
		return IUnitEntity::GetTeamColor(pLocalPlayer);
}


/*====================
  IHeroEntity::GetBaseAttackSpeed
  ====================*/
float	IHeroEntity::GetBaseAttackSpeed() const
{
	return IUnitEntity::GetBaseAttackSpeed() + GetAgility() * hero_attackSpeedPerAgi;
}


/*====================
  IHeroEntity::GetAttributeDamageAdjustment
  ====================*/
float	IHeroEntity::GetAttributeDamageAdjustment() const
{
	float fDamage(0.0f);
	switch (GetPrimaryAttribute())
	{
	case ATTRIBUTE_STRENGTH: return fDamage + GetStrength();
	case ATTRIBUTE_AGILITY: return fDamage + GetAgility();
	case ATTRIBUTE_INTELLIGENCE: return fDamage + GetIntelligence();
	}

	return fDamage;
}


/*====================
  IHeroEntity::GetGoldBounty
  ====================*/
ushort	IHeroEntity::GetGoldBounty() const
{
	ushort unGold(hero_goldBounty + hero_goldBountyPerLevel * GetLevel());
	CPlayer *pPlayer(Game.GetPlayer(GetOwnerClientNumber()));
	if (pPlayer != NULL && pPlayer->GetKillStreak() >= hero_goldBountyMinStreak)
		unGold += MIN(hero_goldBountyMaxStreak.GetValue(), (pPlayer->GetKillStreak() - hero_goldBountyMinStreak + 1)) * hero_goldBountyPerStreak;
	
	return unGold;
}


/*====================
  IHeroEntity::GiveExperience
  ====================*/
float	IHeroEntity::GiveExperience(float fExperience, IUnitEntity *pSource)
{
	CGameInfo *pGameInfo(Game.GetGameInfo());
	if (pGameInfo != NULL)
		fExperience *= pGameInfo->GetExperienceMultiplier();

	fExperience = MIN(fExperience, GetExperienceForLevel(GetMaxLevel()) - m_fExperience);

	if (fExperience <= 0.0f)
		return 0.0f;

	CPlayer *pOwner(GetOwnerPlayer());
	if (pOwner != NULL)
	{
		// store the time at which we last earned experience before the game ended.
		if (!Game.IsGameOver())
		{
			m_uiFinalExpEarnedTime = pOwner->GetPlayTime();
		}
	}

	Game.LogExperience(GAME_LOG_EXP_EARNED, this, pSource, fExperience);

	// Send an experience event to all players can that can control this unit
	CBufferFixed<3> buffer;
	buffer << GAME_CMD_EXPERIENCE_EVENT << ushort(m_uiIndex);

	const PlayerMap &mapPlayers(Game.GetPlayerMap());
	for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
	{
		if (CanReceiveOrdersFrom(itPlayer->first))
			Game.SendGameData(itPlayer->first, buffer, false);
	}

	uint uiPrevLevel(GetLevel());

	m_fExperience += fExperience;

	if (uiPrevLevel != GetLevel())
	{
		Game.LogHero(GAME_LOG_HERO_LEVEL, this);

		if (s_hLevelupEffect != INVALID_RESOURCE)
		{
			CGameEvent ev;
			ev.SetSourceEntity(GetIndex());
			ev.SetEffect(s_hLevelupEffect);
			Game.AddEvent(ev);
		}

		ExecuteActionScript(ACTION_SCRIPT_LEVELUP, this, GetPosition());

		// Send a levelup event to all players can that can control this unit
		CBufferFixed<3> buffer;
		buffer << GAME_CMD_LEVELUP_EVENT << ushort(m_uiIndex);

		const PlayerMap &mapPlayers(Game.GetPlayerMap());
		for (PlayerMap_cit itPlayer(mapPlayers.begin()); itPlayer != mapPlayers.end(); ++itPlayer)
		{
			if (CanReceiveOrdersFrom(itPlayer->first))
				Game.SendGameData(itPlayer->first, buffer, true);
		}
	}

	Game.SendPopup(POPUP_EXPERIENCE, pSource == NULL ? this : pSource, this, ushort(fExperience));
	return fExperience;
}


/*====================
  IHeroEntity::ResetExperience
  ====================*/
void	IHeroEntity::ResetExperience()
{
	m_uiFinalExpEarnedTime = 0;
	m_fExperience = 0.0f;
	for (int i(INVENTORY_START_ABILITIES); i <= INVENTORY_END_ABILITIES; ++i)
	{
		IEntityAbility *pAbility(GetAbility(i));
		if (pAbility != NULL)
			pAbility->SetLevel(0);
	}
}


/*====================
  IHeroEntity::GetAvailablePoints
  ====================*/
int		IHeroEntity::GetAvailablePoints() const	
{
	int iTotal(0);
	for (int i(INVENTORY_START_ABILITIES); i <= INVENTORY_END_ABILITIES; ++i)
	{
		IEntityAbility *pAbility(GetAbility(i));
		if (pAbility != NULL && pAbility->GetMaxLevel() > 0)
			iTotal += pAbility->GetLevel() - pAbility->GetBaseLevel();
	}

	return MAX(0u, GetLevel() - iTotal);
}


/*====================
  IHeroEntity::SpawnIllusion
  ====================*/
IUnitEntity*	IHeroEntity::SpawnIllusion(const CVec3f &v3Position, const CVec3f &v3Angles, uint uiLifetime, 
										   float fReceiveDamageMultiplier, float fInflictDamageMultiplier, 
										   ResHandle hSpawnEffect, ResHandle hDeathEffect, 
										   bool bDeathAnim, bool bInheritActions)
{
	IUnitEntity *pUnit(IUnitEntity::SpawnIllusion(v3Position, v3Angles, uiLifetime, fReceiveDamageMultiplier, fInflictDamageMultiplier, hSpawnEffect, hDeathEffect, bDeathAnim, bInheritActions));
	IHeroEntity *pIllusion(pUnit ? pUnit->GetAsHero() : NULL);
	if (pIllusion != NULL)
	{
		pIllusion->SetExperience(GetExperience());
		pIllusion->SetHealth(GetHealth());
		pIllusion->SetMana(GetMana());
	}

	return pUnit;
}


/*====================
  IHeroEntity::Moved

  Update brain to account for a forced movement
  ====================*/
void	IHeroEntity::Moved()
{
	IUnitEntity::Moved();

	IBehavior *pBehavior(m_cBrain.GetCurrentBehavior());
	if (pBehavior != NULL && pBehavior->GetDefault())
		pBehavior->SetGoal(m_v3Position.xy());
}


/*====================
  IHeroEntity::Damage
  ====================*/
void	IHeroEntity::Damage(CDamageEvent &damage)
{
	IUnitEntity::Damage(damage);

#if 0
	bool bDamaged(m_fLethalDamageAccumulator > 0.0f || m_fNonLethalDamageAccumulator > 0.0f);

	if (bDamaged)
	{
		if (m_uiLastAttackAnnounce == INVALID_TIME || m_uiLastAttackAnnounce + g_heroAnnounceAttackTime < Game.GetGameTime())
		{
			CBufferFixed<5> buffer;
			buffer << GAME_CMD_HERO_UNDER_ATTACK_MESSAGE << GetOwnerClientNumber();
			Game.BroadcastGameDataToTeam(GetTeam(), buffer, true);

			m_uiLastAttackAnnounce = Game.GetGameTime();
		}
	}
#endif
}


/*====================
  IHeroEntity::Interpolate
  ====================*/
void	IHeroEntity::Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState)
{
	IUnitEntity::Interpolate(fLerp, pPrevState, pNextState);

	if (pPrevState->GetNoInterpolateSequence() != pNextState->GetNoInterpolateSequence())
		return;

	IHeroEntity *pPrev(static_cast<IHeroEntity *>(pPrevState));
	IHeroEntity *pNext(static_cast<IHeroEntity *>(pNextState));

	if (pPrev->GetLevel() == pNext->GetLevel())
		m_fExperience = LERP(fLerp, pPrev->m_fExperience, pNext->m_fExperience);
	else
		m_fExperience = pNext->m_fExperience;
}


/*====================
  IHeroEntity::DrawOnMap
  ====================*/
void	IHeroEntity::DrawOnMap(CUITrigger &minimap, CPlayer *pLocalPlayer) const
{
	CPlayer *pPlayer(Game.GetPlayerFromClientNumber(GetOwnerClientNumber()));
	
	if (!IsVisibleOnMap(pLocalPlayer))
		return;

	bool bTalking(pPlayer && pLocalPlayer && pLocalPlayer->GetTeam() == GetTeam() && VoiceManager.IsTalking(pPlayer->GetClientNumber()) && s_hMinimapTalkingIcon != INVALID_RESOURCE);
	{
		CBufferFixed<36> buffer;
		
		buffer << GetPosition().x / Game.GetWorldWidth();
		buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
		
		buffer << GetMapIconSize(pLocalPlayer) << GetMapIconSize(pLocalPlayer);
	
		CVec4f v4Color(pPlayer == NULL ? IUnitEntity::GetMapIconColor(pLocalPlayer) : pPlayer->GetColor());
		buffer << v4Color[R];
		buffer << v4Color[G];
		buffer << v4Color[B];
		buffer << v4Color[A];

		if (bTalking)
			buffer << s_hMinimapTalkingIcon;
		else
			buffer << s_hMinimapIcon0;

		minimap.Execute(_T("icon"), buffer);
	}

	{
		CBufferFixed<36> buffer;
		
		buffer << GetPosition().x / Game.GetWorldWidth();
		buffer << 1.0f - (GetPosition().y / Game.GetWorldHeight());
		
		buffer << GetMapIconSize(pLocalPlayer) << GetMapIconSize(pLocalPlayer);
		
		CVec4f v4Color(IUnitEntity::GetMapIconColor(pLocalPlayer));
		buffer << v4Color[R];
		buffer << v4Color[G];
		buffer << v4Color[B];
		buffer << v4Color[A];

		if (!bTalking)
			buffer << s_hMinimapIcon1;

		minimap.Execute(_T("icon"), buffer);
	}
}


/*====================
  IHeroEntity::AddIndicator
  ====================*/
void	IHeroEntity::AddIndicator()
{
	CPlayer *pLocalPlayer(Game.GetLocalPlayer());
	if (pLocalPlayer == NULL)
		return;

	float fSize(128.0f);
	if (fSize <= 0.0f)
		return;

	if (GetStatus() != ENTITY_STATUS_ACTIVE && GetStatus() != ENTITY_STATUS_DEAD)
		return;

	CSceneEntity sceneEntity;
	sceneEntity.Clear();

	sceneEntity.width = fSize;
	sceneEntity.height = fSize;
	sceneEntity.scale = 1.0f;
	sceneEntity.SetPosition(m_v3Position);
	sceneEntity.hRes = s_hHeroIndicator;
	sceneEntity.flags = SCENEENT_SOLID_COLOR | SCENEENT_USE_AXIS;

	if (GetFlying())
	{
		sceneEntity.objtype = OBJTYPE_BILLBOARD;
		sceneEntity.hSkin = BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT;
		sceneEntity.width *= 2.0f;
		sceneEntity.height *= 2.0f;
		sceneEntity.angle[PITCH] = -90.0f;
	}
	else
	{
		sceneEntity.objtype = OBJTYPE_GROUNDSPRITE;
	}

	sceneEntity.color = GetTeamColor(pLocalPlayer);

	SceneManager.AddEntity(sceneEntity);
}


/*====================
  IHeroEntity::AddToScene
  ====================*/
bool	IHeroEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
	if (!IUnitEntity::AddToScene(v4Color, iFlags))
		return false;

	if (Game.UseHeroIndicators())
		AddIndicator();
	return true;
}
