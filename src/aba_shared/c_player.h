// (C)2008 S2 Games
// c_player.h
//
//=============================================================================
#ifndef __C_PLAYER_H__
#define __C_PLAYER_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_camera.h"
#include "../k2/c_httprequest.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CClientSnapshot;
class CGameStats;
class CClientConnection;
class CHostServer;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const float PLAYER_RANK_UNKNOWN		(-1.0f);
const float PLAYER_RANK_PROVISIONAL	(-2.0f);

enum EPlayerStat
{
	PLAYER_STAT_CREEP_KILLS,
	PLAYER_STAT_NEUTRAL_KILLS,
	PLAYER_STAT_DENIES,

	PLAYER_STAT_HERO_KILLS,
	PLAYER_STAT_DEATHS,
	PLAYER_STAT_ASSISTS,

	PLAYER_STAT_STARTING_GOLD,
	PLAYER_STAT_GOLD_SPENT,

	NUM_PLAYER_STATS
};

enum EPlayerFloatStat
{
	PLAYER_STAT_HERO_DAMAGE,
	PLAYER_STAT_BUILDING_DAMAGE,

	NUM_PLAYER_FLOAT_STATS
};

const ushort PLAYER_FLAG_DISCONNECTED	(BIT(0));
const ushort PLAYER_FLAG_READY			(BIT(1));
const ushort PLAYER_FLAG_HOST			(BIT(2));
const ushort PLAYER_FLAG_HAS_REPICKED	(BIT(3));
const ushort PLAYER_FLAG_LOCAL			(BIT(4));
const ushort PLAYER_FLAG_CAN_PICK		(BIT(5));
const ushort PLAYER_FLAG_LOADING		(BIT(6));
const ushort PLAYER_FLAG_TERMINATED		(BIT(7));
const ushort PLAYER_FLAG_WAS_CONNECTED	(BIT(8));
const ushort PLAYER_FLAG_LOADED_HEROES	(BIT(9));
const ushort PLAYER_FLAG_EXCUSED		(BIT(10));
const ushort PLAYER_FLAG_KICKED			(BIT(11));
const ushort PLAYER_FLAG_STAFF			(BIT(12));
const ushort PLAYER_FLAG_PREMIUM		(BIT(13));
const ushort PLAYER_FLAG_IS_CAPTAIN		(BIT(14));
const ushort PLAYER_FLAG_IS_AFK			(BIT(15));

enum ECommanderOrder
{
	CMDR_ORDER_AUTO = 0,
	CMDR_ORDER_CLEAR,
	CMDR_ORDER_MOVE,
	CMDR_ORDER_STOP,
	CMDR_ORDER_HOLD,
	CMDR_ORDER_PATROL,
	CMDR_ORDER_GIVEITEM,
	CMDR_ORDER_TOUCH,
	CMDR_ORDER_ATTACK,
	CMDR_ORDER_FOLLOW
};

const byte CMDR_ORDER_FLAG_DIRECT_PATHING	(BIT(0));

#if 1
GAME_SHARED_API EXTERN_CVAR_INT(cam_mode);
#else
const int cam_mode(0);
#endif
//=============================================================================

//=============================================================================
// CPlayer
//=============================================================================
class CPlayer : public IGameEntity
{
	DECLARE_ENTITY_DESC

private:
	DECLARE_ENT_ALLOCATOR4(Player);

	// Quick Info
	int			m_iClientNumber;
	uint		m_uiTeamID;
	uint		m_uiHeroIndex;
	bool		m_bIsolated;
	bool		m_bFullVision;
	bool		m_bReferee;

	// Hero
	ushort		m_unSelectedHeroID;
	ushort		m_unPotentialHeroID;
	uint		m_uiStatsIndex;

	// Camera
	CVec3f		m_v3Position;
	CVec3f		m_v3Angles;
	float		m_fCameraDistance;
	float		m_fCameraHeight;

	bool		m_bCameraDrag;
	CCamera		m_cDragCamera;
	CPlane		m_plDragPlane;
	CVec2f		m_v2StartDragCursor;
	CVec3f		m_v3StartDragWorld;
	CVec3f		m_v3StartDragCamera;

	bool		m_bCameraScroll;
	CVec2f		m_v2StartScrollCursor;

	// Identity
	int			m_iAccountID;
	int			m_iClanID;
	ushort		m_unNameIndex;
	ushort		m_unClanNameIndex;
	ushort		m_unClanRankIndex;
	int			m_iKarma;
	float		m_fRank;
	float		m_fMatchWinValue;
	float		m_fMatchLossValue;
	tstring		m_sAddress;
	tstring		m_sMatchComment;
	float		m_fLoadingProgress;

	// Account
	uint		m_uiAccountWins;
	uint		m_uiAccountLosses;
	uint		m_uiAccountDisconnects;
	uint		m_uiAccountKills;
	uint		m_uiAccountAssists;
	uint		m_uiAccountDeaths;
	float		m_fAccountEmPercent;
	float		m_fAccountExpMin;
	float		m_fAccountGoldMin;

	// Affiliations
	byte		m_yVote;
	ushort		m_unFlags;
	byte		m_ySwapRequests;
	uint		m_uiLastVoteCallTime;
	uint		m_uiDraftRound;
	
	int			m_iTeamIndex;
	byte		m_yFullSharedControl;
	byte		m_yPartialSharedControl;
	byte		m_yNoHelp;

	// Assets
	ushort		m_unGold;

	uivector	m_vPetUIDs;

	// Statistics
	ushort		m_unPing;
	uint		m_uiActionCount;
	uint		m_uiStartActionCount;

	uint		m_uiDisconnectTime;
	uint		m_uiTotalDisconnectedTime;
	uint		m_uiTerminationTime;
	uint		m_uiPlayTime;

	uint		m_uiLastInteractionTime;
	bool		m_bAFKWarningSent;

	uint		m_uiLastInputTime;

	byte		m_yKillStreak;
	byte		m_yDeathStreak;
	byte		m_yMultiKill;
	uint		m_uiLastKillTime;

	uint		m_aStatTotals[NUM_PLAYER_STATS];
	float		m_aFloatStatTotals[NUM_PLAYER_FLOAT_STATS];

	uiset		m_setSelection;

	CVec2f		m_v2Cursor;

	ushort		m_unInterfaceIndex;
	ushort		m_unOverlayInterfaceIndex;
	uint		m_uiCameraIndex;

	// Timers and accumulators
	uint		m_uiLastMapPingTime;
	uint		m_uiChatCounter;
	bool		m_bAllowChat;

	// Cache info that should not be revealed in ranked play
	bool		m_bHasSecretInfo;
	int			m_iSecretAccountID;
	tstring		m_sSecretName;
	int			m_iSecretClanID;
	tstring		m_sSecretClanName;
	float		m_fSecretRank;

	// Server-side "notification" bitflags that the client sent.
	uint		m_uiNotificationFlags;

	// Client-side gameplay options that the server needs to be aware of.
	bool		m_bMoveHeroToSpawnOnDisconnect;

	bool		m_bSprinting;

public:
	~CPlayer();
	CPlayer();

	SUB_ENTITY_ACCESSOR(CPlayer, Player)

	GAME_SHARED_API void	Initialize(CClientConnection *pClientConnection, CHostServer *pHostServer);
	GAME_SHARED_API void	RevealSecretInfo();

	// Network
	virtual void	Baseline();
	virtual void	GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
	virtual bool	ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
	virtual void	ReadClientSnapshot(const CClientSnapshot &snapshot);
	
	GAME_SHARED_API	void	PrepareClientSnapshot(CClientSnapshot &snapshot);

	int						GetPrivateClient()					{ return m_iClientNumber; }

	uint					GetLastInputTime() const			{ return m_uiLastInputTime; }
	void					SetLastInputTime(uint uiTime)		{ m_uiLastInputTime = uiTime; }

	uint					GetLastMapPingTime() const			{ return m_uiLastMapPingTime; }
	void					SetLastMapPingTime(uint uiTime)		{ m_uiLastMapPingTime = uiTime; }

	// Hero
	GAME_SHARED_API bool	CanSelectHero(ushort unHeroID);
	void					SelectHero(ushort unHeroID)			{ m_unSelectedHeroID = unHeroID; }
	void					SelectPotentialHero(ushort unHeroID){ m_unPotentialHeroID = unHeroID; }
	ushort					GetSelectedHero() const				{ return m_unSelectedHeroID; }
	bool					HasSelectedHero() const				{ return m_unSelectedHeroID != INVALID_ENT_TYPE; }
	ushort					GetPotentialHero() const			{ return m_unPotentialHeroID; }
	bool					HasPotentialHero() const			{ return m_unPotentialHeroID != INVALID_ENT_TYPE; }
	GAME_SHARED_API bool	SpawnHero();
	IHeroEntity*			GetHero() const						{ return Game.GetHeroEntity(GetHeroIndex()); }
	void					SetHeroIndex(uint uiHeroIndex)		{ m_uiHeroIndex = uiHeroIndex; }
	uint					GetHeroIndex() const				{ return m_uiHeroIndex; }
	bool					HasSpawnedHero() const				{ return GetHero() != NULL; }
	GAME_SHARED_API	void	AssignHero(IHeroEntity *pHero);
	bool					IsHeroLocked() const				{ return HasSelectedHero() && !CanSwap() && !CanRepick(); }

	GAME_SHARED_API	bool	IsIsolated() const;

	CGameStats*				GetStats() const					{ return Game.GetStatsEntity(GetStatsIndex()); }
	uint					GetStatsIndex() const				{ return m_uiStatsIndex; }
	GAME_SHARED_API	void	AssignStats(CGameStats *pStats);

	void					SetLastInteractionTime(uint uiTime)	{ m_uiLastInteractionTime = uiTime; SetAFKWarningSent(false); }
	uint					GetLastInteractionTime() const		{ return m_uiLastInteractionTime; }
	void					SetAFKWarningSent(bool b)			{ m_bAFKWarningSent = b; }
	bool					GetAFKWarningSent() const			{ return m_bAFKWarningSent; }

	// Identity
	int						GetClanID() const					{ return m_iClanID; }
	void					SetRank(float fRank)				{ m_fRank = fRank; }
	void					SetSecretRank(float fRank)			{ m_fSecretRank = fRank; }
	float					GetRank() const						{ return m_fRank; }
	void					StoreMatchPointValues()				{ m_fMatchWinValue = GetAdjustedMatchWinValue(); m_fMatchLossValue = GetAdjustedMatchLossValue(); }
	float					GetStoredMatchWinValue()			{ return m_fMatchWinValue; }
	float					GetStoredMatchLossValue()			{ return m_fMatchLossValue; }
	GAME_SHARED_API float	GetKFactor() const;
	GAME_SHARED_API float	GetMatchWinValue() const;
	GAME_SHARED_API float	GetMatchLossValue() const;
	GAME_SHARED_API float	GetSkillDifferenceAdjustment() const;
	GAME_SHARED_API float	GetAdjustedMatchWinValue() const;
	GAME_SHARED_API float	GetAdjustedMatchLossValue() const;

	// Account
	void					SetAccountWins(uint uiWins)					{ m_uiAccountWins = uiWins; }
	void					SetAccountLosses(uint uiLosses)				{ m_uiAccountLosses = uiLosses; }
	void					SetAccountDisconnects(uint uiDisconnects)	{ m_uiAccountDisconnects = uiDisconnects; }
	void					SetAccountKills(uint uiKills)				{ m_uiAccountKills = uiKills; }
	void					SetAccountAssists(uint uiAssists)			{ m_uiAccountAssists = uiAssists; }
	void					SetAccountDeaths(uint uiDeaths)				{ m_uiAccountDeaths = uiDeaths; }
	void					SetAccountEmPercent(float fEmPercent)		{ m_fAccountEmPercent = fEmPercent; }
	void					SetAccountExpMin(float fExpMin)				{ m_fAccountExpMin = fExpMin; }
	void					SetAccountGoldMin(float fGoldMin)			{ m_fAccountGoldMin = fGoldMin; }
	uint					GetAccountWins() const						{ return m_uiAccountWins; }
	uint					GetAccountLosses() const					{ return m_uiAccountLosses; }
	uint					GetAccountDisconnects() const				{ return m_uiAccountDisconnects; }
	uint					GetAccountKills() const						{ return m_uiAccountKills; }
	uint					GetAccountAssists() const					{ return m_uiAccountAssists; }
	uint					GetAccountDeaths() const					{ return m_uiAccountDeaths; }
	float					GetAccountEmPercent() const					{ return m_fAccountEmPercent; }
	float					GetAccountExpMin() const					{ return m_fAccountExpMin; }
	float					GetAccountGoldMin() const					{ return m_fAccountGoldMin; }

	int						GetAccountID() const				{ return m_iAccountID; }
	int						GetClientNumber() const				{ return m_iClientNumber; }
	const tstring&			GetName() const						{ return NetworkResourceManager.GetString(m_unNameIndex); }
	const tstring&			GetTrueName() const					{ return (m_bHasSecretInfo ? m_sSecretName : GetName()); }
	const tstring&			GetClanName() const					{ return NetworkResourceManager.GetString(m_unClanNameIndex); }
	const tstring&			GetClanRank() const					{ return NetworkResourceManager.GetString(m_unClanRankIndex); }
	int						GetKarma() const					{ return m_iKarma; }
	const tstring&			GetAddress() const					{ return m_sAddress; }

	void					SetMatchComment(const tstring &sComment)	{ m_sMatchComment = sComment; }
	const tstring&			GetMatchComment() const						{ return m_sMatchComment; }
	
	void					SetLoadingProgress(float fProgress)	{ m_fLoadingProgress = fProgress; }
	float					GetLoadingProgress() const			{ return m_fLoadingProgress; }

	void					SetDraftRound(uint uiRound)			{ m_uiDraftRound = uiRound; }
	uint					GetDraftRound() const				{ return m_uiDraftRound; }

	static GAME_SHARED_API	CVec4f	GetColor(uint uiIndex);
	GAME_SHARED_API	CVec4f			GetColor() const;
	
	static GAME_SHARED_API	const tstring&	GetColorName(uint uiIndex);
	GAME_SHARED_API	const tstring&			GetColorName() const;

	// Shared control
	GAME_SHARED_API	void	ShareFullControl(int iClientNumber);
	GAME_SHARED_API	void	SharePartialControl(int iClientNumber);
	GAME_SHARED_API	void	UnshareFullControl(int iClientNumber);
	GAME_SHARED_API	void	UnsharePartialControl(int iClientNumber);

	GAME_SHARED_API	bool	HasSharedFullControl(int iClientNumber);
	GAME_SHARED_API	bool	HasSharedPartialControl(int iClientNumber);

	GAME_SHARED_API void	SetNoHelp(CPlayer *pPlayer, bool bEnable);
	GAME_SHARED_API bool	GetNoHelp(CPlayer *pPlayer);

	// Affiliations
	GAME_SHARED_API	void	ClearAffiliations();

	void					ClearFlags()						{ m_unFlags = 0; }
	void					SetFlags(ushort unFlags)			{ m_unFlags |= unFlags; }
	void					RemoveFlags(ushort unFlags)			{ m_unFlags &= ~unFlags; }
	bool					HasFlags(ushort unFlags) const		{ return (m_unFlags & unFlags) == unFlags; }
	ushort					GetFlags() const					{ return m_unFlags; }

	bool					CanBeKicked() const					{ return !HasFlags(PLAYER_FLAG_HOST) && !HasFlags(PLAYER_FLAG_TERMINATED) && !HasFlags(PLAYER_FLAG_STAFF) && !IsReferee(); }
	bool					CanKick() const						{ return HasFlags(PLAYER_FLAG_HOST); }

	void					SetVote(byte yVote)					{ m_yVote = yVote; }
	byte					GetVote() const						{ return m_yVote; }
	void					SetLastVoteCallTime(uint uiTime)	{ m_uiLastVoteCallTime = uiTime; }
	uint					GetLastVoteCallTime() const			{ return m_uiLastVoteCallTime; }

	void					SetSwapRequest(int iTeamIndex)			{ m_ySwapRequests |= BIT(iTeamIndex); }
	void					ClearSwapRequest(int iTeamIndex)		{ m_ySwapRequests &= ~BIT(iTeamIndex); }
	bool					HasSwapRequest(int iTeamIndex) const	{ return (m_ySwapRequests & BIT(iTeamIndex)) != 0; }
	void					ClearAllSwapRequests()					{ m_ySwapRequests = 0; }

	GAME_SHARED_API	bool	CanRepick() const;
	GAME_SHARED_API	bool	CanSwap() const;
	GAME_SHARED_API	bool	CanSwap(int iTeamMateIndex) const;
	GAME_SHARED_API	bool	IsCurrentPicker() const;
	GAME_SHARED_API bool	IsReferee() const;
	GAME_SHARED_API void	SetReferee(bool bReferee)			{ m_bReferee = bReferee; }

	GAME_SHARED_API void	SetTeam(uint uiTeamID);
	uint					GetTeam() const						{ return m_uiTeamID; }

	void					SetTeamIndex(int iTeamIndex)		{ m_iTeamIndex = iTeamIndex; }
	int						GetTeamIndex() const				{ return m_iTeamIndex; }

	CTeamInfo*				GetTeamInfo() const					{ return Game.GetTeam(GetTeam()); }

	// Assets
	void					SetGold(ushort unGold)				{ m_unGold = unGold; }
	ushort					GetGold() const						{ return m_unGold; }
	uint					GetGoldEarned() const				{ return m_unGold + m_aStatTotals[PLAYER_STAT_GOLD_SPENT]; }
	GAME_SHARED_API void	GiveGold(ushort unGold, IUnitEntity *pSource, IUnitEntity *pTarget = NULL);
	GAME_SHARED_API void	TakeGold(ushort unGold);
	GAME_SHARED_API bool	SpendGold(ushort unCost);

	// Pets
	GAME_SHARED_API void			AddPet(IUnitEntity *pPet, uint uiSummonMax, uint uiControllerUID);
	GAME_SHARED_API void			AddPetClient(uint uiIndex);
	GAME_SHARED_API IUnitEntity*	GetPersistentPet(ushort unTypeID);
	GAME_SHARED_API void			RecallPets(IUnitEntity *pCaller, ushort unPetType);
	GAME_SHARED_API void			LevelPets(IUnitEntity *pCaller, ushort unPetType, uint uiLevel);

	// Statistics
	void			SetPing(ushort unPing)				{ m_unPing = unPing; }
	ushort			GetPing() const						{ return m_unPing; }
	
	uint			GetActionCount() const				{ return m_uiActionCount; }
	void			ResetActionCount()					{ m_uiActionCount = 0; m_uiStartActionCount = Game.GetGameTime(); }
	void			IncrementActionCount()				{ ++m_uiActionCount; }
	uint			GetActionCountPeriod() const		{ if (m_uiStartActionCount == INVALID_TIME) return INVALID_TIME; return Game.GetGameTime() - m_uiStartActionCount; }

	byte			GetKillStreak() const				{ return m_yKillStreak; }
	byte			GetMultiKill() const				{ return m_yMultiKill; }
	void			RewardKill();
	void			ResetKillStreak();

	void			AddDeathStreak()					{ m_yDeathStreak++; }
	byte			GetDeathStreak()					{ return m_yDeathStreak; }

	void			AdjustStat(EPlayerStat eStat, int iAmount)		{ m_aStatTotals[eStat] += iAmount; }
	uint			GetStat(EPlayerStat eStat) const				{ return m_aStatTotals[eStat]; }

	void			AdjustFloatStat(EPlayerFloatStat eStat, float fAmount)	{ m_aFloatStatTotals[eStat] += fAmount; }
	uint			GetFloatStat(EPlayerFloatStat eStat) const				{ return m_aFloatStatTotals[eStat]; }

	void			SetTerminationTime(uint uiTime)		{ m_uiTerminationTime = uiTime; }
	uint			GetTerminationTime() const			{ return m_uiTerminationTime; }
	void			SetDisconnectedTime(uint uiTime)	{ m_uiDisconnectTime = uiTime; }
	uint			GetDisconnectedTime() const			{ return m_uiDisconnectTime; }
	bool			IsDisconnected() const				{ return HasFlags(PLAYER_FLAG_DISCONNECTED); }

	GAME_SHARED_API void	Connected(uint uiTime);
	GAME_SHARED_API void	FinishedLoading(uint uiTime, uint uiMaxDisconnectTime);
	GAME_SHARED_API void	Disconnected(uint uiTime, uint uiMaxDisconnectTime);

	uint			GetPlayTime() const					{ return m_uiPlayTime; }
	void			AddPlayTime(uint uiTime)			{ m_uiPlayTime += uiTime; }

	GAME_SHARED_API void	Terminate();

	void					MatchRemake();

	void					Spawn();
	
	bool					ServerFrameMovement();
	bool					ServerFrameCleanup();

	GAME_SHARED_API void	DrawViewBox(class CUITrigger &minimap, CCamera &camera);

	GAME_SHARED_API void	SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles);

	GAME_SHARED_API void	ZoomIn(CClientSnapshot &snapshot);
	GAME_SHARED_API void	ZoomOut(CClientSnapshot &snapshot);
	GAME_SHARED_API void	AdjustPitch(float fPitch);
	GAME_SHARED_API void	AdjustYaw(float fYaw);
	GAME_SHARED_API void	AdjustCameraHeight(float fAmount);
	GAME_SHARED_API void	StartDrag(CCamera *pCamera);
	GAME_SHARED_API void	EndDrag();
	GAME_SHARED_API void	StartScroll();
	GAME_SHARED_API void	EndScroll();

	GAME_SHARED_API void	ResetCamera();

	const uiset&			GetSelection() const						{ return m_setSelection; }
	void					SetSelection(const uiset &setSelection)		{ m_setSelection = setSelection; }

	GAME_SHARED_API void	DoCommandPositional(ECommanderOrder eCommand, const CVec3f &v3Pos);
	GAME_SHARED_API void	DoCommandEntity(ECommanderOrder eCommand, uint uiEntIndex);

	GAME_SHARED_API bool	IsEnemy(const IUnitEntity *pOther) const;
	GAME_SHARED_API bool	CanSee(const IVisualEntity *pTarget) const;

	const CVec3f&			GetPosition() const		{ return m_v3Position; }
	const CVec3f&			GetAngles() const		{ return m_v3Angles; }
	const CVec2f&			GetCursorPos() const	{ return m_v2Cursor; }

	void					SetIsolated(bool bIsolated)		{ m_bIsolated = bIsolated; }
	bool					GetIsolated() const				{ return m_bIsolated; }

	void					SetFullVision(bool bFullVision)		{ m_bFullVision = bFullVision; }
	bool					GetFullVision() const				{ return m_bFullVision; }

	GAME_SHARED_API uint	GetPlayerIndex() const;

	void					IncrementChatCounter()		{ ++m_uiChatCounter; }
	void					DecrementChatCounter()		{ if (m_uiChatCounter > 0) --m_uiChatCounter; }
	uint					GetChatCounter() const		{ return m_uiChatCounter; }
	bool					GetAllowChat() const		{ return m_bAllowChat; }
	void					SetAllowChat(bool b)		{ m_bAllowChat = b; }

	float					GetCameraHeight() const		{ return m_fCameraHeight; }

	void					SetInterface(const tstring &sName);
	const tstring&			GetInterface() const				{ return NetworkResourceManager.GetString(m_unInterfaceIndex); }

	void					SetOverlayInterface(const tstring &sName);
	const tstring&			GetOverlayInterface() const			{ return NetworkResourceManager.GetString(m_unOverlayInterfaceIndex); }

	void					SetCameraIndex(uint uiCameraIndex)	{ m_uiCameraIndex = uiCameraIndex; }
	uint					GetCameraIndex() const				{ return m_uiCameraIndex; }

	bool					ShouldMoveUnitsOnDisconnect() const	{ return m_bMoveHeroToSpawnOnDisconnect; }
	GAME_SHARED_API void	MoveUnitsToSafety(int iClientNum);

	GAME_SHARED_API bool	ProcessGameplayOption(const tstring &sOption, const tstring &sValue);

	bool					HasNotificationFlags(uint uiFlags) const	{ return (m_uiNotificationFlags & uiFlags) != 0; }
	void					SetNotificationFlags(uint uiFlags)			{ m_uiNotificationFlags |= uiFlags; }
	void					ClearNotificationFlags(uint uiFlags)		{ m_uiNotificationFlags &= ~uiFlags; }

	void					SetSprinting(bool bValue)					{ m_bSprinting = bValue; }
	bool					GetSprinting() const						{ return m_bSprinting; }
};
//=============================================================================

#endif //__C_PLAYER_H__
