// (C)2008 S2 Games
// c_controlleremitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_controlleremitter.h"

#include "c_particlesystem.h"
#include "c_scenelight.h"
#include "c_sceneentity.h"
#include "c_skeleton.h"
#include "c_model.h"
#include "c_effectthread.h"
#include "c_effect.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================

/*====================
  CControllerEmitterDef::~CControllerEmitterDef
  ====================*/
CControllerEmitterDef::~CControllerEmitterDef()
{
}


/*====================
  CControllerEmitterDef::CControllerEmitterDef
  ====================*/
CControllerEmitterDef::CControllerEmitterDef
(
	const tstring &sName,
	const tstring &sOwner,
	const CRangei &riLife,
	const CRangei &riExpireLife,
	const CRangei &riTimeNudge,
	const CRangei &riDelay,
	bool bLoop,
	EDirectionalSpace eDirectionalSpace,
	const tstring &sBone,
	const CVec3f &v3Pos,
	const CVec3f &v3Offset,
	const CTemporalPropertyRangef &trfPitch,
	const CTemporalPropertyRangef &trfRoll,
	const CTemporalPropertyRangef &trfYaw,
	const CTemporalPropertyRangef &trfScale,
	const tsvector &vEmitters,
	bool bLookAt,
	const CVec3f &v3LookAtPos,
	const CVec3f &v3LookAtOffset,
	const tstring &sLookAtOwner,
	const tstring &sLookAtBone,
	EDirectionalSpace eLookAtDirectionalSpace
) :
m_sName(sName),
m_sOwner(sOwner),
m_riLife(riLife),
m_riExpireLife(riExpireLife),
m_riTimeNudge(riTimeNudge),
m_riDelay(riDelay),
m_bLoop(bLoop),
m_eDirectionalSpace(eDirectionalSpace),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_trfPitch(trfPitch),
m_trfRoll(trfRoll),
m_trfYaw(trfYaw),
m_trfScale(trfScale),
m_vEmitters(vEmitters),
m_bLookAt(bLookAt),
m_v3LookAtPos(v3LookAtPos),
m_v3LookAtOffset(v3LookAtOffset),
m_sLookAtOwner(sLookAtOwner),
m_sLookAtBone(sLookAtBone),
m_eLookAtDirectionalSpace(eLookAtDirectionalSpace)
{
}


/*====================
  CControllerEmitterDef::Spawn
  ====================*/
IEmitter*	CControllerEmitterDef::Spawn(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner)
{
	PROFILE("CControllerEmitterdef::Spawn");

	return K2_NEW(ctx_Effects, CControllerEmitter)(uiStartTime, pParticleSystem, pOwner, *this);
}


/*====================
  CControllerEmitterDef::AddEmitterDef
  ====================*/
void	CControllerEmitterDef::AddEmitterDef(IEmitterDef *pEmitterDef)
{
	m_vEmitterDefs.push_back(pEmitterDef);
}


/*====================
  CControllerEmitter::~CControllerEmitter
  ====================*/
CControllerEmitter::~CControllerEmitter()
{
	SAFE_DELETE(m_pImbeddedEmitter);
}


/*====================
  CControllerEmitter::CControllerEmitter
  ====================*/
CControllerEmitter::CControllerEmitter(uint uiStartTime, CParticleSystem *pParticleSystem, IEmitter *pOwner, const CControllerEmitterDef &eSettings) :
IEmitter
(
	eSettings.GetLife(),
	eSettings.GetExpireLife(),
	eSettings.GetTimeNudge(),
	eSettings.GetDelay(),
	eSettings.GetLoop(),
	eSettings.GetName(),
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
m_tfPitch(eSettings.GetPitch()),
m_tfRoll(eSettings.GetRoll()),
m_tfYaw(eSettings.GetYaw()),
m_tfScale(eSettings.GetScale()),
m_bLookAt(eSettings.GetLookAt()),
m_v3LookAtPos(eSettings.GetLookAtPos()),
m_v3LookAtOffset(eSettings.GetLookAtOffset()),
m_pLookAtOwner(GetOwnerPointer(eSettings.GetLookAtOwner())),
m_sLookAtBone(eSettings.GetLookAtBone()),
m_eLookAtDirectionalSpace(eSettings.GetLookAtDirectionalSpace()),
m_pImbeddedEmitter(NULL)
{
	m_uiLastUpdateTime -= m_iTimeNudge;

	m_uiStartTime += m_iDelay;
	m_uiLastUpdateTime += m_iDelay;

	Update(INVALID_TIME, NULL);

	IEmitter *pCurrentEmitter(NULL);
	const tsvector &vEmitters(eSettings.GetEmitters());
	if (!vEmitters.empty())
	{
		CEffect *pEffect(m_pParticleSystem->GetEffect());
		
		tsvector_cit cit(vEmitters.begin());

		IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));

		if (pEmitterDef != NULL)
			m_pImbeddedEmitter = pEmitterDef->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, this);

		if (m_pImbeddedEmitter != NULL)
		{
			++cit;

			pCurrentEmitter = m_pImbeddedEmitter;

			for (; cit != vEmitters.end(); ++cit)
			{
				IEmitterDef *pEmitterDef(pEffect->GetEmitterDef(*cit));
				IEmitter *pNewEmitter(NULL);

				if (pEmitterDef != NULL)
					pNewEmitter = pEmitterDef->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, this);

				pCurrentEmitter->SetNextEmitter(pNewEmitter);
				pCurrentEmitter = pNewEmitter;
			}
		}
	}

	vector<IEmitterDef *>::const_iterator itEnd(eSettings.GetEmitterDefs().end());
	for (vector<IEmitterDef *>::const_iterator it(eSettings.GetEmitterDefs().begin()); it != itEnd; ++it)
	{
		for (int i(0); i < (*it)->GetCount(); ++i)
		{
			IEmitter *pNewEmitter((*it)->Spawn(uiStartTime + m_iDelay, m_pParticleSystem, this));
			if (pNewEmitter != NULL)
			{
				if (pCurrentEmitter == NULL)
				{
					m_pImbeddedEmitter = pNewEmitter;
					pCurrentEmitter = m_pImbeddedEmitter;
				}
				else
				{
					pCurrentEmitter->SetNextEmitter(pNewEmitter);
					pCurrentEmitter = pNewEmitter;
				}
			}
		}
	}

	m_bActive = false;
}


/*====================
  CControllerEmitter::UpdateEmbeddedEmitter
  ====================*/
inline
bool	CControllerEmitter::UpdateEmbeddedEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace, bool bExpire)
{
	if (bExpire)
		m_pImbeddedEmitter->Expire(uiMilliseconds);

	bool bRet(m_pImbeddedEmitter->Update(uiMilliseconds, pfnTrace));

	return bRet;
}


/*====================
  CControllerEmitter::Update
  ====================*/
bool	CControllerEmitter::Update(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
	PROFILE("CControllerEmitter::Update");

	if (uiMilliseconds == INVALID_TIME)
	{
		uiMilliseconds = m_uiStartTime;
	}
	else
	{
		if (m_uiPauseBegin)
			ResumeFromPause(uiMilliseconds);

		int iDeltaTime(uiMilliseconds - m_uiLastUpdateTime);

		if (iDeltaTime <= 0)
		{
			UpdateNextEmitter(uiMilliseconds, pfnTrace);
			return true;
		}

		m_uiLastUpdateTime = uiMilliseconds;
	}

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

	m_bActive = GetVisibility();

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

	CVec3f v3Pos(GetPosition());
	CAxis aAxis(GetAxis());
	float fScale(GetScale());

	if (m_bLookAt)
	{
		CVec3f v3LookAtPos(GetPosition(m_v3LookAtPos, m_pLookAtOwner));
		CAxis aLookAtAxis(GetAxis(m_pLookAtOwner));
		float fLookAtScale(GetScale(m_pLookAtOwner));

		CAxis	aLookAtBoneAxis(0.0f, 0.0f, 0.0f);
		CVec3f	v3LookAtBonePos;

		if (m_eLookAtDirectionalSpace == DIRSPACE_LOCAL)
		{
			GetBoneAxisPos(uiMilliseconds, m_pLookAtOwner, m_sLookAtBone, aLookAtBoneAxis, v3LookAtBonePos);
			v3LookAtPos = TransformPoint(v3LookAtBonePos, aLookAtAxis, v3LookAtPos, fLookAtScale);
		}
		else
		{
			v3LookAtBonePos = GetBonePosition(uiMilliseconds, m_pLookAtOwner, m_sLookAtBone);
			v3LookAtPos = TransformPoint(v3LookAtBonePos, aLookAtAxis, v3LookAtPos, fLookAtScale);
		}

		CAxis aLookAt;
		aLookAt.SetFromForwardVec(v3LookAtPos - v3Pos);

		m_aLastAxis = aLookAt * CAxis(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
	}
	else
	{
		if (m_eDirectionalSpace == DIRSPACE_LOCAL)
		{
			CAxis	aBoneAxis;
			CVec3f	v3BonePos;

			GetBoneAxisPos(uiMilliseconds, m_pOwner, m_sBone, aBoneAxis, v3BonePos);

			v3Pos = TransformPoint(v3BonePos, aAxis, v3Pos, fScale);
			aAxis = aAxis * aBoneAxis;
			m_aLastAxis = aAxis * CAxis(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
		}
		else
		{
			v3Pos = TransformPoint(GetBonePosition(uiMilliseconds, m_pOwner, m_sBone), aAxis, v3Pos, fScale);
			m_aLastAxis = CAxis(m_tfPitch.Evaluate(fLerp, fTime), m_tfRoll.Evaluate(fLerp, fTime), m_tfYaw.Evaluate(fLerp, fTime));
		}
	}

	m_v3LastPos = v3Pos;
	m_fLastScale = GetScale() * m_tfScale.Evaluate(fLerp, fTime);

	UpdateNextEmitter(uiMilliseconds, pfnTrace);

	if (m_pImbeddedEmitter != NULL)
	{
		if (!UpdateEmbeddedEmitter(uiMilliseconds, pfnTrace, !m_bActive))
		{
			IEmitter *pEmitter(m_pImbeddedEmitter);

			if (pEmitter->GetNextEmitter() != NULL)
			{
				m_pImbeddedEmitter = pEmitter->GetNextEmitter();
				pEmitter->SetNextEmitter(NULL);
			}
			else
			{
				m_pImbeddedEmitter = NULL;
			}

			K2_DELETE(pEmitter);
		}
	}

	m_bbBounds.Clear();
	m_bbBounds.AddPoint(m_v3LastPos);

	return true;
}


/*====================
  CControllerEmitter::GetNumEmitters
  ====================*/
uint	CControllerEmitter::GetNumEmitters()
{
	return m_pImbeddedEmitter != NULL ? 1 : 0;
}


/*====================
  CControllerEmitter::GetEmitter
  ====================*/
IEmitter*	CControllerEmitter::GetEmitter(uint uiIndex)
{
	return uiIndex == 0 ? m_pImbeddedEmitter : NULL;
}
