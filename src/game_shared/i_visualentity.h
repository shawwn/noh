// (C)2007 S2 Games
// i_visualentity.h
//
//=============================================================================
#ifndef __I_VISUALENTITY_H__
#define __I_VISUALENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
#include "c_entityevent.h"
#include "i_inventoryitem.h"

#include "../k2/s_traceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IVisualEntity;
class IPropEntity;
class IPlayerEntity;
class IProjectile;
class ILight;
class IGadgetEntity;
class IBuildingEntity;
class INpcEntity;
class IPetEntity;
class ICombatEntity;
class ITriggerEntity;

class IGame;
class CSceneEntity;
class CBufferDynamic;
class CSkeleton;
class IEntityState;
class CWorldEntity;
class CSkeleton;
class CEffectThread;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint	ENTITY_EVENT_BUFFER_LENGTH(4);
const uint	MAX_ACTIVE_ENTITY_STATES(32);
const uint	NUM_EFFECT_CHANNELS(4);
const uint	NUM_CLIENT_EFFECTS(32);
const uint	NUM_CLIENT_EFFECT_THREADS(NUM_EFFECT_CHANNELS + NUM_CLIENT_EFFECTS);

// Effect channels
const uint EFFECT_CHANNEL_BEAM_THIRD_PERSON		(0);
const uint EFFECT_CHANNEL_BEAM_FIRST_PERSON		(1);

const uint EFFECT_CHANNEL_BUILDING_LOW_HEALTH	(0);

const uint EFFECT_CHANNEL_PROJECTILE_TRAIL		(0);

const uint EFFECT_CHANNEL_HELD_ITEM				(0);
const uint EFFECT_CHANNEL_BLOOD_IMPACT			(1);
const uint EFFECT_CHANNEL_BLOOD_SPRAY			(2);
const uint EFFECT_CHANNEL_TRIGGER				(3);
const uint EFFECT_CHANNEL_VOICECOMMAND			(3);
const uint EFFECT_CHANNEL_SOUL					(3);
////

enum EEntityStatus
{
	ENTITY_STATUS_PREVIEW,
	ENTITY_STATUS_DORMANT,
	ENTITY_STATUS_ACTIVE,
	ENTITY_STATUS_DEAD,
	ENTITY_STATUS_CORPSE,
	ENTITY_STATUS_SPAWNING,
	ENTITY_STATUS_HIDDEN,
	
	NUM_ENTITY_STATUSES
};

struct SWorkingMovementVars
{
	float		fFrameTime;
	CPlane		plGround;
	float		fRunSpeed;
	CVec3f		v3OldPosition;
	CVec3f		v3OldVelocity;
	CVec3f		v3OldAngles;
	CVec3f		v3Position;
	CVec3f		v3Velocity;
	CVec3f		v3Angles;
	CVec3f		v3Intent;
	bool		bGroundControl;
	int			iMoveFlags;
	bool		bLanded;
};

struct SDamageRecord
{
	float	fDamage;
	uint	uiTime;

	SDamageRecord() :
	fDamage(0.0f),
	uiTime(0)
	{}

	SDamageRecord(float f, uint ui) :
	fDamage(f),
	uiTime(ui)
	{}
};

// Damage flags
const int DAMAGE_FLAG_MELEE			(BIT(0));
const int DAMAGE_FLAG_PIERCE		(BIT(1));
const int DAMAGE_FLAG_SLASH			(BIT(2));
const int DAMAGE_FLAG_CRUSH			(BIT(3));
const int DAMAGE_FLAG_FIRE			(BIT(4));
const int DAMAGE_FLAG_POISON		(BIT(5));
const int DAMAGE_FLAG_EXPLOSIVE		(BIT(6));
const int DAMAGE_FLAG_SIEGE			(BIT(7));
const int DAMAGE_FLAG_MAGIC			(BIT(8));
const int DAMAGE_FLAG_BLOCKABLE		(BIT(9));
const int DAMAGE_FLAG_INTERRUPT		(BIT(10));
const int DAMAGE_FLAG_SPLASH		(BIT(11));
const int DAMAGE_FLAG_DIRECT		(BIT(12));
const int DAMAGE_FLAG_COLD			(BIT(13));
const int DAMAGE_FLAG_SILENT		(BIT(14));
const int DAMAGE_FLAG_HOLY			(BIT(15));
const int DAMAGE_FLAG_EVIL			(BIT(16));
const int DAMAGE_FLAG_GUN			(BIT(17));
const int DAMAGE_FLAG_SPELL			(BIT(18));
const int DAMAGE_FLAG_STATE			(BIT(19));
const int DAMAGE_FLAG_MELEE_NOTIFY	(BIT(20));
const int DAMAGE_FLAG_SKILL			(BIT(21));
const int DAMAGE_FLAG_PROJECTILE	(BIT(22));

const int DAMAGE_FLAG_NOTIFY	(DAMAGE_FLAG_MELEE | DAMAGE_FLAG_GUN | DAMAGE_FLAG_SIEGE);

const byte ENTITY_INVALID_ANIM	(-1);
const byte ENTITY_STOP_ANIM		(-2);

// Entity Client Render Flags
const int ECRF_SNAPSELECTED			(BIT(0));
const int ECRF_HALFTRANSPARENT		(BIT(1));

const int MOVE_ON_GROUND	(BIT(0));
const int MOVE_JUMP_HELD	(BIT(1));
const int MOVE_JUMPING		(BIT(2));
const int MOVE_ATTACK_HELD	(BIT(3));

const uint ENT_NET_FLAG_SACRIFICED				(BIT(0));
const uint ENT_NET_FLAG_KILLED					(BIT(1));
const uint ENT_NET_FLAG_REVEALED				(BIT(2));
const uint ENT_NET_FLAG_UPKEEP_FAILED			(BIT(3));
const uint ENT_NET_FLAG_BUILD_MODE				(BIT(4));
const uint ENT_NET_FLAG_SACRIFICE_MENU			(BIT(5));
const uint ENT_NET_FLAG_SILENCED				(BIT(6));
const uint ENT_NET_FLAG_NO_CORPSE				(BIT(7));
const uint ENT_NET_FLAG_CHANNELING				(BIT(8));
const uint ENT_NET_FLAG_QUEUED					(BIT(9));
#define ENT_NET_FLAG_COMMANDER_SELECTED(team)	(BIT(10) << ((team) - 1))
// BIT(11) reserved for commander selection
const uint ENT_NET_FLAG_NO_RESURRECT			(BIT(12));
const uint ENT_NET_FLAG_NO_REPAIR               (BIT(13));
const uint ENT_NET_FLAG_SIGHTED					(BIT(14));

const uint ENT_LOCAL_FLAG_DELETE_NEXT_FRAME	(BIT(0));
const uint ENT_LOCAL_FLAG_CHANGING_UNIT		(BIT(1));

enum EInventory
{
	INVENTORY_AUTO_BACKPACK = -1,
	INVENTORY_START_BACKPACK = 10,
	INVENTORY_END_BACKPACK = INVENTORY_START_BACKPACK + 6,
	INVENTORY_PETCMD_ATTACK = INVENTORY_END_BACKPACK,
	INVENTORY_PETCMD_FOLLOW,
	INVENTORY_PETCMD_MOVE,
	INVENTORY_OFFICERCMD_ATTACK,
	INVENTORY_OFFICERCMD_FOLLOW,
	INVENTORY_OFFICERCMD_MOVE,
	INVENTORY_OFFICERCMD_DEFEND,
	MAX_INVENTORY
};
//=============================================================================

//=============================================================================
// IVisualEntity
//=============================================================================
class IVisualEntity : public IGameEntity
{
private:
	static vector<SDataField>	*s_pvFields;

	static ResHandle	s_hSelectionIndicator;

	IVisualEntity();

protected:
	// Cvar settings
	START_ENTITY_CONFIG(IGameEntity)
		DECLARE_ENTITY_CVAR(tstring, Name)
		DECLARE_ENTITY_CVAR(tstring, Description)
		DECLARE_ENTITY_CVAR(tstring, Race)
		DECLARE_ENTITY_CVAR(tstring, ModelPath)
		DECLARE_ENTITY_CVAR(float, Scale)
		DECLARE_ENTITY_CVAR(float, CommanderScale)
		DECLARE_ENTITY_CVAR(float, MaxHealth)
		DECLARE_ENTITY_CVAR(float, HealthRegenRate)
		DECLARE_ENTITY_CVAR(uint, Cost)
		DECLARE_ENTITY_CVAR(float, ExperienceValue)
		DECLARE_ENTITY_CVAR(int, Bounty)
		DECLARE_ENTITY_CVAR(tstring, Prerequisite)
		DECLARE_ENTITY_CVAR(tstring, HitByMeleeEffectPath)
		DECLARE_ENTITY_CVAR(tstring, HitByRangedEffectPath)
		DECLARE_ENTITY_CVAR(tstring, IconPath)
		DECLARE_ENTITY_CVAR(tstring, MinimapIconPath)
		DECLARE_ENTITY_CVAR(int, MinimapIconSize)
		DECLARE_ENTITY_CVAR(tstring, LargeMapIconPath)
		DECLARE_ENTITY_CVAR(int, LargeMapIconSize)
		DECLARE_ENTITY_CVAR(float, SightRange)
		DECLARE_ENTITY_CVAR(bool, IsHidden)
		DECLARE_ENTITY_CVAR(tstring, GameTip)
		DECLARE_ENTITY_CVAR(tstring, CommanderPortraitPath)
		DECLARE_ENTITY_CVAR(float, EffectScale)
		DECLARE_ENTITY_CVAR(float, SelectionRadius)
		DECLARE_ENTITY_CVAR(float, PushMultiplier);
		DECLARE_ENTITY_CVAR(bool, SpawnPoint);
		DECLARE_ENTITY_CVAR(bool, SiegeSpawnPoint);
	END_ENTITY_BASE_CONFIG

	CEntityConfig*	m_pEntityConfig;

	tstring				m_sName;
	uint				m_uiWorldIndex;
	char				m_iTeam;
	byte				m_ySquad;

	byte				m_yStatus;

	uint				m_uiCreationTime;
	uint				m_uiCorpseTime;

	map<uint, SDamageRecord>	m_mapDamage;

	// Stats
	float				m_fHealth;
	bool				m_bInvulnerable;

	// Physics
	CVec3f				m_v3Position;
	CVec3f				m_v3Velocity;
	CVec3f				m_v3Angles;
	float				m_fScale;
	CBBoxf				m_bbBounds;
	uint				m_uiGroundEntityIndex;
	bool				m_bOnGround;

	// Events
	byte				m_yNextEventSlot;
	byte				m_yLastProcessedEvent;
	CEntityEvent		m_aEvents[ENTITY_EVENT_BUFFER_LENGTH];

	// States
	IEntityState*		m_apState[MAX_ACTIVE_ENTITY_STATES];

	uint				m_uiLocalFlags;	// Never transmitted

	// Inventory
	IInventoryItem*		m_apInventory[MAX_INVENTORY];

	// Visual
	uint				m_uiRenderFlags;
	ResHandle			m_hModel;
	CSkeleton*			m_pSkeleton;
	byte				m_ayAnim[NUM_ANIM_CHANNELS];
	byte				m_ayAnimSequence[NUM_ANIM_CHANNELS];
	float				m_afAnimSpeed[NUM_ANIM_CHANNELS];
	byte				m_yDefaultAnim;
	uint				m_auiAnimLockTime[NUM_ANIM_CHANNELS];

	ResHandle			m_ahEffect[NUM_EFFECT_CHANNELS];
	byte				m_ayEffectSequence[NUM_EFFECT_CHANNELS];

	// Client-side only
	uint				m_uiClientRenderFlags;
	CVec4f				m_v4SelectColor;
	CAxis				m_aAxis;
	CVec3f				m_v3AxisAngles;
	bool				m_bSighted;
	bool				m_bPrevSighted;
	CVec3f				m_v3SightedPos;
	ResHandle			m_hMinimapIcon;
	ResHandle			m_hLargeMapIcon;
	tstring				m_sTerrainType;
	uint				m_uiLastTerrainTypeUpdateTime;
	bool				m_bShowEffects;

	CVec4f				m_v4MinimapFlashColor;
	uint				m_uiMinimapFlashEndTime;

	void				AddSelectionRingToScene();

	// Slide-movement
#ifdef __GNUC__
	__attribute__ ((visibility("default")))
#endif
	static SWorkingMovementVars	s_Move;

	bool				Slide(bool bGravity);
	bool				StepSlide(bool bGravity);
	bool				IsPositionValid(const CVec3f &v3Position);
	void				CheckGround();
	void				UpdateStates();

public:
	virtual ~IVisualEntity();
	IVisualEntity(CEntityConfig *pConfig);

	// Accessors
	virtual bool		IsVisual() const					{ return true; }
	
	virtual bool		IsSelectable() const				{ return false; }
		
	virtual bool			HasAltInfo() const				{ return false; }
	virtual const tstring&	GetAltInfoName() const			{ return SNULL; }
	
	virtual bool		GetGameTip(IPlayerEntity *pPlayer, tstring &sMessage);

	const tstring&		GetName() const										{ return m_sName; }
	void				SetName(const tstring &sName)						{ m_sName = sName; }

	uint				GetWorldIndex() const								{ return m_uiWorldIndex; }
	void				SetWorldIndex(uint uiIndex)							{ m_uiWorldIndex = uiIndex; }

	int					GetTeam() const										{ return m_iTeam; }
	virtual	void		SetTeam(char cTeam)									{ m_iTeam = cTeam; }
	void				SetSquad(byte ySquad)								{ m_ySquad = ySquad; }
	byte				GetSquad() const									{ return m_ySquad; }
	
	float				GetHealth() const									{ return m_fHealth; }
	virtual float		GetMaxHealth() const								{ if (m_pEntityConfig == NULL) return 0.0f; return m_pEntityConfig->GetMaxHealth(); }
	float				GetHealthPercent() const							{ if (GetMaxHealth() == 0) return 1.0f; return m_fHealth / GetMaxHealth(); }
	float				GetBaseHealthRegen() const							{ if (m_pEntityConfig == NULL) return 0.0f; return m_pEntityConfig->GetHealthRegenRate(); }
	void				SetHealth(float fHealth)							{ m_fHealth = fHealth; }
	virtual float		Heal(float fHealth, IVisualEntity *pSource);

	tstring				GetHitByMeleeEffectPath() const						{ if (m_pEntityConfig == NULL) return _T(""); return m_pEntityConfig->GetHitByMeleeEffectPath(); }
	tstring				GetHitByRangedEffectPath() const					{ if (m_pEntityConfig == NULL) return _T(""); return m_pEntityConfig->GetHitByRangedEffectPath(); }

	byte				GetStatus() const									{ return m_yStatus; }
	void				SetStatus(byte yStatus)								{ m_yStatus = yStatus; }

	const CVec3f&		GetPosition() const									{ return m_v3Position; }
	void				SetPosition(const CVec3f &v3Pos)					{ m_v3Position = v3Pos; }
	void				SetPosition(float x, float y, float z)				{ m_v3Position = CVec3f(x, y, z); }

	const CVec3f&		GetAngles() const									{ return m_v3Angles; }
	void				SetAngles(const CVec3f &v3Angles)					{ m_v3Angles = v3Angles; }
	void				SetAngles(float fPitch, float fRoll, float fYaw)	{ m_v3Angles = CVec3f(fPitch, fRoll, fYaw); }

	const CVec3f&		GetVelocity() const									{ return m_v3Velocity; }
	void				SetVelocity(const CVec3f &v3Velocity)				{ m_v3Velocity = v3Velocity; }
	void				ApplyVelocity(const CVec3f &v3Velocity)				{ m_v3Velocity += v3Velocity; }

	GAME_SHARED_API float	GetScale2() const;

	float				GetScale() const									{ return m_fScale; }
	void				SetScale(float fScale)								{ m_fScale = fScale; }

	virtual ResHandle	GetModelHandle() const;
	void				SetModelHandle(ResHandle hModel)					{ m_hModel = hModel; }
	const tstring&		GetModelPath() const								{ if (m_pEntityConfig == NULL) return SNULL; return m_pEntityConfig->GetModelPath().GetValue(); }

	void				SetSkeleton(CSkeleton *pSkeleton)					{ m_pSkeleton = pSkeleton; }

	int					GetDefaultAnim()									{ return m_yDefaultAnim; }
	void				SetDefaultAnim(int iAnim)							{ m_yDefaultAnim = iAnim; }
	int					GetAnim(int iChannel)								{ return m_ayAnim[iChannel]; }
	void				SetAnim(int iChannel, int iAnim)					{ m_ayAnim[iChannel] = iAnim; }

	byte				GetAnimSequence(int iChannel)						{ return m_ayAnimSequence[iChannel]; }
	void				SetAnimSequence(int iChannel, byte ySequence)		{ m_ayAnimSequence[iChannel] = ySequence; }
	void				IncAnimSequence(int iChannel)						{ ++m_ayAnimSequence[iChannel]; }

	float				GetAnimSpeed(int iChannel)							{ return m_afAnimSpeed[iChannel]; }
	void				SetAnimSpeed(int iChannel, float fAnimSpeed)		{ m_afAnimSpeed[iChannel] = fAnimSpeed; }

	ResHandle			GetEffect(int iChannel)								{ return m_ahEffect[iChannel]; }
	void				SetEffect(int iChannel, ResHandle hEffect)			{ m_ahEffect[iChannel] = hEffect; }

	byte				GetEffectSequence(int iChannel)						{ return m_ayEffectSequence[iChannel]; }
	void				IncEffectSequence(int iChannel)						{ ++m_ayEffectSequence[iChannel]; }

	const CBBoxf&		GetBounds()											{ return m_bbBounds; }

	CSkeleton*			GetSkeleton()										{ return m_pSkeleton; }

	uint				GetGroundEntityIndex() const						{ return m_uiGroundEntityIndex; }
	bool				IsOnGround() const									{ return m_bOnGround; }

	map<uint, SDamageRecord>	GetDamageRecordMap()						{ return m_mapDamage; }
	void				SetDamageRecordMap(map<uint, SDamageRecord> mapRecord)	{ m_mapDamage = mapRecord; }

	// Network
	virtual void		Baseline();
	virtual void		GetSnapshot(CEntitySnapshot &snapshot) const;
	virtual bool		ReadSnapshot(CEntitySnapshot &snapshot);

	static const vector<SDataField>&	GetTypeVector();
	static void			ClientPrecache(CEntityConfig *pConfig);
	static void			ServerPrecache(CEntityConfig *pConfig);

	virtual void		ApplyWorldEntity(const CWorldEntity &ent);
	virtual void		RegisterEntityScripts(const CWorldEntity &ent);

	// Local flags
	// Clients maintain local flags in m_pCurrentState only
	void				CopyLocalFlags(IVisualEntity *pOther)				{ m_uiLocalFlags = pOther->m_uiLocalFlags; }
	void				SetLocalFlags(uint uiFlags)							{ m_uiLocalFlags |= uiFlags; }
	void				ToggleLocalFlags(uint uiFlags)						{ m_uiLocalFlags ^= uiFlags; }
	void				RemoveLocalFlags(uint uiFlags)						{ m_uiLocalFlags &= ~uiFlags; }
	void				ClearLocalFlags()									{ m_uiLocalFlags = 0; }
	bool				HasLocalFlags(uint uiFlags) const					{ return (m_uiLocalFlags & uiFlags) != 0; }
	bool				HasAllLocalFlags(uint uiFlags) const				{ return (m_uiLocalFlags & uiFlags) == uiFlags; }

	// Actions
	virtual void		Spawn();
	virtual void		GameStart()											{}
	virtual void		WarmupStart()										{}
	virtual bool		ServerFrame();
	virtual void		LocalClientFrame()									{}
	void				RecordDamageCredit(uint uiIndex, float fDamage);
	virtual float		Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);
	virtual void		Hit(CVec3f v3Pos, CVec3f v3Angle, EEntityHitByType eHitBy = ENTITY_HIT_BY_RANGED);
	virtual void		KillReward(IGameEntity *pKiller);
	virtual void		Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE)		{ KillReward(pAttacker); }
	virtual bool		Impact(STraceInfo &trace, IVisualEntity *pSource)	{ return true; }

	// Visual
	virtual CSkeleton*		AllocateSkeleton()					{ return NULL; }
	virtual void			UpdateSkeleton(bool bPose);
	GAME_SHARED_API void	StartAnimation(const tstring &sAnimName, int iChannel, float fSpeed = 1.0f, uint uiLength = 0);
	GAME_SHARED_API void	StopAnimation(int iChannel);
	GAME_SHARED_API void	StopAnimation(const tstring &sAnimName, int iChannel);
	GAME_SHARED_API void	LockAnimation(int iChannel, uint uiTime);
	GAME_SHARED_API int		GetAnimIndex(const tstring &sAnimName);
	virtual void			SetDefaultAnimation(const tstring &sAnimName);
	virtual bool			AddToScene(const CVec4f &v4Color, int iFlags);

	// States
	GAME_SHARED_API void	ClearStates();
	GAME_SHARED_API int		ApplyState(ushort unID, uint uiStartTime, uint uiDuration, uint uiInflictorIndex = INVALID_INDEX);
	GAME_SHARED_API void	RemoveState(int iSlot);

	// Inventory
	GAME_SHARED_API void			ClearInventory();
	GAME_SHARED_API void			RemoveItem(int iSlot);
	IInventoryItem*					GetItem(int iSlot) const					{ if (iSlot < 0 || iSlot >= MAX_INVENTORY) return NULL; else return m_apInventory[iSlot]; }
	GAME_SHARED_API	virtual int		GiveItem(int iSlot, ushort unID, bool bEnabled = true);
	GAME_SHARED_API tstring			GetItemIconPath(int iSlot)					{ if (GetItem(iSlot) == NULL) return _T("$black"); else return GetItem(iSlot)->GetIconPath(); }
	GAME_SHARED_API float			GetCooldownPercent(int iSlot) const;
	GAME_SHARED_API void			SwapItem(int iSlot1, int iSlot2);
	GAME_SHARED_API bool			CanCarryItem(ushort unID);
	
	// Client-side state management
	GAME_SHARED_API void	AddState(IEntityState *pState);
	GAME_SHARED_API void	ClearState(IEntityState *pState);
	
	IEntityState*			GetState(int iSlot)					{ if (iSlot < 0 || iSlot >= MAX_ACTIVE_ENTITY_STATES) return NULL; return m_apState[iSlot]; }
	GAME_SHARED_API uint	GetStateExpireTime(int iSlot);
	GAME_SHARED_API float	GetStateExpirePercent(int iSlot);

	void					SetState(int iSlot, IEntityState *pState)	{ if (iSlot < 0 || iSlot >= MAX_ACTIVE_ENTITY_STATES) return; m_apState[iSlot] = pState; }
	
	GAME_SHARED_API virtual bool	IsStealthed();
	GAME_SHARED_API bool	IsDisguised() const;
	GAME_SHARED_API int		GetDisguiseTeam() const;
	GAME_SHARED_API int		GetDisguiseClient() const;
	GAME_SHARED_API ushort	GetDisguiseItem() const;
	GAME_SHARED_API bool	IsIntangible() const;
	GAME_SHARED_API bool	IsSilenced() const;

	virtual bool			AIShouldTarget()			{ return true; }

	// Events
	void					AddEvent(EEntityEvent eEvent);

	virtual void			Copy(const IGameEntity &B);

	// Physics
	virtual void			Link()												{}
	virtual void			Unlink()											{}

	// Client-side
	void					AddClientRenderFlags(uint uiFlags)			{ m_uiClientRenderFlags |= uiFlags; }
	void					RemoveClientRenderFlags(uint uiFlags)		{ m_uiClientRenderFlags &= ~uiFlags; }
	const CAxis&			GetAxis() const								{ return m_aAxis; }
	bool					GetSighted() const							{ return m_bSighted; }
	void					SetSighted(bool bSighted)					{ m_bSighted = bSighted; }
	bool					GetPrevSighted() const						{ return m_bPrevSighted; }
	void					SetPrevSighted(bool bPrevSighted)			{ m_bPrevSighted = bPrevSighted; }
	const CVec3f&			GetSightedPos() const						{ return m_v3SightedPos; }
	void					SetSightedPos(const CVec3f &v3SightedPos)	{ m_v3SightedPos = v3SightedPos; }
	ResHandle				GetMinimapIcon() const						{ return m_hMinimapIcon; }
	ResHandle				GetLargeMapIcon() const						{ return m_hLargeMapIcon; }
	float					GetGunManaCost(float fManaCost) const;
	GAME_SHARED_API void	MinimapFlash(const CVec4f &v4Color, uint uiDuration);
	virtual	void			UpdateSighting(const vector<IVisualEntity *> &vVision);

	void					SetShowEffects(bool bShowEffects)			{ m_bShowEffects = bShowEffects; }
	bool					GetShowEffects() const						{ return m_bShowEffects; }

	virtual CVec4f			GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap);
	virtual float			GetMapIconSize(IPlayerEntity *pLocalPlayer, bool bLargeMap);
	virtual ResHandle		GetMapIcon(IPlayerEntity *pLocalPlayer, bool bLargeMap);
	virtual bool			IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap);
	virtual void			DrawOnMap(class CUITrigger &minimap, IPlayerEntity *pLocalPlayer, bool bLargeMap);

	void					SetSelectColor(const CVec4f &v4Color)		{ m_v4SelectColor = v4Color; }
	GAME_SHARED_API	bool	IsEnemy(IVisualEntity *pOther) const;
	GAME_SHARED_API bool	LooksLikeEnemy(IVisualEntity *pOther) const;
	bool					IsNeutral() const							{ return m_iTeam == 0; }

	virtual bool			CanTakeDamage(int iFlags, IVisualEntity *pAttacker = NULL);
	virtual bool			CanSee(IVisualEntity *pOther);

	virtual bool			IsSpawnLocation() const			{ return GetSpawnPoint(); }
	virtual bool			CanSpawnSiege() const			{ return GetSiegeSpawnPoint(); }
	virtual bool			CanSpawnFrom(IPlayerEntity *pPlayer);
	virtual void			PlayerSpawnedFrom(IPlayerEntity *pPlayer)	{}

	virtual bool			IsInvulnerable()				{ return m_bInvulnerable; }
	virtual void			SetInvulnerable(bool bValue)	{ m_bInvulnerable = bValue; }
	
	virtual void			Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);
	virtual void			UpdateEffectThread(CEffectThread *pEffectThread);

	virtual CVec3f			GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds)		{ return m_v3Position; }

	GAME_SHARED_API int		GetMoveFlags() const			{ return s_Move.iMoveFlags; }

	// Client-side inventory management
	GAME_SHARED_API void	SetInventorySlot(int iSlot, IInventoryItem *pItem);

	GAME_SHARED_API			const tstring&	GetTerrainType();

	// Settings
	virtual const tstring&		GetEntityName() const						{ if (m_pEntityConfig == NULL) return SNULL; return m_pEntityConfig->GetName().GetValue(); }
	virtual const tstring&		GetEntityDescription() const				{ if (m_pEntityConfig == NULL) return SNULL; return m_pEntityConfig->GetDescription().GetValue(); }
	virtual const tstring&		GetEntityIconPath() const					{ if (m_pEntityConfig == NULL) return SNULL; return m_pEntityConfig->GetIconPath().GetValue(); }
	const tstring&				GetPrerequisite() const						{ if (m_pEntityConfig == NULL) return SNULL; return m_pEntityConfig->GetPrerequisite().GetValue(); }
	
	ENTITY_CVAR_ACCESSOR(uint, Cost, 0)
	virtual ENTITY_CVAR_ACCESSOR(float, SightRange, 0.0f)
	virtual ENTITY_CVAR_ACCESSOR(float, ExperienceValue, 0.0f)
	virtual ENTITY_CVAR_ACCESSOR(int, Bounty, 0)
	ENTITY_CVAR_ACCESSOR(bool, IsHidden, false)
	ENTITY_CVAR_ACCESSOR(tstring, GameTip, _T(""))
	virtual ENTITY_CVAR_ACCESSOR(tstring, CommanderPortraitPath, _T(""))
	virtual ENTITY_CVAR_ACCESSOR(int, MinimapIconSize, 0)
	virtual ENTITY_CVAR_ACCESSOR(tstring, MinimapIconPath, _T(""))
	virtual ENTITY_CVAR_ACCESSOR(int, LargeMapIconSize, 0)
	virtual ENTITY_CVAR_ACCESSOR(tstring, LargeMapIconPath, _T(""))
	virtual ENTITY_CVAR_ACCESSOR(float, CommanderScale, 1.0f)
	virtual ENTITY_CVAR_ACCESSOR(float, EffectScale, 1.0f)
	virtual ENTITY_CVAR_ACCESSOR(float, SelectionRadius, 0.0f)
	virtual ENTITY_CVAR_ACCESSOR(float, PushMultiplier, 1.0f)
	virtual ENTITY_CVAR_ACCESSOR(bool, SpawnPoint, false)
	virtual ENTITY_CVAR_ACCESSOR(bool, SiegeSpawnPoint, false)
};
//=============================================================================

#endif //__I_VISUALENTITY_H__
