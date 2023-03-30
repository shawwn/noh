// (C)2006 S2 Games
// c_orbiteremitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_orbiteremitter.h"
#include "c_orbiter.h"
#include "c_particlesystem.h"
#include "c_vid.h"
#include "c_camera.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  COrbiterEmitterDef::~COrbiterEmitterDef
  ====================*/
COrbiterEmitterDef::~COrbiterEmitterDef()
{
}


/*====================
  COrbiterEmitterDef::COrbiterEmitterDef
  ====================*/
COrbiterEmitterDef::COrbiterEmitterDef
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
m_hMaterial(hMaterial),
m_v3Dir(v3Dir),
m_eDirectionalSpace(eDirectionalSpace),
m_fDrag(fDrag),
m_fFriction(fFriction),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_v3Origin(v3Origin),
m_tv3Offset(tv3Offset),
m_bCylindrical(bCylindrical),
m_tv3Orbit(tv3Orbit),
m_rfMinOrbitAngle(rfMinOrbitAngle),
m_rfMaxOrbitAngle(rfMaxOrbitAngle),
m_tv3ParticleColor(tv3ParticleColor),
m_rfParticleAlpha(rfParticleAlpha),
m_rfParticleScale(rfParticleScale),
m_fDepthBias(fDepthBias)
{
}


/*====================
  COrbiterEmitterDef::Spawn
  ====================*/
IEmitter*   COrbiterEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("COrbiterEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, COrbiterEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  COrbiterEmitter::~COrbiterEmitter
  ====================*/
COrbiterEmitter::~COrbiterEmitter()
{
}


/*====================
  COrbiterEmitter::COrbiterEmitter
  ====================*/
COrbiterEmitter::COrbiterEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const COrbiterEmitterDef &eSettings) :
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
    eSettings.GetDirectionalSpace(),
    &eSettings.GetParticleDefinitions(),
    pParticleSystem,
    pOwner,
    uiStartTime
),
m_fSelectionWeightRange(0.0f),
m_fAccumulator(0.0f),
m_iSpawnCount(0),
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
m_hMaterial(eSettings.GetMaterial()),
m_v3Dir(eSettings.GetDir()),
m_fDrag(eSettings.GetDrag()),
m_fFriction(eSettings.GetFriction()),
m_v3Origin(eSettings.GetOrigin()),
m_tv3Offset(eSettings.GetOrbitOffset()),
m_bCylindrical(eSettings.GetCylindrical()),
m_tv3Orbit(eSettings.GetOrbit()),
m_rfMinOrbitAngle(eSettings.GetMinOrbitAngle()),
m_rfMaxOrbitAngle(eSettings.GetMaxOrbitAngle()),
m_tv3ParticleColor(eSettings.GetParticleColor()),
m_tfParticleAlpha(eSettings.GetParticleAlpha()),
m_tfParticleScale(eSettings.GetParticleScale()),
m_fDepthBias(eSettings.GetDepthBias())
{
    int iMaxActive;

    if (m_riMaxParticleLife.Max() == -1 || m_rfSpawnRate.Max() == 0.0f)
        iMaxActive = m_iCount == -1 ? 1024 : m_iCount + 1;
    else
        iMaxActive = INT_CEIL(m_rfSpawnRate.Max() * (m_riMaxParticleLife.Max() * SEC_PER_MS)) + 1;

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

    m_bLastActive = m_pParticleSystem->GetActive() && GetVisibility();

    for (vector<CSimpleParticleDef *>::const_iterator it(m_pvParticleDefinitions->begin()); it != m_pvParticleDefinitions->end(); ++it)
        m_fSelectionWeightRange += (*it)->GetSelectionWeight();

    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;

    m_bActive = true;

    m_fLastLerp = 0.0f;
    m_fLastTime = 0.0f;

#if 0
    m_fParticleScale = m_tfParticleScale.Evaluate(0.0f, 0.0f);
    m_v3ParticleColor = m_tv3ParticleColor.Evaluate(0.0f, 0.0f);
    m_fParticleAlpha = m_tfParticleAlpha.Evaluate(0.0f, 0.0f);
#endif
}


/*====================
  COrbiterEmitter::ResumeFromPause
  ====================*/
void    COrbiterEmitter::ResumeFromPause(uint uiMilliseconds)
{
    uint uiPauseDuration(uiMilliseconds - m_uiPauseBegin);
    m_uiStartTime += uiPauseDuration;
    m_uiLastUpdateTime = uiMilliseconds;
    m_uiPauseBegin = 0;

    for (OrbiterList::iterator it(m_vParticles.begin()); it != m_vParticles.end(); ++it)
        it->AddToStartTime(uiPauseDuration);
}


/*====================
  COrbiterEmitter::Update
  ====================*/
bool    COrbiterEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("COrbiterEmitter::Update");

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
        Console.Warn << _T("<orbiteremitter> iDeltaTime == ") << iDeltaTime << newl;
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

#if 0
    m_fParticleScale = m_tfParticleScale.Evaluate(fLerp, fTime);
    m_v3ParticleColor = m_tv3ParticleColor.Evaluate(fLerp, fTime);
    m_fParticleAlpha = m_tfParticleAlpha.Evaluate(fLerp, fTime);
#endif

    bool bExpired(m_uiExpireTime != INVALID_TIME);

    m_bbBounds.Clear();

    // Update existing particles
    for (OrbiterList::iterator it(m_vParticles.begin()); it != m_vParticles.end(); ++it)
    {
        if (!it->IsActive())
            continue;

        it->Update(fDeltaTime, CVec3f(0.0f, 0.0f, 0.0f), m_fDrag, m_fFriction);

        if (it->IsDead(uiMilliseconds, bExpired))
            it->SetActive(false);
        else
            m_bbBounds.AddPoint(it->GetPos());
    }

    while (!m_vParticles[m_uiFrontSlot].IsActive() && m_uiFrontSlot != m_uiBackSlot)
        m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();

    bool bActive(m_bActive && m_pParticleSystem->GetActive());

    if (m_uiExpireTime != INVALID_TIME && (m_iLife == -1 || m_bLoop))
        if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
            bActive = false;

    if (!GetVisibility())
        bActive = false;

    // Spawn new particles
    CVec3f v3Pos(GetPosition());
    CAxis aAxis(GetAxis());
    float fScale(GetScale());

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
        CVec3f  v3Offset(m_tv3Offset.Lerp(fLerp));
        CVec3f  v3Orbit(m_tv3Orbit.Lerp(fLerp));
        //float fMinOrbitAngle(m_rfMinOrbitAngle.Lerp(fLerp));
        //float fMaxOrbitAngle(m_rfMaxOrbitAngle.Lerp(fLerp));

        CVec3f  v3Up(M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle));

        CVec3f  v3Position(0.0f, 0.0f, 0.0f);

        if (v3Offset != V3_ZERO)
        {
            CVec3f  v3Rand(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

            v3Position += v3Offset * v3Rand;
        }

        v3Position += m_v3Origin;

        CVec3f  v3Dir(v3Orbit.xy(), v3Orbit.z);

        CVec3f  v3Velocity(v3Dir * M_Randnum(fMinSpeed, fMaxSpeed));
        float   fAcceleration(M_Randnum(fMinAcceleration, fMaxAcceleration));

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

        uint uiSlot(m_uiBackSlot);

        m_vParticles[uiSlot] = COrbiter
        (
            uiMilliseconds - uiMillisecondNudge,
            iParticleLife,
            v3Position,
            v3Velocity,
            v3Dir,
            fAcceleration,
            v3Up,
            **itDef
        );

        m_vParticles[uiSlot].SetActive(true);

        // Update the particle a bit to get rid of any framerate dependent spawning patterns
        m_vParticles[uiSlot].Update(fTimeNudge, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);

        if (m_vParticles[uiSlot].IsDead(uiMilliseconds, bExpired))
            m_vParticles[uiSlot].SetActive(false);
        else
            m_bbBounds.AddPoint(m_vParticles[uiSlot].GetPos());

        m_uiBackSlot = (uiSlot + 1) % m_vParticles.size();

        // Push front slot forward if we wrapped around and caught it
        if (m_uiBackSlot == m_uiFrontSlot)
            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();

        while (!m_vParticles[m_uiFrontSlot].IsActive() && m_uiFrontSlot != m_uiBackSlot)
            m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();

        ++m_iSpawnCount;
    }

    m_v3LastPos = v3Pos;
    m_aLastAxis = aAxis;
    m_fLastScale = fScale;

    GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, m_aLastBoneAxis, m_v3LastBonePos);

    m_uiLastUpdateTime = uiMilliseconds;
    m_fLastLerp = fLerp;
    m_fLastTime = fTime;

    m_bLastActive = bActive;

    UpdateNextEmitter(uiMilliseconds, pfnTrace);

    m_bbBounds.Transform(m_v3LastPos, m_aLastAxis, m_fLastScale);

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
  COrbiterEmitter::GetNumBillboards
  ====================*/
uint    COrbiterEmitter::GetNumBillboards()
{
    if (m_vParticles.size() == 0)
        return 0;

    int iNumParticles(int(m_uiBackSlot) - int(m_uiFrontSlot));

    if (iNumParticles == 0 && m_vParticles[m_uiFrontSlot].IsActive())
        iNumParticles += int(m_vParticles.size());
    else if (iNumParticles < 0)
        iNumParticles += int(m_vParticles.size());

    return uint(iNumParticles);
}


/*====================
  COrbiterEmitter::GetBillboard
  ====================*/
bool    COrbiterEmitter::GetBillboard(uint uiIndex, SBillboard &outBillboard)
{
    uint uiParticle((m_uiFrontSlot + uiIndex) % m_vParticles.size());

    if (!m_vParticles[uiParticle].IsActive())
        return false;

    m_vParticles[uiParticle].GetBillboard(m_uiLastUpdateTime, outBillboard);

    if (m_bCylindrical)
        outBillboard.v3Pos = M_CylindricalToCartesian(outBillboard.v3Pos);
    else
        outBillboard.v3Pos = M_PolarToCartesian(outBillboard.v3Pos);

    // Rotate toward v3Up
    if (m_vParticles[uiParticle].GetUp() != CVec3f(0.0f, 0.0f, 1.0f))
        outBillboard.v3Pos = TransformPoint(outBillboard.v3Pos, GetAxisFromUpVec(m_vParticles[uiParticle].GetUp()));

    outBillboard.v3Pos = TransformPoint(outBillboard.v3Pos, m_aLastBoneAxis, m_v3LastBonePos, 1.0f);

    outBillboard.height *= m_fLastScale;
    outBillboard.width *= m_fLastScale;
    outBillboard.v3Pos = TransformPoint(outBillboard.v3Pos, m_aLastAxis, m_v3LastPos, m_fLastScale);

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

            outBillboard.height *= fScale;
            outBillboard.width *= fScale;
            outBillboard.v3Pos = TransformPoint(outBillboard.v3Pos, aAxis, v3Pos, fScale);

            if (m_vParticles[uiParticle].GetFlags() & BBOARD_TURN)
            {
                const CCamera *pCam(Vid.GetCamera());

                CVec3f v3Pos0(m_vParticles[uiParticle].GetPos());
                {
                    if (m_bCylindrical)
                        v3Pos0 = M_CylindricalToCartesian(v3Pos0);
                    else
                        v3Pos0 = M_PolarToCartesian(v3Pos0);

                    // Rotate toward v3Up
                    if (m_vParticles[uiParticle].GetUp() != CVec3f(0.0f, 0.0f, 1.0f))
                        v3Pos0 = TransformPoint(v3Pos0, GetAxisFromUpVec(m_vParticles[uiParticle].GetUp()));

                    v3Pos0 = TransformPoint(v3Pos0, aAxis, v3Pos, fScale);
                }

                CVec3f v3Pos1(m_vParticles[uiParticle].GetPos() + m_vParticles[uiParticle].GetDir() * 0.01f);
                {
                    if (m_bCylindrical)
                        v3Pos1 = M_CylindricalToCartesian(v3Pos1);
                    else
                        v3Pos1 = M_PolarToCartesian(v3Pos1);

                    // Rotate toward v3Up
                    if (m_vParticles[uiParticle].GetUp() != CVec3f(0.0f, 0.0f, 1.0f))
                        v3Pos1 = TransformPoint(v3Pos1, GetAxisFromUpVec(m_vParticles[uiParticle].GetUp()));

                    v3Pos1 = TransformPoint(v3Pos1, aAxis, v3Pos, fScale);
                }

                CVec3f v3Dir(v3Pos1 - v3Pos0);

                // Orthogonalize
                v3Dir -= pCam->GetViewAxis(FORWARD) * DotProduct(pCam->GetViewAxis(FORWARD), v3Dir);
                v3Dir.Normalize();

                float fDotUp(DotProduct(pCam->GetViewAxis(UP), v3Dir));
                float fDotRight(DotProduct(pCam->GetViewAxis(RIGHT), v3Dir));

                if (fDotUp > 0.0f)
                {
                    if (fDotRight > 0.0f) // Quadrant I
                        outBillboard.angle -= RAD2DEG(acos(fDotUp));
                    else // Quadrant II
                        outBillboard.angle += RAD2DEG(acos(fDotUp));
                }
                else
                {
                    if (fDotRight < 0.0f) // Quadrant III
                        outBillboard.angle += RAD2DEG(acos(fDotUp));
                    else // Quadrant IV
                        outBillboard.angle -= RAD2DEG(acos(fDotUp));
                }
            }

        } break;
    }

    outBillboard.hMaterial = m_hMaterial;

    if (!m_tfParticleScale.IsOne())
    {
        float fScale(m_tfParticleScale.Evaluate(m_fLastLerp, m_fLastTime));
        outBillboard.width *= fScale;
        outBillboard.height *= fScale;
    }

    if (!m_tv3ParticleColor.IsOne())
        outBillboard.color.xyz() *= m_tv3ParticleColor.Evaluate(m_fLastLerp, m_fLastTime);

    if (!m_tfParticleAlpha.IsOne())
        outBillboard.color.w *= m_tfParticleAlpha.Evaluate(m_fLastLerp, m_fLastTime);

    outBillboard.fDepthBias = m_fDepthBias;

    return true;
}
