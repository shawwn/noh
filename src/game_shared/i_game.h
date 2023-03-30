// (C)2006 S2 Games
// i_game.h
//
//=============================================================================
#ifndef __I_GAME_H__
#define __I_GAME_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/c_world.h"

#include "c_eventdirectory.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CTeamDefinition;
class CEntityTeamInfo;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define Game (*IGame::GetCurrentGamePointer())

enum EGamePhase
{
	GAME_PHASE_INVALID,

	GAME_PHASE_WAITING_FOR_PLAYERS,
	GAME_PHASE_SELECTING_COMMANDER,
	GAME_PHASE_SELECTING_OFFICERS,
	GAME_PHASE_FORMING_SQUADS,
	GAME_PHASE_ACTIVE,
	GAME_PHASE_WARMUP,
	GAME_PHASE_ENDED,
	GAME_PHASE_STANDBY,

	NUM_GAME_PHASES
};

enum EHitFeedbackType
{
	HIT_MELEE,
	HIT_RANGED,
	HIT_UNIT_RANGED,
	HIT_BUILDING_MELEE,
	HIT_BUILDING_RANGED,
	HIT_GENERIC,
	HIT_BLOCKED,
	HIT_GOT_BLOCKED,
	HIT_INTERRUPTED,
	HIT_GOT_INTERRUPTED,
	HIT_MELEEHIT,
	HIT_GOT_HITBYMELEE,
	HIT_GOT_HITBYMELEE_NOKICK,
	HIT_MELEE_IMPACT
};

enum EPersistantItems
{
	PERSISTANT_VAULT_START,

	PERSISTANT_VAULT_1 = PERSISTANT_VAULT_START,
	PERSISTANT_VAULT_2,
	PERSISTANT_VAULT_3,
	PERSISTANT_VAULT_4,
	PERSISTANT_VAULT_5,

	MAX_PERSISTANT_ITEMS
};

struct SPersistantItemVault
{
	ushort unItemType[MAX_PERSISTANT_ITEMS];
	uint uiItemID[MAX_PERSISTANT_ITEMS];
};

enum EPlayerMatchStat
{
	// Player stats
	PLAYER_MATCH_KILLS,
	PLAYER_MATCH_DEATHS,
	PLAYER_MATCH_RAZED,
	PLAYER_MATCH_BUILDING_DAMAGE,
	PLAYER_MATCH_PLAYER_DAMAGE,
	PLAYER_MATCH_HEALED,
	PLAYER_MATCH_RESURRECTS,
	PLAYER_MATCH_ASSISTS,
	PLAYER_MATCH_GOLD_EARNED,
	PLAYER_MATCH_REPAIRED,
	PLAYER_MATCH_NPC_KILLS,
	PLAYER_MATCH_SOULS_SPENT,

	// Commander stats
	COMMANDER_MATCH_START,

	COMMANDER_MATCH_BUILDINGS = COMMANDER_MATCH_START,
	COMMANDER_MATCH_EXPERIENCE,
	COMMANDER_MATCH_TEAM_GOLD,
	COMMANDER_MATCH_RAZED,
	COMMANDER_MATCH_HEALED,
	COMMANDER_MATCH_PLAYER_DAMAGE,
	COMMANDER_MATCH_KILLS,
	COMMANDER_MATCH_DEBUFFS,
	COMMANDER_MATCH_BUFFS,
	COMMANDER_MATCH_ORDERS,
	
	NUM_PLAYER_MATCH_STATS
};

enum EVCType
{
	VC_TEAM,
	VC_SQUAD,
	VC_COMMANDER,
	VC_ALL
};

// ChangeUnit flags
const int CHANGE_UNIT_SPAWN						(BIT(0));
const int CHANGE_UNIT_INHERIT_POS				(BIT(1));
const int CHANGE_UNIT_INHERIT_HP				(BIT(2));
const int CHANGE_UNIT_KILL						(BIT(3));
const int CHANGE_UNIT_CHECK_COMMANDER			(BIT(4));
const int CHANGE_UNIT_INHERIT_DAMAGE_RECORD		(BIT(5));
const int CHANGE_UNIT_REFUND_GOLD				(BIT(6));
const int CHANGE_UNIT_CHECK_PREREQUISITES		(BIT(7));
const int CHANGE_UNIT_CHECK_STATUS				(BIT(8));
const int CHANGE_UNIT_CHECK_COST				(BIT(9));
const int CHANGE_UNIT_CHECK_RACE				(BIT(10));
const int CHANGE_UNIT_TRANSFER_ITEMS			(BIT(11));
const int CHANGE_UNIT_TRANSFER_PERSISTANTS		(BIT(12));
const int CHANGE_UNIT_NO_DELETE					(BIT(13));
const int CHANGE_UNIT_CHECK_RULES				(CHANGE_UNIT_CHECK_COMMANDER | CHANGE_UNIT_CHECK_PREREQUISITES | CHANGE_UNIT_CHECK_STATUS | CHANGE_UNIT_CHECK_COST | CHANGE_UNIT_CHECK_RACE);

typedef map<int, class CEntityClientInfo*>	ClientInfoMap;
typedef ClientInfoMap::iterator				ClientInfoMap_it;
typedef ClientInfoMap::reverse_iterator		ClientInfoMap_rit;

const byte INVALID_SQUAD(-1);
//=============================================================================

//=============================================================================
// IGame
//=============================================================================
class IGame
{
private:
	GAME_SHARED_API static IGame*	s_pGame;

	CRandomSequenceGenerator		m_rand;

	EGamePhase						m_eGamePhase;
	uint							m_uiPhaseStartTime;
	uint							m_uiPhaseDuration;
	bool							m_bSuddenDeath;

	uint							m_uiGameTime;
	uint							m_uiFrameLength;
	CWorld*							m_pWorld;
	IEntityDirectory*				m_pEntityDirectory;

	CEventDirectory					m_EventDirectory;

	vector<CEntityTeamInfo*>		m_vTeams;
	int								m_iLastWinningTeam;

protected:
	void			SetGameTime(uint uiTime)			{ m_uiGameTime = uiTime; }
	void			SetFrameLength(uint uiTime)			{ m_uiFrameLength = uiTime; }

	ClientInfoMap	m_mapClients;

public:
	virtual inline ~IGame();
	inline IGame();

	static void		SetCurrentGamePointer(IGame *pGame)	{ s_pGame = pGame; }
	static IGame*	GetCurrentGamePointer()				{ return s_pGame; }
	
	inline bool		Validate() const;

	virtual bool	IsServer()							{ return false; }
	virtual bool	IsClient()							{ return false; }

	virtual int		GetLocalClientNum()					{ return -1; }

	virtual void	SuddenDeathAlert()					{}
	bool			IsSuddenDeathActive() const			{ return m_bSuddenDeath; }
	void			SetSuddenDeath(bool bActive)		{ if (!m_bSuddenDeath && bActive) SuddenDeathAlert(); m_bSuddenDeath = bActive; }

	// Time
	uint			GetGameTime() const					{ return m_uiGameTime; }
	uint			GetFrameLength() const				{ return m_uiFrameLength; }
	uint			GetCurrentGameLength();

	virtual void	StartGame()							{}
	virtual void	EndGame(int iLosingTeam)			{}
	EGamePhase		GetGamePhase() const				{ return m_eGamePhase; }
	uint			GetPhaseStartTime() const			{ return m_uiPhaseStartTime; }
	uint			GetPhaseDuration() const			{ return m_uiPhaseDuration; }
	uint			GetPhaseEndTime() const				{ return (m_uiPhaseDuration == -1) ? -1 : m_uiPhaseStartTime + m_uiPhaseDuration; }
	uint			GetRemainingPhaseTime() const		{ return m_uiPhaseDuration - (m_uiGameTime - m_uiPhaseStartTime); }
	void			SetGamePhase(EGamePhase ePhase, uint uiLength = -1, uint uiStartTime = -1);

	// Random sequences
	void			SeedRand(uint uiSeed)				{ m_rand.Seed(uiSeed); }
	uint			GetRand(uint uiMin, uint uiMax)		{ return m_rand.GetUInt(uiMin, uiMax); }
	int				GetRand(int iMin, int iMax)			{ return m_rand.GetInt(iMin, iMax); }
	float			GetRand(float fMin, float fMax)		{ return m_rand.GetFloat(fMin, fMax); }

	// World
	void			SetWorldPointer(CWorld *pWorld)							{ m_pWorld = pWorld; }
	CWorld*			GetWorldPointer() const									{ return m_pWorld; }
	bool			IsWorldLoaded() const									{ return m_pWorld->IsLoaded(); }
	float			GetWorldWidth() const									{ return m_pWorld->GetWorldWidth(); }
	float			GetWorldHeight() const									{ return m_pWorld->GetWorldHeight(); }
	uint			AllocateNewWorldEntity(uint uiIndex = INVALID_INDEX)	{ return m_pWorld->AllocateNewEntity(uiIndex); }
	virtual void	DeleteWorldEntity(uint uiIndex)							{ m_pWorld->DeleteEntity(uiIndex); }
	bool			WorldEntityExists(uint uiIndex)							{ return m_pWorld->EntityExists(uiIndex); }
	WorldEntMap&	GetWorldEntityMap()										{ return m_pWorld->GetEntityMap(); }
	WorldLightsMap&	GetWorldLightsMap()										{ return m_pWorld->GetLightsMap(); }
	WorldSoundsMap&	GetWorldSoundsMap()										{ return m_pWorld->GetSoundsMap(); }
	smaps&			GetWorldScriptMap()										{ return m_pWorld->GetScriptMap(); }
	CWorldEntity*	GetWorldEntity(uint uiIndex, bool bThrow = NO_THROW)	{ return m_pWorld->GetEntity(uiIndex, bThrow); }
	CWorldLight*	GetWorldLight(uint uiIndex, bool bThrow = NO_THROW)		{ return m_pWorld->GetLight(uiIndex, bThrow); }
	CWorldSound*	GetWorldSound(uint uiIndex, bool bThrow = NO_THROW)		{ return m_pWorld->GetSound(uiIndex, bThrow); }
	void			LinkEntity(uint uiIndex, uint uiLinkFlags, uint uiSurfFlags) const	{ m_pWorld->LinkEntity(uiIndex, uiLinkFlags, uiSurfFlags); }
	void			UnlinkEntity(uint uiIndex)								{ m_pWorld->UnlinkEntity(uiIndex); }
	float			SampleGround(float fX, float fY)						{ return m_pWorld->SampleGround(fX, fY); }
	CVec3f			GetTerrainNormal(float fX, float fY)					{ return m_pWorld->GetTerrainNormal(fX, fY); }
	float			GetTerrainHeight(float fX, float fY)					{ return m_pWorld->GetTerrainHeight(fX, fY); }
	float			CalcMinTerrainHeight(const CRecti &recArea)				{ return m_pWorld->CalcMinHeight(recArea); }
	float			CalcMaxTerrainHeight(const CRecti &recArea)				{ return m_pWorld->CalcMaxHeight(recArea); }
	bool			CalcBlocked(const CRecti &recArea)						{ return m_pWorld->CalcBlocked(recArea); }
	float			GetGroundLevel()										{ return m_pWorld->GetGroundLevel(); }
	
	GAME_SHARED_API bool	TraceLine(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, int iIgnoreSurface = 0, uint uiIgnoreEntity = -1);
	GAME_SHARED_API bool	TraceBox(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface = 0, uint uiIgnoreEntity = -1);
	
	void	GetEntitiesInRegion(uivector &vResult, const CBBoxf &bbRegion, uint uiIgnoreSurface)
				{ m_pWorld->GetEntitiesInRegion(vResult, bbRegion, uiIgnoreSurface); }
	void	GetEntitiesInRegion(uivector &vResult, const CVec3f &v3Min, const CVec3f &v3Max, uint uiIgnoreSurface)	
				{ GetEntitiesInRegion(vResult, CBBoxf(v3Min, v3Max), uiIgnoreSurface); }
	void	GetEntitiesInRegion(uivector &vResult, const CRectf &recArea, uint uiIgnoreSurface)					
				{ GetEntitiesInRegion(vResult, CBBoxf(CVec3f(recArea.lt(), -FAR_AWAY), CVec3f(recArea.rb(), FAR_AWAY)), uiIgnoreSurface); }
	void	GetEntitiesInRegion(uivector &vResult, const CVec2f &v2Min, const CVec2f &v2Max, uint uiIgnoreSurface)
				{ GetEntitiesInRegion(vResult, CBBoxf(CVec3f(v2Min, -FAR_AWAY), CVec3f(v2Max, FAR_AWAY)), uiIgnoreSurface); }
	
	void	GetEntitiesInRadius(uivector &vResult, const CSphere &cRadius, uint uiIgnoreSurface)
				{ m_pWorld->GetEntitiesInRadius(vResult, cRadius, uiIgnoreSurface); }

	void	GetEntitiesInSurface(uivector &vResult, const CConvexPolyhedron &cSurface, uint uiIgnoreSurface)
				{ m_pWorld->GetEntitiesInSurface(vResult, cSurface, uiIgnoreSurface); }

	// Teams
	int									GetNumTeams() const							{ return int(m_vTeams.size()); }
	GAME_SHARED_API void				SetTeam(uint ui, CEntityTeamInfo *pTeam);
	CEntityTeamInfo*					GetTeam(int i)								{ if (i < 0 || i >= GetNumTeams()) return NULL; else return m_vTeams[i]; }
	GAME_SHARED_API CEntityTeamInfo*	GetTeamFromIndex(uint uiIndex);
	GAME_SHARED_API void				ClearTeams();
	const vector<CEntityTeamInfo*>&		GetTeams() const							{ return m_vTeams; }
	GAME_SHARED_API	void				RemoveTeam(int i)							{ if (i < GetNumTeams()) m_vTeams.erase(m_vTeams.begin() + i); }

	void	SetWinningTeam(int iTeam)	{ m_iLastWinningTeam = iTeam; }
	int		GetWinningTeam()			{ return m_iLastWinningTeam; }

	// Entities
	void					SetEntityDirectory(IEntityDirectory* pDirectory)	{ m_pEntityDirectory = pDirectory; }
	virtual IGameEntity*	AllocateEntity(const tstring &sName, uint uiMinIndex = INVALID_INDEX)	{ return NULL; }
	virtual IGameEntity*	AllocateEntity(ushort unType, uint uiMinIndex = INVALID_INDEX)	{ return NULL; }
	virtual void			DeleteEntity(IGameEntity *pEntity)					{}
	virtual void			DeleteEntity(uint uiIndex)							{}
	IGameEntity*			GetEntity(uint uiIndex)								{ return m_pEntityDirectory->GetEntity(uiIndex); }
	IVisualEntity*			GetVisualEntity(uint uiIndex)						{ IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsVisualEnt()); }
	IPlayerEntity*			GetPlayerEntity(uint uiIndex)						{ IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsPlayerEnt()); }
	IBuildingEntity*		GetBuildingEntity(uint uiIndex)						{ IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsBuilding()); }
	IPropEntity*			GetPropEntity(uint uiIndex)							{ IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsProp()); }
	ICombatEntity*			GetCombatEntity(uint uiIndex)						{ IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsCombatEnt()); }
	IPetEntity*				GetPetEntity(uint uiIndex)							{ IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsPet()); }
	IGadgetEntity*			GetGadgetEntity(uint uiIndex)						{ IGameEntity *pEntity(GetEntity(uiIndex)); return ((pEntity == NULL) ? NULL : pEntity->GetAsGadget()); }
	IVisualEntity*			GetEntityFromWorldIndex(uint uiIndex)				{ CWorldEntity *pWorldEnt(GetWorldEntity(uiIndex)); if (pWorldEnt == NULL) return NULL; return GetVisualEntity(pWorldEnt->GetGameIndex()); }
	IPlayerEntity*			GetPlayerEntityFromClientID(int iClientNum)				{ return m_pEntityDirectory->GetPlayerEntityFromClientID(iClientNum); }
	IGameEntity*			GetEntityFromUniqueID(uint uiUniqueID)				{ return m_pEntityDirectory->GetEntityFromUniqueID(uiUniqueID); }
	uint					GetGameIndexFromUniqueID(uint uiUniqueID)			{ return m_pEntityDirectory->GetGameIndexFromUniqueID(uiUniqueID); }
	uint					GetGameIndexFromWorldIndex(uint uiIndex)			{ CWorldEntity *pWorldEnt(GetWorldEntity(uiIndex)); if (pWorldEnt == NULL) return INVALID_INDEX; return pWorldEnt->GetGameIndex(); }
	IGameEntity*			GetFirstEntity()									{ return m_pEntityDirectory->GetFirstEntity(); }
	IGameEntity*			GetNextEntity(IGameEntity *pEntity)					{ return m_pEntityDirectory->GetNextEntity(pEntity); }
	IVisualEntity*			GetEntityFromName(const tstring &sName)				{ return m_pEntityDirectory->GetEntityFromName(sName); }
	IVisualEntity*			GetNextEntityFromName(IVisualEntity *pEntity)		{ return m_pEntityDirectory->GetNextEntityFromName(pEntity); }
	CEntityClientInfo*		GetClientInfo(int iClientNumber)					{ ClientInfoMap_it itClient(m_mapClients.find(iClientNumber)); if (itClient == m_mapClients.end()) return NULL; return itClient->second; }
	ClientInfoMap&			GetClientMap()										{ return m_mapClients; }

	// Resources
	virtual ResHandle		RegisterModel(const tstring &sPath) = 0;
	virtual ResHandle		RegisterEffect(const tstring &sPath) = 0;

	// Events
	void	ClearEventList()						{ m_EventDirectory.Clear(); }
	uint	AddEvent(const CGameEvent &ev)			{ return m_EventDirectory.AddEvent(ev); }
	void	DeleteEvent(uint uiEvent)				{ return m_EventDirectory.DeleteEvent(uiEvent); }
	void	GetEventSnapshot(CSnapshot &snapshot)	{ m_EventDirectory.GetSnapshot(snapshot); }
	void	EventsFrame()							{ m_EventDirectory.Frame(); }
	void	SynchNewEvents()						{ m_EventDirectory.SynchNewEvents(); }

	virtual void	DoUpkeepEvent(bool bFailed, int iTeam)	{}

	// Client
	virtual CClientSnapshot*	GetCurrentSnapshot()											{ return NULL; }
	virtual void				AssignEntityToClient(int iClientNum, IPlayerEntity *pNewPlayer)	{}
	virtual bool				IsCommander() const												{ return false; }
	virtual void				SelectItem(int iSlot)											{}
	virtual void				UpdateBuildingPlacement(IBuildingEntity *pBuilding)				{}

	virtual bool	IsEntityHoverSelected(uint uiIndex)	{ return false; }
	virtual bool	LooksLikeEnemy(uint uiIndex)		{ return false; }

	virtual IPlayerEntity*	ChangeUnit(int iClientNum, const tstring &sNewUnitName, int iFlags = 0)	{ return NULL; }
	virtual IPlayerEntity*	ChangeUnit(int iClientNum, ushort unNewUnitID, int iFlags = 0)			{ return NULL; }

	virtual	void		AddCameraEffectAngleOffset(const CVec3f &v3Angles)		{}
	virtual void		AddCameraEffectOffset(const CVec3f &v3Position)			{}
	virtual CCamera*	GetCamera() const										{ return NULL; }
	virtual void		AddOverlay(const CVec4f &v4Color)						{}
	virtual void		AddOverlay(const CVec4f &v4Color, ResHandle hMaterial)	{}

	virtual tstring		GetPropType(const tstring &sPath) const	{ return _T(""); }

	virtual void	SendMessage(const tstring &sMsg, int iClientNum)									{}
	virtual void	SendGameData(int iClient, const IBuffer &buffer, bool bReliable)					{}
	virtual void	BroadcastGameData(const IBuffer &buffer, bool bReliable, int iExcludeClient = -1)	{}

	virtual	CStateString&	GetStateString(uint uiID) = 0;

	virtual uint			GetServerFrame() = 0;
	virtual uint			GetServerTime() = 0;
	virtual uint			GetPrevServerTime() = 0;
	virtual uint			GetServerFrameLength() = 0;

	virtual void			RegisterEntityScript(uint uiEntity, const tstring &sName, const tstring &sScript)		{}
	virtual void			RegisterGlobalScript(const tstring &sName, const tstring &sScript)						{}
	virtual bool			TriggerEntityScript(uint uiEntity, const tstring &sName)								{ return true; }
	virtual bool			TriggerGlobalScript(const tstring &sName)												{ return true; }

	virtual bool			TriggerScript(const tstring &sName) = 0;

	virtual void			CopyEntityScripts(uint uiIndex, uint uiTargetIndex)										{}
	virtual void			ClearEntityScripts(uint uiIndex)														{}
	virtual void			ClearAllEntityScripts()																	{}
	virtual void			ClearGlobalScripts()																	{}

	virtual void			RegisterTriggerParam(const tstring &sName, const tstring &sValue)						{}

	virtual ushort			GetRandomItem(IVisualEntity *pEntity = NULL)											{ return INVALID_ENT_TYPE; }
	virtual ushort			GetRandomPersistantItem()																{ return 0; }

	virtual void			MatchStatEvent(int iClientNumber, EPlayerMatchStat eStat, float fValue, int iTargetClientID = -1, ushort unInflictorType = INVALID_ENT_TYPE, ushort unTargetType = INVALID_ENT_TYPE, uint uiTime = INVALID_TIME)	{}
	virtual void			MatchStatEvent(int iClientNumber, EPlayerMatchStat eStat, int iValue, int iTargetClientID = -1, ushort unInflictorType = INVALID_ENT_TYPE, ushort unTargetType = INVALID_ENT_TYPE, uint uiTime = INVALID_TIME)		{}

	// Alan's pathing
	virtual PoolHandle	FindPath(uint uiWorldEntityIndex, const CVec2f &goal) const							{ return INVALID_POOL_HANDLE; }
	virtual PoolHandle	FindPath(const CVec2f &v2Src, float fEntityWidth, uint uiNavigationFlags, const CVec2f &v2Goal) const	{ return INVALID_POOL_HANDLE; }
	virtual CPath*		AccessPath(PoolHandle hPath) const													{ return NULL; }
	virtual void		FreePath(PoolHandle hPath) const													{}
	virtual void		BlockPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight)		{}
	virtual void		BlockPath(uint uiFlags, const CConvexPolyhedron &cSurf, float fStepHeight)								{}
	virtual void		ClearPath(uint uiFlags, const CVec2f &v2Position, float fWidth, float fHeight)		{}
	virtual void		ClearPath(uint uiFlags, const CConvexPolyhedron &cSurf)								{}

	virtual int			GetConnectedClientCount(int iTeam = -1)												{ return 0; }
};
//=============================================================================

/*====================
  IGame::~IGame
  ====================*/
IGame::~IGame()
{
}


/*====================
  IGame::IGame
  ====================*/
inline
IGame::IGame() :
m_eGamePhase(GAME_PHASE_INVALID),
m_uiPhaseStartTime(0),
m_uiPhaseDuration(-1),
m_bSuddenDeath(false),

m_uiGameTime(0),
m_pWorld(NULL),
m_pEntityDirectory(NULL),

m_iLastWinningTeam(-1)
{
}


/*====================
  IGame::Validate
  ====================*/
inline
bool	IGame::Validate() const
{
	try
	{
		if (m_pWorld == NULL)
			EX_ERROR(_T("Invalid world pointer"));
		
		if (m_pEntityDirectory == NULL)
			EX_ERROR(_T("Invalid entity directory pointer"));

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IGame::Validate() - "));
		return false;
	}
}


/*====================
  IGame::SetGamePhase
  ====================*/
inline
void	IGame::SetGamePhase(EGamePhase ePhase, uint uiLength, uint uiStartTime)
{
	m_eGamePhase = ePhase;
	m_uiPhaseStartTime = (uiStartTime == -1) ? m_uiGameTime : uiStartTime;
	m_uiPhaseDuration = uiLength;

	Console << _T("SetGamePhase(): ") << m_eGamePhase
		<< _T(" start: ") << m_uiPhaseStartTime
		<< _T(" length: ") << m_uiPhaseDuration
		<< _T(" now: ") << m_uiGameTime
		<< newl;

	if (IsServer())
	{
		RegisterTriggerParam(_T("phase"), XtoA(m_eGamePhase));
		RegisterTriggerParam(_T("start"), XtoA(m_uiPhaseStartTime));
		RegisterTriggerParam(_T("length"), XtoA(m_uiPhaseDuration));
		TriggerGlobalScript(_T("phasechange"));
	}
}


/*====================
  IGame::GetCurrentGameLength
  ====================*/
inline
uint	IGame::GetCurrentGameLength()
{
	if (GetGameTime() == -1 || GetGamePhase() != GAME_PHASE_ACTIVE)
		return 0;
	else
		return GetGameTime() - GetPhaseStartTime();
}

#endif //__I_GAME_H__
