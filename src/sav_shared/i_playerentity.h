// (C)2006 S2 Games
// i_playerentity.h
//
//=============================================================================
#ifndef __I_PLAYERENTITY_H__
#define __I_PLAYERENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_combatentity.h"
#include "i_petentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CEntityDirectory;
class CWorld;
class CCamera;
class CSceneEntity;
class CEntitySnapshot;
class CClientSnapshot;
class CPlayerCommander;
class IPetEntity;
class CEffectThread;

enum EOfficerCommand
{
    OFFICERCMD_INVALID,

    OFFICERCMD_ATTACK,
    OFFICERCMD_MOVE,
    OFFICERCMD_FOLLOW,
    OFFICERCMD_DEFEND,
    OFFICERCMD_RALLY,
    OFFICERCMD_PING,
    OFFICERCMD_CLEAR,

    NUM_OFFICERCMDS
};

extern GAME_SHARED_API CCvar<float> g_tradeInMultiplier;
extern GAME_SHARED_API CCvar<float> g_hoverEntityRange;
extern GAME_SHARED_API CCvar<int>   g_hoverEntityDisplayTime;
extern GAME_SHARED_API CCvar<int>   g_hoverEntityAcquireTime;
extern GAME_SHARED_API CCvar<float> g_altInfoRange;
extern GAME_SHARED_API CCvar<float> p_meleeMinPitch;
extern GAME_SHARED_API CCvar<float> p_meleeMaxPitch;
extern GAME_SHARED_API CCvar<float> p_siegeMinPitch;
extern GAME_SHARED_API CCvar<float> p_siegeMaxPitch;
extern GAME_SHARED_API CCvar<float> p_siegeYaw;
extern GAME_SHARED_API CCvar<float> p_vehicleMeleeMinPitch;
extern GAME_SHARED_API CCvar<float> p_vehicleMeleeMaxPitch;
extern GAME_SHARED_API CCvar<float> p_vehicleMeleeYaw;
extern GAME_SHARED_API CCvar<bool>  p_stepDown;

EXTERN_CVAR_UINT(g_economyInterval);
EXTERN_CVAR_UINT(g_corpseTime);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const tstring g_sPersistantStatNames[] =
{
    _T("Rank"),
    _T("Experience"),
    _T("Level"),
    _T("Wins"),
    _T("Losses"),
    _T("Ties"),
    _T("Kills"),
    _T("Deaths"),
    _T("Assists"),
    _T("Souls"),
    _T("Razed"),
    _T("PlayerDamage"),
    _T("BuildingDamage"),
    _T("NPCKills"),
    _T("HPHealed"),
    _T("Resurrects"),
    _T("Karma"),
    _T("GoldEarned"),
    _T("HPRepaired"),
    _T("HoursPlayed"),
    _T("MinutesPlayed"),
    _T("SecondsPlayed"),
    _T("KillsRatio"),
    _T("DeathsRatio"),
    _T("SkillFactor")
};

const int PLAYER_MOVE_IDLE          (0);
const int PLAYER_MOVE_FWD           (BIT(0));
const int PLAYER_MOVE_BACK          (BIT(1));
const int PLAYER_MOVE_LEFT          (BIT(2));
const int PLAYER_MOVE_RIGHT         (BIT(3));
const int PLAYER_MOVE_JUMP          (BIT(5));
const int PLAYER_MOVE_IMMOBILE      (BIT(6));
const int PLAYER_MOVE_FWD_LEFT      (PLAYER_MOVE_FWD | PLAYER_MOVE_LEFT);
const int PLAYER_MOVE_FWD_RIGHT     (PLAYER_MOVE_FWD | PLAYER_MOVE_RIGHT);
const int PLAYER_MOVE_BACK_LEFT     (PLAYER_MOVE_BACK | PLAYER_MOVE_LEFT);
const int PLAYER_MOVE_BACK_RIGHT    (PLAYER_MOVE_BACK | PLAYER_MOVE_RIGHT);
const int PLAYER_MOVE_NO_FLAGS      (0x00ff);
const int PLAYER_MOVE_FLAGS         (0xff00);
const int PLAYER_MOVE_RESTING       (BIT(13));
const int PLAYER_MOVE_SPRINT        (BIT(14));
const int PLAYER_MOVE_TIRED         (BIT(15));
const int PLAYER_MOVE_DASH          (BIT(16));

const int PLAYER_MOVE_IGNORE_FLAGS  (PLAYER_MOVE_RESTING);
const int PLAYER_MOVE_RUN_FLAGS     (PLAYER_MOVE_FWD | PLAYER_MOVE_BACK | PLAYER_MOVE_LEFT | PLAYER_MOVE_RIGHT);

enum EPlayerAttributes
{
    ATTRIBUTE_NULL,

    ATTRIBUTE_ENDURANCE,
    ATTRIBUTE_INTELLIGENCE,
    ATTRIBUTE_AGILITY,
    ATTRIBUTE_STRENGTH,

    NUM_PLAYER_ATTRIBUTES
};

const tstring g_asPlayerAttibutes[] =
{
    _T("INVALID_STAT"),

    _T("Endurance"),
    _T("Intelligence"),
    _T("Agility"),
    _T("Strength")
};

enum ECommanderOrder
{
    CMDR_ORDER_AUTO = 0,
    CMDR_ORDER_CLEAR,
    CMDR_ORDER_MOVE,
    CMDR_ORDER_ATTACK,
};
//=============================================================================

//=============================================================================
// IPlayerEntity
//=============================================================================
class IPlayerEntity : public ICombatEntity
{
private:
    IPlayerEntity();
    static vector<SDataField>*  s_pvFields;

    static ResHandle    s_hOfficerStarIcon;

protected:
    START_ENTITY_CONFIG(ICombatEntity)
        DECLARE_ENTITY_CVAR(bool, CanEnterLoadout)
        DECLARE_ENTITY_CVAR(int, SoulCost)
        DECLARE_ENTITY_CVAR(float, CamHeight)
        DECLARE_ENTITY_CVAR(float, CamDistance)
        DECLARE_ENTITY_CVAR(float, CamPitch)
        DECLARE_ENTITY_CVAR(bool, YawStrafe)
        DECLARE_ENTITY_CVAR(bool, IsVehicle)
        DECLARE_ENTITY_CVAR(float, VehicleTurnSpeed)
        DECLARE_ENTITY_CVAR(bool, CanPurchase)
        DECLARE_ENTITY_CVAR(tstring, SpawnEffectPath)
        DECLARE_ENTITY_CVAR(tstring, PainEffectPath)
        DECLARE_ENTITY_CVAR(bool, ApplyAttributes)
        DECLARE_ENTITY_CVAR(bool, CanDash)
        DECLARE_ENTITY_CVAR(bool, ShowResurrectable)
        DECLARE_ENTITY_CVAR(float, TransparentAngle)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    int                         m_iClientNum;

    byte                        m_yFirstPersonAnim;
    byte                        m_yFirstPersonAnimSequence;
    float                       m_fFirstPersonAnimSpeed;
    ResHandle                   m_hFirstPersonModel;
    CVec3f                      m_v3FirstPersonModelOffset;
    CVec3f                      m_v3FirstPersonModelAngles;
    float                       m_fFirstPersonModelFov;
    CSkeleton*                  m_pFirstPersonSkeleton;

    uint                        m_uiDeathTime;
    uint                        m_uiLoadoutTime;

    uint                        m_uiPreviewBuildingIndex;
    CVec2f                      m_v2Cursor;

    // Actions
    int                         m_iCurrentMovement;
    int                         m_iMoveFlags;
    uint                        m_uiWheelTime;
    uint                        m_uiDashStartTime;
    uint                        m_uiLastDashTime;

    // Orders
    vector<byte>                m_vecOrders;
    uivector                    m_vecOrderEntIndex;
    vector<CVec3f>              m_vecOrderPos;

    // Officer Orders
    byte                        m_yOfficerOrder;
    byte                        m_yOfficerOrderSequence;
    byte                        m_yLastAcknowledgedOfficerOrder;    // Client
    uint                        m_uiOfficerOrderEntIndex;
    CVec3f                      m_v3OfficerOrderPos;

    // Pet
    uint                        m_uiPetIndex;
    
    // Client-side order management
    byte                        m_yCurrentOrder;
    CVec3f                      m_v3CurrentOrderPos;
    uint                        m_uiCurrentOrderEntIndex;
    byte                        m_yOrderUniqueID;

    // Turning (Client-side only)
    float                       m_fBaseYaw;
    float                       m_fLastYaw;
    float                       m_fCurrentYaw;
    uint                        m_uiTurnStartTime;
    byte                        m_ayTurnSequence[NUM_ANIM_CHANNELS];
    int                         m_iTurnAction;
    float                       m_fYawLerpTime;
    float                       m_fTiltPitch;
    float                       m_fTiltRoll;
    float                       m_fTiltHeight;

    float                       m_fCurrentSpeed;

    // View bob
    float                       m_fBobTime;
    float                       m_fBobCycle;
    float                       m_fBobHalfCycle;
    CVec3f                      m_v3BobPos;
    CVec3f                      m_v3BobAngles;

    // Server-side stuff
    uint                        m_uiSpawnLocation;
    ushort                      m_unNextUnit;
    uint                        m_uiNextPainTime;
    int                         m_iDashStateSlot;
    byte                        m_yKillStreak;

    void    Accelerate(const CVec3f &v3Intent, float fAcceleration);
    void    Friction(float fFriction);

    void    MoveWalkGround();
    void    MoveWalkAir();
    void    MoveWalkWater();

    CVec3f  GetSpawnLocation(uint uiIndex);

public:
    virtual ~IPlayerEntity();
    IPlayerEntity(CEntityConfig *pConfig);

    bool                IsPlayer() const                    { return true; }

    bool                IsSelectable() const                { return true; }
    
    bool                    HasAltInfo() const              { return true; }
    virtual const tstring&  GetAltInfoName() const;

    GAME_SHARED_API const tstring&  GetClientName() const;

    virtual bool        GetGameTip(IPlayerEntity *pPlayer, tstring &sMessage);

    virtual bool        IsObserver() const                                  { return false; }

    void                SetClientID(int iClientID)                          { m_iClientNum = iClientID; }
    int                 GetClientID() const                                 { return m_iClientNum; }

    void                SetWorldPointer(CWorld *pWorld);

    void                SetTargetIndex(uint uiIndex)                        { m_uiTargetIndex = uiIndex; }
    uint                GetTargetIndex() const                              { return m_uiTargetIndex; }
    uint                GetSpellTargetIndex() const                         { return m_uiSpellTargetIndex; }
    void                SetPetIndex(uint uiIndex)                           { m_uiPetIndex = uiIndex; }
    uint                GetPetIndex() const                                 { return m_uiPetIndex; }
    int                 GetCurrentMovement()                                { return m_iCurrentMovement; }

    uint                GetPreviewBuildingIndex() const                     { return m_uiPreviewBuildingIndex; }
    void                SetPreviewBuildingIndex(uint uiIndex)               { m_uiPreviewBuildingIndex = uiIndex; }

    int                 GetFirstPersonAnim()                                { return m_yFirstPersonAnim; }
    void                SetweaponAnim(int iAnim)                            { m_yFirstPersonAnim = iAnim; }

    byte                GetFirstPersonAnimSequence()                        { return m_yFirstPersonAnimSequence; }
    void                SetFirstPersonAnimSequence(byte ySequence)          { m_yFirstPersonAnimSequence = ySequence; }
    void                IncFirstPersonAnimSequence()                        { ++m_yFirstPersonAnimSequence; }

    float               GetFirstPersonAnimSpeed()                           { return m_fFirstPersonAnimSpeed; }

    GAME_SHARED_API void    StartFirstPersonAnimation(const tstring &sAnimName, float fSpeed = 1.0f, uint uiLength = 0);
    GAME_SHARED_API void    StopFirstPersonAnimation();
    GAME_SHARED_API void    StopFirstPersonAnimation(const tstring &sAnimName);

    ResHandle           GetFirstPersonModelHandle() const                   { return m_hFirstPersonModel; }
    void                SetFirstPersonModelHandle(ResHandle hModel)         { m_hFirstPersonModel = hModel; }

    const CVec3f&       GetFirstPersonModelOffset() const                   { return m_v3FirstPersonModelOffset; }
    void                SetFirstPersonModelOffset(const CVec3f &v3Offset)   { m_v3FirstPersonModelOffset = v3Offset; }

    const CVec3f&       GetFirstPersonModelAngles() const                   { return m_v3FirstPersonModelAngles; }
    void                SetFirstPersonModelAngles(const CVec3f &v3Angles)   { m_v3FirstPersonModelAngles = v3Angles; }

    float               GetFirstPersonModelFov() const                      { return m_fFirstPersonModelFov; }
    void                SetFirstPersonModelFov(float fFov)                  { m_fFirstPersonModelFov = fFov; }

    void                SetFirstPersonSkeleton(CSkeleton *pSkeleton)        { m_pFirstPersonSkeleton = pSkeleton; }
    CSkeleton*          GetFirstPersonSkeleton()                            { return m_pFirstPersonSkeleton; }

    CVec2f              GetCursorPos() const                                { return m_v2Cursor; }

    // Attributes
    GAME_SHARED_API float   GetAttributeBoost(int iAttribute) const;
    GAME_SHARED_API tstring GetAttributeBoostDescription(int iAttribute) const;

    virtual float           Heal(float fHealth, IVisualEntity *pSource);
    virtual float           Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);

    GAME_SHARED_API virtual void    GiveGold(ushort unGold, const CVec3f &v3Pos, bool bUseTax = false, bool bUseIncomeMod = false);
    GAME_SHARED_API virtual ushort  GiveGold(ushort unGold, bool bUseTax = false, bool bUseIncomeMod = false);

    void    GiveExperience(float fExperience, const CVec3f &v3Pos);
    void    GiveExperience(float fExperience);
    float   GetExperienceValue() const;

    void                    SetDeathTime(uint uiTime)                       { m_uiDeathTime = uiTime; }
    float                   GetDeathTime() const                            { return m_uiDeathTime; }
    GAME_SHARED_API float   GetDeathPercent() const;
    GAME_SHARED_API uint    GetRemainingDeathTime() const;

    GAME_SHARED_API void    EnterLoadout();

    // Network
    virtual void            Baseline();
    virtual void            GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool            ReadSnapshot(CEntitySnapshot &snapshot);
    virtual void            ReadClientSnapshot(const CClientSnapshot &snapshot);
    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();

    virtual int             GetPrivateClient()                              { return m_iClientNum; }

    static void             ClientPrecache(CEntityConfig *pConfig);
    static void             ServerPrecache(CEntityConfig *pConfig);

    GAME_SHARED_API bool    IsOfficer() const;

    // Actions
    virtual void            Spawn();
    virtual bool            ServerFrame();
    virtual void            LocalClientFrame();
    virtual void            KillReward(IGameEntity *pKiller);
    virtual void            Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
    virtual void            CancelBuild();
    void                    IncreaseKillStreak();

    virtual CVec3f          GetTargetPosition(float fRange, float fMinRange = 0.0f);

    // Visual
    GAME_SHARED_API virtual CVec3f  GetCameraPosition(const CVec3f &v3PlayerPos, const CVec3f &v3PlayerAngles);
    GAME_SHARED_API virtual CVec3f  GetCameraAngles(const CVec3f &v3InputAngle, bool bForceThirdPerson = false);
    virtual void                    SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles);
    virtual CSkeleton*              AllocateSkeleton();
    virtual void                    UpdateSkeleton(bool bPose);
    virtual bool                    AddToScene(const CVec4f &v4Color, int iFlags);
    GAME_SHARED_API void            UpdateViewBob(uint uiFrameTime);
    GAME_SHARED_API bool            AddFirstPersonToScene(const CCamera &camera);
    GAME_SHARED_API void            UpdateFirstPersonSkeleton();

    virtual bool            IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap);
    virtual ResHandle       GetMapIcon(IPlayerEntity *pLocalPlayer, bool bLargeMap);
    virtual CVec4f          GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap);
    virtual float           GetMapIconSize(IPlayerEntity *pLocalPlayer, bool bLargeMap);

    // Physics
    virtual void            Link();
    virtual void            Unlink();
    virtual void            Move(const CClientSnapshot &snapshot) = 0;
    void                    MoveFly(const CClientSnapshot &snapshot);
    void                    MoveWalk(const CClientSnapshot &snapshot);
    virtual void            MoveTrajectory(const CClientSnapshot &snapshot);
    virtual void            MoveCorpse(const CClientSnapshot &snapshot);

    GAME_SHARED_API bool            GetCanPurchase()                            { return m_pEntityConfig->GetCanPurchase(); }

    GAME_SHARED_API virtual float   GetStaminaRegen(int iMoveState = PLAYER_MOVE_IDLE) const;

    byte            GetOrder(byte ySequence)                { if (ySequence < m_vecOrders.size()) return m_vecOrders[ySequence]; return CMDR_ORDER_CLEAR; }
    uint            GetOrderEntIndex(byte ySequence)        { if (ySequence < m_vecOrderEntIndex.size()) return m_vecOrderEntIndex[ySequence]; return INVALID_INDEX; }
    CVec3f          GetOrderPos(byte ySequence)             { if (ySequence < m_vecOrderPos.size()) return m_vecOrderPos[ySequence]; return V3_ZERO; }

    byte            GetCurrentOrder()                       { return m_yCurrentOrder; }
    uint            GetCurrentOrderEntIndex()               { return m_uiCurrentOrderEntIndex; }
    CVec3f          GetCurrentOrderPos()                    { return m_v3CurrentOrderPos; }

    uint            GetNumOrders()                          { return INT_SIZE(m_vecOrders.size()); }
    void            DeleteOrder(byte ySequence);
    
    void            IncOrderSequence();
    GAME_SHARED_API void    AddOrder(byte yOrder, uint uiOrderEntIndex, const CVec3f &v3OrderPos);

    GAME_SHARED_API void    ClearOrders();

    byte            GetOrderUniqueID() const                { return m_yOrderUniqueID; }
    
    byte            GetOfficerOrder() const                 { return m_yOfficerOrder; }
    byte            GetOfficerOrderSequence() const         { return m_yOfficerOrderSequence; }
    uint            GetOfficerOrderEntIndex() const         { return m_uiOfficerOrderEntIndex; }
    CVec3f          GetOfficerOrderPos() const              { return m_v3OfficerOrderPos; }
    bool            GetOfficerOrderChanged() const          { return m_yOfficerOrderSequence != m_yLastAcknowledgedOfficerOrder; }
    void            AcknowledgeOfficerOrder()               { m_yLastAcknowledgedOfficerOrder = m_yOfficerOrderSequence; }

    void            SetOfficerOrder(byte yOrder)                        { m_yOfficerOrder = yOrder; ++m_yOfficerOrderSequence; }
    void            SetOfficerOrderEntIndex(uint uiOrderEntIndex)       { m_uiOfficerOrderEntIndex = uiOrderEntIndex; }
    void            SetOfficerOrderPos(const CVec3f &v3OrderPos)        { m_v3OfficerOrderPos = v3OrderPos; }

    // Inventory activation
    virtual bool    ActivatePrimary(int iSlot, int iButtonStatus);
    virtual bool    ActivateSecondary(int iSlot, int iButtonStatus);
    virtual bool    ActivateTertiary(int iSlot, int iButtonStatus);

    // HACK: Commander stuff
    virtual CPlayerCommander*       GetAsCommander()        { return NULL; }
    virtual const CPlayerCommander* GetAsCommander() const  { return NULL; }

    // Operators
    GAME_SHARED_API virtual void    Copy(const IGameEntity &B);

    // Client-side
    float                           GetCurrentYaw() const       { return m_fCurrentYaw; }
    float                           GetCurrentSpeed() const     { return m_fCurrentSpeed; }
    GAME_SHARED_API float           GetDashRemaining() const;
    GAME_SHARED_API float           GetDashCooldownPercent() const;
    void                            ResetDash()                 { m_uiLastDashTime = INVALID_TIME; }

    GAME_SHARED_API virtual void    DrawViewBox(class CUITrigger &minimap, CCamera &camera);

    // Server-side
    GAME_SHARED_API void    Spawn2();
    GAME_SHARED_API void    Spawn3();
    GAME_SHARED_API bool    SetSpawnLocation(uint uiIndex);
    void                    SetNextUnit(ushort unUnitID)        { m_unNextUnit = unUnitID; }
    GAME_SHARED_API void    PetCommand(EPetCommand ePetCmd, uint uiIndex, const CVec3f &v3Pos);
    GAME_SHARED_API void    OfficerCommand(EOfficerCommand eOfficerCmd, uint uiIndex, const CVec3f &v3Pos);

    virtual void            Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);
    virtual void            UpdateEffectThread(CEffectThread *pEffectThread);

    virtual bool            CanDash();

    GAME_SHARED_API bool    IsFirstPerson();

    ENTITY_CVAR_ACCESSOR(bool, CanEnterLoadout, true)
    ENTITY_CVAR_ACCESSOR(int, SoulCost, 0)
    ENTITY_CVAR_ACCESSOR(float, CamHeight, 50.0f)
    ENTITY_CVAR_ACCESSOR(float, CamDistance, 150.0f)
    ENTITY_CVAR_ACCESSOR(float, CamPitch, 0.0f)
    ENTITY_CVAR_ACCESSOR(bool, YawStrafe, true)
    ENTITY_CVAR_ACCESSOR(bool, IsVehicle, false)
    ENTITY_CVAR_ACCESSOR(float, VehicleTurnSpeed, 90.0f)
    ENTITY_CVAR_ACCESSOR(bool, CanPurchase, false)
    ENTITY_CVAR_ACCESSOR(tstring, SpawnEffectPath, _T(""))
    ENTITY_CVAR_ACCESSOR(tstring, PainEffectPath, _T(""))
    ENTITY_CVAR_ACCESSOR(bool, ApplyAttributes, true)
    ENTITY_CVAR_ACCESSOR(bool, ShowResurrectable, false)
    ENTITY_CVAR_ACCESSOR(float, TransparentAngle, 0.0f)
};
//=============================================================================

#endif //__I_PLAYERENTITY_H__

