// (C)2006 S2 Games
// c_meshemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_meshemitter.h"
#include "c_simpleparticle.h"
#include "c_particlesystem.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CMeshEmitterDef::~CMeshEmitterDef
  ====================*/
CMeshEmitterDef::~CMeshEmitterDef()
{
}


/*====================
  CMeshEmitterDef::CMeshEmitterDef
  ====================*/
CMeshEmitterDef::CMeshEmitterDef
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
m_eDirectionalSpace(eDirectionalSpace),
m_fDrag(fDrag),
m_fFriction(fFriction),
m_sMesh(sMesh),
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
m_bCollide(bCollide),
m_eParticleDirectionalSpace(eParticleDirectionalSpace),
m_tv3ParticleColor(tv3ParticleColor),
m_rfParticleAlpha(rfParticleAlpha),
m_rfParticleScale(rfParticleScale),
m_fDepthBias(fDepthBias)
{
}


/*====================
  CMeshEmitterDef::Spawn
  ====================*/
IEmitter*	CMeshEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
	PROFILE("CMeshEmitterDef::Spawn");

	return K2_NEW(ctx_Effects, CMeshEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CMeshEmitter::~CMeshEmitter
  ====================*/
CMeshEmitter::~CMeshEmitter()
{
}


/*====================
  CMeshEmitter::CMeshEmitter
  ====================*/
CMeshEmitter::CMeshEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CMeshEmitterDef &eSettings) :
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
m_sMesh(eSettings.GetMesh()),
m_tv3OffsetSphere(eSettings.GetOffsetSphere()),
m_tv3OffsetCube(eSettings.GetOffsetCube()),
m_bCollide(eSettings.GetCollide()),
m_eParticleDirectionalSpace(eSettings.GetParticleDirectionalSpace()),
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

	m_v3LastBasePos = TransformPoint(GetBonePosition(uiStartTime, m_pOwner, m_sBone), m_aLastAxis, m_v3LastPos, m_fLastScale);

	m_bLastActive = m_pParticleSystem->GetActive() && GetVisibility();

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
  CMeshEmitter::ResumeFromPause
  ====================*/
void	CMeshEmitter::ResumeFromPause(uint uiMilliseconds)
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
  CMeshEmitter::UpdateEmbeddedEmitter
  ====================*/
inline
bool	CMeshEmitter::UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, IEmitter *pEmitter, CSimpleParticle &cParticle)
{
	if (!cParticle.IsActive() && !pEmitter->GetExpire())
		pEmitter->Expire(uiMilliseconds);

	float fLerp(cParticle.GetLerp(uiMilliseconds));
	float fTime(cParticle.GetTime(uiMilliseconds));

	pEmitter->SetCustomPos(cParticle.GetPos());
	pEmitter->SetCustomScale(cParticle.GetScale(fLerp, fTime)/* * m_fParticleScale*/);

	CAxis aAxis(m_eParticleDirectionalSpace == DIRSPACE_LOCAL ? GetAxis() : AXIS_IDENTITY);

	if (cParticle.GetFlags() & BBOARD_TURN)
	{
		CAxis aDir;

		const CVec3f &v3Velocity(cParticle.GetVelocity());
		if (v3Velocity == V3_ZERO)
			aDir.SetFromForwardVec(cParticle.GetDir());
		else
			aDir.SetFromForwardVec(cParticle.GetVelocity());

		aAxis = aAxis * aDir;
	}

	aAxis = aAxis * CAxis(cParticle.GetPitch(fLerp, fTime), cParticle.GetRoll(fLerp, fTime), cParticle.GetYaw(fLerp, fTime));

	pEmitter->SetCustomAxis(aAxis);

	bool bRet(pEmitter->Update(uiMilliseconds, pfnTrace));

	return bRet;
}


/*====================
  CMeshEmitter::Update
  ====================*/
bool	CMeshEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
	PROFILE("CMeshEmitter::Update");

	if (m_uiPauseBegin)
		ResumeFromPause(uiMilliseconds);

	if (m_vParticles.size() == 0)
		return false;

    int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

	if (iDeltaTime <= 0)
	{
		UpdateNextEmitter(uiMilliseconds, pfnTrace);
		return true;
	}

#if 0
	if (iDeltaTime > 0xffff)
	{
		Console.Warn << _T("<meshemitter> iDeltaTime == ") << iDeltaTime << newl;
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

	float	fSpawnRate(m_rfSpawnRate.Evaluate(fLerp, fTime));
	float	fGravity(m_rfGravity.Evaluate(fLerp, fTime));

	bool bExpired(m_uiExpireTime != INVALID_TIME);

	START_PROFILE("Update Particles");

	m_bbBounds.Clear();

	// Update existing particles
	if (fGravity == 0.0f && m_fDrag == 0.0f && m_fFriction == 0.0f && !m_bCollide)
	{
		ParticleList::iterator itEnd(m_vParticles.end());
		for (ParticleList::iterator it(m_vParticles.begin()); it != itEnd; ++it)
		{
			if (it->IsActive())
			{
				if (it->IsDead(uiMilliseconds, bExpired))
					it->SetActive(false);
				else
				{
					it->Update(fDeltaTime);
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
	}
	else if (m_fDrag == 0.0f && m_fFriction == 0.0f && !m_bCollide)
	{
		CVec3f v3Acceleration(0.0f, 0.0f, fGravity * -20.0f);
		CVec3f v3AccelTime(v3Acceleration * fDeltaTime);
		CVec3f v3AccelTimeSq(v3AccelTime * (0.5f * fDeltaTime));

		ParticleList::iterator itEnd(m_vParticles.end());
		for (ParticleList::iterator it(m_vParticles.begin()); it != itEnd; ++it)
		{
			if (it->IsActive())
			{
				if (it->IsDead(uiMilliseconds, bExpired))
					it->SetActive(false);
				else
				{
					it->Update(fDeltaTime, v3Acceleration, v3AccelTime, v3AccelTimeSq);
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
	}
	else
	{
		CVec3f v3Acceleration(0.0f, 0.0f, fGravity * -20.0f);

		ParticleList::iterator itEnd(m_vParticles.end());
		for (ParticleList::iterator it(m_vParticles.begin()); it != itEnd; ++it)
		{
			if (it->IsActive())
			{
				if (it->IsDead(uiMilliseconds, bExpired))
					it->SetActive(false);
				else
				{
					it->Update(fDeltaTime, v3Acceleration, m_fDrag, m_fFriction, m_bCollide ? pfnTrace : NULL);
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
	}

	END_PROFILE; // Update Particles

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

	vec4_t quat0;
	vec4_t quat1;

	M_AxisToQuat((const vec3_t *)(&m_aLastAxis), quat0);
	M_AxisToQuat((const vec3_t *)(&aAxis), quat1);

	CVec3f	v3BasePos(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone));
	v3BasePos = TransformPoint(v3BasePos, aAxis, v3Pos, fScale);

	CVec3f	v3BaseVelocity(m_bLastActive && bActive ? (v3BasePos - m_v3LastBasePos) / fDeltaTime : V3_ZERO);

	CVec3f	v3OffsetSphere(m_tv3OffsetSphere.Lerp(fLerp));
	CVec3f	v3OffsetCube(m_tv3OffsetCube.Lerp(fLerp));

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

		float	fTimeNudge(fSpawnRate > 0.0f ? m_fAccumulator / fSpawnRate : 0.0f);
		uint	uiMillisecondNudge(INT_FLOOR(fTimeNudge * MS_PER_SEC));
		float	fLerp(m_iLife != -1 ? CLAMP(ILERP(uiMilliseconds - m_uiStartTime - uiMillisecondNudge, 0u, uint(m_iLife)), 0.0f, 1.0f) : 0.0f);

		// Calculate new particle temporal properties
		int		iMinParticleLife(m_riMinParticleLife.Lerp(fLerp));
		int		iMaxParticleLife(m_riMaxParticleLife.Lerp(fLerp));
		int		iParticleTimeNudge(m_riParticleTimeNudge.Lerp(fLerp));

		int iParticleLife(M_Randnum(iMinParticleLife, iMaxParticleLife));
		if (iParticleLife != -1 && uint(iParticleLife) < uiMillisecondNudge + iParticleTimeNudge)
		{
			// Skip dead particles
			++m_iSpawnCount;
			continue;
		}

		float	fMinSpeed(m_rfMinSpeed.Lerp(fLerp));
		float	fMaxSpeed(m_rfMaxSpeed.Lerp(fLerp));
		float	fMinAcceleration(m_rfMinAcceleration.Lerp(fLerp));
		float	fMaxAcceleration(m_rfMaxAcceleration.Lerp(fLerp));
		float	fMinAngle(m_rfMinAngle.Lerp(fLerp));
		float	fMaxAngle(m_rfMaxAngle.Lerp(fLerp));
		float	fMinInheritVelocity(m_rfMinInheritVelocity.Lerp(fLerp));
		float	fMaxInheritVelocity(m_rfMaxInheritVelocity.Lerp(fLerp));
		float	fMinOffsetDirection(m_rfMinOffsetDirection.Lerp(fLerp));
		float	fMaxOffsetDirection(m_rfMaxOffsetDirection.Lerp(fLerp));
		float	fMinOffsetRadial(m_rfMinOffsetRadial.Lerp(fLerp));
		float	fMaxOffsetRadial(m_rfMaxOffsetRadial.Lerp(fLerp));
		float	fMinOffsetRadialAngle(m_rfMinOffsetRadialAngle.Lerp(fLerp));
		float	fMaxOffsetRadialAngle(m_rfMaxOffsetRadialAngle.Lerp(fLerp));

		float fFrameLerp(CLAMP(1.0f - fTimeNudge / fDeltaTime, 0.0f, 1.0f));

		CVec3f	v3LerpedPos(LERP(fFrameLerp, m_v3LastPos, v3Pos));
		CAxis	aLerpedAxis;

		vec4_t	lerpedQuat;
		M_LerpQuat(fFrameLerp, quat0, quat1, lerpedQuat);

		M_QuatToAxis(lerpedQuat, (vec3_t *)(&aLerpedAxis));

		float fLerpedScale(LERP(fFrameLerp, m_fLastScale, fScale));

		CVec3f	v3MeshPos;
		CVec3f	v3Dir;

		if (m_sMesh.empty())
		{
			v3MeshPos = V3_ZERO;
			v3Dir = M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle);
		}
		else if (m_v3Dir != CVec3f(0.0f, 0.0f, 1.0f))
		{
			CVec3f v3FaceNormal;
			v3MeshPos = m_pParticleSystem->GetSourceRandomPositionWithNormalOnMesh(m_sMesh, v3FaceNormal);

			CAxis	aAxis(GetAxisFromUpVec(v3FaceNormal));
			CVec3f	v3Tmp(TransformPoint(m_v3Dir, aAxis));
			v3Dir = M_RandomDirection((v3Tmp == V3_ZERO) ? v3FaceNormal : v3Tmp, fMinAngle, fMaxAngle);
		}
		else if (true)
		{
			CVec3f v3FaceNormal;
			v3MeshPos = m_pParticleSystem->GetSourceRandomPositionWithNormalOnMesh(m_sMesh, v3FaceNormal);
			v3Dir = M_RandomDirection(v3FaceNormal, fMinAngle, fMaxAngle);
		}
		else
		{
			v3MeshPos = m_pParticleSystem->GetSourceRandomPositionOnMesh(m_sMesh);
			v3Dir = M_RandomDirection(m_v3Dir, fMinAngle, fMaxAngle);
		}

		if (m_eDirectionalSpace == DIRSPACE_LOCAL)
			v3Dir = TransformPoint(v3Dir, aLerpedAxis);

		//
		// Position
		//

		CVec3f	v3OffsetPos(0.0f, 0.0f, 0.0f);
		{
			if (v3OffsetSphere != V3_ZERO)
			{
				CVec3f	v3Rand(M_RandomPointInSphere());

				v3OffsetPos += v3OffsetSphere * v3Rand;
			}

			if (v3OffsetCube != V3_ZERO)
			{
				CVec3f	v3Rand(M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f), M_Randnum(-1.0f, 1.0f));

				v3OffsetPos += v3OffsetCube * v3Rand;
			}

			float	fOffsetDirection(M_Randnum(fMinOffsetDirection, fMaxOffsetDirection));
			if (fOffsetDirection != 0.0f)
			{
				v3OffsetPos += v3Dir * fOffsetDirection;
			}

			float	fOffsetRadial(M_Randnum(fMinOffsetRadial, fMaxOffsetRadial));
			if (fOffsetRadial != 0.0f)
			{
				v3OffsetPos += M_RandomDirection(m_v3Dir, fMinOffsetRadialAngle, fMaxOffsetRadialAngle) * fOffsetRadial;
			}
		}

		CVec3f	v3Position(TransformPoint(v3OffsetPos + v3MeshPos, aLerpedAxis, v3LerpedPos, fLerpedScale));

		float	fLimitInheritVelocity(m_rfLimitInheritVelocity.Lerp(fLerp) * fLerpedScale);

		CVec3f	v3Velocity(v3Dir * (M_Randnum(fMinSpeed, fMaxSpeed) * fLerpedScale));
		CVec3f	v3InheritVelocity(LERP(fTimeNudge / fDeltaTime, v3BaseVelocity, m_v3LastBaseVelocity) * M_Randnum(fMinInheritVelocity, fMaxInheritVelocity));
		if (fLimitInheritVelocity > 0.0f && v3InheritVelocity.Length() > fLimitInheritVelocity)
			v3InheritVelocity.SetLength(fLimitInheritVelocity);
		v3Velocity += v3InheritVelocity;

		float	fAcceleration(M_Randnum(fMinAcceleration, fMaxAcceleration) * fLerpedScale);

		//
		// Particle selection and spawning
		//

		uiMillisecondNudge += iParticleTimeNudge;
		fTimeNudge += iParticleTimeNudge * SEC_PER_MS;

		vector<CSimpleParticleDef *>::const_iterator	itDef(m_pvParticleDefinitions->begin());
		if (m_pvParticleDefinitions->size() > 1)
		{
			float	fRand(M_Randnum(0.0f, m_fSelectionWeightRange));

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
			fLerpedScale,
			TransformPoint(v3MeshPos, aLerpedAxis, v3LerpedPos, fLerpedScale),
			NULL,
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
	m_v3LastBasePos = v3BasePos;
	m_v3LastBaseVelocity = v3BaseVelocity;

	CAxis	aBoneAxis;
	CVec3f	v3BonePos;

	GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, aBoneAxis, v3BonePos);

	m_v3LastEmitterPos = TransformPoint(v3BonePos, m_aLastAxis, m_v3LastPos, m_fLastScale);

	m_uiLastUpdateTime = uiMilliseconds;
	m_fLastLerp = fLerp;
	m_fLastTime = fTime;

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
  CMeshEmitter::GetNumBillboards
  ====================*/
uint	CMeshEmitter::GetNumBillboards()
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
  CMeshEmitter::GetBillboard
  ====================*/
bool	CMeshEmitter::GetBillboard(uint uiIndex, SBillboard &outBillboard)
{
	uint uiParticle((m_uiFrontSlot + uiIndex) % m_vParticles.size());

	if (!m_vParticles[uiParticle].IsActive())
		return false;

	m_vParticles[uiParticle].GetBillboard(m_uiLastUpdateTime, outBillboard);

	float fStickiness(/*m_vParticles[uiParticle].GetStickiness(m_uiLastUpdateTime)*/0.0f);

	if (fStickiness == 1.0f)
		outBillboard.v3Pos = m_v3LastEmitterPos;
	else if (fStickiness != 0.0f)
		outBillboard.v3Pos = LERP(fStickiness, outBillboard.v3Pos, m_v3LastEmitterPos);

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
	case ENTITY_SPACE:
	case BONE_SPACE:
		{
			const CVec3f	&v3Pos(m_pParticleSystem->GetSourcePosition());
			const CAxis		&aAxis(m_pParticleSystem->GetSourceAxis());
			float			fScale(m_pParticleSystem->GetSourceScale());

			outBillboard.height *= fScale;
			outBillboard.width *= fScale;
			outBillboard.v3Pos = TransformPoint(outBillboard.v3Pos, aAxis, v3Pos, fScale);

			if (outBillboard.uiFlags & (BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT | BBOARD_GENERATE_AXIS))
			{
				if (m_eDirectionalSpace == DIRSPACE_LOCAL)
					outBillboard.aAxis = aAxis * m_aLastAxis;
				else
					outBillboard.aAxis = AXIS_IDENTITY;
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

