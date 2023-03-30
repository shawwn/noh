// (C)2006 S2 Games
// c_simpleparticle.h
//
//=============================================================================
#ifndef __C_SIMPLEPARTICLE_H__
#define __C_SIMPLEPARTICLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "i_emitter.h"
#include "c_particlepool.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SBillboard;
//=============================================================================

//=============================================================================
// CSimpleParticleDef
//=============================================================================
class CSimpleParticleDef
{
private:
    CTemporalPropertyv3         m_tv3Color;
    CTemporalPropertyRangef     m_trfAlpha;

    CTemporalPropertyRangef     m_trfWidth;
    CTemporalPropertyRangef     m_trfHeight;
    CTemporalPropertyRangef     m_trfScale;
    CTemporalPropertyRangef     m_trfAngle;
    CTemporalPropertyRangef     m_trfPitch;
    CTemporalPropertyRangef     m_trfYaw;
    CTemporalPropertyRangef     m_trfFrame;
    CTemporalPropertyRangef     m_trfParam;
    CTemporalPropertyRangef     m_trfStickiness;
    CTemporalPropertyRangef     m_trfAnchor;
    
    // Speed distortions
    CTemporalPropertyRangef     m_trfWidthDistort;
    CTemporalPropertyRangef     m_trfHeightDistort;

    float                       m_fScaleU;
    float                       m_fScaleV;
    float                       m_fOffsetU;
    float                       m_fOffsetV;
    CVec2f                      m_v2Center;

    float                       m_fSelectionWeight;
    uint                        m_uiFlags;

    tsvector                        m_vEmitters;
    vector<IEmitterDef *>       m_vEmitterDefs;

public:
    ~CSimpleParticleDef() {};
    CSimpleParticleDef() {};

    CSimpleParticleDef
    (
        const CTemporalPropertyv3 &tv3Color,
        const CTemporalPropertyRangef &trfAlpha,
        const CTemporalPropertyRangef &trfWidth,
        const CTemporalPropertyRangef &trfHeight,
        const CTemporalPropertyRangef &trfScale,
        const CTemporalPropertyRangef &trfAngle,
        const CTemporalPropertyRangef &trfPitch,
        const CTemporalPropertyRangef &trfYaw,
        const CTemporalPropertyRangef &trfFrame,
        const CTemporalPropertyRangef &trfParam,
        const CTemporalPropertyRangef &trfStickiness,
        const CTemporalPropertyRangef &trfAnchor,
        const CTemporalPropertyRangef &trfWidthDistort,
        const CTemporalPropertyRangef &trfHeightDistort,
        float fScaleU,
        float fScaleV,
        float fOffsetU,
        float fOffsetV,
        const CVec2f &v2Center,
        float fSelectionWeight,
        uint uiFlags,
        const tsvector &vEmitters
    );

    const CTemporalPropertyv3&  GetColor() const    { return m_tv3Color; }
    CTemporalPropertyf      GetAlpha() const    { return m_trfAlpha; }

    CTemporalPropertyf      GetWidth() const    { return m_trfWidth; }
    CTemporalPropertyf      GetHeight() const   { return m_trfHeight; }
    CTemporalPropertyf      GetScale() const    { return m_trfScale; }
    CTemporalPropertyf      GetAngle() const    { return m_trfAngle; }
    CTemporalPropertyf      GetPitch() const    { return m_trfPitch; }
    CTemporalPropertyf      GetYaw() const      { return m_trfYaw; }
    CTemporalPropertyf      GetFrame() const    { return m_trfFrame; }
    CTemporalPropertyf      GetParam() const    { return m_trfParam; }
    CTemporalPropertyf      GetStickiness() const   { return m_trfStickiness; }
    CTemporalPropertyf      GetAnchor() const   { return m_trfAnchor; }
    CTemporalPropertyf      GetWidthDistort() const { return m_trfWidthDistort; }
    CTemporalPropertyf      GetHeightDistort() const    { return m_trfHeightDistort; }
    float                   GetScaleU() const       { return m_fScaleU; }
    float                   GetScaleV() const       { return m_fScaleV; }
    float                   GetOffsetU() const      { return m_fOffsetU; }
    float                   GetOffsetV() const      { return m_fOffsetV; }
    const CVec2f&           GetCenter() const       { return m_v2Center; }
    float                   GetSelectionWeight()    { return m_fSelectionWeight; }
    uint                    GetFlags() const        { return m_uiFlags; }
    const tsvector&         GetEmitters() const     { return m_vEmitters; }

    void        AddEmitterDef(IEmitterDef *pEmitterDef);
    const vector<IEmitterDef *>&    GetEmitterDefs() const  { return m_vEmitterDefs; }
};
//=============================================================================

//=============================================================================
// CSimpleParticle
//=============================================================================
class CSimpleParticle
{
private:
    bool        m_bActive;
    uint        m_uiStartTime;
    int         m_iLife;
    IEmitter    *m_pImbeddedEmitter;
    
    CVec3f      m_v3Pos;
    CVec3f      m_v3Velocity;
    CVec3f      m_v3Dir;
    float       m_fAcceleration;
    float       m_fScale;
    CVec3f      m_v3EmitterPos;

    CTemporalPropertyv3 m_tv3Color;
    CTemporalPropertyf  m_tfAlpha;
    CTemporalPropertyf  m_tfWidth;
    CTemporalPropertyf  m_tfHeight;
    CTemporalPropertyf  m_tfScale;
    CTemporalPropertyf  m_tfAngle;
    CTemporalPropertyf  m_tfPitch;
    CTemporalPropertyf  m_tfYaw;
    CTemporalPropertyf  m_tfFrame;
    CTemporalPropertyf  m_tfParam;
    CTemporalPropertyf  m_tfStickiness;
    CTemporalPropertyf  m_tfAnchor;

    // Speed distortions
    CTemporalPropertyf  m_tfWidthDistort;
    CTemporalPropertyf  m_tfHeightDistort;

    float       m_fS1, m_fS2;
    float       m_fT1, m_fT2;

    CVec2f      m_v2Center;

    uint        m_uiFlags;

public:
    ~CSimpleParticle();
    CSimpleParticle() : m_bActive(false), m_pImbeddedEmitter(NULL) {};

    void    Spawn
    (
        uint uiStartTime,
        int iLife,
        const CVec3f &v3Pos,
        const CVec3f &v3Velocity,
        const CVec3f &v3Dir,
        float fAcceleration,
        float fScale,
        const CVec3f &v3EmitterPos,
        IEmitter *pImbeddedEmitter,
        const CSimpleParticleDef &settings
    );

    bool    IsDead(uint uiLastUpdateTime, bool bExpired)    { return m_iLife == -1 ? bExpired : uiLastUpdateTime > m_uiStartTime + m_iLife; }

    void    Update(float fDeltaTime, const CVec3f &v3Acceleration, float fDrag, float fFriction, ParticleTraceFn_t pfnTrace = NULL);
    void    Update(float fDeltaTime, const CVec3f &v3Acceleration, const CVec3f &v3AccelTime, const CVec3f &v3AccelTimeSq);
    void    Update(float fDeltaTime);

    void    GetBillboard(uint uiMilliseconds, SBillboard &outBillboard, float *pfStickiness = NULL, float *pfAnchor = NULL);

    bool    IsActive()              { return m_bActive; }
    void    SetActive(bool bActive) { m_bActive = bActive; }

    uint    GetStartTime() const    { return m_uiStartTime; }
    void    AddToStartTime(uint uiPauseTime) { m_uiStartTime += uiPauseTime; }
    float   GetStickiness(uint uiMilliseconds) const    { return m_tfStickiness.Evaluate((m_iLife == -1) ? 0.0f : float(uiMilliseconds - m_uiStartTime) / m_iLife, (uiMilliseconds - m_uiStartTime) * SEC_PER_MS); }
    float   GetAnchor(uint uiMilliseconds) const    { return m_tfAnchor.Evaluate((m_iLife == -1) ? 0.0f : float(uiMilliseconds - m_uiStartTime) / m_iLife, (uiMilliseconds - m_uiStartTime) * SEC_PER_MS); }
    
    float   GetLerp(uint uiMilliseconds) const          { return (m_iLife == -1) ? 0.0f : float(uiMilliseconds - m_uiStartTime) / m_iLife; }
    float   GetTime(uint uiMilliseconds) const          { return (uiMilliseconds - m_uiStartTime) * SEC_PER_MS; }
    float   GetScale(float fLerp, float fTime) const    { return m_tfScale.Evaluate(fLerp, fTime) * m_fScale; }
    float   GetRoll(float fLerp, float fTime) const     { return m_tfAngle.Evaluate(fLerp, fTime); }
    float   GetPitch(float fLerp, float fTime) const    { return m_tfPitch.Evaluate(fLerp, fTime); }
    float   GetYaw(float fLerp, float fTime) const      { return m_tfYaw.Evaluate(fLerp, fTime); }
    
    const CVec3f&   GetPos() const          { return m_v3Pos; }
    const CVec3f&   GetDir() const          { return m_v3Dir; }
    const CVec3f&   GetVelocity() const     { return m_v3Velocity; }
    int             GetLife() const         { return m_iLife; }
    uint            GetFlags() const        { return m_uiFlags; }
    
    const CVec3f&   GetEmitterPos()         { return m_v3EmitterPos; }
    IEmitter*       GetImbeddedEmitter()    { return m_pImbeddedEmitter; }
    void            SetImbeddedEmitter(IEmitter *pEmitter)  { m_pImbeddedEmitter = pEmitter; }
    void            SetPos(const CVec3f &v3Pos)             { m_v3Pos = v3Pos; }
};

typedef vector<CSimpleParticle, CParticleAllocator<CSimpleParticle> >       ParticleList;
//=============================================================================

#endif  //__C_SIMPLEPARTICLE_H__
