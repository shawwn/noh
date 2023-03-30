// (C)2006 S2 Games
// c_meshemitter.h
//
//=============================================================================
#ifndef __C_MESHEMITTER_H__
#define __C_MESHEMITTER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_emitter.h"
#include "c_simpleparticle.h"
#include "c_temporalproperty.h"
#include "c_temporalpropertyrange.h"
#include "c_range.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CMeshEmitterDef
//=============================================================================
class CMeshEmitterDef : public IEmitterDef
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
    CTemporalPropertyRangef     m_rfMinInheritVelocity;
    CTemporalPropertyRangef     m_rfMaxInheritVelocity;
    CTemporalPropertyRangef     m_rfLimitInheritVelocity;
    ResHandle                   m_hMaterial;
    CVec3f                      m_v3Dir;
    EDirectionalSpace           m_eDirectionalSpace;
    float                       m_fDrag;
    float                       m_fFriction;
    tstring                     m_sMesh;
    CVec3f                      m_v3Pos;
    CVec3f                      m_v3Offset;
    CTemporalPropertyv3         m_tv3OffsetSphere;
    CTemporalPropertyv3         m_tv3OffsetCube;
    CTemporalPropertyRangef     m_rfMinOffsetDirection;
    CTemporalPropertyRangef     m_rfMaxOffsetDirection;
    CTemporalPropertyRangef     m_rfMinOffsetRadial;
    CTemporalPropertyRangef     m_rfMaxOffsetRadial;
    CTemporalPropertyRangef     m_rfMinOffsetRadialAngle;
    CTemporalPropertyRangef     m_rfMaxOffsetRadialAngle;
    bool                        m_bCollide;
    EDirectionalSpace           m_eParticleDirectionalSpace;
    CTemporalPropertyv3         m_tv3ParticleColor;
    CTemporalPropertyRangef     m_rfParticleAlpha;
    CTemporalPropertyRangef     m_rfParticleScale;
    float                       m_fDepthBias;

public:
    virtual ~CMeshEmitterDef();
    CMeshEmitterDef
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
        const CTemporalPropertyRangef &rfMinInheritVelocity,
        const CTemporalPropertyRangef &rfMaxInheritVelocity,
        const CTemporalPropertyRangef &rfLimitInheritVelocity,
        ResHandle hMaterial,
        const CVec3f &v3Dir,
        EDirectionalSpace eDirectionalSpace,
        float fDrag,
        float fFriction,
        const tstring &sMesh,
        const CVec3f &v3Pos,
        const CVec3f &v3Offset,
        const CTemporalPropertyv3 &tv3OffsetSphere,
        const CTemporalPropertyv3 &tv3OffsetCube,
        const CTemporalPropertyRangef &rfMinOffsetDirection,
        const CTemporalPropertyRangef &rfMaxOffsetDirection,
        const CTemporalPropertyRangef &rfMinOffsetRadial,
        const CTemporalPropertyRangef &rfMaxOffsetRadial,
        const CTemporalPropertyRangef &rfMinOffsetRadialAngle,
        const CTemporalPropertyRangef &rfMaxOffsetRadialAngle,
        bool bCollide,
        EDirectionalSpace eParticleDirectionalSpace,
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
    CTemporalPropertyf      GetMinInheritVelocity() const       { return m_rfMinInheritVelocity; }
    CTemporalPropertyf      GetMaxInheritVelocity() const       { return m_rfMaxInheritVelocity; }
    CTemporalPropertyf      GetLimitInheritVelocity() const     { return m_rfLimitInheritVelocity; }

    ResHandle               GetMaterial() const             { return m_hMaterial; }
    const CVec3f&           GetDir() const                  { return m_v3Dir; }
    EDirectionalSpace       GetDirectionalSpace() const     { return m_eDirectionalSpace; }
    float                   GetDrag() const                 { return m_fDrag; }
    float                   GetFriction() const             { return m_fFriction; }
    const tstring&          GetMesh() const                 { return m_sMesh; }
    const CVec3f&           GetPos() const                  { return m_v3Pos; }
    const CVec3f&           GetOffset() const               { return m_v3Offset; }

    CTemporalPropertyv3     GetOffsetSphere() const     { return m_tv3OffsetSphere; }
    CTemporalPropertyv3     GetOffsetCube() const       { return m_tv3OffsetCube; }
    CTemporalPropertyf      GetMinOffsetDirection() const   { return m_rfMinOffsetDirection; }
    CTemporalPropertyf      GetMaxOffsetDirection() const   { return m_rfMaxOffsetDirection; }
    CTemporalPropertyf      GetMinOffsetRadial() const  { return m_rfMinOffsetRadial; }
    CTemporalPropertyf      GetMaxOffsetRadial() const  { return m_rfMaxOffsetRadial; }
    CTemporalPropertyf      GetMinOffsetRadialAngle() const { return m_rfMinOffsetRadialAngle; }
    CTemporalPropertyf      GetMaxOffsetRadialAngle() const { return m_rfMaxOffsetRadialAngle; }

    bool                    GetCollide() const          { return m_bCollide; }

    EDirectionalSpace       GetParticleDirectionalSpace() const     { return m_eParticleDirectionalSpace; }

    const CTemporalPropertyv3&  GetParticleColor() const    { return m_tv3ParticleColor; }
    CTemporalPropertyf      GetParticleAlpha() const        { return m_rfParticleAlpha; }
    CTemporalPropertyf      GetParticleScale() const        { return m_rfParticleScale; }

    float                   GetDepthBias() const            { return m_fDepthBias; }
};

//=============================================================================
// CMeshEmitter
//=============================================================================
class CMeshEmitter : public IEmitter
{
private:
    float       m_fSelectionWeightRange;

    float       m_fAccumulator;

    uint        m_uiFrontSlot;
    uint        m_uiBackSlot;
    
    ParticleList    m_vParticles;

    int         m_iSpawnCount;

    CVec3f      m_v3LastBasePos;
    CVec3f      m_v3LastBaseVelocity;

    CVec3f      m_v3LastEmitterPos;

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
    CTemporalPropertyf      m_rfMinInheritVelocity;
    CTemporalPropertyf      m_rfMaxInheritVelocity;
    CTemporalPropertyf      m_rfLimitInheritVelocity;
    CTemporalPropertyf      m_rfMinOffsetDirection;
    CTemporalPropertyf      m_rfMaxOffsetDirection;
    CTemporalPropertyf      m_rfMinOffsetRadial;
    CTemporalPropertyf      m_rfMaxOffsetRadial;
    CTemporalPropertyf      m_rfMinOffsetRadialAngle;
    CTemporalPropertyf      m_rfMaxOffsetRadialAngle;

    ResHandle   m_hMaterial;
    CVec3f      m_v3Dir;
    float       m_fDrag;
    float       m_fFriction;
    tstring     m_sMesh;

    CTemporalPropertyv3     m_tv3OffsetSphere;
    CTemporalPropertyv3     m_tv3OffsetCube;

    bool        m_bCollide;

    EDirectionalSpace   m_eParticleDirectionalSpace;

    CTemporalPropertyv3         m_tv3ParticleColor;
    CTemporalPropertyf          m_tfParticleAlpha;
    CTemporalPropertyf          m_tfParticleScale;

    float           m_fDepthBias;

    bool    UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, IEmitter *pEmitter, CSimpleParticle &cParticle);

public:
    virtual ~CMeshEmitter();
    CMeshEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CMeshEmitterDef &eSettings);

    virtual void    ResumeFromPause(uint uiMilliseconds);
    bool    Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace);

    uint    GetNumBillboards();
    bool    GetBillboard(uint uiIndex, SBillboard &outBillboard);
};
//=============================================================================
#endif  //__C_MESHEMITTER_H__
