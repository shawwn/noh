// (C)2006 S2 Games
// c_trailemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_trailemitter.h"
#include "c_simpleparticle.h"
#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
#include "c_vid.h"
#include "c_camera.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CTrailEmitterDef::~CTrailEmitterDef
  ====================*/
CTrailEmitterDef::~CTrailEmitterDef()
{
}


/*====================
  CTrailEmitterDef::CTrailEmitterDef
  ====================*/
CTrailEmitterDef::CTrailEmitterDef
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
    const tstring &sBone,
    const CVec3f &v3Pos,
    const CVec3f &v3Offset,
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
    const CTemporalPropertyRangef &rfParticleAlpha,
    const CTemporalPropertyRangef &rfParticleScale,
    float fDepthBias

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
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
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
m_rfParticleAlpha(rfParticleAlpha),
m_rfParticleScale(rfParticleScale),
m_fDepthBias(fDepthBias)
{
}


/*====================
  CTrailEmitterDef::Spawn
  ====================*/
IEmitter*   CTrailEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CTrailEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CTrailEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CTrailEmitter::~CTrailEmitter
  ====================*/
CTrailEmitter::~CTrailEmitter()
{
}


/*====================
  CTrailEmitter::CTrailEmitter
  ====================*/
CTrailEmitter::CTrailEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CTrailEmitterDef &eSettings) :
IEmitter
(
    eSettings.GetLife(),
    eSettings.GetExpireLife(),
    eSettings.GetTimeNudge(),
    eSettings.GetDelay(),
    eSettings.GetLoop(),
    TSNULL,
    eSettings.GetOwner(),
    eSettings.GetBone(),
    eSettings.GetPos(),
    eSettings.GetOffset(),
    DIRSPACE_LOCAL,
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_fSelectionWeightRange(0.0f),
m_fAccumulator(0.0f),
m_iSpawnCount(0),
m_v3LastBasePos(m_v3Pos),
m_v3LastBaseVelocity(V3_ZERO),
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
m_tv3OffsetSphere(eSettings.GetOffsetSphere()),
m_tv3OffsetCube(eSettings.GetOffsetCube()),
m_riTexPosTime(eSettings.GetTexPosTime()),
m_rfTexPosScale(eSettings.GetTexPosScale()),
m_rfTexStretchScale(eSettings.GetTexStretchScale()),
m_bSubFramePose(eSettings.GetSubFramePose()),
m_tv3ParticleColor(eSettings.GetParticleColor()),
m_tfParticleAlpha(eSettings.GetParticleAlpha()),
m_tfParticleScale(eSettings.GetParticleScale()),
m_fDepthBias(eSettings.GetDepthBias())
{
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

    m_vTrack.resize(iMaxActive);

    m_uiFrontSlot = 0;
    m_uiBackSlot = 0;

    // Initialize Last* vars
    m_v3LastPos = GetPosition();
    m_aLastAxis = GetAxis();
    m_fLastScale = GetScale();

    m_v3LastBasePos = TransformPoint(GetBonePosition(uiStartTime, m_pOwner, m_sBone), m_aLastAxis, m_v3LastPos, m_fLastScale);

    if (!m_bSubFramePose)
        GetBoneAxisPos(uiStartTime, m_pOwner, m_sBone, m_aLastBoneAxis, m_v3LastBonePos);

    m_bLastActive = m_pParticleSystem->GetActive() && GetVisibility();

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
  CTrailEmitter::ResumeFromPause
  ====================*/
void    CTrailEmitter::ResumeFromPause(uint uiMilliseconds)
{
    uint uiPauseDuration(uiMilliseconds - m_uiPauseBegin);
    m_uiStartTime += uiPauseDuration;
    m_uiLastUpdateTime = uiMilliseconds;
    m_uiPauseBegin = 0;

    for (TrackBuffer::iterator it(m_vTrack.begin()); it != m_vTrack.end(); ++it)
        it->AddToStartTime(uiPauseDuration);
}


/*====================
  CTrailEmitter::Update
  ====================*/
bool    CTrailEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CTrailEmitter::Update");

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
        Console.Warn << m_pParticleSystem->GetEffect()->GetPath() << _T(": <trailemitter> iDeltaTime == ") << iDeltaTime << newl;
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
    TrackBuffer::iterator it(m_vTrack.begin());

    for (; it != m_vTrack.end(); ++it)
    {
        if (!it->IsActive())
            continue;

        it->Update(fDeltaTime, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);

        if (it->IsDead(uiMilliseconds, bExpired))
            it->SetActive(false);
        else
            m_bbBounds.AddPoint(it->GetPos());
    }

    while (!m_vTrack[m_uiFrontSlot].IsActive() && m_uiFrontSlot != m_uiBackSlot)
        m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vTrack.size();

    bool bActive(m_bActive && m_pParticleSystem->GetActive());

    if (m_uiExpireTime != INVALID_TIME && (m_iLife == -1 || m_bLoop))
        if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
            bActive = false;

    if (!GetVisibility())
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

    CAxis   aBoneAxis(0.0f, 0.0f, 0.0f);
    CVec3f  v3BonePos;
    CVec3f  v3BasePos;

    bool bLerpBoneAxis(false);
    vec4_t boneQuat0;
    vec4_t boneQuat1;

    if (m_bSubFramePose)
    {
        v3BasePos = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone), aAxis, v3Pos, fScale);
    }
    else
    {
        if (m_eDirectionalSpace == DIRSPACE_LOCAL)
        {
            GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, aBoneAxis, v3BonePos);
            v3BasePos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);

            M_AxisToQuat((const vec3_t *)(&m_aLastBoneAxis), boneQuat0);
            M_AxisToQuat((const vec3_t *)(&aBoneAxis), boneQuat1);

            bLerpBoneAxis = !M_CompareVec4(boneQuat0, boneQuat1, 0.001f);
        }
        else
        {
            v3BonePos = GetBonePosition(uiMilliseconds, m_pOwner, m_sBone);
            v3BasePos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);
        }
    }

    CVec3f  v3BaseVelocity(m_bLastActive && bActive ? (v3BasePos - m_v3LastBasePos) / fDeltaTime : V3_ZERO);

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

        CVec3f  v3CurrentBonePos;

        if (m_eDirectionalSpace == DIRSPACE_LOCAL)
        {
            CAxis   aCurrentBoneAxis;

            if (m_bSubFramePose)
            {
                GetBoneAxisPos(uiMilliseconds - uiMillisecondNudge, m_pOwner, m_sBone, aCurrentBoneAxis, v3CurrentBonePos);
            }
            else
            {
                if (bLerpBoneAxis)
                {
                    vec4_t  lerpedQuat;
                    M_LerpQuat(fFrameLerp, boneQuat0, boneQuat1, lerpedQuat);

                    M_QuatToAxis(lerpedQuat, (vec3_t *)(&aCurrentBoneAxis));
                }
                else
                    aCurrentBoneAxis = aBoneAxis; // It is too initialized!

                v3CurrentBonePos = LERP(fFrameLerp, m_v3LastBonePos, v3BonePos);
            }

            v3Dir = TransformPoint(v3Dir, aCurrentBoneAxis);
            v3Dir = TransformPoint(v3Dir, aLerpedAxis);
        }
        else
        {
            if (m_bSubFramePose)
                v3CurrentBonePos = GetBonePosition(uiMilliseconds - uiMillisecondNudge, m_pOwner, m_sBone);
            else
                v3CurrentBonePos = LERP(fFrameLerp, m_v3LastBonePos, v3BonePos);
        }

        //
        // Track Position
        //

        CVec3f  v3OffsetPos(0.0f, 0.0f, 0.0f);
        {
            if (v3OffsetSphere != V3_ZERO)
            {
                CVec3f  v3Rand(M_RandomPointInSphere());

                v3OffsetPos += v3OffsetSphere * v3Rand;
            }

            if (v3OffsetCube != V3_ZERO)
            {
                CVec3f  v3Rand(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

                v3OffsetPos += v3OffsetCube * v3Rand;
            }

            float   fOffsetDirection(M_Randnum(fMinOffsetDirection, fMaxOffsetDirection));
            if (fOffsetDirection != 0.0f)
            {
                v3OffsetPos += v3Dir * fOffsetDirection;
            }

            float   fOffsetRadial(M_Randnum(fMinOffsetRadial, fMaxOffsetRadial));
            if (fOffsetRadial != 0.0f)
            {
                v3OffsetPos += M_RandomDirection(m_v3Dir, fMinOffsetRadialAngle, fMaxOffsetRadialAngle) * fOffsetRadial;
            }
        }

        CVec3f  v3Position(TransformPoint(v3OffsetPos + v3CurrentBonePos, aLerpedAxis, v3LerpedPos, fLerpedScale));

        CVec3f  v3Velocity(v3Dir * (M_Randnum(fMinSpeed, fMaxSpeed) * fLerpedScale));

        float   fLimitInheritVelocity(m_rfLimitInheritVelocity.Lerp(fLerp) * fLerpedScale);

        CVec3f  v3InheritVelocity(LERP(fTimeNudge / fDeltaTime, v3BaseVelocity, m_v3LastBaseVelocity) * M_Randnum(fMinInheritVelocity, fMaxInheritVelocity));
        if (fLimitInheritVelocity > 0.0f && v3InheritVelocity.Length() > fLimitInheritVelocity)
            v3InheritVelocity.SetLength(fLimitInheritVelocity);

        v3Velocity += v3InheritVelocity;

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

        uint uiSlot(m_uiBackSlot % m_vTrack.size());

        m_vTrack[uiSlot].Spawn
        (
            uiMilliseconds - uiMillisecondNudge,
            iParticleLife,
            v3Position,
            v3Velocity,
            v3Dir,
            fAcceleration,
            fLerpedScale,
            TransformPoint(v3CurrentBonePos, aLerpedAxis, v3LerpedPos, fLerpedScale),
            NULL,
            **itDef
        );
        m_vTrack[uiSlot].SetActive(true);

        // Update the new particle to catch up to this frame
        m_vTrack[uiSlot].Update(fTimeNudge, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);

        if (m_vTrack[uiSlot].IsDead(uiMilliseconds, bExpired))
            m_vTrack[uiSlot].SetActive(false);
        else
            m_bbBounds.AddPoint(m_vTrack[uiSlot].GetPos());

        m_uiBackSlot = (uiSlot + 1) % m_vTrack.size();

        // Push front slot forward if we wrapped around and caught it
        if (m_uiBackSlot == m_uiFrontSlot)
            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vTrack.size();

        while (!m_vTrack[m_uiFrontSlot].IsActive() && m_uiFrontSlot != m_uiBackSlot)
            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vTrack.size();

        ++m_iSpawnCount;
    }

    m_v3LastPos = v3Pos;
    m_aLastAxis = aAxis;
    m_fLastScale = fScale;
    m_v3LastBasePos = v3BasePos;
    m_v3LastBaseVelocity = v3BaseVelocity;

    if (!m_bSubFramePose)
    {
        m_v3LastBonePos = v3BonePos;
        m_aLastBoneAxis = aBoneAxis;
    }

    m_uiLastUpdateTime = uiMilliseconds;
    m_fLastLerp = fLerp;
    m_fLastTime = fTime;

    m_bLastActive = bActive;

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    if (m_uiExpireTime != INVALID_TIME && m_uiFrontSlot == m_uiBackSlot && !m_vTrack[m_uiFrontSlot].IsActive())
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
                return m_vTrack[m_uiFrontSlot].IsActive();
            else
                return true;
        }
    }
    else if (m_iCount != -1 && m_iSpawnCount >= m_iCount)
    {
        if (m_uiFrontSlot == m_uiBackSlot)
            return m_vTrack[m_uiFrontSlot].IsActive();
        else
            return true;
    }

    return true;
}


/*====================
  CTrailEmitter::GetNumTriangles
  ====================*/
uint    CTrailEmitter::GetNumTriangles()
{
    int iNumPoints(int(m_uiBackSlot) - int(m_uiFrontSlot));

    if (iNumPoints <= 0 && m_vTrack[m_uiFrontSlot].IsActive())
        iNumPoints += int(m_vTrack.size());

    return uint(MAX((iNumPoints - 1) * 2, 0));
}


/*====================
  CTrailEmitter::GetTriangle
  ====================*/
bool    CTrailEmitter::GetTriangle(uint uiIndex, STriangle &outTriangle)
{
    uint uiQuad(uiIndex >> 1);

    uint uiStart((m_uiFrontSlot + uiQuad) % m_vTrack.size());
    uint uiEnd((m_uiFrontSlot + uiQuad + 1) % m_vTrack.size());

    SBillboard bill0, bill1, bill2, bill3;

    uint ui0((uiStart + m_vTrack.size() - 1) % m_vTrack.size());
    uint ui3((uiEnd + 1) % m_vTrack.size());

    if (m_vTrack[ui0].IsActive())
        m_vTrack[ui0].GetBillboard(m_uiLastUpdateTime, bill0);
    else
        m_vTrack[uiStart].GetBillboard(m_uiLastUpdateTime, bill0);

    m_vTrack[uiStart].GetBillboard(m_uiLastUpdateTime, bill1);
    m_vTrack[uiEnd].GetBillboard(m_uiLastUpdateTime, bill2);

    if (m_vTrack[ui3].IsActive())
        m_vTrack[ui3].GetBillboard(m_uiLastUpdateTime, bill3);
    else
        m_vTrack[uiEnd].GetBillboard(m_uiLastUpdateTime, bill3);

    float fStickiness;

    fStickiness = m_vTrack[uiStart].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        bill0.v3Pos = LERP(fStickiness, bill0.v3Pos, m_v3LastBasePos);

    fStickiness = m_vTrack[uiStart].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        bill1.v3Pos = LERP(fStickiness, bill1.v3Pos, m_v3LastBasePos);

    fStickiness = m_vTrack[uiEnd].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        bill2.v3Pos = LERP(fStickiness, bill2.v3Pos, m_v3LastBasePos);

    fStickiness = m_vTrack[uiEnd].GetStickiness(m_uiLastUpdateTime);
    if (fStickiness != 0.0f)
        bill3.v3Pos = LERP(fStickiness, bill3.v3Pos, m_v3LastBasePos);

    switch (m_pParticleSystem->GetSpace())
    {
    case WORLD_SPACE:
        {
        } break;
    case ENTITY_SPACE:
        {
            const CVec3f    &v3Pos(m_pParticleSystem->GetSourcePosition());
            const CAxis     &aAxis(m_pParticleSystem->GetSourceAxis());
            float           fScale(m_pParticleSystem->GetSourceScale());

            bill0.v3Pos = TransformPoint(bill0.v3Pos, aAxis, v3Pos, fScale);
            bill1.v3Pos = TransformPoint(bill1.v3Pos, aAxis, v3Pos, fScale);
            bill2.v3Pos = TransformPoint(bill2.v3Pos, aAxis, v3Pos, fScale);
            bill3.v3Pos = TransformPoint(bill3.v3Pos, aAxis, v3Pos, fScale);
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
        fTex0Pos = (m_vTrack[uiStart].GetStartTime() - m_uiStartTime) / float(iTexPosTime);
        fTex1Pos = (m_vTrack[uiEnd].GetStartTime() - m_uiStartTime) / float(iTexPosTime);
    }
#endif

    float fTex0Stretch;
    float fTex1Stretch;

    if (int(m_uiLastUpdateTime - m_uiStartTime) < m_vTrack[uiStart].GetLife())
    {
        fTex0Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrack[uiStart].GetStartTime()) / float(m_uiLastUpdateTime - m_uiStartTime);
        fTex1Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrack[uiEnd].GetStartTime()) / float(m_uiLastUpdateTime - m_uiStartTime);
    }
    else
    {
        fTex0Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrack[uiStart].GetStartTime()) / float(m_vTrack[uiStart].GetLife());
        fTex1Stretch = 1.0f - (m_uiLastUpdateTime - m_vTrack[uiEnd].GetStartTime()) / float(m_vTrack[uiEnd].GetLife());
    }

    float fTex0(fTex0Pos * m_fTexPosScale + fTex0Stretch * m_fTexStretchScale);
    float fTex1(fTex1Pos * m_fTexPosScale + fTex1Stretch * m_fTexStretchScale);

    CVec3f v3Up(0.0f, 0.0f, 1.0f);

    const CCamera *pCam(Vid.GetCamera());

    CVec3f v3Dir0;
    {
        CVec2f v2Dir(Normalize(pCam->WorldToView(bill1.v3Pos) - pCam->WorldToView(bill0.v3Pos)));

        v3Dir0 = pCam->GetViewAxis(RIGHT) * v2Dir.x + pCam->GetViewAxis(UP) * v2Dir.y;
    }

    CVec3f v3Dir1;
    {
        CVec2f v2Dir(Normalize(pCam->WorldToView(bill2.v3Pos) - pCam->WorldToView(bill1.v3Pos)));

        v3Dir1 = pCam->GetViewAxis(RIGHT) * v2Dir.x + pCam->GetViewAxis(UP) * v2Dir.y;
    }

    CVec3f v3Dir2;
    {
        CVec2f v2Dir(Normalize(pCam->WorldToView(bill3.v3Pos) - pCam->WorldToView(bill2.v3Pos)));

        v3Dir2 = pCam->GetViewAxis(RIGHT) * v2Dir.x + pCam->GetViewAxis(UP) * v2Dir.y;
    }

    CVec3f v3Width1(Normalize(CrossProduct(v3Dir0, pCam->GetViewAxis(FORWARD)) + CrossProduct(v3Dir1, pCam->GetViewAxis(FORWARD))));
    CVec3f v3Width2(Normalize(CrossProduct(v3Dir1, pCam->GetViewAxis(FORWARD)) + CrossProduct(v3Dir2, pCam->GetViewAxis(FORWARD))));

    if (!m_tfParticleScale.IsOne())
    {
        float fScale(m_tfParticleScale.Evaluate(m_fLastLerp, m_fLastTime));
        bill1.height *= fScale;
        bill2.height *= fScale;
    }

    if (!m_tv3ParticleColor.IsOne())
    {
        CVec3f v3Color(m_tv3ParticleColor.Evaluate(m_fLastLerp, m_fLastTime));

        bill1.color.xyz() *= v3Color;
        bill2.color.xyz() *= v3Color;
    }

    if (!m_tfParticleAlpha.IsOne())
    {
        float fAlpha(m_tfParticleAlpha.Evaluate(m_fLastLerp, m_fLastTime));

        bill1.color.w *= fAlpha;
        bill2.color.w *= fAlpha;
    }

    dword color1(bill1.color.GetAsDWord());
    dword color2(bill2.color.GetAsDWord());

    if ((uiIndex & 0x1) == 0)
    {
        // A1->B1->B2
        outTriangle.vert[0].v = bill1.v3Pos + v3Width1 * (bill1.height * 0.5f);
        outTriangle.vert[0].color = color1;
        outTriangle.vert[0].t = CVec4f(fTex0, 0.0f, bill1.frame, bill1.param);

        outTriangle.vert[1].v = bill1.v3Pos - v3Width1 * (bill1.height * 0.5f);
        outTriangle.vert[1].color = color1;
        outTriangle.vert[1].t = CVec4f(fTex0, 1.0f, bill1.frame, bill1.param);

        outTriangle.vert[2].v = bill2.v3Pos - v3Width2 * (bill2.height * 0.5f);
        outTriangle.vert[2].color = color2;
        outTriangle.vert[2].t = CVec4f(fTex1, 1.0f, bill2.frame, bill2.param);
    }
    else
    {
        // A1->B2->A2
        outTriangle.vert[0].v = bill1.v3Pos + v3Width1 * (bill1.height * 0.5f);
        outTriangle.vert[0].color = color1;
        outTriangle.vert[0].t = CVec4f(fTex0, 0.0f, bill1.frame, bill1.param);

        outTriangle.vert[1].v = bill2.v3Pos - v3Width2 * (bill2.height * 0.5f);
        outTriangle.vert[1].color = color2;
        outTriangle.vert[1].t = CVec4f(fTex1, 1.0f, bill2.frame, bill2.param);

        outTriangle.vert[2].v = bill2.v3Pos + v3Width2 * (bill2.height * 0.5f);
        outTriangle.vert[2].color = color2;
        outTriangle.vert[2].t = CVec4f(fTex1, 0.0f, bill2.frame, bill2.param);
    }

    outTriangle.hMaterial = m_hMaterial;

    return true;
}
