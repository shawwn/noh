// (C)2007 S2 Games
// c_precipemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_precipemitter.h"
#include "c_simpleparticle.h"
#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
#include "c_effect.h"
#include "c_camera.h"
#include "c_scenemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CPrecipEmitterDef::~CPrecipEmitterDef
  ====================*/
CPrecipEmitterDef::~CPrecipEmitterDef()
{
}


/*====================
  CPrecipEmitterDef::CPrecipEmitterDef
  ====================*/
CPrecipEmitterDef::CPrecipEmitterDef
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
    const CRangef &rfDrawDistance,
    bool bCollide,
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
m_rfDrawDistance(rfDrawDistance),
m_bCollide(bCollide),
m_tv3ParticleColor(tv3ParticleColor),
m_rfParticleAlpha(rfParticleAlpha),
m_rfParticleScale(rfParticleScale),
m_fDepthBias(fDepthBias)
{
}


/*====================
  CPrecipEmitterDef::Spawn
  ====================*/
IEmitter*   CPrecipEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
    PROFILE("CPrecipEmitterDef::Spawn");

    return K2_NEW(ctx_Effects, CPrecipEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CPrecipEmitter::~CPrecipEmitter
  ====================*/
CPrecipEmitter::~CPrecipEmitter()
{
}


/*====================
  CPrecipEmitter::CPrecipEmitter
  ====================*/
CPrecipEmitter::CPrecipEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CPrecipEmitterDef &eSettings) :
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
m_hMaterial(eSettings.GetMaterial()),
m_v3Dir(eSettings.GetDir()),
m_fDrag(eSettings.GetDrag()),
m_fFriction(eSettings.GetFriction()),
m_fDrawDistance(eSettings.GetDrawDistance()),
m_bCollide(eSettings.GetCollide()),
m_tv3ParticleColor(eSettings.GetParticleColor()),
m_tfParticleAlpha(eSettings.GetParticleAlpha()),
m_tfParticleScale(eSettings.GetParticleScale()),
m_fDepthBias(eSettings.GetDepthBias())
{
    int iMaxActive;

    if (m_riMaxParticleLife.Max() == -1 || m_rfSpawnRate.Max() == 0.0f)
        iMaxActive = m_iCount == -1 ? 1024 : m_iCount + 1;
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
    m_v3LastPos = m_pParticleSystem->GetCamera()->GetOrigin();
    m_aLastAxis = m_pParticleSystem->GetCamera()->GetViewAxis();
    m_fLastScale = 1.0f;

    for (vector<CSimpleParticleDef *>::const_iterator it(m_pvParticleDefinitions->begin()); it != m_pvParticleDefinitions->end(); ++it)
        m_fSelectionWeightRange += (*it)->GetSelectionWeight();

    m_uiLastUpdateTime -= m_iTimeNudge;

    m_uiStartTime += m_iDelay;
    m_uiLastUpdateTime += m_iDelay;

    m_bActive = true;

    m_fLastLerp = 0.0f;
    m_fLastTime = 0.0f;

    m_bbBounds = CBBoxf(-FAR_AWAY, FAR_AWAY); // Never cull
}


/*====================
  CPrecipEmitter::ResumeFromPause
  ====================*/
void    CPrecipEmitter::ResumeFromPause(uint uiMilliseconds)
{
    uint uiPauseDuration(uiMilliseconds - m_uiPauseBegin);
    m_uiStartTime += uiPauseDuration;
    m_uiLastUpdateTime = uiMilliseconds;
    m_uiPauseBegin = 0;

    ParticleList::iterator itEnd(m_vParticles.end());
    for (ParticleList::iterator it(m_vParticles.begin()); it != itEnd; ++it)
        it->AddToStartTime(uiPauseDuration);
}


/*====================
  CPrecipEmitter::Update
  ====================*/
bool    CPrecipEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
    PROFILE("CPrecipEmitter::Update");

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
        Console.Warn << _T("<precipemitter> iDeltaTime == ") << iDeltaTime << newl;
        iDeltaTime = 0xffff;
    }
#else
    if (iDeltaTime > 0x1000)
        iDeltaTime = 0x1000;
#endif

    float fDeltaTime(iDeltaTime * SEC_PER_MS);

    CVec3f v3Pos(m_pParticleSystem->GetCamera()->GetOrigin());
    CAxis aWorldAxis(CAxis(0.0f, 0.0f, 0.0f));
    CAxis aAxis(m_pParticleSystem->GetCamera()->GetViewAxis());
    float fScale(1.0f);

    // Calculate temporal properties
    float fTime((uiMilliseconds - m_uiStartTime) * SEC_PER_MS);
    float fLerp;

    if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= uiMilliseconds && (m_iLife == -1 || m_bLoop) && m_iExpireLife != 0.0f)
        fLerp = m_iExpireLife != -1 ? MIN(float(uiMilliseconds - m_uiExpireTime) / m_iExpireLife, 1.0f) : 0.0f;
    else
        fLerp = m_iLife != -1 ? MIN(float(uiMilliseconds - m_uiStartTime) / m_iLife, 1.0f) : 0.0f;

    float   fSpawnRate(m_rfSpawnRate.Evaluate(fLerp, fTime));
    float   fGravity(m_rfGravity.Evaluate(fLerp, fTime));

    bool bExpired(m_uiExpireTime != INVALID_TIME);

    CVec3f  v3BaseVelocity((v3Pos - m_v3LastPos) / fDeltaTime);

    // Update existing particles
    CVec3f v3Acceleration(0.0f, 0.0f, fGravity * -20.0f);

    ParticleList::iterator itEnd(m_vParticles.end());
    for (ParticleList::iterator it(m_vParticles.begin()); it != itEnd; ++it)
    {
        if (it->IsActive())
        {
            it->Update(fDeltaTime, v3Acceleration, m_fDrag, m_fFriction, m_bCollide ? pfnTrace : nullptr);

            if (it->IsDead(uiMilliseconds, bExpired))
                it->SetActive(false);
            else
            {
                const CVec3f &v3ParticlePos(it->GetPos());

                for (int iDim(X); iDim <= Z; ++iDim)
                {
                    if (v3ParticlePos[iDim] < v3Pos[iDim] - m_fDrawDistance || v3ParticlePos[iDim] > v3Pos[iDim] + m_fDrawDistance)
                    {
                        uint    uiMillisecondNudge(uiMilliseconds - it->GetStartTime());
                        float   fTimeNudge(uiMillisecondNudge * SEC_PER_MS);

                        float   fLerp(m_iLife != -1 ? CLAMP(ILERP(uiMilliseconds - m_uiStartTime - uiMillisecondNudge, 0u, uint(m_iLife)), 0.0f, 1.0f) : 0.0f);

                        // Calculate new particle temporal properties
                        int     iMinParticleLife(m_riMinParticleLife.Lerp(fLerp));
                        int     iMaxParticleLife(m_riMaxParticleLife.Lerp(fLerp));
                        int     iParticleTimeNudge(m_riParticleTimeNudge.Lerp(fLerp));
                        float   fMinSpeed(m_rfMinSpeed.Lerp(fLerp));
                        float   fMaxSpeed(m_rfMaxSpeed.Lerp(fLerp));
                        float   fMinAcceleration(m_rfMinAcceleration.Lerp(fLerp));
                        float   fMaxAcceleration(m_rfMaxAcceleration.Lerp(fLerp));
                        float   fMinAngle(m_rfMinAngle.Lerp(fLerp));
                        float   fMaxAngle(m_rfMaxAngle.Lerp(fLerp));
                        float   fMinInheritVelocity(m_rfMinInheritVelocity.Lerp(fLerp));
                        float   fMaxInheritVelocity(m_rfMaxInheritVelocity.Lerp(fLerp));
                        float   fLimitInheritVelocity(m_rfLimitInheritVelocity.Lerp(fLerp));

                        CVec3f  v3Dir(fMaxAngle - fMinAngle >= 180.0f ? M_RandomDirection() : M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle));

                        if (m_eDirectionalSpace == DIRSPACE_LOCAL)
                        {
                            v3Dir = TransformPoint(v3Dir, aAxis);
                        }

                        CVec3f  v3OffsetPos;

                        if (iDim != X)
                            v3OffsetPos.x = M_Randnum(-m_fDrawDistance, m_fDrawDistance);
                        else
                        {
                            if (v3ParticlePos.x < v3Pos.x - m_fDrawDistance)
                                v3OffsetPos.x = m_fDrawDistance + fmod(v3ParticlePos.x - v3Pos.x + m_fDrawDistance, 2.0f * m_fDrawDistance);
                            else
                                v3OffsetPos.x = -m_fDrawDistance + fmod(v3ParticlePos.x - v3Pos.x - m_fDrawDistance, 2.0f * m_fDrawDistance);
                        }

                        if (iDim != Y)
                            v3OffsetPos.y = M_Randnum(-m_fDrawDistance, m_fDrawDistance);
                        else
                        {
                            if (v3ParticlePos.y < v3Pos.y - m_fDrawDistance)
                                v3OffsetPos.y = m_fDrawDistance + fmod(v3ParticlePos.y - v3Pos.y + m_fDrawDistance, 2.0f * m_fDrawDistance);
                            else
                                v3OffsetPos.y = -m_fDrawDistance + fmod(v3ParticlePos.y - v3Pos.y - m_fDrawDistance, 2.0f * m_fDrawDistance);
                        }

                        if (iDim != Z)
                            v3OffsetPos.z = M_Randnum(-m_fDrawDistance, m_fDrawDistance);
                        else
                        {
                            if (v3ParticlePos.z < v3Pos.z - m_fDrawDistance)
                                v3OffsetPos.z = m_fDrawDistance + fmod(v3ParticlePos.z - v3Pos.z + m_fDrawDistance, 2.0f * m_fDrawDistance);
                            else
                                v3OffsetPos.z = -m_fDrawDistance + fmod(v3ParticlePos.z - v3Pos.z - m_fDrawDistance, 2.0f * m_fDrawDistance);
                        }

                        CVec3f  v3Position(TransformPoint(v3OffsetPos, m_eDirectionalSpace == DIRSPACE_LOCAL ? aAxis : aWorldAxis, v3Pos));

                        CVec3f  v3Velocity(v3Dir * M_Randnum(fMinSpeed, fMaxSpeed));
                        CVec3f  v3InheritVelocity(LERP(fTimeNudge / fDeltaTime, v3BaseVelocity, m_v3LastBaseVelocity) * M_Randnum(fMinInheritVelocity, fMaxInheritVelocity));
                        if (fLimitInheritVelocity > 0.0f && v3InheritVelocity.Length() > fLimitInheritVelocity)
                            v3InheritVelocity.SetLength(fLimitInheritVelocity);
                        v3Velocity += v3InheritVelocity;

                        float   fAcceleration(M_Randnum(fMinAcceleration, fMaxAcceleration));

                        uiMillisecondNudge += iParticleTimeNudge;
                        fTimeNudge += iParticleTimeNudge * SEC_PER_MS;

                        int iParticleLife(M_Randnum(iMinParticleLife, iMaxParticleLife));
                        if (iParticleLife == -1 || uint(iParticleLife) > uiMillisecondNudge)
                        {
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

                            uint uiSlot(it - m_vParticles.begin());

                            m_vParticles[uiSlot].Spawn
                            (
                                uiMilliseconds - uiMillisecondNudge,
                                iParticleLife,
                                v3Position,
                                v3Velocity,
                                v3Dir,
                                fAcceleration,
                                1.0f,
                                v3Position,
                                nullptr,
                                **itDef
                            );

                            m_vParticles[uiSlot].SetActive(true);

                            // Update the particle a bit to get rid of any framerate dependent spawning patterns
                            m_vParticles[uiSlot].Update(fTimeNudge, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);

                            m_vParticles[uiSlot].SetPos(v3Position);

                            if (m_vParticles[uiSlot].IsDead(uiMilliseconds, bExpired))
                                m_vParticles[uiSlot].SetActive(false);
                        }
                    }
                }
            }
        }
    }

    while (!m_vParticles[m_uiFrontSlot].IsActive() && m_uiFrontSlot != m_uiBackSlot)
        m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();

    bool bActive(m_bActive && m_pParticleSystem->GetActive());

    if (m_uiExpireTime != INVALID_TIME && (m_iLife == -1 || m_bLoop))
        if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
            bActive = false;

    float fClampedDeltaTime;

    if (m_iLife != -1)
        fClampedDeltaTime = MIN(fDeltaTime, (m_iLife + int(m_uiStartTime) - int(m_uiLastUpdateTime)) * SEC_PER_MS);
    else
        fClampedDeltaTime = fDeltaTime;

    if (fClampedDeltaTime < 0.0f)
        fClampedDeltaTime = 0.0f;

    if (m_iCount != -1)
    {
        if (bActive)
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
    else if (bActive)
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
        float   fMinSpeed(m_rfMinSpeed.Lerp(fLerp));
        float   fMaxSpeed(m_rfMaxSpeed.Lerp(fLerp));
        float   fMinAcceleration(m_rfMinAcceleration.Lerp(fLerp));
        float   fMaxAcceleration(m_rfMaxAcceleration.Lerp(fLerp));
        float   fMinAngle(m_rfMinAngle.Lerp(fLerp));
        float   fMaxAngle(m_rfMaxAngle.Lerp(fLerp));
        float   fMinInheritVelocity(m_rfMinInheritVelocity.Lerp(fLerp));
        float   fMaxInheritVelocity(m_rfMaxInheritVelocity.Lerp(fLerp));
        float   fLimitInheritVelocity(m_rfLimitInheritVelocity.Lerp(fLerp));

        CVec3f  v3Dir(fMaxAngle - fMinAngle >= 180.0f ? M_RandomDirection() : M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle));

        if (m_eDirectionalSpace == DIRSPACE_LOCAL)
        {
            v3Dir = TransformPoint(v3Dir, aAxis);
        }

        CVec3f  v3OffsetPos(CVec3f(M_Randnum(-m_fDrawDistance, m_fDrawDistance), M_Randnum(-m_fDrawDistance, m_fDrawDistance), M_Randnum(-m_fDrawDistance, m_fDrawDistance)));

        CVec3f  v3Position(TransformPoint(v3OffsetPos, m_eDirectionalSpace == DIRSPACE_LOCAL ? aAxis : aWorldAxis, v3Pos));

        CVec3f  v3Velocity(v3Dir * M_Randnum(fMinSpeed, fMaxSpeed));
        CVec3f  v3InheritVelocity(LERP(fTimeNudge / fDeltaTime, v3BaseVelocity, m_v3LastBaseVelocity) * M_Randnum(fMinInheritVelocity, fMaxInheritVelocity));
        if (fLimitInheritVelocity > 0.0f && v3InheritVelocity.Length() > fLimitInheritVelocity)
            v3InheritVelocity.SetLength(fLimitInheritVelocity);
        v3Velocity += v3InheritVelocity;

        float   fAcceleration(M_Randnum(fMinAcceleration, fMaxAcceleration));

        uiMillisecondNudge += iParticleTimeNudge;
        fTimeNudge += iParticleTimeNudge * SEC_PER_MS;

        int iParticleLife(M_Randnum(iMinParticleLife, iMaxParticleLife));
        if (iParticleLife == -1 || uint(iParticleLife) > uiMillisecondNudge)
        {
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

            m_vParticles[uiSlot].Spawn
            (
                uiMilliseconds - uiMillisecondNudge,
                iParticleLife,
                v3Position,
                v3Velocity,
                v3Dir,
                fAcceleration,
                1.0f,
                v3Position,
                nullptr,
                **itDef
            );

            m_vParticles[uiSlot].SetActive(true);

            // Update the particle a bit to get rid of any framerate dependent spawning patterns
            m_vParticles[uiSlot].Update(fTimeNudge, CVec3f(0.0f, 0.0f, fGravity * -20.0f), m_fDrag, m_fFriction);

            if (m_vParticles[uiSlot].IsDead(uiMilliseconds, bExpired))
                m_vParticles[uiSlot].SetActive(false);

            m_uiBackSlot = (uiSlot + 1) % m_vParticles.size();

            // Push front slot forward if we wrapped around and caught it
            if (m_uiBackSlot == m_uiFrontSlot)
            {
                if (m_vParticles[m_uiFrontSlot].GetImbeddedEmitter())
                {
                    K2_DELETE(m_vParticles[m_uiFrontSlot].GetImbeddedEmitter());
                    m_vParticles[m_uiFrontSlot].SetImbeddedEmitter(nullptr);
                }

                m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();
            }

            while ((!m_vParticles[m_uiFrontSlot].IsActive() && m_vParticles[m_uiFrontSlot].GetImbeddedEmitter() == nullptr) && m_uiFrontSlot != m_uiBackSlot)
                m_uiFrontSlot = (m_uiFrontSlot + 1) % m_vParticles.size();
        }

        ++m_iSpawnCount;
    }

    m_v3LastPos = v3Pos;
    m_aLastAxis = aAxis;
    m_fLastScale = fScale;
    m_v3LastBaseVelocity = v3BaseVelocity;

    m_uiLastUpdateTime = uiMilliseconds;
    m_fLastLerp = fLerp;
    m_fLastTime = fTime;

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
  CPrecipEmitter::GetNumBillboards
  ====================*/
uint    CPrecipEmitter::GetNumBillboards()
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
  CPrecipEmitter::GetBillboard
  ====================*/
bool    CPrecipEmitter::GetBillboard(uint uiIndex, SBillboard &outBillboard)
{
    uint uiParticle((m_uiFrontSlot + uiIndex) % m_vParticles.size());

    if (!m_vParticles[uiParticle].IsActive())
        return false;

    if (!SceneManager.PointInFrustum(m_vParticles[uiParticle].GetPos(), true))
        return false;

    m_vParticles[uiParticle].GetBillboard(m_uiLastUpdateTime, outBillboard);

    float fStickiness(m_vParticles[uiParticle].GetStickiness(m_uiLastUpdateTime));

    if (fStickiness == 1.0f)
        outBillboard.v3Pos = m_v3LastPos;
    else if (fStickiness != 0.0f)
        outBillboard.v3Pos = LERP(fStickiness, outBillboard.v3Pos, m_v3LastPos);

    float fAnchor(m_vParticles[uiParticle].GetAnchor(m_uiLastUpdateTime));

    if (fAnchor == 1.0f)
        outBillboard.v3Pos += m_v3LastPos - m_vParticles[uiParticle].GetEmitterPos();
    else if (fAnchor != 0.0f)
        outBillboard.v3Pos += (m_v3LastPos - m_vParticles[uiParticle].GetEmitterPos()) * fAnchor;

    if (outBillboard.uiFlags & (BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT | BBOARD_GENERATE_AXIS))
        outBillboard.aAxis = AXIS_IDENTITY;

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

