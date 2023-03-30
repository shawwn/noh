// (C)2006 S2 Games
// c_billboardemitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_billboardemitter.h"
#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CBillboardEmitterDef::~CBillboardEmitterDef
  ====================*/
CBillboardEmitterDef::~CBillboardEmitterDef()
{
}


/*====================
  CBillboardEmitterDef::CBillboardEmitterDef
  ====================*/
CBillboardEmitterDef::CBillboardEmitterDef
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
	const CTemporalPropertyf &tfAlpha,
	const CTemporalPropertyRangef &trfPitch,
	const CTemporalPropertyRangef &trfRoll,
	const CTemporalPropertyRangef &trfYaw,
	const CTemporalPropertyRangef &trfWidth,
	const CTemporalPropertyRangef &trfHeight,
	const CTemporalPropertyRangef &trfScale,
	const CTemporalPropertyRangef &trfFrame,
	const CTemporalPropertyRangef &trfParam,
	float fDepthBias,
	uint uiFlags,
	ResHandle hMaterial,
	EDirectionalSpace eDirectionalSpace,
	const CTemporalPropertyRangef &trfScaleU,
	const CTemporalPropertyRangef &trfScaleV,
	const CTemporalPropertyRangef &trfOffsetU,
	const CTemporalPropertyRangef &trfOffsetV
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
m_tfAlpha(tfAlpha),
m_trfPitch(trfPitch),
m_trfRoll(trfRoll),
m_trfYaw(trfYaw),
m_trfWidth(trfWidth),
m_trfHeight(trfHeight),
m_trfScale(trfScale),
m_trfFrame(trfFrame),
m_trfParam(trfParam),
m_fDepthBias(fDepthBias),
m_uiFlags(uiFlags),
m_hMaterial(hMaterial),
m_eDirectionalSpace(eDirectionalSpace),
m_trfScaleU(trfScaleU),
m_trfScaleV(trfScaleV),
m_trfOffsetU(trfOffsetU),
m_trfOffsetV(trfOffsetV)
{
}


/*====================
  CBillboardEmitterDef::Spawn
  ====================*/
IEmitter*	CBillboardEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
	PROFILE("CBillboardEmitterDef::Spawn");

	return K2_NEW(ctx_Effects, CBillboardEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CBillboardEmitter::~CBillboardEmitter
  ====================*/
CBillboardEmitter::~CBillboardEmitter()
{
}


/*====================
  CBillboardEmitter::CBillboardEmitter
  ====================*/
CBillboardEmitter::CBillboardEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CBillboardEmitterDef &eSettings) :
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
m_tv3Color(eSettings.GetColor(), pParticleSystem->GetColor()),
m_tfAlpha(eSettings.GetAlpha()),
m_tfPitch(eSettings.GetPitch()),
m_tfRoll(eSettings.GetRoll()),
m_tfYaw(eSettings.GetYaw()),
m_tfWidth(eSettings.GetWidth()),
m_tfHeight(eSettings.GetHeight()),
m_tfScale(eSettings.GetScale()),
m_tfFrame(eSettings.GetFrame()),
m_tfParam(eSettings.GetParam()),
m_fDepthBias(eSettings.GetDepthBias()),
m_uiFlags(eSettings.GetFlags()),
m_hMaterial(eSettings.GetMaterial()),
m_tfScaleU(eSettings.GetScaleU()),
m_tfScaleV(eSettings.GetScaleV()),
m_tfOffsetU(eSettings.GetOffsetU()),
m_tfOffsetV(eSettings.GetOffsetV())
{
	m_v3LastPos = GetPosition();
	m_fLastScale = GetScale();

	m_uiLastUpdateTime -= m_iTimeNudge;

	m_uiStartTime += m_iDelay;
	m_uiLastUpdateTime += m_iDelay;
}


/*====================
  CBillboardEmitter::Update
  ====================*/
bool	CBillboardEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
	PROFILE("CBillboardEmitter::Update");

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

	m_uiLastUpdateTime = uiMilliseconds;

	m_bActive = (m_sBone.empty() || GetVisibility());

	CVec3f v3Pos(GetPosition());
	CAxis aAxis(GetAxis());
	float fScale(GetScale());

	if (m_eDirectionalSpace == DIRSPACE_LOCAL)
	{
		CAxis	aBoneAxis;
		CVec3f	v3BonePos;

		GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, aBoneAxis, v3BonePos);

		v3Pos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);
		aAxis = aAxis * aBoneAxis;
	}
	else
	{
		v3Pos = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone), aAxis, v3Pos, fScale);
	}

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

	float fSize(MAX(m_tfWidth.Evaluate(fLerp, fTime), m_tfHeight.Evaluate(fLerp, fTime)) * m_tfScale.Evaluate(fLerp, fTime) * m_fLastScale * DIAG);

	m_bbBounds.AddBox(CBBoxf(CVec3f(m_v3LastPos.x - fSize, m_v3LastPos.y - fSize, m_v3LastPos.z - fSize),
		CVec3f(m_v3LastPos.x + fSize, m_v3LastPos.y + fSize, m_v3LastPos.z + fSize)));

	return true;
}


/*====================
  CBillboardEmitter::GetNumBillboards
  ====================*/
uint	CBillboardEmitter::GetNumBillboards()
{
	if (m_bActive)
		return 1;
	else
		return 0;
}


/*====================
  CBillboardEmitter::GetBillboard
  ====================*/
bool	CBillboardEmitter::GetBillboard(uint uiIndex, SBillboard &outBillboard)
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

	outBillboard.v3Pos = m_v3LastPos;
	outBillboard.color = CVec4f(m_tv3Color.Evaluate(fLerp, fTime), m_tfAlpha.Evaluate(fLerp, fTime)).GetAsDWord();
	outBillboard.angle = m_tfRoll.Evaluate(fLerp, fTime);
	outBillboard.fPitch = m_tfPitch.Evaluate(fLerp, fTime);
	outBillboard.fYaw = m_tfYaw.Evaluate(fLerp, fTime);

	float fScale(m_tfScale.Evaluate(fLerp, fTime) * m_fLastScale);
	outBillboard.width = m_tfWidth.Evaluate(fLerp, fTime) * fScale;
	outBillboard.height = m_tfHeight.Evaluate(fLerp, fTime) * fScale;
	
	outBillboard.frame = m_tfFrame.Evaluate(fLerp, fTime);
	outBillboard.param = m_tfParam.Evaluate(fLerp, fTime);

	float fScaleU(m_tfScaleU.Evaluate(fLerp, fTime));
	float fScaleV(m_tfScaleV.Evaluate(fLerp, fTime));
	float fOffsetU(m_tfOffsetU.Evaluate(fLerp, fTime));
	float fOffsetV(m_tfOffsetV.Evaluate(fLerp, fTime));

	if (fScaleU < 0.0f)
	{
		outBillboard.s1 = fOffsetU - fScaleU;
		outBillboard.s2 = fOffsetU;
	}
	else
	{
		outBillboard.s1 = fOffsetU;
		outBillboard.s2 = fOffsetU + fScaleU;
	}

	if (fScaleV < 0.0f)
	{
		outBillboard.t1 = fOffsetV - fScaleV;
		outBillboard.t2 = fOffsetV;
	}
	else
	{
		outBillboard.t1 = fOffsetV;
		outBillboard.t2 = fOffsetV + fScaleV;
	}
	
	switch (m_pParticleSystem->GetSpace())
	{
	case WORLD_SPACE:
		{
			if (m_uiFlags & (BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT | BBOARD_GENERATE_AXIS))
			{
				if (m_eDirectionalSpace == DIRSPACE_LOCAL)
					outBillboard.aAxis = m_aLastAxis;
				else 
					outBillboard.aAxis = AXIS_IDENTITY;
			}
		} break;
	case ENTITY_SPACE:
		{
			const CVec3f	&v3Pos(m_pParticleSystem->GetSourcePosition());
			const CAxis		&aAxis(m_pParticleSystem->GetSourceAxis());
			float			fScale(m_pParticleSystem->GetSourceScale());

			outBillboard.width *= fScale;
			outBillboard.height *= fScale;
			outBillboard.v3Pos = TransformPoint(outBillboard.v3Pos, aAxis, v3Pos, fScale);
			
			if (m_uiFlags & (BBOARD_LOCK_UP | BBOARD_LOCK_RIGHT | BBOARD_GENERATE_AXIS))
			{
				if (m_eDirectionalSpace == DIRSPACE_LOCAL)
					outBillboard.aAxis = aAxis * m_aLastAxis;
				else 
					outBillboard.aAxis = AXIS_IDENTITY;
			}
		} break;
	}

	outBillboard.fDepthBias = m_fDepthBias;
	outBillboard.hMaterial = m_hMaterial;
	outBillboard.uiFlags = m_uiFlags;

	return true;
}
