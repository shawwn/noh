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
#include "c_player.h"

#include "../k2/s_traceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IVisualEntity;
class IPropEntity;
class IProjectile;
class ILight;
class IGadgetEntity;
class IBuildingEntity;
class IUnitEntity;

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
const uint  ENTITY_EVENT_BUFFER_LENGTH(4);
const uint  NUM_EFFECT_CHANNELS(4);

// Effect channels
const uint EFFECT_CHANNEL_PROJECTILE_TRAIL      (0);

const uint EFFECT_CHANNEL_PASSIVE               (0);

const uint EFFECT_CHANNEL_BUILDING_LOW_HEALTH   (1);

const uint EFFECT_CHANNEL_TRIGGER               (3);
const uint EFFECT_CHANNEL_VOICECOMMAND          (3);
////

enum EEntityStatus
{
    ENTITY_STATUS_DORMANT,
    ENTITY_STATUS_ACTIVE,
    ENTITY_STATUS_DEAD,
    ENTITY_STATUS_CORPSE,
    ENTITY_STATUS_HIDDEN,
    
    NUM_ENTITY_STATUSES
};

const byte NO_INTERPOLATE_MASK          (BIT(0) | BIT(1));
const byte RESET_MASK                   (BIT(2) | BIT(3));

struct SWorkingMovementVars
{
    float       fFrameTime;
    CPlane      plGround;
    float       fRunSpeed;
    CVec3f      v3OldPosition;
    CVec3f      v3OldVelocity;
    CVec3f      v3OldAngles;
    CVec3f      v3Position;
    CVec3f      v3Velocity;
    CVec3f      v3Angles;
    CVec3f      v3Intent;
    bool        bGroundControl;
    int         iMoveFlags;
    bool        bLanded;
};

const byte ENTITY_INVALID_ANIM  (-1);
const byte ENTITY_STOP_ANIM     (-2);

// Entity Client Render Flags
const int ECRF_HALFTRANSPARENT      (BIT(0));

const int MOVE_ON_GROUND    (BIT(0));
const int MOVE_JUMP_HELD    (BIT(1));
const int MOVE_JUMPING      (BIT(2));
const int MOVE_ATTACK_HELD  (BIT(3));

const uint ENT_LOCAL_DELETE_NEXT_FRAME  (BIT(0));
const uint ENT_LOCAL_CHANGING_UNIT      (BIT(1));
const uint ENT_LOCAL_SHOW_EFFECTS       (BIT(2));
const uint ENT_LOCAL_RECIPE_EFFECT      (BIT(3));
const uint ENT_LOCAL_FIXED_POSITION     (BIT(4));

const uint ENT_BIND_TURN                (BIT(0));
const uint ENT_BIND_UNBIND_ON_DEATH     (BIT(1));
const uint ENT_BIND_NO_PUSH             (BIT(2));

struct SEntityBind
{
    uint    uiEntityUID;
    CVec3f  v3Offset;
    uint    uiFlags;
};
//=============================================================================

//=============================================================================
// IVisualEntity
//=============================================================================
class IVisualEntity : public IGameEntity
{
    DECLARE_ENTITY_DESC
    
protected:
    tstring             m_sName;
    uint                m_uiWorldIndex;
    uint                m_uiTeamID;

    byte                m_yStatus;
    byte                m_ySequence;
    ushort              m_unVisibilityFlags;

    // Physics
    CVec3f              m_v3Position;
    CVec3f              m_v3Velocity;
    CVec3f              m_v3Angles;
    float               m_fScale;
    CBBoxf              m_bbBounds;
    uint                m_uiGroundEntityIndex;
    
    uint                m_uiBindTargetUID;
    vector<SEntityBind> m_vBinds;
    uivector            m_vBindStateUIDs;

    // Events
    byte                m_yNextEventSlot;
    byte                m_yLastProcessedEvent;
    CEntityEvent        m_aEvents[ENTITY_EVENT_BUFFER_LENGTH];

    uint                m_uiLocalFlags; // Never transmitted

    // Visual
    CSkeleton*          m_pSkeleton;
    tstring             m_asAnim[NUM_ANIM_CHANNELS];
    byte                m_ayAnim[NUM_ANIM_CHANNELS];
    byte                m_ayAnimSequence[NUM_ANIM_CHANNELS];
    float               m_afAnimSpeed[NUM_ANIM_CHANNELS];
    uint                m_auiAnimLockTime[NUM_ANIM_CHANNELS];

    ResHandle           m_ahEffect[NUM_EFFECT_CHANNELS];
    byte                m_ayEffectSequence[NUM_EFFECT_CHANNELS];

    // Client-side only
    uint                m_uiClientRenderFlags;
    uint                m_uiSelectFrame;
    CVec4f              m_v4HighlightColor;
    CAxis               m_aAxis;
    CVec3f              m_v3AxisAngles;
    tstring             m_sTerrainType;
    uint                m_uiLastTerrainTypeUpdateTime;
    uint                m_uiOrderTime;

    CVec4f              m_v4MinimapFlashColor;
    uint                m_uiMinimapFlashEndTime;

    // Server-side only
    bool                m_bAlwaysTransmitData;

#ifdef __GNUC__
    __attribute__ ((visibility("default")))
#endif

        // Slide-movement
    bool                IsPositionValid(const CVec3f &v3Position);

public:
    virtual ~IVisualEntity()    {}
    IVisualEntity();

    // Accessors
    SUB_ENTITY_ACCESSOR(IVisualEntity, Visual)
    
    const tstring&      GetName() const                                     { return m_sName; }
    void                SetName(const tstring &sName)                       { m_sName = sName; }

    uint                GetWorldIndex() const                               { return m_uiWorldIndex; }
    void                SetWorldIndex(uint uiIndex)                         { m_uiWorldIndex = uiIndex; }

    uint                GetTeam() const                                     { return m_uiTeamID; }
    virtual void        SetTeam(uint uiTeam)                                { m_uiTeamID = uiTeam; }
    
    byte                GetStatus() const                                   { return m_yStatus; }
    void                SetStatus(byte yStatus)                             { m_yStatus = yStatus; }

    void                ClearVisibilityFlags()                              { m_unVisibilityFlags = 0; }
    void                ClearVisibilityFlagsDead()                          { m_unVisibilityFlags &= ~(REVEALED_FLAG_MASK | VISION_FLAG_MASK); }
    void                RemoveVisibilityFlags(ushort unFlags)               { m_unVisibilityFlags &= ~unFlags; }
    void                SetVisibilityFlags(ushort unFlags)                  { m_unVisibilityFlags |= unFlags; }
    bool                HasVisibilityFlags(ushort unFlags) const            { return (m_unVisibilityFlags & unFlags) == unFlags; }
    ushort              GetVisibilityFlags() const                          { return m_unVisibilityFlags; }

    void                IncNoInterpolateSequence()                          { m_ySequence = (m_ySequence & ~NO_INTERPOLATE_MASK) | (NO_INTERPOLATE_MASK & ((m_ySequence | ~NO_INTERPOLATE_MASK) + 1)); }
    byte                GetNoInterpolateSequence() const                    { return (m_ySequence & NO_INTERPOLATE_MASK); }

    void                IncResetSequence()                                  { m_ySequence = (m_ySequence & ~RESET_MASK) | (RESET_MASK & ((m_ySequence | ~RESET_MASK) + 1)); }
    byte                GetResetSequence() const                            { return (m_ySequence & RESET_MASK); }

    const CVec3f&       GetPosition() const                                 { return m_v3Position; }
    void                SetPosition(const CVec3f &v3Pos)                    { m_v3Position = v3Pos; }
    void                SetPosition(float x, float y, float z)              { m_v3Position = CVec3f(x, y, z); }

    const CVec3f&       GetAngles() const                                   { return m_v3Angles; }
    void                SetAngles(const CVec3f &v3Angles)                   { m_v3Angles = v3Angles; }
    void                SetAngles(float fPitch, float fRoll, float fYaw)    { m_v3Angles = CVec3f(fPitch, fRoll, fYaw); }

    const CVec3f&       GetVelocity() const                                 { return m_v3Velocity; }
    void                SetVelocity(const CVec3f &v3Velocity)               { m_v3Velocity = v3Velocity; }
    void                ApplyVelocity(const CVec3f &v3Velocity)             { m_v3Velocity += v3Velocity; }

    virtual float       GetBaseScale() const                                { return 1.0f; }

    virtual float       GetScale() const                                    { return m_fScale; }
    void                SetScale(float fScale)                              { m_fScale = fScale; }
    
    virtual float       GetEffectScale() const                              { return 1.0f; }
    virtual float       GetModelScale() const                               { return 1.0f; }
    virtual ResHandle   GetModel() const                                    { return INVALID_RESOURCE; }

    void                SetSkeleton(CSkeleton *pSkeleton)                   { m_pSkeleton = pSkeleton; }

    int                 GetAnim(int iChannel)                               { return m_ayAnim[iChannel]; }
    const tstring&      GetAnimName(int iChannel)                           { return m_asAnim[iChannel]; }
    void                SetAnim(int iChannel, int iAnim)                    { m_ayAnim[iChannel] = iAnim; }
    void                SetAnim(int iChannel, int iAnim, float fAnimSpeed)  { m_ayAnim[iChannel] = iAnim; m_afAnimSpeed[iChannel] = fAnimSpeed; }
    void                SetAnim(int iChannel, const tstring &sAnim)                     { m_asAnim[iChannel] = sAnim; }
    void                SetAnim(int iChannel, const tstring &sAnim, float fAnimSpeed)   { m_asAnim[iChannel] = sAnim; m_afAnimSpeed[iChannel] = fAnimSpeed; }
    void                SetAnim(int iChannel, const tstring &sAnim, float fAnimSpeed, byte ySequence)   { m_asAnim[iChannel] = sAnim; m_afAnimSpeed[iChannel] = fAnimSpeed; m_ayAnimSequence[iChannel] = ySequence; }

    byte                GetAnimSequence(int iChannel)                       { return m_ayAnimSequence[iChannel]; }
    void                SetAnimSequence(int iChannel, byte ySequence)       { m_ayAnimSequence[iChannel] = ySequence; }
    void                IncAnimSequence(int iChannel)                       { ++m_ayAnimSequence[iChannel]; }

    float               GetAnimSpeed(int iChannel)                          { return m_afAnimSpeed[iChannel]; }
    void                SetAnimSpeed(int iChannel, float fAnimSpeed)        { m_afAnimSpeed[iChannel] = fAnimSpeed; }

    virtual void        GetAnimState(int iChannel, int &iAnim, byte &ySequence, float &fSpeed)
    {
        iAnim = m_ayAnim[iChannel];
        ySequence = m_ayAnimSequence[iChannel];
        fSpeed = m_afAnimSpeed[iChannel];
    }

    ResHandle           GetEffect(int iChannel)                             { return m_ahEffect[iChannel]; }
    void                SetEffect(int iChannel, ResHandle hEffect)          { m_ahEffect[iChannel] = hEffect; }

    byte                GetEffectSequence(int iChannel)                     { return m_ayEffectSequence[iChannel]; }
    void                IncEffectSequence(int iChannel)                     { ++m_ayEffectSequence[iChannel]; }

    const CBBoxf&       GetBounds()                                         { return m_bbBounds; }

    CSkeleton*          GetSkeleton()                                       { return m_pSkeleton; }

    uint                GetGroundEntityIndex() const                        { return m_uiGroundEntityIndex; }

    // Network
    virtual void        Baseline();
    virtual void        GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool        ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    static void         ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
    static void         ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);

    virtual void        ApplyWorldEntity(const CWorldEntity &ent);

    // Local flags
    // Clients maintain local flags in m_pCurrentState only
    void                CopyLocalFlags(IVisualEntity *pOther)               { m_uiLocalFlags = pOther->m_uiLocalFlags; }
    void                SetLocalFlags(uint uiFlags)                         { m_uiLocalFlags |= uiFlags; }
    void                ToggleLocalFlags(uint uiFlags)                      { m_uiLocalFlags ^= uiFlags; }
    void                RemoveLocalFlags(uint uiFlags)                      { m_uiLocalFlags &= ~uiFlags; }
    void                ClearLocalFlags()                                   { m_uiLocalFlags = 0; }
    bool                HasLocalFlags(uint uiFlags) const                   { return (m_uiLocalFlags & uiFlags) != 0; }
    bool                HasAllLocalFlags(uint uiFlags) const                { return (m_uiLocalFlags & uiFlags) == uiFlags; }

    // Visual
    virtual CSkeleton*      AllocateSkeleton()                              { return NULL; }
    virtual void            UpdateSkeleton(bool bPose);
    GAME_SHARED_API void    StartAnimation(const tstring &sAnimName, int iChannel, float fSpeed = 1.0f, uint uiLength = 0);
    GAME_SHARED_API void    StartRandomAnimation(const tstring &sAnimName, int iNumAnims, int iChannel, float fSpeed = 1.0f, uint uiLength = 0);
    GAME_SHARED_API void    StopAnimation(int iChannel);
    GAME_SHARED_API void    StopAnimation(const tstring &sAnimName, int iChannel);
    GAME_SHARED_API void    LockAnimation(int iChannel, uint uiTime);
    GAME_SHARED_API int     GetAnimIndex(const tstring &sAnimName);
    virtual bool            AddToScene(const CVec4f &v4Color, int iFlags);

    // Events
    void                    AddEvent(EEntityEvent eEvent);

    virtual void            Copy(const IGameEntity &B);

    // Physics
    virtual void            Link()      {}
    virtual void            Unlink()    {}

    // Client-side
    GAME_SHARED_API void    MinimapFlash(const CVec4f &v4Color, uint uiDuration);
    void                    AddClientRenderFlags(uint uiFlags)          { m_uiClientRenderFlags |= uiFlags; }
    void                    RemoveClientRenderFlags(uint uiFlags)       { m_uiClientRenderFlags &= ~uiFlags; }
    uint                    GetClientRenderFlags() const                { return m_uiClientRenderFlags; }
    void                    SetHighlightFrame()                         { m_uiSelectFrame = Host.GetFrame(); }
    bool                    IsHighlighted() const                       { return m_uiSelectFrame == Host.GetFrame(); }
    const CAxis&            GetAxis() const                             { return m_aAxis; }
    void                    SetOrderTime(uint uiOrderTime)              { m_uiOrderTime = uiOrderTime; }
    uint                    GetOrderTime() const                        { return m_uiOrderTime; }

    void                    SetShowEffects(bool bShowEffects)           { if (bShowEffects) m_uiLocalFlags |= ENT_LOCAL_SHOW_EFFECTS; else m_uiLocalFlags &= ~ENT_LOCAL_SHOW_EFFECTS; }
    bool                    GetShowEffects() const                      { return (m_uiLocalFlags & ENT_LOCAL_SHOW_EFFECTS) != 0; }

    virtual CVec4f          GetMapIconColor(CPlayer *pLocalPlayer) const    { return WHITE; }
    virtual float           GetMapIconSize(CPlayer *pLocalPlayer) const     { return 0.0f; }
    virtual ResHandle       GetMapIcon(CPlayer *pLocalPlayer) const         { return INVALID_RESOURCE; }
    virtual bool            IsVisibleOnMap(CPlayer *pLocalPlayer) const     { return pLocalPlayer->CanSee(this) && GetStatus() == ENTITY_STATUS_ACTIVE; }
    virtual void            DrawOnMap(class CUITrigger &minimap, CPlayer *pLocalPlayer) const;

    void                    SetHighlightColor(const CVec4f &v4Color)    { m_v4HighlightColor = v4Color; }
    const CVec4f&           GetHighlightColor() const                   { return m_v4HighlightColor; }

    virtual void            Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);
    virtual void            UpdateEffectThreadSource(CEffectThread *pEffectThread);
    virtual void            UpdateEffectThreadTarget(CEffectThread *pEffectThread);

    virtual CVec3f          GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds)  { return m_v3Position; }

    GAME_SHARED_API const tstring&  GetTerrainType();

    virtual void            Bind(IUnitEntity *pTarget, const CVec3f &v3Offset, uint uiFlags);
    virtual void            ReleaseBinds();
    virtual void            ReleaseBind(uint uiUID);
    virtual uint            GetBindFlags(uint uiUID);
    virtual uint            GetBindFlags();
    virtual bool            HasBinds();
    virtual void            AddBindState(uint uiUID)            { m_vBindStateUIDs.push_back(uiUID); }
    virtual void            Unbind();

    virtual bool            ServerFrameMovement()           { if (!ServerFrameMovementStart()) return false; return ServerFrameMovementEnd(); }
    virtual bool            ServerFrameMovementStart()      { return IGameEntity::ServerFrameMovement(); }
    virtual bool            ServerFrameMovementEnd();

    virtual bool            GetAlwaysTransmitData() const                                   { return m_bAlwaysTransmitData; }
    virtual void            SetAlwaysTransmitData(bool bAlwaysTransmitData)                 { m_bAlwaysTransmitData = bAlwaysTransmitData; }
};
//=============================================================================

#endif //__I_VISUALENTITY_H__
