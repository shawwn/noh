// (C)2005 S2 Games
// c_gameclient.h
//
//=============================================================================
#ifndef __C_GAMECLIENT_H__
#define __C_GAMECLIENT_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_cliententity.h"


#include "../game_shared/i_game.h"
#include "../game_shared/i_gameentity.h"
#include "../game_shared/c_playercommander.h"
#include "../game_shared/c_entityclientinfo.h"

#include "../k2/c_clientsnapshot.h"
#include "../k2/c_snapshot.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CGameClient;

class CHostClient;
class CWorld;
class CCamera;
class IGameEntity;
class IPlayerEntity;
class CSnapshot;
class CClientEntityDirectory;
class CGameInterfaceManager;
class CPlayerCommander;
class CClientCommander;
class CEntityGameInfo;
class CEntityEffect;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define GameClient (*CGameClient::GetCurrentClientGamePointer())

enum EClientEventTarget
{
	CG_EVENT_TARGET_ENTITY,
	CG_EVENT_TARGET_HANDS
};

enum EGameInterface
{
	CG_INTERFACE_NONE,
	CG_INTERFACE_MESSAGE,
	CG_INTERFACE_LOBBY,
	CG_INTERFACE_LOADOUT,
	CG_INTERFACE_SACRIFICED,
	CG_INTERFACE_PLAYER,
	CG_INTERFACE_DEAD,
	CG_INTERFACE_PLAYER_INFO,
	CG_INTERFACE_PLAYER_BUILD,
	CG_INTERFACE_PLAYER_OFFICER,
	CG_INTERFACE_COMMANDER,
	CG_INTERFACE_COMMANDER_INFO,
	CG_INTERFACE_GAME_OVER,
	CG_INTERFACE_SPAWN,
	CG_INTERFACE_MENU,
	CG_INTERFACE_PURCHASE,
	CG_INTERFACE_OBSERVER,
	CG_INTERFACE_STANDBY,

	NUM_CG_INTERFACES
};

typedef deque<CClientSnapshot>			ClientSnapshotDeque;
typedef ClientSnapshotDeque::iterator	ClientSnapshotDeque_it;

struct SOverlayInfo
{
	ResHandle	hMaterial;
	CVec4f		v4Color;
};

struct SLittleTextPopupMessage
{
	tstring		sText;
	CVec3f		V3Pos;
	uint		uiSpawnTime;
	CVec4f		v4Color;
};

struct SMinimapPing
{
	ResHandle	hTexture;
	CVec2f		v2Pos;
	uint		uiSpawnTime;
	CVec4f		v4Color;
};

enum EClientGameEffect
{
	CGFX_PING = 0,
	NUM_CLIENT_GAME_EFFECT_CHANNELS
};

const tstring g_sGameEndAwardNames[] =
{
	_T("Hero"),
	_T("Sadist"),
	_T("Veteran"),
	_T("Homewrecker"),
	_T("TeachersPet"),
	_T("Newbie"),
	_T("MotherTeresa"),
	_T("MMORPGFan"),
	_T("Vegan"),
	_T("Feeder"),
	_T("Ghandi"),
	_T("Entrepreneur")
};

enum EGameEndAwardNames
{
	AWARD_HERO,
	AWARD_SADIST,
	AWARD_VETERAN,
	AWARD_HOMEWRECKER,
	AWARD_TEACHERSPET,
	AWARD_NEWBIE,
	AWARD_MOTHERTERESA,
	AWARD_MMORPGFAN,
	AWARD_VEGAN,
	AWARD_FEEDER,
	AWARD_GHANDI,
	AWARD_ENTREPRENEUR,

	NUM_END_GAME_AWARDS
};

const uint	NUM_CLIENT_GAME_EFFECTS(32);
const uint	NUM_CLIENT_GAME_EFFECT_THREADS(NUM_CLIENT_GAME_EFFECT_CHANNELS + NUM_CLIENT_GAME_EFFECTS);

const uint	VC_MIN_TIME(2000); // min time between using VC's
const uint	MIN_PING_TIME(5000); // min time between sending pings

// Sub-menu voice command options
struct VCSub
{
	tstring sDesc;
	ResHandle hSound;
	EButton eButton;
};

typedef map<tstring, VCSub>				VCSubMap;
typedef pair<tstring, VCSub>			VCSubPair;
typedef VCSubMap::iterator				VCSubMap_it;


// Main menu voice command categories
struct VCCategory
{
	tstring sDesc;
	EButton eButton;
	EVCType eType;
	map<EButton, VCSubMap> mapSubItems;
};

typedef map<EButton, VCCategory>		VCMap;
typedef pair<EButton, VCCategory>		VCPair;
typedef VCMap::iterator					VCMap_it;

// Sent as 16-bit, so increase if you add more buttons
const uint GAME_BUTTON_ACTIVATE_PRIMARY		(BIT(0));
const uint GAME_BUTTON_ACTIVATE_SECONDARY	(BIT(1));
const uint GAME_BUTTON_ACTIVATE_TERTIARY	(BIT(2));
const uint GAME_BUTTON_FORWARD				(BIT(3));
const uint GAME_BUTTON_BACK					(BIT(4));
const uint GAME_BUTTON_LEFT					(BIT(5));
const uint GAME_BUTTON_RIGHT				(BIT(6));
const uint GAME_BUTTON_UP					(BIT(7));
const uint GAME_BUTTON_DOWN					(BIT(8));
const uint GAME_BUTTON_SPRINT				(BIT(9));
const uint GAME_BUTTON_USE					(BIT(10));
const uint GAME_BUTTON_CTRL					(BIT(11));
const uint GAME_BUTTON_SHIFT				(BIT(12));
const uint GAME_BUTTON_CANCEL				(BIT(13));
const uint GAME_BUTTON_DASH					(BIT(14));
const uint GAME_BUTTON_ALL					(UINT_MAX);

const uint GAME_CMDR_BUTTON_EDGESCROLL_UP		(BIT(21));
const uint GAME_CMDR_BUTTON_EDGESCROLL_DOWN		(BIT(22));
const uint GAME_CMDR_BUTTON_EDGESCROLL_LEFT		(BIT(23));
const uint GAME_CMDR_BUTTON_EDGESCROLL_RIGHT	(BIT(24));
const uint GAME_CMDR_BUTTON_DRAGSCROLL			(BIT(25));
//=============================================================================

//=============================================================================
// CGameClient
//=============================================================================
class CGameClient : public IGame
{
private:
	CHostClient*			m_pHostClient;
	CClientEntityDirectory*	m_pClientEntityDirectory;
	CGameInterfaceManager*	m_pInterfaceManager;
	CCamera*				m_pCamera;

	CVec3f					m_v3CameraPosition;
	CVec3f					m_v3CameraAngles;
	uint					m_uiCameraFrame;
	uint					m_uiCameraIndex;

	CSnapshot				m_CurrentServerSnapshot;
	vector<PoolHandle>		m_vServerSnapshots;
	PoolHandle				m_hServerSnapshotFallback;
	uint					m_uiSnapshotBufferPos;

	// Interface
	bool					m_bShowInfoScreen;
	bool					m_bShowMenu;
	bool					m_bShowPurchase;
	bool					m_bShowLobby;
	bool					m_bShowBuildMenu;
	ResHandle				m_hAltInfo;
	IPlayerEntity*			m_pPreviewUnit;
	EGamePhase				m_eLastGamePhase;
	EGameInterface			m_eLastInterface;

	tstring					m_sLoadoutUnitMouseover;
	tstring					m_sBuildFailure;

	uint					m_uiLastTeamTipTime;

	CEntityClientInfo*		m_pLocalClient;
	CClientEntity*			m_pCurrentEntity;
	CClientCommander*		m_pClientCommander;
	EClientEventTarget		m_eEventTarget;

	vector<SLittleTextPopupMessage>		m_vLittleTextPopupMessage;
	ResHandle				m_hLittleTextPopupFont;

	vector<SMinimapPing>	m_vMinimapPing;

	uiset					m_setTransparent;

	ResHandle				m_hLocatorFont;
	ResHandle				m_hOfficerLocator;
	ResHandle				m_hWaypointLocator;
	ResHandle				m_hPetLocator;

	uint					m_uiGameTipExpireTime;

	int						m_iGameMatchID;

	// Input
	CClientSnapshot			m_CurrentClientSnapshot;
	CClientSnapshot			m_PendingClientSnapshot;
	ClientSnapshotDeque		m_deqClientSnapshots;

	ResHandle				m_hMinimapReference;
	ResHandle				m_hMinimapTexture;
	class CBitmap*			m_pMinimapBitmap; // UTTAR
	ResHandle				m_hSnapSample;
	ResHandle				m_hBuildingPlacedSample;
	ResHandle				m_hBuildingCannotPlaceSample;
	ResHandle				m_hSpawnportalPlacedSample[2];
	ResHandle				m_hMeleeHitUnitSample;
	ResHandle				m_hMeleeHitBuildingSample;
	ResHandle				m_hRangedHitSample;
	ResHandle				m_hRangedHitUnitSample;
	ResHandle				m_hRangedHitBuildingSample;
	uint					m_uiLastHitSoundNotificationTime;
	ResHandle				m_hMeleeHitEffect;
	ResHandle				m_hRangedHitEffect;
	ResHandle				m_hMeleeHitUnitEffect;
	ResHandle				m_hRangedHitUnitEffect;
	ResHandle				m_hMeleeHitBuildingEffect;
	ResHandle				m_hRangedHitBuildingEffect;
	ResHandle				m_hOrderMoveSample[2];
	ResHandle				m_hOrderAttackSample[2];
	ResHandle				m_hOrderAttackUnitSample[2];
	ResHandle				m_hOrderAttackBuildingSample[2];
	ResHandle				m_hOrderDefendBuildingSample[2];
	ResHandle				m_hOrderRepairBuildingSample[2];
	ResHandle				m_hPingSample;
	ResHandle				m_hBuildingAttackedSample;
	ResHandle				m_hGoldMineLowSample;
	ResHandle				m_hGoldMineDepletedSample;
	ResHandle				m_hPingTexture;
	ResHandle				m_hMinimapTriangle;
	ResHandle				m_hFogofWarCircle;
	ResHandle				m_hKillSample;
	ResHandle				m_hAssistSample;
	ResHandle				m_hRazedSample;
	ResHandle				m_hCommanderChatSample;
	ResHandle				m_hItemPickupSample;

	ResHandle				m_hWaypointCommanderMoveEffect;
	ResHandle				m_hWaypointCommanderAttackEffect;
	ResHandle				m_hWaypointOfficerMoveEffect;
	ResHandle				m_hWaypointOfficerAttackEffect;

	ResHandle				m_hUpkeepNotification[8];
	ResHandle				m_hSuddenDeathSample;
	
	ResHandle				m_hKillStreakNotification[8];

	// melee feedback
	ResHandle				m_hBlockedFeedbackEffect;
	ResHandle				m_hGotBlockedFeedbackEffect;
	ResHandle				m_hInterruptedFeedbackEffect;
	ResHandle				m_hGotInterruptedFeedbackEffect;
	ResHandle				m_hMeleeHitFeedbackEffect;
	ResHandle				m_hGotHitByMeleeFeedbackEffect;
	ResHandle				m_hGotHitByMeleeFeedbackEffectNoKick;
	ResHandle				m_hMeleeImpactFeedbackEffect;

	uint					m_uiLastOrderNotifyTime;

	bool					m_bForceMouseHidden;
	bool					m_bMouseHidden;
	bool					m_bForceMouseCentered;
	bool					m_bMouseCentered;

	bool					m_bAllowMouseAim;
	bool					m_bAllowMovement;
	bool					m_bAllowAttacks;

	// Visual
	vector<SOverlayInfo>	m_vOverlays;
	CVec4f					m_v4ScreenEffectColor;
	uint					m_uiHoverEntity;
	uint					m_uiHoverEntityAcquiring;
	uint					m_uiHoverEntityAcquireTime;
	uint					m_uiHoverEntityDisplayTime;
	CVec3f					m_v3CameraEffectAngleOffset;
	CVec3f					m_v3CameraEffectOffset;
	uint					m_uiOrderEvent;
	byte					m_yOrderUniqueID;
	uint					m_uiOfficerOrderEvent;
	CEffectThread*			m_apEffectThreads[NUM_CLIENT_GAME_EFFECT_THREADS];

	// Sounds
	SoundHandle				m_ahSoundHandle[NUM_CLIENT_SOUND_HANDLES];

	uint					m_uiLastUpdateCheck;

	// Local vault items
	SPersistantItemVault	m_sItemVault;

	// World Sounds
	struct	SWorldSound
	{
		SoundHandle	hSound;
		ResHandle	hSample;
		uint		uiNextStartTime;
		SWorldSound(SoundHandle sound = INVALID_INDEX, ResHandle sample = INVALID_INDEX, uint start = 0) : hSound(sound), hSample(sample), uiNextStartTime(start) {}
	};
	typedef	map<int, SWorldSound>			SWorldSoundsMap;
	typedef SWorldSoundsMap::iterator		SWorldSoundsMap_it;
	SWorldSoundsMap							m_mapWorldSounds;
	SoundHandle								m_hWorldAmbientSound;

	// Building
	bool					m_bValidPosition;
	CVec3f					m_v3BuildPosition;
	CVec3f					m_v3BuildAngles;
	uint					m_uiBuildFoundation;
	IGadgetEntity*			m_pPreviewGadget;
	bool					m_bAppliedEffect;
	bool					m_bBuildingRotate;

	// Pinging
	bool					m_bPinging;
	bool					m_bPingEffectActive;
	byte					m_yPingSquad;
	uint					m_uiLastPingTime;

	bool					m_bAutoRun;
	uint					m_uiLastForwardPress;
	
	// Snapcast
	struct SSnapcastSettings
	{
		bool	bActive;
		float	fTraceSize;
		float	fDist;
		float	fCumulativeDecay;
		float	fCumulativeBreakAngle;
		uint	uiRequireTime;
		float	fLerp;
		float	fBreakAngle;
		CVec3f	v3SelectColor;
	} m_sSnapcast;
	
	struct SSnapcastVars
	{
		uint	uiNextAquire;
		uint	uiLockedIndex;
		CVec3f	v3OldAngles;
		CVec3f	v3CumulativeDelta;
	} m_sSnap;
	
	struct SOrders
	{
		byte	yOrder;
		CVec3f	v3OrderPos;

		bool operator<(const SOrders &B) const
		{
			if (yOrder < B.yOrder)
				return true;
			else if (yOrder > B.yOrder)
				return false;

			if (v3OrderPos.x < B.v3OrderPos.x)
				return true;
			else if (v3OrderPos.x > B.v3OrderPos.x)
				return false;

			if (v3OrderPos.y < B.v3OrderPos.y)
				return true;
			else if (v3OrderPos.y > B.v3OrderPos.y)
				return false;
			
			if (v3OrderPos.z < B.v3OrderPos.z)
				return true;
			else if (v3OrderPos.z > B.v3OrderPos.z)
				return false;

			return false;
		}
	};

	// Interface
	CBufferDynamic			m_bufEndGameData;

	// Voice chat
	class CVoiceManager*	m_pVoiceManager;
	ICvar*					m_pSoundVoiceMicMuted;

	// Voice commands
	VCMap					m_mapVC;
	bool					m_bVCActive;
	EButton					m_VCSubActive;
	map<uint, uint>			m_mapVoiceMarkers;
	uint					m_uiNextVCTime;
	uint					m_uiVCUses;

	// First Person Weapons
	int						m_iActiveFirstPersonAnim;
	byte					m_yActiveFirstPersonAnimSequence;

	ResHandle				m_hFirstPersonModelHandle;
	CEffectThread*			m_apFirstPersonEffectThread[NUM_CLIENT_EFFECT_THREADS];
	SoundHandle				m_ahFirstPersonSoundHandle[NUM_CLIENT_SOUND_HANDLES];
	vector<CEffectThread *>	m_vFirstPersonEffects;

	bool					m_bWasCommander;

	// Scripts
	smaps					m_mapScripts;
	smaps					m_mapScriptParams;

	// Prop type
	ResHandle				m_hPropTypeStringTable;

	vector<IVisualEntity *>	m_vVision;

	void		Snapcast();
	void		CommanderSnapcast();
	void		DrawAreaCast();
	void		UpdateGameTips();
	void		UpdateHoverEntity();
	void		UpdateOrders();
	void		DrawAltInfo();
	void		DrawVoiceInfo();
	void		UpdatePetInfo();
	void		UpdateLoadout();

	void		DrawLittleTextPopupMessages();
	void		DrawOrderLocator();
	void		DrawOfficerLocator();
	void		DrawPetLocator();
	void		DrawLocator(const CVec3f &v3Pos, ResHandle hLocator, const CVec4f &v4LocatorColor, float fLocatorSize, const tstring &sText, const CVec4f &v4TextColor);
	void		DrawMeleeTraces();

	void		DrawFogofWar();

	void		UpdateClientGameEffect(int iChannel, bool bActive, const CVec3f &v3Pos);
	void		AddClientGameEffects();

	void		UpdatePinging();

	void		SendClientSnapshot();

	void		StopWorldSounds();
	void		WorldSoundsFrame();

	void		SetupFrame();
	void		ActiveFrame();
	void		EndedFrame();
	void		BackgroundFrame();

	void		PrecacheEntities();

	void		ProcessVoicePacket(CPacket &pkt);

	void		DrawPlayerStats();
	void		DrawHoverStats();

	void		SetFirstPersonModelHandle(ResHandle hFirstPersonModelHandle);
	void		UpdateFirstPerson();
	void		RenderFirstPerson();
	void		RenderSelectedPlayerView(uint uiPlayerIndex);

	void		UpdateVision();

public:
	~CGameClient();
	CGameClient();

	bool				IsClient()										{ return true; }
	int					GetLocalClientNum();

	static CGameClient*	GetCurrentClientGamePointer()					{ return static_cast<CGameClient*>(GetCurrentGamePointer()); }

	// API Functions
	void				SetGamePointer()								{ IGame::SetCurrentGamePointer(this); }
	bool				Initialize(CHostClient *pHostClient);
	void				Reinitialize();
	bool				LoadWorld();
	bool				LoadResources();
	void				PreFrame();
	void				Frame();
	uint				Shutdown();
	bool				ProcessGameEvents(CSnapshot &snapshot);
	bool				ProcessSnapshot(CSnapshot &snapshot);
	bool				ProcessGameData(CPacket &pkt);
	void				PrecacheAll();
	void				Connect(const tstring &sAddr);
	
	bool				IsEntityHoverSelected(uint uiIndex);
	bool				LooksLikeEnemy(uint uiIndex);

	void				SendGameData(const IBuffer &buffer, bool bReliable);

	void				SendGameData(int iClientNum, const IBuffer &buffer, bool bReliable)	{ SendGameData(buffer, bReliable); }

	void				SetCurrentEntity(CClientEntity *pCurrentEntity)	{ m_pCurrentEntity = pCurrentEntity; }
	void				SetEventTarget(EClientEventTarget eTarget)		{ m_eEventTarget = eTarget; }

	bool					InterfaceNeedsUpdate();
	EGameInterface			GetCurrentInterface() const;
	bool					ToggleInfoScreen(const tstring &sTab = SNULL);
	void					ShowInfoScreen(const tstring &sTab = SNULL);
	void					ToggleMenu()									{ m_bShowMenu = !m_bShowMenu; }
	void					HideMenu()										{ m_bShowMenu = false; }
	void					TogglePurchase()								{ m_bShowPurchase = !m_bShowPurchase; }
	void					HidePurchase()									{ m_bShowPurchase = false; }
	void					ToggleLobby()									{ m_bShowLobby = !m_bShowLobby; }
	void					HideLobby()										{ m_bShowLobby = false; }
	void					ShowBuildMenu()									{ m_bShowBuildMenu = true; }
	void					HideBuildMenu()									{ m_bShowBuildMenu = false; }
	bool					IsBuildMenuVisible() const						{ return m_bShowBuildMenu; }
	CGameInterfaceManager*	GetInterfaceManager()							{ return m_pInterfaceManager; }


	// Input
	CClientSnapshot*	GetCurrentSnapshot()							{ return &m_CurrentClientSnapshot; }
	void				SelectItem(int iSlot);
	void				InvNext();
	void				InvPrev();
	bool				TraceCursor(STraceInfo &trace, int iIgnoreSurface);
	void				Cancel();

	// Resources
	ResHandle			RegisterModel(const tstring &sPath);
	ResHandle			RegisterEffect(const tstring &sPath);

	// Visual
	void				AddOverlay(const CVec4f &v4Color, ResHandle hMaterial);
	void				AddOverlay(const CVec4f &v4Color)				{ AddOverlay(v4Color, g_ResourceManager.GetWhiteTexture()); }

	void				StartEffect(const tstring &sEffect, int iChannel, int iTimeNudge);
	void				StopEffect(int iChannel);
	void				PlaySound(const tstring &sSound, int iChannel, float fFalloff, float fVolume, int iSoundFlags, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, bool bOverride = true, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0);
	void				PlaySoundStationary(const tstring &sSound, int iChannel, float fFalloff, float fVolume);
	void				StopSound(int iChannel);

	void				PlayClientGameSound(const tstring &sSound, int iChannel, float fVolume, int iSoundFlags, int iFadeIn = 0, int iFadeOutStartTime = 0, int iFadeOut = 0, bool bOverride = true, int iSpeedUpTime = 0, float fSpeed1 = 1.0, float fSpeed2 = 1.0, int iSlowDownTime = 0);
	void				StopClientGameSound(int iChannel);

	int					StartClientGameEffect(const tstring &sEffect, int iChannel, int iTimeNudge, const CVec3f &v3Pos);
	void				StopClientGameEffect(int iChannel);

	CEntityClientInfo*	GetLocalClient() const							{ return m_pLocalClient; }
	IPlayerEntity*		GetLocalPlayer() const							{ return (m_pLocalClient == NULL) ? NULL : m_pLocalClient->GetPlayerEntity(); }
	IPlayerEntity*		GetLocalPlayerCurrent() const					{ if (GetLocalPlayer() == NULL) return NULL; IVisualEntity *pEntity(GetClientEntityCurrent(GetLocalPlayer()->GetIndex())); return (pEntity == NULL) ? NULL : pEntity->GetAsPlayerEnt(); }
	IPlayerEntity*		GetLocalPlayerNext() const						{ if (GetLocalPlayer() == NULL) return NULL; IVisualEntity *pEntity(GetClientEntityNext(GetLocalPlayer()->GetIndex())); return (pEntity == NULL) ? NULL : pEntity->GetAsPlayerEnt(); }
	uint				GetLocalPlayerIndex() const						{ return (m_pLocalClient == NULL) ? INVALID_INDEX : m_pLocalClient->GetPlayerEntityIndex(); }
	CCamera*			GetCamera() const								{ return m_pCamera; }
	bool				IsCommander() const;
	CClientCommander*	GetClientCommander() const						{ if (!IsCommander()) return NULL; return m_pClientCommander; }
	CClientEntity*		GetClientEntity(uint uiIndex) const;
	IVisualEntity*		GetClientEntityCurrent(uint uiIndex) const;
	IVisualEntity*		GetClientEntityNext(uint uiIndex) const;
	IVisualEntity*		GetClientEntityPrev(uint uiIndex) const;
	IPlayerEntity*		GetPlayer(int iClientNum) const;
	IPlayerEntity*		GetPlayerByName(const tstring &sName) const;
	uint				GetNumClients() const;

	// Interface
	void				UpdateMinimap();
	void				UpdateMinimapPlayerWaypoint(IPlayerEntity *pPlayer);
	void				UpdateMinimapPings();
	void				UpdateMinimapTexture();

	void				ForceInterfaceRefresh();

	// Camera
	void				AddCameraEffectAngleOffset(const CVec3f &v3Angles)	{ m_v3CameraEffectAngleOffset += v3Angles; } 
	void				AddCameraEffectOffset(const CVec3f &v3Position)		{ m_v3CameraEffectOffset += v3Position; }

	// Building placement
	bool				IsBuildModeActive();
	void				StartBuildingPlacement(ushort unType);
	bool				TryBuildingPlacement();
	bool				StopBuildingPlacement();
	void				UpdateBuildingPlacement(IBuildingEntity *pBuilding);

	void				SetBuildingUpkeepLevel(uint uiIndex, float fLevel);
	void				ToggleBuildingUpkeep(uint uiIndex);
	void				RotateBuildingPlacement(float fValue);

	void				PromotePlayer(int iClientID);
	void				DemotePlayer(byte ySquad);
	void				SetVote(int iVote);

	void				SetPreviewUnit(ushort unID);
	bool				IsUnitAffordable(const tstring &sName);
	bool				IsUnitAvailable(const tstring &sName);
	tstring				GetUnitCost(const tstring &sName);
	int					GetUnitsInService(const tstring &sName);
	int					GetUnitsInServiceSquad(const tstring &sName);

	virtual void		SendMessage(const tstring &sMsg, int iClientNum);

	void				SpawnLittleTextPopupMessage(const tstring &sText, const CVec3f &v3Pos, const CVec4f &v4Color);
	void				SpawnMinimapPing(ResHandle hTexture, const CVec2f &v2Pos, const CVec4f &v4Color, bool bPlaySound = true);

	void				HitFeedback(EHitFeedbackType eType, uint uiIndex);
	void				HitFeedback(EHitFeedbackType eType, const CVec3f &v3Pos);

	void				StartPinging(byte ySquad = -1)		{ m_bPinging = true; m_yPingSquad = ySquad; }
	bool				StopPinging()						{ if (!m_bPinging) return false; m_bPinging = false; return true; }
	uint				GetLastPingTime() const				{ return m_uiLastPingTime; }
	void				SetLastPingTime(uint uiTime)		{ m_uiLastPingTime = uiTime; }

	void				ToggleAutoRun();
	void				BreakAutoRun();
	bool				CheckDash();
	bool				GetAutoRun() const					{ return m_bAutoRun; }

	virtual CStateString&	GetStateString(uint uiID);

	virtual uint		GetServerFrame();
	virtual uint		GetServerTime();
	virtual uint		GetPrevServerTime();
	virtual uint		GetServerFrameLength();

	void				PetCommand(EPetCommand ePetCmd);
	void				OfficerCommand(EOfficerCommand eOfficerCmd);

	void				SetGameMatchID(int iMatchID)		{ m_iGameMatchID = iMatchID; }
	int					GetGameMatchID() const				{ return m_iGameMatchID; }

	void				StartVoiceRecording();
	void				StopVoiceRecording();

	void				StoppedTalking(int iClientNum);
	void				StartedTalking(int iClientNum);
	bool				IsTalking(int iClientNum);
	bool				IsMuted(int iClientNum);
	void				SetVoiceMute(int iClientNum, bool bValue);
	
	int					StartFirstPersonEffect(ResHandle hEffect, int iChannel, int iTimeNudge);
	int					StartFirstPersonEffect(const tstring &sEffect, int iChannel, int iTimeNudge);
	void				StopFirstPersonEffect(int iChannel);
	void				PushFirstPersonEffect(CEffectThread *pEffectThread);

	void				CheckVoiceCvars();

	// Voice commands
	bool				AddVCCategory(EButton button, const tstring &sType, const tstring &sDesc);
	VCCategory*			GetVCCategory(EButton button);
	VCCategory*			GetActiveVCCategory();
	bool				AddVCSubItem(VCCategory *pCategory, EButton button, const tstring &sRace, const tstring &sDesc, const tstring &sPath);
	VCMap*				GetVCMap()							{ return &m_mapVC; }

	bool				IsVCMenuActive()					{ return m_bVCActive; }
	bool				IsVCSubMenuActive()					{ return (m_VCSubActive != BUTTON_INVALID); }
	void				VCMenuActive(bool bValue)			{ m_bVCActive = bValue; }
	void				VCSubMenuActive(EButton button);

	void				DoVoiceCommand(EButton button);

	// Mouse settings
	bool				ForceMouseHidden()				{ return m_bForceMouseHidden; }
	void				ForceMouseHidden(bool bValue)	{ m_bForceMouseHidden = bValue; }
	bool				IsMouseHidden()					{ return m_bMouseHidden; }
	void				IsMouseHidden(bool bValue)		{ m_bMouseHidden = bValue; }

	bool				ForceMouseCentered()			{ return m_bForceMouseCentered; }
	void				ForceMouseCentered(bool bValue)	{ m_bForceMouseCentered = bValue; }
	bool				IsMouseCentered()				{ return m_bMouseCentered; }
	void				IsMouseCentered(bool bValue)	{ m_bMouseCentered = bValue; }

	bool				AllowMouseAim();
	void				AllowMouseAim(bool bValue)		{ m_bAllowMouseAim = bValue; }

	bool				AllowMovement();
	void				AllowMovement(bool bValue);

	bool				AllowAttacks()					{ return m_bAllowAttacks; }
	void				AllowAttacks(bool bValue)		{ m_bAllowAttacks = bValue; }

	bool				TriggerScript(const tstring &sName);
	
	bool				CanBuild(const tstring &sBuilding, bool bSetReason = false);

	void				AddPersistantItem(int iVaultNum, ushort unType, uint uiID);
	ushort				GetPersistantItemType(int iVaultNum)		{ return m_sItemVault.unItemType[iVaultNum]; }
	uint				GetPersistantItemID(int iVaultNum)			{ return m_sItemVault.uiItemID[iVaultNum]; }

	void				SetReplayClient(int iClientNum);

	void				UpdateTeamRosters(int iClientNum, int iNewTeam, byte ySquad, bool bOfficer, bool bCommander);

	void				DoUpkeepEvent(bool bFailed, int iTeam);
	void				SuddenDeathAlert();

	void				SetBuildingRotate(bool bRotate)	{ m_bBuildingRotate = bRotate; }
	bool				GetBuildingRotate()				{ return m_bBuildingRotate; }

	void				NextReplayClient();
	void				PrevReplayClient();

	tstring				GetPropType(const tstring &sPath) const;

	void				SetLoadoutUnitMouseover(const tstring &sUnit)		{ m_sLoadoutUnitMouseover = sUnit; }
	tstring				GetLoadoutUnitMouseover()							{ return m_sLoadoutUnitMouseover; }

	bool				TeamHasBuilding(int iTeam, const tstring &sBuilding);
	bool				CanJoinTeam(int iTeam);

	tstring				GetTerrainType();

	const vector<IVisualEntity *>&	GetVision() const						{ return m_vVision; }

	int					GetConnectedClientCount(int iTeam = -1);

	void				SetHoverEntity(uint uiHoverEntity)			{ m_uiHoverEntity = uiHoverEntity; }
	uint				GetHoverEntity() const						{ return m_uiHoverEntity; }

	void				RemoveClient(int iClientNum);

	void				ClearTransparentEntities();
	void				AddTransparentEntity(uint uiIndex)			{ m_setTransparent.insert(uiIndex); }
};
//=============================================================================

#endif //__C_GAMECLIENT_H__
