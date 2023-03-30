// (C)2006 S2 Games
// i_emitter.h
//
//=============================================================================
#ifndef __I_EMITTER_H__
#define __I_EMITTER_H__

//=============================================================================
// Definitions
//=============================================================================
class IEmitter;
class CSimpleParticleDef;
class CParticleSystem;
class CSceneLight;
class CSceneEntity;
class CSkeleton;

const uint BBOARD_LOCK_UP       (BIT(0));
const uint BBOARD_LOCK_RIGHT    (BIT(1));
const uint BBOARD_TURN          (BIT(2));
const uint BBOARD_FLARE         (BIT(3));
const uint BBOARD_GENERATE_AXIS (BIT(4));
const uint BBOARD_ALPHACOLOR    (BIT(5));
const uint BBOARD_OFFCENTER     (BIT(6));

struct SBillboard
{
    CVec3f  v3Pos;
    float   width;
    float   height;
    float   angle;
    float   s1;
    float   t1;
    float   s2;
    float   t2;
    float   frame;
    float   param;
    CVec4f  color;
    ResHandle   hMaterial;
    uint    uiFlags;
    float   fPitch;
    float   fYaw;
    float   fDepthBias;
    int     iEffectLayer;
    float   fDepth;
    CVec2f  v2Center;
    CAxis   aAxis;
};

struct SBeam
{
    CVec3f  v3Start;
    CVec3f  v3End;
    float   fStartSize;
    float   fEndSize;
    float   fTile;
    float   fTaper;
    CVec4f  v4StartColor;
    CVec4f  v4EndColor;
    float   fStartFrame;
    float   fEndFrame;
    float   fStartParam;
    float   fEndParam;
    ResHandle   hMaterial;
    int     iEffectLayer;
};

struct STriangleVertex
{
    CVec3f  v;
    dword   color;
    CVec4f  t;
};

struct STriangle
{
    STriangleVertex vert[3];
    ResHandle       hMaterial;
};

enum EDirectionalSpace
{
    DIRSPACE_GLOBAL = 0,
    DIRSPACE_LOCAL
};

typedef bool (*ParticleTraceFn_t)(const CVec3f &v3Start, const CVec3f &v3End, CVec3f &v3EndPos, CVec3f &v3Normal);

IEmitter * const OWNER_SOURCE((IEmitter *)1);
IEmitter * const OWNER_TARGET((IEmitter *)2);
IEmitter * const OWNER_CUSTOM((IEmitter *)3);
//=============================================================================

//=============================================================================
// IEmitterDef
//=============================================================================
class IEmitterDef
{
protected:
    vector<CSimpleParticleDef *>        m_vParticleDefinitions;

public:
    virtual ~IEmitterDef();
    IEmitterDef();

    virtual IEmitter*   Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner) = 0;

    void    AddParticleDef(CSimpleParticleDef *pParticle);
    int     GetCount()          { return 1; }
    const vector<CSimpleParticleDef *>& GetParticleDefinitions() const { return m_vParticleDefinitions; }
};
//=============================================================================

//=============================================================================
// IEmitter
//=============================================================================
class IEmitter
{
protected:
    // Emitter properties
    int         m_iLife;
    int         m_iExpireLife;
    int         m_iTimeNudge;
    int         m_iDelay;
    bool        m_bLoop;

    tstring     m_sName;
    IEmitter    *m_pOwner;
    tstring     m_sBone;
    CVec3f      m_v3Pos;
    CVec3f      m_v3Offset;
    bool        m_bActive;
    IEmitter    *m_pNextEmitter;

    CVec3f      m_v3LastPos;
    CAxis       m_aLastAxis;
    float       m_fLastScale;

    CVec3f      m_v3CustomPos;
    CAxis       m_aCustomAxis;
    float       m_fCustomScale;
    bool        m_bCustomVisibility;

    EDirectionalSpace   m_eDirectionalSpace;

    const vector<CSimpleParticleDef *>  *m_pvParticleDefinitions;

    CParticleSystem *m_pParticleSystem;

    uint        m_uiStartTime;
    uint        m_uiExpireTime;
    uint        m_uiLastUpdateTime;
    uint        m_uiPauseBegin;

    CBBoxf      m_bbBounds;

    IEmitter*   GetOwnerPointer(const tstring &sOwner);

    bool        GetVisibility();
    bool        GetVisibility(const tstring &sBone);
    bool        GetVisibility(const tstring &sBone, IEmitter *pOwner);
    CVec3f      GetBonePosition(uint uiTime, IEmitter *pOwner, const tstring &sBone);
    void        GetBoneAxisPos(uint uiTime, IEmitter *pOwner, const tstring &sBone, CAxis &aOutAxis, CVec3f &v3OutPos);

    CVec3f      GetPosition();
    CAxis       GetAxis();
    float       GetScale();

    CVec3f      GetPosition(const CVec3f &v3Pos, IEmitter *pOwner);
    CAxis       GetAxis(IEmitter *pOwner);
    float       GetScale(IEmitter *pOwner);

    CSkeleton*  GetSkeleton();
    CModel*     GetModel();

    void        UpdateNextEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

public:
    virtual ~IEmitter();
    IEmitter();
    IEmitter
    (
        int iLife,
        int iExpireLife,
        int iTimeNudge,
        int iDelay,
        bool bLoop,
        const tstring &sName,
        const tstring &sOwner,
        const tstring &sBone,
        const CVec3f &v3Pos,
        const CVec3f &v3Offset,
        EDirectionalSpace eDirectionalSpace,
        const vector<CSimpleParticleDef *> *pvParticleDefinitions,
        CParticleSystem *pParticleSystem,
        IEmitter *pOwner,
        uint uiStartTime
    );

    const tstring&      GetName() const     { return m_sName; }
    
    IEmitter*           GetOwner() const            { return m_pOwner; }
    void                SetOwner(IEmitter *pOwner)  { m_pOwner = pOwner; }

    IEmitter*           GetNextEmitter() const                  { return m_pNextEmitter; }
    void                SetNextEmitter(IEmitter *pNextEmitter)  { m_pNextEmitter = pNextEmitter; }
    
    const tstring&      GetBone() const     { return m_sBone; }
    bool                IsActive() const    { return m_bActive; }

    virtual bool        Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace) = 0;
    virtual void        ResumeFromPause(uint uiMilliseconds);
    void                UpdatePaused(uint uiMilliseconds);

    virtual uint        GetNumBillboards()                                      { return 0; }
    virtual bool        GetBillboard(uint uiIndex, SBillboard &outBillboard)    { return true; }

    virtual uint        GetNumBeams()                                           { return 0; }
    virtual bool        GetBeam(uint uiIndex, SBeam &outBeam)                   { return true; }

    virtual uint        GetNumLights()                                          { return 0; }
    virtual bool        GetLight(uint uiIndex, CSceneLight &outLight)           { return true; }

    virtual uint        GetNumTriangles()                                       { return 0; }
    virtual bool        GetTriangle(uint uiIndex, STriangle &outTriangle)       { return true; }

    virtual uint        GetNumEntities()                                        { return 0; }
    virtual bool        GetEntity(uint uiIndex, CSceneEntity &outEntity)        { return true; }

    virtual uint        GetNumEmitters()                                        { return 0; }
    virtual IEmitter*   GetEmitter(uint uiIndex)                                { return NULL; }

    virtual CSkeleton*  GetCustomSkeleton()                                     { return NULL; }

    const CVec3f&       GetLastPos() const                                      { return m_v3LastPos; }
    const CAxis&        GetLastAxis() const                                     { return m_aLastAxis; }
    const float         GetLastScale() const                                    { return m_fLastScale; }

    void                SetPos(const CVec3f &v3Pos)                             { m_v3Pos = v3Pos; }
    void                SetActive(bool bActive)                                 { m_bActive = bActive; }

    void                SetCustomPos(const CVec3f &v3Pos)                       { m_v3CustomPos = v3Pos; }
    const CVec3f&       GetCustomPos() const                                    { return m_v3CustomPos; }

    void                SetCustomAxis(const CAxis &aAxis)                       { m_aCustomAxis = aAxis; }
    const CAxis&        GetCustomAxis() const                                   { return m_aCustomAxis; }

    void                SetCustomScale(float fScale)                            { m_fCustomScale = fScale; }
    float               GetCustomScale() const                                  { return m_fCustomScale; }

    void                SetCustomVisibility(bool bVisible)                      { m_bCustomVisibility = bVisible; }
    bool                GetCustomVisibility() const                             { return m_bCustomVisibility; }

    const CBBoxf&       GetBounds() const                                       { return m_bbBounds; }

    virtual void        OnDelete(IEmitter *pEmitter);

    virtual void        Expire(uint uiMilliseconds)
    {
        if (m_uiExpireTime == INVALID_TIME)
            m_uiExpireTime = uiMilliseconds;
        if (m_pNextEmitter != NULL)
            m_pNextEmitter->Expire(uiMilliseconds);
    }
    
    bool                GetExpire() const                                       { return m_uiExpireTime != INVALID_TIME; }
};
//=============================================================================

#endif  //__I_EMITTER_H__
