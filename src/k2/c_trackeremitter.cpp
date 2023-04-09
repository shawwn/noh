// (C)2006 S2 Games
// c_trackeremitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_trackeremitter.h"
#include "c_tracker.h"
#include "c_particlesystem.h"
#include "c_effect.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CTrackerEmitterDef::~CTrackerEmitterDef
  ====================*/
CTrackerEmitterDef::~CTrackerEmitterDef()
{
}


/*====================
  CTrackerEmitterDef::CTrackerEmitterDef
  ====================*/
CTrackerEmitterDef::CTrackerEmitterDef
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
    int iParticleLifeAlloc,
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
    const tstring &sBone,
    const CVec3f &v3Pos,
    const CVec3f &v3Offset,
    const tstring &sTargetBone,
    const CVec3f &v3TargetPos,
    const tstring &sTargetOwner,
    const CTemporalPropertyv3 &tv3OffsetSphere,
    const CTemporalPropertyv3 &tv3OffsetCube,
    const CTemporalPropertyRangef &rfMinOffsetDirection,
    const CTemporalPropertyRangef &rfMaxOffsetDirection,
    const CTemporalPropertyRangef &rfMinOffsetRadial,
    const CTemporalPropertyRangef &rfMaxOffsetRadial,
    const CTemporalPropertyRangef &rfMinOffsetRadialAngle,
    const CTemporalPropertyRangef &rfMaxOffsetRadialAngle,
    ETrackType eTrackType,
    const CTemporalPropertyRangef &rfMinTrackSpeed,
    const CTemporalPropertyRangef &rfMaxTrackSpeed,
    bool bDistanceLife,
    bool bSubFramePose,
    EDirectionalSpace eParticleDirectionalSpace,
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
m_iParticleLifeAlloc(iParticleLifeAlloc),
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
m_eDirectionalSpace(eDirectionalSpace),
m_fDrag(fDrag),
m_fFriction(fFriction),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_sTargetBone(sTargetBone),
m_v3TargetPos(v3TargetPos),
m_sTargetOwner(sTargetOwner),
m_tv3OffsetSphere(tv3OffsetSphere),
m_tv3OffsetCube(tv3OffsetCube),
m_rfMinOffsetDirection(rfMinOffsetDirection),
m_rfMaxOffsetDirection(rfMaxOffsetDirection),
m_rfMinOffsetRadial(rfMinOffsetRadial),
m_rfMaxOffsetRadial(rfMaxOffsetRadial),
m_rfMinOffsetRadialAngle(rfMinOffsetRadialAngle),
m_rfMaxOffsetRadialAngle(rfMaxOffsetRadialAngle),
m_eTrackType(eTrackType),
m_rfMinTrackSpeed(rfMinTrackSpeed),
m_rfMaxTrackSpeed(rfMaxTrackSpeed),
m_bDistanceLife(bDistanceLife),
m_bSubFramePose(bSubFramePose),
m_eParticleDirectionalSpace(eParticleDirectionalSpace),
m_tv3ParticleColor(tv3ParticleColor),
m_rfParticleAlpha(rfParticleAlpha),
m_rfParticleScale(rfParticleScale),
m_fDepthBias(fDepthBias)
{
}


/*====================
  CTrackerEmitterDef::Spawn
  ====================*/
IEmitter*   CTrackerEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CTrackerEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CTrackerEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CTrackerEmitter::~CTrackerEmitter
  ====================*/
CTrackerEmitter::~CTrackerEmitter()
{
}


/*====================
  CTrackerEmitter::CTrackerEmitter
  ====================*/
CTrackerEmitter::CTrackerEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CTrackerEmitterDef &eSettings) :
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
    V3_ZERO,
    eSettings.GetDirectionalSpace(),
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_fSelectionWeightRange(0.0f),
m_fAccumulator(0.0f),
m_iSpawnCount(0),
m_v3LastBaseVelocity(V3_ZERO),
m_bChildEmitters(false),
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
m_sTargetBone(eSettings.GetTargetBone()),
m_v3TargetPos(eSettings.GetTargetPos()),
m_tv3OffsetSphere(eSettings.GetOffsetSphere()),
m_tv3OffsetCube(eSettings.GetOffsetCube()),
m_eTrackType(eSettings.GetTrackType()),
m_rfMinTrackSpeed(eSettings.GetMinTrackSpeed()),
m_rfMaxTrackSpeed(eSettings.GetMaxTrackSpeed()),
m_bDistanceLife(eSettings.GetDistanceLife()),
m_bSubFramePose(eSettings.GetSubFramePose()),
m_eParticleDirectionalSpace(eSettings.GetParticleDirectionalSpace()),
m_tv3ParticleColor(eSettings.GetParticleColor()),
m_tfParticleAlpha(eSettings.GetParticleAlpha()),
m_tfParticleScale(eSettings.GetParticleScale()),
m_fDepthBias(eSettings.GetDepthBias())
{
    m_pTargetOwner = GetOwnerPointer(eSettings.GetTargetOwner());

    int iMaxActive;

    if (m_riMaxParticleLife.Max() == -1 || m_rfSpawnRate.Max() == 0.0f)
    {
        if (m_iCount == -1)
        {
            iMaxActive = 1024;
            Console.Warn << m_pParticleSystem->GetEffect()->GetPath() << _T(": <trackeremitter> Infinite spawning emitter with infinite life particles") << newl;
        }
        else
            iMaxActive = m_iCount + 1;
    }
    else
        iMaxActive = INT_CEIL(m_rfSpawnRate.Max() * ((eSettings.GetParticleLifeAlloc() != -1) ? eSettings.GetParticleLifeAlloc() : m_riMaxParticleLife.Max()) * SEC_PER_MS) + 1;

    if (m_iCount != -1)
    {
        if (iMaxActive > m_iCount + 1)
            iMaxActive = m_iCount + 1;
    }

    if (m_iLife != -1 && m_rfSpawnRate.Max() > 0.0f)
    {
        int iMaxSpawned(INT_CEIL(m_rfSpawnRate.Max() * (m_iLife * SEC_PER_MS)));

        if (iMaxActive > iMaxSpawned)
            iMaxActive = iMaxSpawned;
    }

    if (m_iCount == -1 && m_iLife == -1 && m_rfSpawnRate.Max() == 0.0f)
        iMaxActive = 0;

    m_vParticles.resize(iMaxActive);

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

    for (vector<CSimpleParticleDef *>::const_iterator it(m_pvParticleDefinitions->begin()); it != m_pvParticleDefinitions->end(); ++it)
        m_fSelectionWeightRange += (*it)->GetSelectionWeight();

    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;

    m_bActive = true;

    m_fParticleScale = m_tfParticleScale.Evaluate(0.0f, 0.0f);
    m_v3ParticleColor = m_tv3ParticleColor.Evaluate(0.0f, 0.0f);
    m_fParticleAlpha = m_tfParticleAlpha.Evaluate(0.0f, 0.0f);
}


/*====================
  CTrackerEmitter::ResumeFromPause
  ====================*/
void    CTrackerEmitter::ResumeFromPause(uint uiMilliseconds)
{
    uint uiPauseDuration(uiMilliseconds - m_uiPauseBegin);
    m_uiStartTime += uiPauseDuration;
    m_uiLastUpdateTime = uiMilliseconds;
    m_uiPauseBegin = 0;

    for (TrackerList::iterator it(m_vParticles.begin()); it != m_vParticles.end(); ++it)
        it->AddToStartTime(uiPauseDuration);
}


/*====================
  CTrackerEmitter::UpdateEmbeddedEmitter
  ====================*/
inline
bool    CTrackerEmitter::UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, IEmitter *pEmitter, CTracker &cTracker)
{
    if (!cTracker.IsActive() && !pEmitter->GetExpire())
        pEmitter->Expire(uiMilliseconds);

    float fLerp(cTracker.GetLerp(uiMilliseconds, m_bDistanceLife, m_v3LastTargetPos));
    float fTime(cTracker.GetTime(uiMilliseconds));

    pEmitter->SetCustomPos(cTracker.GetPos());
    pEmitter->SetCustomScale(cTracker.GetScale(fLerp, fTime) * m_fParticleScale);

    CAxis aAxis(m_eParticleDirectionalSpace == DIRSPACE_LOCAL ? GetAxis() : AXIS_IDENTITY);

    if (cTracker.GetFlags() & BBOARD_TURN)
    {
        CAxis aDir;

        const CVec3f &v3Velocity(cTracker.GetVelocity());
        if (v3Velocity == V3_ZERO)
            aDir.SetFromForwardVec(cTracker.GetDir());
        else
            aDir.SetFromForwardVec(cTracker.GetVelocity());

        aAxis = aAxis * aDir;
    }

    aAxis = aAxis * CAxis(cTracker.GetPitch(fLerp, fTime), cTracker.GetRoll(fLerp, fTime), cTracker.GetYaw(fLerp, fTime));

    pEmitter->SetCustomAxis(aAxis);

    bool bRet(pEmitter->Update(uiMilliseconds, pfnTrace));

    return bRet;
}


/*====================
  CTrackerEmitter::Update
  ====================*/
bool    CTrackerEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CTrackerEmitter::Update");

    if (m_vParticles.size() == 0)
        return false;

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
        Console.Warn << m_pParticleSystem->GetEffect()->GetPath() << _T(": <trackeremitter> iDeltaTime == ") << iDeltaTime << newl;
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

    m_fParticleScale = m_tfParticleScale.Evaluate(fLerp, fTime);
    m_v3ParticleColor = m_tv3ParticleColor.Evaluate(fLerp, fTime);
    m_fParticleAlpha = m_tfParticleAlpha.Evaluate(fLerp, fTime);

    CVec3f v3TargetPos(GetPosition(m_v3TargetPos, m_pTargetOwner));
    CAxis aTargetAxis(GetAxis(m_pTargetOwner));
    float fTargetScale(GetScale(m_pTargetOwner));

    CAxis   aTargetBoneAxis(0.0f, 0.0f, 0.0f);
    CVec3f  v3TargetBonePos;

    if (m_eDirectionalSpace == DIRSPACE_LOCAL)
    {
        GetBoneAxisPos(uiMilliseconds, m_pTargetOwner, m_sTargetBone, aTargetBoneAxis, v3TargetBonePos);
        v3TargetPos = TransformPoint(v3TargetBonePos, aTargetAxis, v3TargetPos, fTargetScale);
    }
    else
    {
        v3TargetBonePos = GetBonePosition(uiMilliseconds, m_pTargetOwner, m_sTargetBone);
        v3TargetPos = TransformPoint(v3TargetBonePos, aTargetAxis, v3TargetPos, fTargetScale);
    }

    m_bbBounds.Clear();

    bool bExpired(m_uiExpireTime != INVALID_TIME);

    // Update existing particles
    for (TrackerList::iterator it(m_vParticles.begin()); it != m_vParticles.end(); ++it)
    {
        if (it->IsActive())
        {
            if (it->IsDead(uiMilliseconds, bExpired))
                it->SetActive(false);
            else
            {
                it->Update(fDeltaTime, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction, v3TargetPos, m_eTrackType, m_bDistanceLife);
                m_bbBounds.AddPoint(it->GetPos());
            }
        }

        if (it->GetImbeddedEmitter() != NULL)
        {
            IEmitter *pEmitter(it->GetImbeddedEmitter());

            if (!UpdateEmbeddedEmitter(uiMilliseconds, pfnTrace, pEmitter, *it))
            {
                if (pEmitter->GetNextEmitter() != NULL)
                {
                    it->SetImbeddedEmitter(pEmitter->GetNextEmitter());
                    pEmitter->SetNextEmitter(NULL);
                }
                else
                {
                    it->SetImbeddedEmitter(NULL);
                }

                K2_DELETE(pEmitter);
            }
            else
            {
                m_bbBounds.AddPoint(it->GetPos());
            }
        }
    }

    while ((!m_vParticles[m_uiFrontSlot].IsActive() && m_vParticles[m_uiFrontSlot].GetImbeddedEmitter() == NULL) && m_uiFrontSlot != m_uiBackSlot)
        m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();

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
            if (fSpawnRate == 0.0f)
                m_fAccumulator = float(m_iCount);
            else
                m_fAccumulator += fSpawnRate * fClampedDeltaTime;
        }
        else
        {
            if (fSpawnRate == 0.0f)
            {
                m_iSpawnCount = m_iCount;
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
    }
    else if (bActive && m_bLastActive)
    {
        m_fAccumulator += fSpawnRate * fClampedDeltaTime;
    }

    while (m_fAccumulator >= 1.0f && (m_iCount == -1 || m_iSpawnCount < m_iCount))
    {
        m_fAccumulator -= 1.0f;

        float   fTimeNudge(fSpawnRate > 0.0f ? m_fAccumulator / fSpawnRate : 0.0f);
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
        float   fMinTrackSpeed(m_rfMinTrackSpeed.Lerp(fLerp));
        float   fMaxTrackSpeed(m_rfMaxTrackSpeed.Lerp(fLerp));

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

        CVec3f  v3Dir(M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle));

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
        // Position
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

        // Spawn embedded emitters
        IEmitter *pImbeddedEmitter(NULL);
        IEmitter *pCurrentEmitter(NULL);
        const tsvector &vEmitters((*itDef)->GetEmitters());
        if (!vEmitters.empty())
        {
            CEffect *pEffect(m_pParticleSystem->GetEffect());

            tsvector_cit cit(vEmitters.begin());

            IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));

            m_pParticleSystem->SetCustomPos(v3Position);

            CAxis aAxis(m_eParticleDirectionalSpace == DIRSPACE_LOCAL ? aLerpedAxis : AXIS_IDENTITY);

            if ((*itDef)->GetFlags() & BBOARD_TURN)
            {
                CAxis aDir;

                if (v3Velocity == V3_ZERO)
                    aDir.SetFromForwardVec(v3Dir);
                else
                    aDir.SetFromForwardVec(v3Velocity);

                aAxis = aAxis * aDir;
            }

            //aAxis = aAxis * CAxis(cParticle.GetPitch(fLerp, fTime), cParticle.GetRoll(fLerp, fTime), cParticle.GetYaw(fLerp, fTime));

            m_pParticleSystem->SetCustomAxis(aAxis);
            m_pParticleSystem->SetCustomScale(fLerpedScale * m_fParticleScale);

            if (pEmitterDef != NULL)
                pImbeddedEmitter = pEmitterDef->Spawn(uiMilliseconds - uiMillisecondNudge, m_pParticleSystem, OWNER_CUSTOM);

            if (pImbeddedEmitter != NULL)
            {
                m_bChildEmitters = true;

                ++cit;

                pCurrentEmitter = pImbeddedEmitter;
                for (; cit != vEmitters.end(); ++cit)
                {
                    IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));
                    IEmitter *pNewEmitter(NULL);

                    if (pEmitterDef != NULL)
                        pNewEmitter = pEmitterDef->Spawn(uiMilliseconds - uiMillisecondNudge, m_pParticleSystem, OWNER_CUSTOM);

                    pCurrentEmitter->SetNextEmitter(pNewEmitter);
                    pCurrentEmitter = pNewEmitter;
                }
            }
        }

        vector<IEmitterDef *>::const_iterator itEnd((*itDef)->GetEmitterDefs().end());
        for (vector<IEmitterDef *>::const_iterator it((*itDef)->GetEmitterDefs().begin()); it != itEnd; ++it)
        {
            m_pParticleSystem->SetCustomPos(v3Position);

            CAxis aAxis(m_eParticleDirectionalSpace == DIRSPACE_LOCAL ? aLerpedAxis : AXIS_IDENTITY);

            if ((*itDef)->GetFlags() & BBOARD_TURN)
            {
                CAxis aDir;

                if (v3Velocity == V3_ZERO)
                    aDir.SetFromForwardVec(v3Dir);
                else
                    aDir.SetFromForwardVec(v3Velocity);

                aAxis = aAxis * aDir;
            }

            //aAxis = aAxis * CAxis(cParticle.GetPitch(fLerp, fTime), cParticle.GetRoll(fLerp, fTime), cParticle.GetYaw(fLerp, fTime));

            m_pParticleSystem->SetCustomAxis(aAxis);
            m_pParticleSystem->SetCustomScale(fLerpedScale * m_fParticleScale);

            for (int i(0); i < (*it)->GetCount(); ++i)
            {
                IEmitter *pNewEmitter((*it)->Spawn(uiMilliseconds - uiMillisecondNudge, m_pParticleSystem, OWNER_CUSTOM));
                if (pNewEmitter != NULL)
                {
                    m_bChildEmitters = true;

                    if (pCurrentEmitter == NULL)
                    {
                        pImbeddedEmitter = pNewEmitter;
                        pCurrentEmitter = pImbeddedEmitter;
                    }
                    else
                    {
                        pCurrentEmitter->SetNextEmitter(pNewEmitter);
                        pCurrentEmitter = pNewEmitter;
                    }
                }
            }
        }

        uint uiSlot(m_uiBackSlot);

        m_vParticles[uiSlot].Spawn
        (
            uiMilliseconds - uiMillisecondNudge,
            iParticleLife,
            v3Position,
            v3Velocity,
            v3Dir,
            fAcceleration,
            fLerpedScale,
            v3TargetPos,
            M_Randnum(fMinTrackSpeed, fMaxTrackSpeed),
            pImbeddedEmitter,
            **itDef
        );

        m_vParticles[uiSlot].SetActive(true);

        if (m_vParticles[uiSlot].IsDead(uiMilliseconds, bExpired))
            m_vParticles[uiSlot].SetActive(false);
        else
        {
            // Update the particle a bit to get rid of any framerate dependent spawning patterns
            m_vParticles[uiSlot].Update(fTimeNudge, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction, v3TargetPos, m_eTrackType, m_bDistanceLife);
            m_bbBounds.AddPoint(m_vParticles[uiSlot].GetPos());
        }

        if (m_vParticles[uiSlot].GetImbeddedEmitter() != NULL)
        {
            IEmitter *pEmitter(m_vParticles[uiSlot].GetImbeddedEmitter());

            if (!UpdateEmbeddedEmitter(uiMilliseconds, pfnTrace, pEmitter, m_vParticles[uiSlot]))
            {
                if (pEmitter->GetNextEmitter() != NULL)
                {
                    m_vParticles[uiSlot].SetImbeddedEmitter(pEmitter->GetNextEmitter());
                    pEmitter->SetNextEmitter(NULL);
                }
                else
                {
                    m_vParticles[uiSlot].SetImbeddedEmitter(NULL);
                }

                K2_DELETE(pEmitter);
            }
        }

        m_uiBackSlot = (uiSlot + 1) % m_vParticles.size();

        // Push front slot forward if we wrapped around and caught it
        if (m_uiBackSlot == m_uiFrontSlot)
        {
            if (m_vParticles[m_uiFrontSlot].GetImbeddedEmitter())
            {
                K2_DELETE(m_vParticles[m_uiFrontSlot].GetImbeddedEmitter());
                m_vParticles[m_uiFrontSlot].SetImbeddedEmitter(NULL);
            }

            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();
        }

        while ((!m_vParticles[m_uiFrontSlot].IsActive() && m_vParticles[m_uiFrontSlot].GetImbeddedEmitter() == NULL) && m_uiFrontSlot != m_uiBackSlot)
            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();

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

    m_v3LastTargetPos = v3TargetPos;

    m_uiLastUpdateTime = uiMilliseconds;

    m_bLastActive = bActive;

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    if (m_uiExpireTime != INVALID_TIME && m_uiFrontSlot == m_uiBackSlot && !m_vParticles[m_uiFrontSlot].IsActive())
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
                return m_vParticles[m_uiFrontSlot].IsActive();
            else
                return true;
        }
    }
    else if (m_iCount != -1 && m_iSpawnCount >= m_iCount)
    {
        if (m_uiFrontSlot == m_uiBackSlot)
            return m_vParticles[m_uiFrontSlot].IsActive();
        else
            return true;
    }

    return true;
}


/*====================
  CTrackerEmitter::GetNumBillboards
  ====================*/
uint    CTrackerEmitter::GetNumBillboards()
{
    if (m_vParticles.size() == 0 || m_hMaterial == INVALID_RESOURCE)
        return 0;

    int iNumParticles(int(m_uiBackSlot) - int(m_uiFrontSlot));

    if (iNumParticles == 0 && m_vParticles[m_uiFrontSlot].IsActive())
        iNumParticles += int(m_vParticles.size());
    else if (iNumParticles < 0)
        iNumParticles += int(m_vParticles.size());

    return uint(iNumParticles);
}


/*====================
  CTrackerEmitter::GetBillboard
  ====================*/
bool    CTrackerEmitter::GetBillboard(uint uiIndex, SBillboard &outBillboard)
{
    uint uiParticle((m_uiFrontSlot + uiIndex) % m_vParticles.size());

    if (!m_vParticles[uiParticle].IsActive())
        return false;

    m_vParticles[uiParticle].GetBillboard(m_uiLastUpdateTime, m_bDistanceLife, m_v3LastTargetPos, outBillboard);

    switch (m_pParticleSystem->GetSpace())
    {
    case WORLD_SPACE:
        {
            if (outBillboard.uiFlags & (BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT | BBOARD_GENERATE_AXIS))
            {
                if (m_eParticleDirectionalSpace == DIRSPACE_LOCAL)
                    outBillboard.aAxis = m_aLastAxis;
                else
                    outBillboard.aAxis = AXIS_IDENTITY;
            }
        } break;
    case BONE_SPACE:
    case ENTITY_SPACE:
        {
            const CVec3f    &v3Pos(m_pParticleSystem->GetSourcePosition());
            const CAxis     &aAxis(m_pParticleSystem->GetSourceAxis());
            float           fScale(m_pParticleSystem->GetSourceScale());

            outBillboard.height *= fScale;
            outBillboard.width *= fScale;
            outBillboard.v3Pos = TransformPoint(outBillboard.v3Pos, aAxis, v3Pos, fScale);

            if (outBillboard.uiFlags & (BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT | BBOARD_GENERATE_AXIS))
            {
                if (m_eParticleDirectionalSpace == DIRSPACE_LOCAL)
                    outBillboard.aAxis = aAxis * m_aLastAxis;
                else
                    outBillboard.aAxis = AXIS_IDENTITY;
            }
        } break;
    }

    outBillboard.hMaterial = m_hMaterial;

    if (!m_tfParticleScale.IsOne())
    {
        outBillboard.width *= m_fParticleScale;
        outBillboard.height *= m_fParticleScale;
    }

    if (!m_tv3ParticleColor.IsOne())
        outBillboard.color.xyz() *= m_v3ParticleColor;

    if (!m_tfParticleAlpha.IsOne())
        outBillboard.color.w *= m_fParticleAlpha;

    outBillboard.fDepthBias = m_fDepthBias;

    return true;
}


/*====================
  CTrackerEmitter::GetNumEmitters
  ====================*/
uint    CTrackerEmitter::GetNumEmitters()
{
    if (!m_bChildEmitters || m_vParticles.size() == 0)
        return 0;

    int iNumParticles(int(m_uiBackSlot) - int(m_uiFrontSlot));

    if (iNumParticles == 0 && m_vParticles[m_uiFrontSlot].IsActive())
        iNumParticles += int(m_vParticles.size());
    else if (iNumParticles < 0)
        iNumParticles += int(m_vParticles.size());

    return uint(iNumParticles);
}


/*====================
  CTrackerEmitter::GetEmitter
  ====================*/
IEmitter*   CTrackerEmitter::GetEmitter(uint uiIndex)
{
    uint uiParticle((m_uiFrontSlot + uiIndex) % m_vParticles.size());

    return m_vParticles[uiParticle].GetImbeddedEmitter();
}
