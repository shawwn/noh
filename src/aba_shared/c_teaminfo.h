// (C)2006 S2 Games
// c_teaminfo.h
//
//=============================================================================
#ifndef __C_TEAMINFO_H__
#define __C_TEAMINFO_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
#include "c_player.h"
#include "c_gamestats.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint INVALID_TEAM_SLOT(-1);

const byte TEAM_FLAG_ABANDONED	(BIT(0));
const byte TEAM_FLAG_PAUSED		(BIT(1));
const byte TEAM_FLAG_CAN_UNPAUSE(BIT(2));


class CShopInfo;

enum ETeamStat
{
	TEAM_STAT_TOWER_KILLS,
	TEAM_STAT_TOWER_DENIES,

	NUM_TEAM_STATS
};

//=============================================================================

//=============================================================================
// CTeamInfo
//=============================================================================
class CTeamInfo : public IGameEntity
{
	DECLARE_ENTITY_DESC

private:
	DECLARE_ENT_ALLOCATOR3(TeamInfo);

	bool				m_bRosterChanged;
	uint				m_uiRosterChangeSequence;

	uint				m_uiTeamID;
	ushort				m_unNameIndex;
	CVec4f				m_v4Color;
	uint				m_uiTeamSize;
	ivector				m_vClients;
	byte				m_ySlotLocks;

	float				m_fWinChance;

	uint				m_uiBaseBuildingIndex;
	uiset				m_setBuildingUIDs;
	uint				m_uiStartingTowerCount;
	uint				m_uiCurrentTowerCount;

	uint				m_uiCurrentRangedCount;
	uint				m_uiCurrentMeleeCount;

	map<uint, float>	m_mapDamagedBuildings;
	uint				m_uiLastAttackNotifyTime;
	bool				m_bDamagedBuildingSetCleared;

	iset				m_setAlliedTeams;

	byte				m_yKillStreak;
	bool				m_bSentMegaCreepMessage;
	bool				m_bAllKilled;

	uint				m_uiStatsIndex;

	byte				m_yFlags;

	float				m_fBaseHealthPercent;

	byte				m_yRemainingPauses;
	uint				m_uiBanCount;

	uint				m_uiLastBuildingAttackAnnouncement;

	uint				m_uiShopInfoIndex;

	uint				m_aStatTotals[NUM_TEAM_STATS];

	uint				m_uiExtraTime;
	bool				m_bUsingExtraTime;
	uint				m_uiLastIncomeTime;

	void	PassiveIncome();

public:
	~CTeamInfo();
	CTeamInfo();

	SUB_ENTITY_ACCESSOR(CTeamInfo, TeamInfo)

	// Network
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
	
	GAME_SHARED_API void			Initialize();
	GAME_SHARED_API void			MatchRemake();

	bool							IsValid() const							{ return true; }
	bool							IsActiveTeam() const					{ return m_uiTeamID > TEAM_SPECTATOR && m_uiTeamID < TEAM_INVALID; }

	void							SetTeamID(uint uiTeamID)				{ m_uiTeamID = uiTeamID; }
	uint							GetTeamID() const						{ return m_uiTeamID; }

	uint							GetBanCount() const						{ return m_uiBanCount; }
	void							IncrementBanCount()						{ ++m_uiBanCount; }

	CGameStats*						GetStats() const						{ return Game.GetStatsEntity(GetStatsIndex()); }
	uint							GetStatsIndex() const					{ return m_uiStatsIndex; }
	GAME_SHARED_API	void			AssignStats(CGameStats *pStats);

	void							GameStart();

	void							IncrementRosterChangeSequence()			{ ++m_uiRosterChangeSequence; }
	bool							IsRosterChanged() const					{ return m_bRosterChanged; }
	void							AckowledgeRosterChange()				{ m_bRosterChanged = false; }

	void							SetFlags(byte y)						{ m_yFlags |= y; }
	bool							HasFlags(byte y) const					{ return (m_yFlags & y) == y; }
	void							RemoveFlags(byte y)						{ m_yFlags &= ~y; }
	void							ClearFlags()							{ m_yFlags = 0; }

	GAME_SHARED_API void			Abandoned();

	// Clients
	GAME_SHARED_API void			AddClient(int iClientNumber, uint uiSlot = INVALID_TEAM_SLOT);
	GAME_SHARED_API void			RemoveClient(int iClientID);
	GAME_SHARED_API uint			GetNumClients() const;
	const ivector&					GetClientList() const					{ return m_vClients; }
	ivector&						GetClientList()							{ return m_vClients; }
	GAME_SHARED_API tstring			GetClientName(uint uiTeamIndex);
	GAME_SHARED_API int				GetClientIDFromTeamIndex(uint uiTeamIndex) const;
	GAME_SHARED_API uint			GetTeamIndexFromClientID(int iClientID) const;
	CPlayer*						GetPlayer(uint uiTeamIndex) const		{ return Game.GetPlayer(GetClientIDFromTeamIndex(uiTeamIndex)); }
	GAME_SHARED_API CPlayer*		GetCaptain() const;

	void							LockSlot(uint uiSlot)					{ if (uiSlot >= m_uiTeamSize) return; m_ySlotLocks |= BIT(uiSlot); }
	void							UnlockSlot(uint uiSlot)					{ if (uiSlot >= m_uiTeamSize) return; m_ySlotLocks &= ~BIT(uiSlot); }
	void							ToggleSlotLock(uint uiSlot)				{ if (uiSlot >= m_uiTeamSize) return; m_ySlotLocks ^= BIT(uiSlot); }
	bool							IsSlotLocked(uint uiSlot)				{ if (uiSlot >= m_uiTeamSize) return false; return (m_ySlotLocks & BIT(uiSlot)) != 0; }

	GAME_SHARED_API void			UpdateVoiceTargets(int iClientNumber);

	void							SetName(const tstring &sString)			{ NetworkResourceManager.SetString(m_unNameIndex, sString); }
	const tstring&					GetName() const							{ return NetworkResourceManager.GetString(m_unNameIndex); }

	void							SetColor(const CVec4f &v4Color)			{ m_v4Color = v4Color; }
	const CVec4f&					GetColor() const						{ return m_v4Color; }

	GAME_SHARED_API void			SetTeamSize(uint uiSize);
	uint							GetTeamSize() const						{ return m_uiTeamSize; }
	bool							IsFull() const							{ return GetNumClients() >= m_uiTeamSize; }

	GAME_SHARED_API bool			IsFullyLoaded() const;

	GAME_SHARED_API bool			CanJoinTeam(int iClientNumber, uint uiSlot = INVALID_TEAM_SLOT);
	GAME_SHARED_API bool			CanSee(const IVisualEntity *pTarget) const;

	// Base buildings
	void							SetBaseBuildingIndex(uint uiIndex)		{ m_uiBaseBuildingIndex = uiIndex; }
	uint							GetBaseBuildingIndex() const			{ return m_uiBaseBuildingIndex; }

	// Buildings
	void							AddBuildingUID(uint uiUID)				{ m_setBuildingUIDs.insert(uiUID); }
	GAME_SHARED_API bool			HasBuilding(const tstring &sName) const;
	GAME_SHARED_API uint			GetBuildingCount(const tstring &sUnitType) const;
	const uiset&					GetBuildingSet() const					{ return m_setBuildingUIDs; }
	uint							GetStartingTowerCount() const			{ return m_uiStartingTowerCount; }
	uint							GetCurrentTowerCount() const			{ return m_uiCurrentTowerCount; }
	uint							GetCurrentRangedCount() const			{ return m_uiCurrentRangedCount; }
	uint							GetCurrentMeleeCount() const			{ return m_uiCurrentMeleeCount; }

	bool							ServerFrameSetup();
	bool							ServerFrameMovement();
	bool							ServerFrameCleanup();

	GAME_SHARED_API bool			IsAlliedTeam(int iTeam)					{ return (m_setAlliedTeams.find(iTeam) != m_setAlliedTeams.end()); }
	GAME_SHARED_API void			AddAlliedTeam(int iTeam)				{ m_setAlliedTeams.insert(iTeam); }
	GAME_SHARED_API void			RemoveAlliedTeam(int iTeam)				{ m_setAlliedTeams.erase(iTeam); }

	GAME_SHARED_API int				GetNumActiveClients();
	GAME_SHARED_API int				GetNumConnectedClients();

	GAME_SHARED_API void			SetHeroSpawnPosition(IUnitEntity *pUnit) const;
	GAME_SHARED_API CVec3f			GetHeroSpawnPosition() const;
	GAME_SHARED_API IUnitEntity*	SpawnCreep(const tstring &sName, IVisualEntity *pSpawnPoint, int iLevel, uint uiUpgradeLevel);
	GAME_SHARED_API void			SendMegaCreepMessage(uint uiAttackingTeam);

	GAME_SHARED_API byte			GetRemainingPauses() const				{ return m_yRemainingPauses; }
	GAME_SHARED_API void			Pause()									{ SetFlags(TEAM_FLAG_PAUSED | TEAM_FLAG_CAN_UNPAUSE); m_yRemainingPauses--; }
	GAME_SHARED_API void			Unpause()								{ RemoveFlags(TEAM_FLAG_PAUSED | TEAM_FLAG_CAN_UNPAUSE); }

	float							GetBaseHealthPercent() const			{ return m_fBaseHealthPercent; }

	uint							GetLastBuildingAttackAnnouncement() const		{ return m_uiLastBuildingAttackAnnouncement; }
	void							SetLastBuildingAttackAnnouncement(uint uiTime)	{ m_uiLastBuildingAttackAnnouncement = uiTime; }

	GAME_SHARED_API void			RegisterShopInfo(uint uiIndex);
	GAME_SHARED_API CShopInfo*		GetShopInfo();

	void							SetExtraTime(uint uiExtraTime)			{ m_uiExtraTime = uiExtraTime; }
	uint							GetExtraTime() const					{ return m_uiExtraTime; }

	void							SetUsingExtraTime(bool bUsingExtraTime)	{ m_bUsingExtraTime = bUsingExtraTime; }
	bool							GetUsingExtraTime() const				{ return m_bUsingExtraTime; }

	byte			GetKillStreak() const				{ return m_yKillStreak; }
	void			RewardKill();
	void			ResetKillStreak()					{ m_yKillStreak = 0; }
	bool			AreAllDead() const;
	bool			AreAllAlive() const;
	bool			GetAllKilled() const				{ return m_bAllKilled; }
	void			SetAllKilled(bool b)				{ m_bAllKilled = b; }

	GAME_SHARED_API void	GiveGold(ushort unGold);
	GAME_SHARED_API void	DistributeGold(ushort unGold);

	GAME_SHARED_API float	GetRank() const;
	GAME_SHARED_API float	GetAverageRank() const;
	float					GetWinChance() const		{ return m_fWinChance; }
	void					SetWinChance(float fChance)	{ m_fWinChance = fChance; }

	GAME_SHARED_API uint	GetTeamStat(EPlayerStat eStat) const;
	GAME_SHARED_API float	GetExperienceEarned() const;
	GAME_SHARED_API uint	GetGoldEarned() const;

	float							GetHeroAlivePercent();

	void			AdjustStat(ETeamStat eStat, uint uiAmount)		{ m_aStatTotals[eStat] += uiAmount; }
	void			SetStat(ETeamStat eStat, uint uiAmount)			{ m_aStatTotals[eStat] = uiAmount; }
	uint			GetStat(ETeamStat eStat) const					{ return m_aStatTotals[eStat]; }

#define TEAM_STAT_TOTAL(type, name) \
type	GetTotal##name() const \
{ \
	CGameStats *pStats(GetStats()); \
	if (pStats == NULL) \
		return type(0); \
\
	return pStats->Get##name(); \
}

	TEAM_STAT_TOTAL(float, Experience)
	TEAM_STAT_TOTAL(uint, Deaths)
	TEAM_STAT_TOTAL(uint, HeroKills)
	TEAM_STAT_TOTAL(float, HeroDamage)
	TEAM_STAT_TOTAL(uint, HeroAssists)
	TEAM_STAT_TOTAL(uint, HeroBounty)
	TEAM_STAT_TOTAL(float, HeroExperience)
	TEAM_STAT_TOTAL(uint, CreepKills)
	TEAM_STAT_TOTAL(float, CreepDamage)
	TEAM_STAT_TOTAL(uint, CreepBounty)
	TEAM_STAT_TOTAL(float, CreepExperience)
	TEAM_STAT_TOTAL(uint, Denies)
	TEAM_STAT_TOTAL(float, DeniedExperience)
	TEAM_STAT_TOTAL(uint, NeutralKills)
	TEAM_STAT_TOTAL(float, NeutralDamage)
	TEAM_STAT_TOTAL(uint, NeutralBounty)
	TEAM_STAT_TOTAL(float, NeutralExperience)
	TEAM_STAT_TOTAL(uint, BuildingKills)
	TEAM_STAT_TOTAL(float, BuildingDamage)
	TEAM_STAT_TOTAL(uint, BuildingBounty)
	TEAM_STAT_TOTAL(float, BuildingExperience)
	TEAM_STAT_TOTAL(uint, GoldEarned)
	TEAM_STAT_TOTAL(uint, GoldSpent)
	TEAM_STAT_TOTAL(uint, GoldLost)
	TEAM_STAT_TOTAL(uint, ActionCount)
	TEAM_STAT_TOTAL(uint, BuyBacks)
};
//=============================================================================

#endif //__C_TEAMINFO_H__
