// (C)2008 S2 Games
// c_gamestats.h
//
//=============================================================================
#ifndef __C_GAMESTATS_H__
#define __C_GAMESTATS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GAME_STAT(name) \
private: \
	uint	m_ui##name; \
public: \
	uint	Get##name() const			{ return m_ui##name; } \
	void	Set##name(uint ui##name)	{ m_ui##name = ui##name; } \
	void	Clear##name()				{ m_ui##name = 0; } \
	void	Add##name(uint ui##name)	{ m_ui##name += ui##name; }

enum EItemHistoryAction
{
	ITEM_HISTORY_TRANSFER,
	ITEM_HISTORY_PURCHASE,
	ITEM_HISTORY_ASSEMBLE,
	ITEM_HISTORY_SELL,
	ITEM_HISTORY_DISASSEMBLE,
	ITEM_HISTORY_DROP,
	ITEM_HISTORY_PICKUP
};
//=============================================================================

//=============================================================================
// CGameStats
//=============================================================================
class CGameStats : public IGameEntity
{
	DECLARE_ENTITY_DESC

private:
	DECLARE_ENT_ALLOCATOR3(GameStats);

	GAME_STAT(HeroDamage)
	GAME_STAT(HeroKills)
	GAME_STAT(HeroAssists)
	GAME_STAT(HeroExperience)

	GAME_STAT(CreepDamage)
	GAME_STAT(CreepKills)
	GAME_STAT(CreepExperience)

	GAME_STAT(NeutralDamage)
	GAME_STAT(NeutralKills)
	GAME_STAT(NeutralExperience)

	GAME_STAT(BuildingDamage)
	GAME_STAT(BuildingKills)
	GAME_STAT(BuildingExperience)

	GAME_STAT(Denies)
	GAME_STAT(DeniedExperience)
	GAME_STAT(Experience)

	GAME_STAT(HeroBounty)
	GAME_STAT(CreepBounty)
	GAME_STAT(NeutralBounty)
	GAME_STAT(BuildingBounty)
	GAME_STAT(GoldEarned)
	GAME_STAT(GoldSpent)
	GAME_STAT(GoldLost)

	GAME_STAT(Deaths)
	GAME_STAT(TimeDead)
	GAME_STAT(TimePlayed)
	GAME_STAT(BuyBacks)

	GAME_STAT(ActionCount)

	GAME_STAT(WardsPurchased)
	GAME_STAT(ConsumablesPurchased)

	GAME_STAT(ConcedeCalls)

	struct SAbilityUpgradeEntry
	{
		uint	uiTimeStamp;
		ushort	unAbilityTypeID;
		byte	yLevel;
		byte	ySlot;

		SAbilityUpgradeEntry(uint _uiTimeStamp, ushort _unAbilityTypeID, byte _yLevel, byte _ySlot) :
		uiTimeStamp(_uiTimeStamp),
		unAbilityTypeID(_unAbilityTypeID),
		yLevel(_yLevel),
		ySlot(_ySlot)
		{}
	};
	typedef vector<SAbilityUpgradeEntry>		AbilityUpgradeLog;
	typedef AbilityUpgradeLog::iterator			AbilityUpgradeLog_it;
	typedef AbilityUpgradeLog::const_iterator	AbilityUpgradeLog_cit;

	AbilityUpgradeLog	m_vAbilityUpgrades;
	
	struct SItemHistoryEntry
	{
		uint	uiTimeStamp;
		ushort	unItemTypeID;
		byte	yAction;

		SItemHistoryEntry(uint _uiTimeStamp, ushort _unItemTypeID, byte _yAction) :
		uiTimeStamp(_uiTimeStamp),
		unItemTypeID(_unItemTypeID),
		yAction(_yAction)
		{}
	};
	typedef vector<SItemHistoryEntry>		ItemHistoryLog;
	typedef ItemHistoryLog::iterator		ItemHistoryLog_it;
	typedef ItemHistoryLog::const_iterator	ItemHistoryLog_cit;

	ItemHistoryLog	m_vItemHistory;

	struct SHeroKillHistoryEvent
	{
		uint	uiTimeStamp;
		int		iVictim;
		ivector	vAssists;

		SHeroKillHistoryEvent(uint _uiTimeStamp, int _iVictim, const ivector &_vAssists) :
		uiTimeStamp(_uiTimeStamp),
		iVictim(_iVictim),
		vAssists(_vAssists)
		{}

	};
	typedef vector<SHeroKillHistoryEvent>	HeroKillLog;
	typedef HeroKillLog::iterator			HeroKillLog_it;
	typedef HeroKillLog::const_iterator		HeroKillLog_cit;

	HeroKillLog	m_vKills;

	typedef pair<uint, int>					KillLogEvent;
	typedef vector<KillLogEvent>			KillLogVector;
	typedef KillLogVector::iterator			KillLogVector_it;
	typedef KillLogVector::const_iterator	KillLogVector_cit;

	KillLogVector	m_vDeaths;
	KillLogVector	m_vAssists;

	uint			m_uiClientID;

protected:

public:
	~CGameStats()	{}
	CGameStats();

	SUB_ENTITY_ACCESSOR(CGameStats, Stats)

	// Network
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
	
	void						LogAbilityUpgrade(SAbilityUpgradeEntry &entry)	{ m_vAbilityUpgrades.push_back(entry); }
	const AbilityUpgradeLog&	GetAbilityUpgradeLog() const					{ return m_vAbilityUpgrades; }

	void					LogKill(const SHeroKillHistoryEvent &entry)			{ m_vKills.push_back(entry); }
	const HeroKillLog&		GetKillLog() const									{ return m_vKills; }

	void					LogDeath(KillLogEvent entry)						{ m_vDeaths.push_back(entry); }
	const KillLogVector&	GetDeathLog() const									{ return m_vDeaths; }

	void					LogAssist(KillLogEvent entry)						{ m_vAssists.push_back(entry); }
	const KillLogVector&	GetAssistLog() const								{ return m_vAssists; }

	void					LogItemHistory(SItemHistoryEntry &entry)			{ m_vItemHistory.push_back(entry); }
	const ItemHistoryLog&	GetItemHistoryLog() const							{ return m_vItemHistory; }

	GAME_SHARED_API void	GetExtendedData(IBuffer &buffer) const;
	GAME_SHARED_API void	ReadExtendedData(CPacket &pkt);

	void					SetPlayerClientID(uint uiID)						{ m_uiClientID = uiID; }
	GAME_SHARED_API uint	GetPlayerClientID() const							{ return m_uiClientID; }
};
//=============================================================================

#endif //__C_GAMESTATS_H__
