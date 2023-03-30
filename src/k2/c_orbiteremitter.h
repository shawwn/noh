// (C)2006 S2 Games
// c_orbiteremitter.h
//
//=============================================================================
#ifndef __C_ORBITEREMITTER_H__
#define __C_ORBITEREMITTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_emitter.h"
#include "c_orbiter.h"
#include "c_range.h"
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<COrbiter> OrbiterList;

class CParticleSystem;
//=============================================================================

//=============================================================================
// COrbiterEmitterDef
//=============================================================================
class COrbiterEmitterDef : public IEmitterDef
{
private:
    // Emitter Properties
    tstring                     m_sOwner;
    CRangei                     m_riLife;
    CRangei                     m_riExpireLife;
    CRangei                     m_riCount;
    CRangei                     m_riTimeNudge;
    CRangei                     m_riDelay;
    bool                        m_bLoop;
    CTemporalPropertyRangef     m_rfSpawnRate;
    CTemporalPropertyRangei     m_riMinParticleLife;
    CTemporalPropertyRangei     m_riMaxParticleLife;
    CTemporalPropertyRangei     m_riParticleTimeNudge;
    CTemporalPropertyRangef     m_rfGravity;
    CTemporalPropertyRangef     m_rfMinSpeed;
    CTemporalPropertyRangef     m_rfMaxSpeed;
    CTemporalPropertyRangef     m_rfMinAcceleration;
    CTemporalPropertyRangef     m_rfMaxAcceleration;
    CTemporalPropertyRangef     m_rfMinAngle;
    CTemporalPropertyRangef     m_rfMaxAngle;
    ResHandle                   m_hMaterial;
    CVec3f                      m_v3Dir;
    EDirectionalSpace           m_eDirectionalSpace;
    float                       m_fDrag;
    float                       m_fFriction;
    tstring                     m_sBone;
    CVec3f                      m_v3Pos;
    CVec3f                      m_v3Offset;
    CVec3f                      m_v3Origin;
    CTemporalPropertyv3         m_tv3Offset;
    bool                        m_bCylindrical;
    CTemporalPropertyv3         m_tv3Orbit;
    CTemporalPropertyRangef     m_rfMinOrbitAngle;
    CTemporalPropertyRangef     m_rfMaxOrbitAngle;
    CTemporalPropertyv3         m_tv3ParticleColor;
    CTemporalPropertyRangef     m_rfParticleAlpha;
    CTemporalPropertyRangef     m_rfParticleScale;
    float                       m_fDepthBias;

public:
    virtual ~COrbiterEmitterDef();
    COrbiterEmitterDef
    (
        const tstring &sOwner,
        const CRangei &riLife,
        const CRangei &riExpireLife,
        const CRangei &riCount,
        const CRangei &riTimeNudge,
        const CRangei &riDelay,
        bool bLoop,
        const CTemporalPropertyRangef &rfSpawnRate,
        const CTemporalPropertyRangei &riMinParticleLife,
        const CTemporalPropertyRangei &riMaxParticleLife,
        const CTemporalPropertyRangei &riParticleTimeNudge,
        const CTemporalPropertyRangef &rfGravity,
        const CTemporalPropertyRangef &rfMinSpeed,
        const CTemporalPropertyRangef &rfMaxSpeed,
        const CTemporalPropertyRangef &rfMinAcceleration,
        const CTemporalPropertyRangef &rfMaxAcceleration,
        const CTemporalPropertyRangef &rfMinAngle,
        const CTemporalPropertyRangef &rfMaxAngle,
        ResHandle hMaterial,
        const CVec3f &v3Dir,
        EDirectionalSpace eDirectionalSpace,
        float fDrag,
        float fFriction,
        const tstring &sBone,
        const CVec3f &v3Pos,
        const CVec3f &v3Offset,
        const CVec3f &v3Origin,
        const CTemporalPropertyv3 &tv3Offset,
        bool bCylindrical,
        const CTemporalPropertyv3 &tv3Orbit,
        const CTemporalPropertyRangef &rfMinOrbitAngle,
        const CTemporalPropertyRangef &rfMaxOrbitAngle,
        const CTemporalPropertyv3 &tv3ParticleColor,
        const CTemporalPropertyRangef &m_rfParticleAlpha,
        const CTemporalPropertyRangef &m_rfParticleScale,
        float fDepthBias
    );

    IEmitter*   Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner);

    const tstring&  GetOwner() const    { return m_sOwner; }
    int         GetLife() const         { return m_riLife; }
    int         GetExpireLife() const   { return m_riExpireLife; }
    int         GetCount() const        { return m_riCount; }
    int         GetTimeNudge() const    { return m_riTimeNudge; }
    int         GetDelay() const        { return m_riDelay; }
    bool        GetLoop() const         { return m_bLoop; }

    CTemporalPropertyf      GetSpawnRate() const    { return m_rfSpawnRate; }
    CTemporalPropertyi      GetMinParticleLife() const  { return m_riMinParticleLife; }
    CTemporalPropertyi      GetMaxParticleLife() const  { return m_riMaxParticleLife; }
    CTemporalPropertyi      GetParticleTimeNudge() const    { return m_riParticleTimeNudge; }
    CTemporalPropertyf      GetGravity() const      { return m_rfGravity; }
    CTemporalPropertyf      GetMinSpeed() const     { return m_rfMinSpeed; }
    CTemporalPropertyf      GetMaxSpeed() const     { return m_rfMaxSpeed; }
    CTemporalPropertyf      GetMinAcceleration() const  { return m_rfMinAcceleration; }
    CTemporalPropertyf      GetMaxAcceleration() const  { return m_rfMaxAcceleration; }
    CTemporalPropertyf      GetMinAngle() const     { return m_rfMinAngle; }
    CTemporalPropertyf      GetMaxAngle() const     { return m_rfMaxAngle; }

    ResHandle               GetMaterial() const     { return m_hMaterial; }
    const CVec3f&           GetDir() const          { return m_v3Dir; }
    EDirectionalSpace       GetDirectionalSpace() const     { return m_eDirectionalSpace; }
    float                   GetDrag() const         { return m_fDrag; }
    float                   GetFriction() const     { return m_fFriction; }
    const tstring&          GetBone() const         { return m_sBone; }
    const CVec3f&           GetPos() const          { return m_v3Pos; }
    const CVec3f&           GetOffset() const       { return m_v3Offset; }
    const CVec3f&           GetOrigin() const       { return m_v3Origin; }

    CTemporalPropertyv3     GetOrbitOffset() const  { return m_tv3Offset; }
    bool                    GetCylindrical() const  { return m_bCylindrical; }
    CTemporalPropertyv3     GetOrbit() const        { return m_tv3Orbit; }
    CTemporalPropertyf      GetMinOrbitAngle() const    { return m_rfMinOrbitAngle; }
    CTemporalPropertyf      GetMaxOrbitAngle() const    { return m_rfMaxOrbitAngle; }

    const CTemporalPropertyv3&  GetParticleColor() const    { return m_tv3ParticleColor; }
    CTemporalPropertyf      GetParticleAlpha() const        { return m_rfParticleAlpha; }
    CTemporalPropertyf      GetParticleScale() const        { return m_rfParticleScale; }

    float                   GetDepthBias() const            { return m_fDepthBias; }
};

//=============================================================================
// COrbiterEmitter
//=============================================================================
class COrbiterEmitter : public IEmitter
{
private:
    float       m_fSelectionWeightRange;

    float       m_fAccumulator;

    uint        m_uiFrontSlot;
    uint        m_uiBackSlot;

    OrbiterList     m_vParticles;

    int         m_iSpawnCount;

    CAxis       m_aLastBoneAxis;
    CVec3f      m_v3LastBonePos;

    float       m_fLastLerp;
    float       m_fLastTime;

    bool        m_bLastActive;

    // Emitter Properties
    int         m_iCount;

    CTemporalPropertyf      m_rfSpawnRate;
    CTemporalPropertyi      m_riMinParticleLife;
    CTemporalPropertyi      m_riMaxParticleLife;
    CTemporalPropertyi      m_riParticleTimeNudge;
    CTemporalPropertyf      m_rfGravity;
    CTemporalPropertyf      m_rfMinSpeed;
    CTemporalPropertyf      m_rfMaxSpeed;
    CTemporalPropertyf      m_rfMinAcceleration;
    CTemporalPropertyf      m_rfMaxAcceleration;
    CTemporalPropertyf      m_rfMinAngle;
    CTemporalPropertyf      m_rfMaxAngle;

    ResHandle   m_hMaterial;
    CVec3f      m_v3Dir;
    float       m_fDrag;
    float       m_fFriction;
    CVec3f      m_v3Origin;

    CTemporalPropertyv3     m_tv3Offset;
    bool        m_bCylindrical;
    CTemporalPropertyv3     m_tv3Orbit;
    CTemporalPropertyf      m_rfMinOrbitAngle;
    CTemporalPropertyf      m_rfMaxOrbitAngle;

    CTemporalPropertyv3         m_tv3ParticleColor;
    CTemporalPropertyf          m_tfParticleAlpha;
    CTemporalPropertyf          m_tfParticleScale;

    float           m_fDepthBias;

public:
    virtual ~COrbiterEmitter();
    COrbiterEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const COrbiterEmitterDef &eSettings);

    virtual void    ResumeFromPause(uint uiMilliseconds);
    bool    Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

    uint    GetNumBillboards();
    bool    GetBillboard(uint uiIndex, SBillboard &outBillboard);
};
//=============================================================================
#endif  //__C_ORBITEREMITTER_H__
