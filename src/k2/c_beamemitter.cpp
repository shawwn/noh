// (C)2006 S2 Games
// c_beamemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_beamemitter.h"
#include "c_particlesystem.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CBeamEmitterDef::~CBeamEmitterDef
  ====================*/
CBeamEmitterDef::~CBeamEmitterDef()
{
}


/*====================
  CBeamEmitterDef::CBeamEmitterDef
  ====================*/
CBeamEmitterDef::CBeamEmitterDef
(
	const CRangei &riLife,
	const CRangei &riExpireLife,
	const CRangei &riTimeNudge,
	const CRangei &riDelay,
	bool bLoop,
	const tstring &sOwnerA,
	const tstring &sOwnerB,
	const tstring &sBoneA,
	const tstring &sBoneB,
	const CVec3f &v3PosA,
	const CVec3f &v3PosB,
	const CTemporalPropertyv3 &tv3Color,
	const CTemporalPropertyf &tfAlpha,
	const CTemporalPropertyRangef &trfSize,
	const CTemporalPropertyRangef &trfTaper,
	const CTemporalPropertyRangef &trfTile,
	const CTemporalPropertyRangef &trfFrame,
	const CTemporalPropertyRangef &trfParam,
	ResHandle hMaterial
) :
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_sOwnerA(sOwnerA),
m_sOwnerB(sOwnerB),
m_sBoneA(sBoneA),
m_sBoneB(sBoneB),
m_v3PosA(v3PosA),
m_v3PosB(v3PosB),
m_tv3Color(tv3Color),
m_tfAlpha(tfAlpha),
m_trfSize(trfSize),
m_trfTaper(trfTaper),
m_trfTile(trfTile),
m_trfFrame(trfFrame),
m_trfParam(trfParam),
m_hMaterial(hMaterial)
{
}


/*====================
  CBeamEmitterDef::Spawn
  ====================*/
IEmitter*	CBeamEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
	PROFILE("CBeamEmitterDef::Spawn");

	return K2_NEW(ctx_Effects, CBeamEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CBeamEmitter::~CBeamEmitter
  ====================*/
CBeamEmitter::~CBeamEmitter()
{
}


/*====================
  CBeamEmitter::CBeamEmitter
  ====================*/
CBeamEmitter::CBeamEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CBeamEmitterDef &eSettings) :
IEmitter
(
	eSettings.GetLife(),
	eSettings.GetExpireLife(),
	eSettings.GetTimeNudge(),
	eSettings.GetDelay(),
	eSettings.GetLoop(),
	TSNULL,
	eSettings.GetOwnerA(),
	TSNULL,
	V3_ZERO,
	V3_ZERO,
	DIRSPACE_LOCAL,
	&eSettings.GetParticleDefinitions(),
	pParticleSystem,
	pOwner,
	uiStartTime
),
m_sBoneA(eSettings.GetBoneA()),
m_sBoneB(eSettings.GetBoneB()),
m_v3PosA(eSettings.GetPosA()),
m_v3PosB(eSettings.GetPosB()),
m_tv3Color(eSettings.GetColor()),
m_tfAlpha(eSettings.GetAlpha()),
m_tfSize(eSettings.GetSize()),
m_tfTaper(eSettings.GetTaper()),
m_tfTile(eSettings.GetTile()),
m_tfFrame(eSettings.GetFrame()),
m_tfParam(eSettings.GetParam()),
m_hMaterial(eSettings.GetMaterial())
{
	if (pOwner != NULL)
	{
		m_pOwnerA = pOwner;
		m_pOwnerB = pOwner;
	}
	else
	{
		m_pOwnerA = GetOwnerPointer(eSettings.GetOwnerA());
		m_pOwnerB = GetOwnerPointer(eSettings.GetOwnerB());
	}

	// Initialize m_v3LastPos
	m_v3LastPosA = TransformPoint(m_v3PosA + GetBonePosition(uiStartTime, m_pOwnerA, m_sBoneA), GetAxis(m_pOwnerA), GetPosition(m_v3PosA, m_pOwnerA));
	m_v3LastPosB = TransformPoint(m_v3PosB + GetBonePosition(uiStartTime, m_pOwnerB, m_sBoneB), GetAxis(m_pOwnerB), GetPosition(m_v3PosA, m_pOwnerB));

	m_uiLastUpdateTime -= m_iTimeNudge;

	m_uiStartTime += m_iDelay;
	m_uiLastUpdateTime += m_iDelay;
}


/*====================
  CBeamEmitter::Update
  ====================*/
bool	CBeamEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
	PROFILE("CBeamEmitter::Update");

	if (m_uiPauseBegin)
		ResumeFromPause(uiMilliseconds);

    int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

	if (iDeltaTime <= 0)
	{
		UpdateNextEmitter(uiMilliseconds, pfnTrace);
		return true;
	}

	m_uiLastUpdateTime = uiMilliseconds;

	m_bActive = m_pParticleSystem->GetActive() && m_uiExpireTime == INVALID_TIME;

	if (!GetVisibility(m_sBoneA, m_pOwnerA) || !GetVisibility(m_sBoneB, m_pOwnerB))
		m_bActive = false;

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

	float fScale(GetScale());

	m_v3LastPosA = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwnerA, m_sBoneA), GetAxis(m_pOwnerA), GetPosition(m_v3PosA, m_pOwnerA), GetScale(m_pOwnerA));
	m_v3LastPosB = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwnerB, m_sBoneB), GetAxis(m_pOwnerB), GetPosition(m_v3PosB, m_pOwnerB), GetScale(m_pOwnerB));

	m_fLastScale = fScale;

	UpdateNextEmitter(uiMilliseconds, pfnTrace);

	if (m_uiExpireTime != INVALID_TIME && m_uiExpireTime <= m_uiLastUpdateTime && (m_iLife == -1 || m_bLoop))
		return false;

	m_bbBounds.Clear();
	m_bbBounds.AddPoint(m_v3LastPosA);
	m_bbBounds.AddPoint(m_v3LastPosB);

	return true;
}


/*====================
  CBeamEmitter::GetNumBeams
  ====================*/
uint	CBeamEmitter::GetNumBeams()
{
	if (m_bActive)
		return 1;
	else
		return 0;
}


/*====================
  CBeamEmitter::GetBeam
  ====================*/
bool	CBeamEmitter::GetBeam(uint uiIndex, SBeam &outBeam)
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

	outBeam.v3Start = m_v3LastPosA;
	outBeam.v3End = m_v3LastPosB;
	outBeam.v4StartColor = outBeam.v4EndColor = CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime));
	outBeam.fStartSize = outBeam.fEndSize = m_tfSize.Evaluate(fLerp, fTime) * m_fLastScale;
	outBeam.fTaper = m_tfTaper.Evaluate(fLerp, fTime);
	outBeam.fTile = m_tfTile.Evaluate(fLerp, fTime);
	outBeam.fStartFrame = outBeam.fEndFrame = m_tfFrame.Evaluate(fLerp, fTime);
	outBeam.fStartParam = outBeam.fEndParam = m_tfParam.Evaluate(fLerp, fTime);

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

			outBeam.v3Start = TransformPoint(outBeam.v3Start, aAxis, v3Pos, fScale);
			outBeam.v3End = TransformPoint(outBeam.v3End, aAxis, v3Pos, fScale);
			outBeam.fStartSize *= fScale;
			outBeam.fEndSize *= fScale;

		} break;
	}

	outBeam.hMaterial = m_hMaterial;

	return true;
}


/*====================
  CBeamEmitter::OnDelete
  ====================*/
void	CBeamEmitter::OnDelete(IEmitter *pEmitter)
{
	if (m_pOwnerA == pEmitter)
		m_pOwnerA = NULL;

	if (m_pOwnerB == pEmitter)
		m_pOwnerB = NULL;
}
