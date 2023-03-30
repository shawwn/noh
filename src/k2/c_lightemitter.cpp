// (C)2006 S2 Games
// c_lightemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_lightemitter.h"
#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CLightEmitterDef::~CLightEmitterDef
  ====================*/
CLightEmitterDef::~CLightEmitterDef()
{
}


/*====================
  CLightEmitterDef::CLightEmitterDef
  ====================*/
CLightEmitterDef::CLightEmitterDef
(
	const tstring &sOwner,
	const CRangei &riLife,
	const CRangei &riExpireLife,
	const CRangei &riTimeNudge,
	const CRangei &riDelay,
	bool bLoop,
	const tstring &sBone,
	const CVec3f &v3Pos,
	const CVec3f &v3Offset,
	const CTemporalPropertyv3 &tv3Color,
	const CTemporalPropertyRangef &trfFalloffStart,
	const CTemporalPropertyRangef &trfFalloffEnd,
	const CTemporalPropertyRangef &trfFlickerAmount,
	const CTemporalPropertyRangef &trfFlickerFrequency
) :
m_sOwner(sOwner),
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_tv3Color(tv3Color),
m_trfFalloffStart(trfFalloffStart),
m_trfFalloffEnd(trfFalloffEnd),
m_trfFlickerAmount(trfFlickerAmount),
m_trfFlickerFrequency(trfFlickerFrequency)
{
}


/*====================
  CLightEmitterDef::Spawn
  ====================*/
IEmitter*	CLightEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
	PROFILE("CLightEmitterdef::Spawn");

	return K2_NEW(ctx_Effects, CLightEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CLightEmitter::~CLightEmitter
  ====================*/
CLightEmitter::~CLightEmitter()
{
}


/*====================
  CLightEmitter::CLightEmitter
  ====================*/
CLightEmitter::CLightEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CLightEmitterDef &eSettings) :
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
m_tv3Color(eSettings.GetColor()),
m_tfFalloffStart(eSettings.GetFalloffStart()),
m_tfFalloffEnd(eSettings.GetFalloffEnd()),
m_tfFlickerAmount(eSettings.GetFlickerAmount()),
m_tfFlickerFrequency(eSettings.GetFlickerFrequency())
{
	m_v3LastPos = GetPosition();
	m_fLastScale = GetScale();

	m_uiLastUpdateTime -= m_iTimeNudge;

	m_uiStartTime += m_iDelay;
	m_uiLastUpdateTime += m_iDelay;
}


/*====================
  CLightEmitter::Update
  ====================*/
bool	CLightEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
	PROFILE("CLightEmitter::Update");

	if (m_uiPauseBegin)
		ResumeFromPause(uiMilliseconds);

	int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

	if (iDeltaTime <= 0)
	{
		UpdateNextEmitter(uiMilliseconds, pfnTrace);
		return true;
	}
	
	// Kill us if we've lived out our entire life
	if (m_iLife != -1 && (uiMilliseconds > m_iLife + m_uiStartTime))
	{
		if (m_bLoop)
		{
			m_uiStartTime += m_iLife * ((uiMilliseconds - m_uiStartTime) / m_iLife);
		}
		else
		{
			m_bActive = false;
			return false;
		}
	}

	m_bActive = GetVisibility();

	m_uiLastUpdateTime = uiMilliseconds;

	CVec3f v3Pos(GetPosition());
	CAxis aAxis(GetAxis());
	float fScale(GetScale());

	v3Pos = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone), aAxis, v3Pos, fScale);
	
	m_aLastAxis = aAxis;
	m_v3LastPos = v3Pos;
	m_fLastScale = fScale;

	UpdateNextEmitter(uiMilliseconds, pfnTrace);

	// Calculate temporal properties
	float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);
	float fLerp;

	if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= m_uiLastUpdateTime && (m_iLife == -1 || m_bLoop))
	{
		// Kill us if we've lived out our entire expire life
		if ((m_iExpireLife != -1 && (uiMilliseconds > m_iExpireLife + m_uiExpireTime)) || m_iExpireLife == 0)
		{
			m_bActive = false;
			return false;
		}
		
		if (m_iExpireLife != -1)
			fLerp = float(m_uiLastUpdateTime - m_uiExpireTime) / m_iExpireLife;
		else
			fLerp = 0.0f;
	}
	else
	{
		if (m_iLife != -1)
			fLerp = float(m_uiLastUpdateTime - m_uiStartTime) / m_iLife;
		else
			fLerp = 0.0f;
	}

	m_bbBounds.Clear();

	float fSize(m_tfFalloffEnd.Evaluate(fLerp, fTime) * m_fLastScale * DIAG);

	m_bbBounds.AddBox(CBBoxf(CVec3f(m_v3LastPos.x - fSize, m_v3LastPos.y - fSize, m_v3LastPos.z - fSize),
		CVec3f(m_v3LastPos.x + fSize, m_v3LastPos.y + fSize, m_v3LastPos.z + fSize)));

	return true;
}


/*====================
  CLightEmitter::GetNumLights
  ====================*/
uint	CLightEmitter::GetNumLights()
{
	if (m_bActive)
		return 1;
	else
		return 0;
}


/*====================
  CLightEmitter::GetLight
  ====================*/
bool	CLightEmitter::GetLight(uint uiIndex, CSceneLight &outLight)
{
	if (uiIndex != 0)
		return false;

	float fTime((m_uiLastUpdateTime - m_uiStartTime) * SEC_PER_MS);
	float fLerp;

	if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= m_uiLastUpdateTime && (m_iLife == -1 || m_bLoop))
	{
		if (m_iExpireLife != -1)
			fLerp = float(m_uiLastUpdateTime - m_uiExpireTime) / m_iExpireLife;
		else
			fLerp = 0.0f;
	}
	else
	{
		if (m_iLife != -1)
			fLerp = float(m_uiLastUpdateTime - m_uiStartTime) / m_iLife;
		else
			fLerp = 0.0f;
	}

	float fFlicker(1.0f);

	float fFlickerAmount(m_tfFlickerAmount.Evaluate(fLerp, fTime));
	if (fFlickerAmount != 0.0f)
		fFlicker += M_SmoothRand1(fTime * m_tfFlickerFrequency.Evaluate(fLerp, fTime)) * fFlickerAmount;

	outLight.SetPosition(m_v3LastPos);
	outLight.SetColor(m_tv3Color.Evaluate(fLerp, fTime));
	outLight.SetFalloffStart(m_tfFalloffStart.Evaluate(fLerp, fTime) * m_fLastScale * fFlicker);
	outLight.SetFalloffEnd(m_tfFalloffEnd.Evaluate(fLerp, fTime) * m_fLastScale * fFlicker);

	switch (m_pParticleSystem->GetSpace())
	{
	case WORLD_SPACE:
		{
		} break;
	case ENTITY_SPACE:
		{
			const CVec3f	&v3Pos(m_pParticleSystem->GetSourcePosition());
			const CAxis		&aAxis(m_pParticleSystem->GetSourceAxis());
			float			fScale(m_pParticleSystem->GetSourceScale());

			outLight.SetFalloffStart(outLight.GetFalloffStart() * fScale);
			outLight.SetFalloffEnd(outLight.GetFalloffEnd() * fScale);
			outLight.SetPosition(TransformPoint(outLight.GetPosition(), aAxis, v3Pos, fScale));

		} break;
	}

	return true;
}
