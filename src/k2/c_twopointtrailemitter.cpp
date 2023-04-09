// (C)2006 S2 Games
// c_twopointtrailemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_twopointtrailemitter.h"
#include "c_simpleparticle.h"
#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CTwoPointTrailEmitterDef::~CTwoPointTrailEmitterDef
  ====================*/
CTwoPointTrailEmitterDef::~CTwoPointTrailEmitterDef()
{
}


/*====================
  CTwoPointTrailEmitterDef::CTwoPointTrailEmitterDef
  ====================*/
CTwoPointTrailEmitterDef::CTwoPointTrailEmitterDef
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
    float fDrag,
    float fFriction,
    const tstring &sBoneA,
    const tstring &sBoneB,
    const CVec3f &v3PosA,
    const CVec3f &v3PosB,
    const CTemporalPropertyv3   &tv3OffsetSphere,
    const CTemporalPropertyv3   &tv3OffsetCube,
    const CTemporalPropertyRangef &rfMinOffsetDirection,
    const CTemporalPropertyRangef &rfMaxOffsetDirection,
    const CTemporalPropertyRangef &rfMinOffsetRadial,
    const CTemporalPropertyRangef &rfMaxOffsetRadial,
    const CTemporalPropertyRangef &rfMinOffsetRadialAngle,
    const CTemporalPropertyRangef &rfMaxOffsetRadialAngle,
    const CTemporalPropertyRangei &riTexPosTime,
    const CTemporalPropertyRangef &rfTexPosScale,
    const CTemporalPropertyRangef &rfTexStretchScale,
    bool bSubFramePose,
    const CTemporalPropertyv3 &tv3ParticleColor,
    const CTemporalPropertyRangef &rfParticleAlpha
) :
m_sOwner(sOwner),
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riCount(riCount),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_rfSpawnRate(rfSpawnRate),
m_riMinParticleLife(riMinParticleLife),
m_riMaxParticleLife(riMaxParticleLife),
m_riParticleTimeNudge(riParticleTimeNudge),
m_rfGravity(rfGravity),
m_rfMinSpeed(rfMinSpeed),
m_rfMaxSpeed(rfMaxSpeed),
m_rfMinAcceleration(rfMinAcceleration),
m_rfMaxAcceleration(rfMaxAcceleration),
m_rfMinAngle(rfMinAngle),
m_rfMaxAngle(rfMaxAngle),
m_rfMinInheritVelocity(rfMinInheritVelocity),
m_rfMaxInheritVelocity(rfMaxInheritVelocity),
m_rfLimitInheritVelocity(rfLimitInheritVelocity),
m_hMaterial(hMaterial),
m_v3Dir(v3Dir),
m_fDrag(fDrag),
m_fFriction(fFriction),
m_sBoneA(sBoneA),
m_sBoneB(sBoneB),
m_v3PosA(v3PosA),
m_v3PosB(v3PosB),
m_tv3OffsetSphere(tv3OffsetSphere),
m_tv3OffsetCube(tv3OffsetCube),
m_rfMinOffsetDirection(rfMinOffsetDirection),
m_rfMaxOffsetDirection(rfMaxOffsetDirection),
m_rfMinOffsetRadial(rfMinOffsetRadial),
m_rfMaxOffsetRadial(rfMaxOffsetRadial),
m_rfMinOffsetRadialAngle(rfMinOffsetRadialAngle),
m_rfMaxOffsetRadialAngle(rfMaxOffsetRadialAngle),
m_riTexPosTime(riTexPosTime),
m_rfTexPosScale(rfTexPosScale),
m_rfTexStretchScale(rfTexStretchScale),
m_bSubFramePose(bSubFramePose),
m_tv3ParticleColor(tv3ParticleColor),
m_rfParticleAlpha(rfParticleAlpha)
{
}


/*====================
  CTwoPointTrailEmitterDef::Spawn
  ====================*/
IEmitter*   CTwoPointTrailEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CTwoPointTrailEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CTwoPointTrailEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CTwoPointTrailEmitter::~CTwoPointTrailEmitter
  ====================*/
CTwoPointTrailEmitter::~CTwoPointTrailEmitter()
{
}


/*====================
  CTwoPointTrailEmitter::CTwoPointTrailEmitter
  ====================*/
CTwoPointTrailEmitter::CTwoPointTrailEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CTwoPointTrailEmitterDef &eSettings) :
IEmitter
(
    eSettings.GetLife(),
    eSettings.GetExpireLife(),
    eSettings.GetTimeNudge(),
    eSettings.GetDelay(),
    eSettings.GetLoop(),
    TSNULL,
    eSettings.GetOwner(),
    TSNULL,
    V3_ZERO,
    V3_ZERO,
    DIRSPACE_LOCAL,
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_fSelectionWeightRange(0.0f),
m_fAccumulator(0.0f),
m_iSpawnCount(0),
m_v3LastBasePosA(V3_ZERO),
m_v3LastBasePosB(V3_ZERO),
m_v3LastBaseVelocityA(V3_ZERO),
m_v3LastBaseVelocityB(V3_ZERO),
m_iCount(eSettings.GetCount()),
m_rfSpawnRate(eSettings.GetSpawnRate()),
m_riMinParticleLife(eSettings.GetMinParticleLife()),
m_riMaxParticleLife(eSettings.GetMaxParticleLife()),
m_riParticleTimeNudge(eSettings.GetParticleTimeNudge()),
m_rfGravity(eSettings.GetGravity()),
m_rfMinSpeed(eSettings.GetMinSpeed()),
m_rfMaxSpeed(eSettings.GetMaxSpeed()),
m_rfMinAcceleration(eSettings.GetMinAcceleration()),
m_rfMaxAcceleration(eSettings.GetMaxAcceleration()),
m_rfMinAngle(eSettings.GetMinAngle()),
m_rfMaxAngle(eSettings.GetMaxAngle()),
m_rfMinInheritVelocity(eSettings.GetMinInheritVelocity()),
m_rfMaxInheritVelocity(eSettings.GetMaxInheritVelocity()),
m_rfLimitInheritVelocity(eSettings.GetLimitInheritVelocity()),
m_rfMinOffsetDirection(eSettings.GetMinOffsetDirection()),
m_rfMaxOffsetDirection(eSettings.GetMaxOffsetDirection()),
m_rfMinOffsetRadial(eSettings.GetMinOffsetRadial()),
m_rfMaxOffsetRadial(eSettings.GetMaxOffsetRadial()),
m_rfMinOffsetRadialAngle(eSettings.GetMinOffsetRadialAngle()),
m_rfMaxOffsetRadialAngle(eSettings.GetMaxOffsetRadialAngle()),
m_hMaterial(eSettings.GetMaterial()),
m_v3Dir(eSettings.GetDir()),
m_fDrag(eSettings.GetDrag()),
m_fFriction(eSettings.GetFriction()),
m_sBoneA(eSettings.GetBoneA()),
m_sBoneB(eSettings.GetBoneB()),
m_v3PosA(eSettings.GetPosA()),
m_v3PosB(eSettings.GetPosB()),
m_tv3OffsetSphere(eSettings.GetOffsetSphere()),
m_tv3OffsetCube(eSettings.GetOffsetCube()),
m_riTexPosTime(eSettings.GetTexPosTime()),
m_rfTexPosScale(eSettings.GetTexPosScale()),
m_rfTexStretchScale(eSettings.GetTexStretchScale()),
m_bSubFramePose(eSettings.GetSubFramePose()),
m_tv3ParticleColor(eSettings.GetParticleColor()),
m_tfParticleAlpha(eSettings.GetParticleAlpha())
{
    m_v3LastBasePosA = m_v3PosA;
    m_v3LastBasePosB = m_v3PosB;

    int iMaxActive;

    if (m_riMaxParticleLife.Max() == -1)
        iMaxActive = 1024;
    else
        iMaxActive = INT_CEIL(m_rfSpawnRate.Max() * (m_riMaxParticleLife.Max() * SEC_PER_MS)) + 2;

    if (m_iCount != -1)
    {
        if (iMaxActive > m_iCount + 1)
            iMaxActive = m_iCount + 1;
    }

    if (m_iLife != -1)
    {
        int iMaxSpawned(INT_CEIL(m_rfSpawnRate.Max() * (m_iLife * SEC_PER_MS)) + 2);

        if (iMaxActive > iMaxSpawned)
            iMaxActive = iMaxSpawned;
    }

    m_vTrackA.resize(iMaxActive);
    m_vTrackB.resize(iMaxActive);

    m_uiFrontSlot = 0;
    m_uiBackSlot = 0;

    // Initialize Last* vars
    m_v3LastPos = GetPosition();
    m_aLastAxis = GetAxis();
    m_fLastScale = GetScale();

    m_v3LastBasePosA = TransformPoint(m_v3PosA + GetBonePosition(uiStartTime, m_pOwner, m_sBoneA), m_aLastAxis, m_v3LastPos, m_fLastScale);
    m_v3LastBasePosB = TransformPoint(m_v3PosB + GetBonePosition(uiStartTime, m_pOwner, m_sBoneB), m_aLastAxis, m_v3LastPos, m_fLastScale);

    if (!m_bSubFramePose)
    {
        GetBoneAxisPos(uiStartTime, m_pOwner, m_sBoneA, m_aLastBoneAxisA, m_v3LastBonePosA);
        GetBoneAxisPos(uiStartTime, m_pOwner, m_sBoneB, m_aLastBoneAxisB, m_v3LastBonePosB);
    }

    m_bLastActive = m_pParticleSystem->GetActive() && GetVisibility(m_sBoneA, m_pOwner) && GetVisibility(m_sBoneB, m_pOwner);

    // Force first particle to connect to spawn location
    if (m_bLastActive)
        m_fAccumulator += 1.0f;

    // Initialize m_fSelectionWeightRange
    for (vector<CSimpleParticleDef *>::const_iterator it(m_pvParticleDefinitions->begin()); it != m_pvParticleDefinitions->end(); ++it)
        m_fSelectionWeightRange += (*it)->GetSelectionWeight();

    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;

    m_bActive = true;

    m_fLastLerp = 0.0f;
    m_fLastTime = 0.0f;
}


/*====================
  CTwoPointTrailEmitter::ResumeFromPause
  ====================*/
void    CTwoPointTrailEmitter::ResumeFromPause(uint uiMilliseconds)
{
    uint uiPauseDuration(uiMilliseconds - m_uiPauseBegin);
    m_uiStartTime += uiPauseDuration;
    m_uiLastUpdateTime = uiMilliseconds;
    m_uiPauseBegin = 0;

    TrackBuffer::iterator itA(m_vTrackA.begin());
    TrackBuffer::iterator itB(m_vTrackB.begin());

    for (; itA != m_vTrackA.end() && itB != m_vTrackB.end(); ++itA, ++itB)
    {
        itA->AddToStartTime(uiPauseDuration);
        itB->AddToStartTime(uiPauseDuration);
    }
}


/*====================
  CTwoPointTrailEmitter::Update
  ====================*/
bool    CTwoPointTrailEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CTwoPointTrailEmitter::Update");

    if (m_uiPauseBegin)
        ResumeFromPause(uiMilliseconds);

    int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

    if (iDeltaTime <= 0)
    {
        UpdateNextEmitter(uiMilliseconds, pfnTrace);
        return true;
    }

#if 0
    if (iDeltaTime > 0xffff)
    {
        Console.Warn << m_pParticleSystem->GetEffect()->GetPath() << _T(": <twopointtrailemitter> iDeltaTime == ") << iDeltaTime << newl;
        iDeltaTime = 0xffff;
    }
#else
    if (iDeltaTime > 0x1000)
        iDeltaTime = 0x1000;
#endif

    float fDeltaTime(iDeltaTime * SEC_PER_MS);

    // Calculate temporal properties
    float fTime((uiMilliseconds - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= uiMilliseconds && (m_iLife == -1 || m_bLoop) && m_iExpireLife != 0.0f)
        fLerp = m_iExpireLife != -1 ? MIN(float(uiMilliseconds - m_uiExpireTime) / m_iExpireLife, 1.0f) : 0.0f;
    else
        fLerp = m_iLife != -1 ? MIN(float(uiMilliseconds - m_uiStartTime) / m_iLife, 1.0f) : 0.0f;

    float   fSpawnRate(m_rfSpawnRate.Evaluate(fLerp, fTime));
    float   fGravity(m_rfGravity.Evaluate(fLerp, fTime));

    m_bbBounds.Clear();

    bool bExpired(m_uiExpireTime != INVALID_TIME);

    // Update existing particles
    TrackBuffer::iterator itA(m_vTrackA.begin());
    TrackBuffer::iterator itB(m_vTrackB.begin());

    for (; itA != m_vTrackA.end() && itB != m_vTrackB.end(); ++itA, ++itB)
    {
        if (!itA->IsActive() || !itB->IsActive())
            continue;

        itA->Update(fDeltaTime, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);
        itB->Update(fDeltaTime, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);

        if (itA->IsDead(uiMilliseconds, bExpired) || itB->IsDead(uiMilliseconds, bExpired))
        {
            itA->SetActive(false);
            itB->SetActive(false);
        }
        else
        {
            m_bbBounds.AddPoint(itA->GetPos());
            m_bbBounds.AddPoint(itB->GetPos());
        }
    }

    while (!m_vTrackA[m_uiFrontSlot].IsActive() && m_uiFrontSlot != m_uiBackSlot)
        m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vTrackA.size();

    bool bActive(m_bActive && m_pParticleSystem->GetActive());

    if (m_uiExpireTime != INVALID_TIME && (m_iLife == -1 || m_bLoop))
        if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
            bActive = false;

    if (!GetVisibility(m_sBoneA) || !GetVisibility(m_sBoneB))
        bActive = false;

    CVec3f v3Pos(GetPosition());
    CAxis aAxis(GetAxis());
    float fScale(GetScale());

    bool bLerpAxis(false);
    vec4_t quat0;
    vec4_t quat1;

    M_AxisToQuat((const vec3_t *)(&m_aLastAxis), quat0);
    M_AxisToQuat((const vec3_t *)(&aAxis), quat1);

    bLerpAxis = !M_CompareVec4(quat0, quat1, 0.001f);

    CAxis   aBoneAxisA;

    CAxis   aBoneAxisB;
    CVec3f  v3BonePosA;
    CVec3f  v3BonePosB;
    CVec3f  v3BasePosA;
    CVec3f  v3BasePosB;

    if (m_bSubFramePose)
    {
        v3BasePosA = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBoneA), aAxis, v3Pos, fScale);
        v3BasePosB = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBoneB), aAxis, v3Pos, fScale);
    }
    else
    {
        v3BonePosA = GetBonePosition(uiMilliseconds, m_pOwner, m_sBoneA);
        v3BasePosA = TransformPoint(v3BonePosA, aAxis, v3Pos, fScale);

        v3BonePosB = GetBonePosition(uiMilliseconds, m_pOwner, m_sBoneB);
        v3BasePosB = TransformPoint(v3BonePosB, aAxis, v3Pos, fScale);
    }

    CVec3f  v3BaseVelocityA((v3BasePosA - m_v3LastBasePosA) / fDeltaTime);
    CVec3f  v3BaseVelocityB((v3BasePosB - m_v3LastBasePosB) / fDeltaTime);

    CVec3f  v3OffsetSphere(m_tv3OffsetSphere.Lerp(fLerp));
    CVec3f  v3OffsetCube(m_tv3OffsetCube.Lerp(fLerp));

    float fClampedDeltaTime;

    if (m_iLife != -1)
        fClampedDeltaTime = MIN(fDeltaTime, (m_iLife + int(m_uiStartTime) - int(m_uiLastUpdateTime)) * SEC_PER_MS);
    else
        fClampedDeltaTime = fDeltaTime;

    if (fClampedDeltaTime < 0.0f)
        fClampedDeltaTime = 0.0f;

    if (m_iCount != -1)
    {
        if (bActive && m_bLastActive)
        {
            m_fAccumulator += fSpawnRate * fClampedDeltaTime;
        }
        else
        {
            m_fAccumulator += fSpawnRate * fClampedDeltaTime;

            if (m_fAccumulator >= 1.0f)
            {
                m_iSpawnCount += INT_FLOOR(m_fAccumulator);
                m_fAccumulator = fmod(m_fAccumulator, 1.0f);
            }
        }
    }
    else if (bActive && m_bLastActive)
    {
        m_fAccumulator += fSpawnRate * fClampedDeltaTime;
    }

    // Spawn new particles
    while (m_fAccumulator >= 1.0f && (m_iCount == -1 || m_iSpawnCount < m_iCount))
    {
        m_fAccumulator -= 1.0f;

        float   fTimeNudge(m_fAccumulator / fSpawnRate);
        uint    uiMillisecondNudge(INT_FLOOR(fTimeNudge * MS_PER_SEC));
        float   fLerp(m_iLife != -1 ? CLAMP(ILERP(uiMilliseconds - m_uiStartTime - uiMillisecondNudge, 0u, uint(m_iLife)), 0.0f, 1.0f) : 0.0f);

        // Calculate new particle temporal properties
        int     iMinParticleLife(m_riMinParticleLife.Lerp(fLerp));
        int     iMaxParticleLife(m_riMaxParticleLife.Lerp(fLerp));
        int     iParticleTimeNudge(m_riParticleTimeNudge.Lerp(fLerp));

        int iParticleLife(M_Randnum(iMinParticleLife, iMaxParticleLife));
        if (iParticleLife != -1 && uint(iParticleLife) < uiMillisecondNudge + iParticleTimeNudge)
        {
            // Skip dead particles
            ++m_iSpawnCount;
            continue;
        }

        float   fMinSpeed(m_rfMinSpeed.Lerp(fLerp));
        float   fMaxSpeed(m_rfMaxSpeed.Lerp(fLerp));
        float   fMinAcceleration(m_rfMinAcceleration.Lerp(fLerp));
        float   fMaxAcceleration(m_rfMaxAcceleration.Lerp(fLerp));
        float   fMinAngle(m_rfMinAngle.Lerp(fLerp));
        float   fMaxAngle(m_rfMaxAngle.Lerp(fLerp));
        float   fMinInheritVelocity(m_rfMinInheritVelocity.Lerp(fLerp));
        float   fMaxInheritVelocity(m_rfMaxInheritVelocity.Lerp(fLerp));
        float   fMinOffsetDirection(m_rfMinOffsetDirection.Lerp(fLerp));
        float   fMaxOffsetDirection(m_rfMaxOffsetDirection.Lerp(fLerp));
        float   fMinOffsetRadial(m_rfMinOffsetRadial.Lerp(fLerp));
        float   fMaxOffsetRadial(m_rfMaxOffsetRadial.Lerp(fLerp));
        float   fMinOffsetRadialAngle(m_rfMinOffsetRadialAngle.Lerp(fLerp));
        float   fMaxOffsetRadialAngle(m_rfMaxOffsetRadialAngle.Lerp(fLerp));

        float fFrameLerp(CLAMP(1.0f - fTimeNudge / fDeltaTime, 0.0f, 1.0f));

        CVec3f  v3LerpedPos(LERP(fFrameLerp, m_v3LastPos, v3Pos));
        CAxis   aLerpedAxis;

        if (bLerpAxis)
        {
            vec4_t  lerpedQuat;
            M_LerpQuat(fFrameLerp, quat0, quat1, lerpedQuat);

            M_QuatToAxis(lerpedQuat, (vec3_t *)(&aLerpedAxis));
        }
        else
            aLerpedAxis = aAxis;

        float fLerpedScale(LERP(fFrameLerp, m_fLastScale, fScale));

        CVec3f  v3Dir(fMaxAngle - fMinAngle >= 180.0f ? M_RandomDirection() : M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle));

        if (m_eDirectionalSpace == DIRSPACE_LOCAL)
            v3Dir = TransformPoint(v3Dir, aLerpedAxis);

        //
        // Track A Position
        //

        CVec3f  v3CurrentBonePosA;

        if (m_bSubFramePose)
            v3CurrentBonePosA = GetBonePosition(uiMilliseconds - uiMillisecondNudge, m_pOwner, m_sBoneA);
        else
            v3CurrentBonePosA = LERP(fFrameLerp, m_v3LastBonePosA, v3BonePosA);

        CVec3f  v3OffsetPosA(0.0f, 0.0f, 0.0f);
        {
            if (v3OffsetSphere != V3_ZERO)
            {
                CVec3f  v3Rand(M_RandomPointInSphere());

                v3OffsetPosA += v3OffsetSphere * v3Rand;
            }

            if (v3OffsetCube != V3_ZERO)
            {
                CVec3f  v3Rand(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

                v3OffsetPosA += v3OffsetCube * v3Rand;
            }

            float   fOffsetDirection(M_Randnum(fMinOffsetDirection, fMaxOffsetDirection));
            if (fOffsetDirection != 0.0f)
            {
                v3OffsetPosA += v3Dir * fOffsetDirection;
            }

            float   fOffsetRadial(M_Randnum(fMinOffsetRadial, fMaxOffsetRadial));
            if (fOffsetRadial != 0.0f)
            {
                v3OffsetPosA += M_RandomDirection(m_v3Dir, fMinOffsetRadialAngle, fMaxOffsetRadialAngle) * fOffsetRadial;
            }
        }

        CVec3f  v3PositionA(TransformPoint(v3OffsetPosA + v3CurrentBonePosA, aLerpedAxis, v3LerpedPos, fLerpedScale));

        //
        // Track B Position
        //

        CVec3f  v3CurrentBonePosB;

        if (m_bSubFramePose)
            v3CurrentBonePosB = GetBonePosition(uiMilliseconds - uiMillisecondNudge, m_pOwner, m_sBoneB);
        else
            v3CurrentBonePosB = LERP(fFrameLerp, m_v3LastBonePosB, v3BonePosB);

        CVec3f  v3OffsetPosB(0.0f, 0.0f, 0.0f);
        {
            if (v3OffsetSphere != V3_ZERO)
            {
                CVec3f  v3Rand(M_RandomPointInSphere());

                v3OffsetPosB += v3OffsetSphere * v3Rand;
            }

            if (v3OffsetCube != V3_ZERO)
            {
                CVec3f  v3Rand(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

                v3OffsetPosB += v3OffsetCube * v3Rand;
            }

            float   fOffsetDirection(M_Randnum(fMinOffsetDirection, fMaxOffsetDirection));
            if (fOffsetDirection != 0.0f)
            {
                v3OffsetPosB += v3Dir * fOffsetDirection;
            }

            float   fOffsetRadial(M_Randnum(fMinOffsetRadial, fMaxOffsetRadial));
            if (fOffsetRadial != 0.0f)
            {
                v3OffsetPosB += M_RandomDirection(m_v3Dir, fMinOffsetRadialAngle, fMaxOffsetRadialAngle) * fOffsetRadial;
            }
        }

        CVec3f  v3PositionB(TransformPoint(v3OffsetPosB + v3CurrentBonePosB, aLerpedAxis, v3LerpedPos, fLerpedScale));

        //
        // Velocity
        //

        float   fLimitInheritVelocity(m_rfLimitInheritVelocity.Lerp(fLerp) * fLerpedScale);

        CVec3f  v3VelocityA(v3Dir * (M_Randnum(fMinSpeed, fMaxSpeed) * fLerpedScale));
        CVec3f  v3InheritVelocityA(LERP(fTimeNudge / fDeltaTime, v3BaseVelocityA, m_v3LastBaseVelocityA) * M_Randnum(fMinInheritVelocity, fMaxInheritVelocity));
        if (fLimitInheritVelocity > 0.0f && v3InheritVelocityA.Length() > fLimitInheritVelocity)
            v3InheritVelocityA.SetLength(fLimitInheritVelocity);
        v3VelocityA += v3InheritVelocityA;

        CVec3f  v3VelocityB(v3Dir * (M_Randnum(fMinSpeed, fMaxSpeed) * fLerpedScale));
        CVec3f  v3InheritVelocityB(LERP(fTimeNudge / fDeltaTime, v3BaseVelocityB, m_v3LastBaseVelocityB) * M_Randnum(fMinInheritVelocity, fMaxInheritVelocity));
        if (fLimitInheritVelocity > 0.0f && v3InheritVelocityB.Length() > fLimitInheritVelocity)
            v3InheritVelocityB.SetLength(fLimitInheritVelocity);
        v3VelocityB += v3InheritVelocityB;

        float   fAcceleration(M_Randnum(fMinAcceleration, fMaxAcceleration) * fLerpedScale);

        //
        // Particle selection and spawning
        //

        uiMillisecondNudge += iParticleTimeNudge;
        fTimeNudge += iParticleTimeNudge * SEC_PER_MS;

        vector<CSimpleParticleDef *>::const_iterator    itDef(m_pvParticleDefinitions->begin());
        if (m_pvParticleDefinitions->size() > 1)
        {
            float   fRand(M_Randnum(0.0f, m_fSelectionWeightRange));

            while (itDef != m_pvParticleDefinitions->end())
            {
                fRand -= (*itDef)->GetSelectionWeight();

                if (fRand > 0.0f)
                    ++itDef;
                else
                    break;
            }
        }

        if (itDef == m_pvParticleDefinitions->end())
            continue;

        uint uiSlot(m_uiBackSlot % m_vTrackA.size());

        m_vTrackA[uiSlot].Spawn
        (
            uiMilliseconds - uiMillisecondNudge,
            iParticleLife,
            v3PositionA,
            v3VelocityA,
            v3Dir,
            fAcceleration,
            1.0f,
            TransformPoint(v3CurrentBonePosA, aLerpedAxis, v3LerpedPos, fLerpedScale),
            NULL,
            **itDef
        );
        m_vTrackA[uiSlot].SetActive(true);

        m_vTrackB[uiSlot].Spawn
        (
            uiMilliseconds - uiMillisecondNudge,
            iParticleLife,
            v3PositionB,
            v3VelocityB,
            v3Dir,
            fAcceleration,
            1.0f,
            TransformPoint(v3CurrentBonePosB, aLerpedAxis, v3LerpedPos, fLerpedScale),
            NULL,
            **itDef
        );
        m_vTrackB[uiSlot].SetActive(true);

        // Update the new particle to catch up to this frame
        m_vTrackA[uiSlot].Update(fTimeNudge, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);
        m_vTrackB[uiSlot].Update(fTimeNudge, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);

        if (m_vTrackA[uiSlot].IsDead(uiMilliseconds, bExpired) || m_vTrackB[uiSlot].IsDead(uiMilliseconds, bExpired))
        {
            m_vTrackA[uiSlot].SetActive(false);
            m_vTrackB[uiSlot].SetActive(false);
        }
        else
        {
            m_bbBounds.AddPoint(m_vTrackA[uiSlot].GetPos());
            m_bbBounds.AddPoint(m_vTrackB[uiSlot].GetPos());
        }

        m_uiBackSlot = (uiSlot + 1) % m_vTrackA.size();

        // Push front slot forward if we wrapped around and caught it
        if (m_uiBackSlot == m_uiFrontSlot)
            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vTrackA.size();

        while (!m_vTrackA[m_uiFrontSlot].IsActive() && m_uiFrontSlot != m_uiBackSlot)
            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vTrackA.size();

        ++m_iSpawnCount;
    }

    m_v3LastPos = v3Pos;
    m_aLastAxis = aAxis;
    m_fLastScale = fScale;
    m_v3LastBasePosA = v3BasePosA;
    m_v3LastBasePosB = v3BasePosB;
    m_v3LastBaseVelocityA = v3BaseVelocityA;
    m_v3LastBaseVelocityB = v3BaseVelocityB;

    if (!m_bSubFramePose)
    {
        m_v3LastBonePosA = v3BonePosA;
        m_v3LastBonePosB = v3BonePosB;

        m_v3LastEmitterPosA = TransformPoint(v3BonePosA, m_aLastAxis, m_v3LastPos, m_fLastScale);
        m_v3LastEmitterPosB = TransformPoint(v3BonePosB, m_aLastAxis, m_v3LastPos, m_fLastScale);
    }
    else
    {
        m_v3LastEmitterPosA = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBoneA), m_aLastAxis, m_v3LastPos, m_fLastScale);
        m_v3LastEmitterPosB = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBoneB), m_aLastAxis, m_v3LastPos, m_fLastScale);
    }

    m_uiLastUpdateTime = uiMilliseconds;
    m_fLastLerp = fLerp;
    m_fLastTime = fTime;

    m_bLastActive = bActive;

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    if (m_uiExpireTime != INVALID_TIME && m_uiFrontSlot == m_uiBackSlot && !m_vTrackA[m_uiFrontSlot].IsActive())
        return false;

    if (m_iLife != -1 && (uiMilliseconds > m_iLife + m_uiStartTime))
    {
        if (m_bLoop)
        {
            m_uiStartTime += m_iLife * ((uiMilliseconds - m_uiStartTime) / m_iLife);
        }
        else
        {
            if (m_uiFrontSlot == m_uiBackSlot)
                return m_vTrackA[m_uiFrontSlot].IsActive();
            else
                return true;
        }
    }
    else if (m_iCount != -1 && m_iSpawnCount >= m_iCount)
    {
        if (m_uiFrontSlot == m_uiBackSlot)
            return m_vTrackA[m_uiFrontSlot].IsActive();
        else
            return true;
    }

    return true;
}


/*====================
  CTwoPointTrailEmitter::GetNumTriangles
  ====================*/
uint    CTwoPointTrailEmitter::GetNumTriangles()
{
    int iNumPoints(int(m_uiBackSlot) - int(m_uiFrontSlot));

    if (iNumPoints < 0)
        iNumPoints += int(m_vTrackA.size());

    return uint(MAX((iNumPoints - 1) * 2, 0));
}


/*====================
  CTwoPointTrailEmitter::GetTriangle
  ====================*/
bool    CTwoPointTrailEmitter::GetTriangle(uint uiIndex, STriangle &outTriangle)
{
    uint uiQuad(uiIndex >> 1);

    uint uiStart((m_uiFrontSlot + uiQuad) % m_vTrackA.size());
    uint uiEnd((m_uiFrontSlot + uiQuad + 1) % m_vTrackA.size());

    SBillboard billA1, billA2, billB1, billB2;

    m_vTrackA[uiStart].GetBillboard(m_uiLastUpdateTime, billA1);
    m_vTrackA[uiEnd].GetBillboard(m_uiLastUpdateTime, billA2);
    m_vTrackB[uiStart].GetBillboard(m_uiLastUpdateTime, billB1);
    m_vTrackB[uiEnd].GetBillboard(m_uiLastUpdateTime, billB2);

    float fStickiness;

    fStickiness = m_vTrackA[uiStart].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        billA1.v3Pos = LERP(fStickiness, billA1.v3Pos, m_v3LastBasePosA);

    fStickiness = m_vTrackA[uiEnd].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        billA2.v3Pos = LERP(fStickiness, billA2.v3Pos, m_v3LastBasePosA);

    fStickiness = m_vTrackB[uiStart].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        billB1.v3Pos = LERP(fStickiness, billB1.v3Pos, m_v3LastBasePosB);

    fStickiness = m_vTrackB[uiEnd].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        billB2.v3Pos = LERP(fStickiness, billB2.v3Pos, m_v3LastBasePosB);

    switch (m_pParticleSystem->GetSpace())
    {
    case WORLD_SPACE:
        {
        } break;
    case BONE_SPACE:
    case ENTITY_SPACE:
        {
            const CVec3f    &v3Pos(m_pParticleSystem->GetSourcePosition());
            const CAxis     &aAxis(m_pParticleSystem->GetSourceAxis());
            float           fScale(m_pParticleSystem->GetSourceScale());

            billA1.v3Pos = TransformPoint(billA1.v3Pos, aAxis, v3Pos, fScale);
            billA2.v3Pos = TransformPoint(billA2.v3Pos, aAxis, v3Pos, fScale);
            billB1.v3Pos = TransformPoint(billB1.v3Pos, aAxis, v3Pos, fScale);
            billB2.v3Pos = TransformPoint(billB2.v3Pos, aAxis, v3Pos, fScale);

        } break;
    }

    float   fLerp(m_iLife != -1 ? ILERP(m_uiLastUpdateTime - m_uiStartTime, 0u, uint(m_iLife)) : 0.0f);
    int     iTexPosTime(m_riTexPosTime.Lerp(fLerp));
    float   m_fTexPosScale(m_rfTexPosScale.Lerp(fLerp));
    float   m_fTexStretchScale(m_rfTexStretchScale.Lerp(fLerp));

#if 0
    float fTex0Pos(((m_vTrackA[uiStart].GetStartTime() - m_uiStartTime) % iTexPosTime) / float(iTexPosTime));
    float fTex1Pos(((m_vTrackA[uiEnd].GetStartTime() - m_uiStartTime) % iTexPosTime) / float(iTexPosTime));

    if (fTex0Pos > fTex1Pos)
        fTex0Pos -= 1.0f;
#else
    float fTex0Pos;
    float fTex1Pos;

    if (iTexPosTime == 0)
    {
        fTex0Pos = 0.0f;
        fTex1Pos = 0.0f;
    }
    else
    {
        fTex0Pos = (m_vTrackA[uiStart].GetStartTime() - m_uiStartTime) / float(iTexPosTime);
        fTex1Pos = (m_vTrackA[uiEnd].GetStartTime() - m_uiStartTime) / float(iTexPosTime);
    }
#endif

    float fTex0Stretch;
    float fTex1Stretch;

    if (int(m_uiLastUpdateTime - m_uiStartTime) < m_vTrackA[uiStart].GetLife())
    {
        fTex0Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrackA[uiStart].GetStartTime()) / float(m_uiLastUpdateTime - m_uiStartTime);
        fTex1Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrackA[uiEnd].GetStartTime()) / float(m_uiLastUpdateTime - m_uiStartTime);
    }
    else
    {
        fTex0Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrackA[uiStart].GetStartTime()) / float(m_vTrackA[uiStart].GetLife());
        fTex1Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrackA[uiEnd].GetStartTime()) / float(m_vTrackA[uiEnd].GetLife());
    }

    float fTex0(fTex0Pos * m_fTexPosScale + fTex0Stretch * m_fTexStretchScale);
    float fTex1(fTex1Pos * m_fTexPosScale + fTex1Stretch * m_fTexStretchScale);

    if (!m_tv3ParticleColor.IsOne())
    {
        CVec3f v3Color(m_tv3ParticleColor.Evaluate(m_fLastLerp, m_fLastTime));

        billA1.color.xyz() *= v3Color;
        billA2.color.xyz() *= v3Color;
        billB1.color.xyz() *= v3Color;
        billB2.color.xyz() *= v3Color;
    }

    if (!m_tfParticleAlpha.IsOne())
    {
        float fAlpha(m_tfParticleAlpha.Evaluate(m_fLastLerp, m_fLastTime));

        billA1.color.w *= fAlpha;
        billA2.color.w *= fAlpha;
        billB1.color.w *= fAlpha;
        billB2.color.w *= fAlpha;
    }

    if ((uiIndex & 0x1) == 0)
    {
        // A1->B1->B2
        outTriangle.vert[0].v = billA1.v3Pos;
        outTriangle.vert[0].color = billA1.color.GetAsDWord();
        outTriangle.vert[0].t = CVec4f(fTex0, 0.0f, billA1.frame, billA1.param);

        outTriangle.vert[1].v = billB1.v3Pos;
        outTriangle.vert[1].color = billB1.color.GetAsDWord();
        outTriangle.vert[1].t = CVec4f(fTex0, 1.0f, billB1.frame, billB1.param);

        outTriangle.vert[2].v = billB2.v3Pos;
        outTriangle.vert[2].color = billB2.color.GetAsDWord();
        outTriangle.vert[2].t = CVec4f(fTex1, 1.0f, billB2.frame, billB2.param);
    }
    else
    {
        // A1->B2->A2
        outTriangle.vert[0].v = billA1.v3Pos;
        outTriangle.vert[0].color = billA1.color.GetAsDWord();
        outTriangle.vert[0].t = CVec4f(fTex0, 0.0f, billA1.frame, billA1.param);

        outTriangle.vert[1].v = billB2.v3Pos;
        outTriangle.vert[1].color = billB2.color.GetAsDWord();
        outTriangle.vert[1].t = CVec4f(fTex1, 1.0f, billB2.frame, billB2.param);

        outTriangle.vert[2].v = billA2.v3Pos;
        outTriangle.vert[2].color = billA2.color.GetAsDWord();
        outTriangle.vert[2].t = CVec4f(fTex1, 0.0f, billA2.frame, billA2.param);
    }

    outTriangle.hMaterial = m_hMaterial;

    return true;
}
