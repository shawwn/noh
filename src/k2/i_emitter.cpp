// (C)2006 S2 Games
// i_emitter.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_emitter.h"
#include "c_simpleparticle.h"
#include "c_particlesystem.h"
#include "c_modelemitter.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================


/*====================
  IEmitterDef::~IEmitterDef
  ====================*/
IEmitterDef::~IEmitterDef()
{
	for (vector<CSimpleParticleDef *>::iterator it(m_vParticleDefinitions.begin()); it != m_vParticleDefinitions.end(); ++it)
		K2_DELETE(*it);
}


/*====================
  IEmitterDef::IEmitterDef
  ====================*/
IEmitterDef::IEmitterDef()
{
}


/*====================
  IEmitterDef::AddParticleDef
  ====================*/
void	IEmitterDef::AddParticleDef(CSimpleParticleDef *pParticle)
{
	m_vParticleDefinitions.push_back(pParticle);
}


/*====================
  IEmitter::~IEmitter
  ====================*/
IEmitter::~IEmitter()
{
	SAFE_DELETE(m_pNextEmitter);
}


/*====================
  IEmitter::IEmitter
  ====================*/
IEmitter::IEmitter() :
m_pvParticleDefinitions(NULL),
m_uiPauseBegin(0)
{
}


/*====================
  IEmitter::IEmitter
  ====================*/
IEmitter::IEmitter
(
	int iLife,
	int iExpireLife,
	int iTimeNudge,
	int iDelay,
	bool bLoop,
	const tstring &sName,
	const tstring &sOwner,
	const tstring &sBone,
	const CVec3f &v3Pos,
	const CVec3f &v3Offset,
	EDirectionalSpace eDirectionalSpace,
	const vector<CSimpleParticleDef *> *pvParticleDefinitions,
	CParticleSystem *pParticleSystem,
	IEmitter *pOwner,
	uint uiStartTime
) :
m_iLife(iLife),
m_iExpireLife(iExpireLife),
m_iTimeNudge(iTimeNudge),
m_iDelay(iDelay),
m_bLoop(bLoop),
m_sName(sName),
m_sBone(sBone),
m_v3Pos(v3Pos),
m_v3Offset(v3Offset),
m_bActive(false),
m_v3LastPos(V3_ZERO),
m_aLastAxis(0.0f, 0.0f, 0.0f),
m_fLastScale(1.0f),
m_eDirectionalSpace(eDirectionalSpace),
m_pvParticleDefinitions(pvParticleDefinitions),
m_pParticleSystem(pParticleSystem),
m_uiStartTime(uiStartTime),
m_uiExpireTime(INVALID_TIME),
m_uiLastUpdateTime(uiStartTime),
m_uiPauseBegin(0),
m_pNextEmitter(NULL),
m_v3CustomPos(pParticleSystem->GetCustomPos()),
m_aCustomAxis(pParticleSystem->GetCustomAxis()),
m_fCustomScale(pParticleSystem->GetCustomScale()),
m_bCustomVisibility(pParticleSystem->GetCustomVisibility())
{
	PROFILE("IEmitter::IEmitter");

	if (pOwner != NULL)
		m_pOwner = pOwner;
	else
		m_pOwner = GetOwnerPointer(sOwner);
}


/*====================
  IEmitter::GetOwnerPointer
  ====================*/
IEmitter*	IEmitter::GetOwnerPointer(const tstring &sOwner)
{
	if (sOwner == _T("source"))
		return OWNER_SOURCE;
	else if (sOwner == _T("target"))
		return OWNER_TARGET;
	else
		return m_pParticleSystem->GetEmitter(sOwner);
}


/*====================
  IEmitter::GetVisibility
  ====================*/
bool	IEmitter::GetVisibility()
{
	if (m_pOwner == OWNER_SOURCE)
	{
		if (!m_sBone.empty())
			return m_pParticleSystem->GetSourceVisibility() && m_pParticleSystem->GetSourceVisibility(m_sBone);
		else
			return m_pParticleSystem->GetSourceVisibility();
	}
	else if (m_pOwner == OWNER_TARGET)
	{
		if (!m_sBone.empty())
			return m_pParticleSystem->GetTargetVisibility() && m_pParticleSystem->GetTargetVisibility(m_sBone);
		else
			return m_pParticleSystem->GetTargetVisibility();
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_bCustomVisibility;
	}
	else
	{
		IEmitter *pEmitter(m_pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return false;

		if (!pEmitter->GetVisibility())
			return false;

		CSkeleton *pSkeleton(pEmitter->GetCustomSkeleton());

		if (pSkeleton != NULL && !m_sBone.empty())
			return m_pParticleSystem->GetCustomVisibility(pSkeleton, m_sBone);

		return true;
	}
}


/*====================
  IEmitter::GetVisibility
  ====================*/
bool	IEmitter::GetVisibility(const tstring &sBone)
{
	if (m_pOwner == OWNER_SOURCE)
	{
		if (!sBone.empty())
			return m_pParticleSystem->GetSourceVisibility() && m_pParticleSystem->GetSourceVisibility(sBone);
		else
			return m_pParticleSystem->GetSourceVisibility();
	}
	else if (m_pOwner == OWNER_TARGET)
	{
		if (!sBone.empty())
			return m_pParticleSystem->GetTargetVisibility() && m_pParticleSystem->GetTargetVisibility(sBone);
		else
			return m_pParticleSystem->GetTargetVisibility();
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_bCustomVisibility;
	}
	else
	{
		IEmitter *pEmitter(m_pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return false;

		if (!pEmitter->GetVisibility())
			return false;

		CSkeleton *pSkeleton(pEmitter->GetCustomSkeleton());

		if (pSkeleton != NULL && !sBone.empty())
			return m_pParticleSystem->GetCustomVisibility(pSkeleton, sBone);

		return true;
	}
}


/*====================
  IEmitter::GetVisibility
  ====================*/
bool	IEmitter::GetVisibility(const tstring &sBone, IEmitter *pOwner)
{
	if (pOwner == OWNER_SOURCE)
	{
		if (!sBone.empty())
			return m_pParticleSystem->GetSourceVisibility() && m_pParticleSystem->GetSourceVisibility(sBone);
		else
			return m_pParticleSystem->GetSourceVisibility();
	}
	else if (pOwner == OWNER_TARGET)
	{
		if (!sBone.empty())
			return m_pParticleSystem->GetTargetVisibility() && m_pParticleSystem->GetTargetVisibility(sBone);
		else
			return m_pParticleSystem->GetTargetVisibility();
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_bCustomVisibility;
	}
	else
	{
		IEmitter *pEmitter(pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return false;

		if (!pEmitter->GetVisibility())
			return false;

		CSkeleton *pSkeleton(pEmitter->GetCustomSkeleton());

		if (pSkeleton != NULL && !sBone.empty())
			return m_pParticleSystem->GetCustomVisibility(pSkeleton, sBone);

		return true;
	}
}


/*====================
  IEmitter::GetPosition
  ====================*/
CVec3f	IEmitter::GetPosition()
{
	if (m_pOwner == OWNER_SOURCE)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return TransformPoint(m_v3Pos,
				m_pParticleSystem->GetSourceAxis(),
				m_pParticleSystem->GetSourcePosition(),
				m_pParticleSystem->GetSourceScale() * m_pParticleSystem->GetScale()) + m_v3Offset;
		else
			return m_v3Pos + m_v3Offset;
	}
	else if (m_pOwner == OWNER_TARGET)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return TransformPoint(m_v3Pos,
				m_pParticleSystem->GetTargetAxis(),
				m_pParticleSystem->GetTargetPosition(),
				m_pParticleSystem->GetTargetScale() * m_pParticleSystem->GetScale()) + m_v3Offset;
		else
			return m_v3Pos + m_v3Offset;
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_v3CustomPos + TransformPoint(m_v3Pos, GetAxis(), V3_ZERO, GetScale()) + m_v3Offset;
		else
			return m_v3CustomPos + m_v3Pos + m_v3Offset;
	}
	else
	{
		IEmitter *pEmitter(m_pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return TransformPoint(m_v3Pos, GetAxis(), V3_ZERO, GetScale()) + m_v3Offset;

		return TransformPoint(m_v3Pos, pEmitter->GetLastAxis(), pEmitter->GetLastPos(), pEmitter->GetLastScale()) + m_v3Offset;
	}
}


/*====================
  IEmitter::GetAxis
  ====================*/
CAxis	IEmitter::GetAxis()
{
	if (m_pOwner == OWNER_SOURCE)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetSourceAxis();
		else
			return CAxis(0.0f, 0.0f, 0.0f);

	}
	else if (m_pOwner == OWNER_TARGET)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetTargetAxis();
		else
			return CAxis(0.0f, 0.0f, 0.0f);
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_aCustomAxis;
	}
	else
	{
		IEmitter *pEmitter(m_pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return CAxis(0.0f, 0.0f, 0.0f);

		return pEmitter->GetLastAxis();
	}
}


/*====================
  IEmitter::GetScale
  ====================*/
float	IEmitter::GetScale()
{
	if (m_pOwner == OWNER_SOURCE)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetSourceScale() * m_pParticleSystem->GetScale();
		else
			return m_pParticleSystem->GetScale();
	}
	else if (m_pOwner == OWNER_TARGET)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetTargetScale() * m_pParticleSystem->GetScale();
		else
			return m_pParticleSystem->GetScale();
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_fCustomScale;
	}
	else
	{
		IEmitter *pEmitter(m_pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return m_pParticleSystem->GetScale();

		return pEmitter->GetLastScale();
	}
}


/*====================
  IEmitter::GetPosition
  ====================*/
CVec3f	IEmitter::GetPosition(const CVec3f &v3Pos, IEmitter *pOwner)
{
	if (pOwner == OWNER_SOURCE)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return TransformPoint(v3Pos,
				m_pParticleSystem->GetSourceAxis(),
				m_pParticleSystem->GetSourcePosition(),
				m_pParticleSystem->GetSourceScale() * m_pParticleSystem->GetScale());
		else
			return v3Pos;
	}
	else if (pOwner == OWNER_TARGET)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return TransformPoint(v3Pos,
				m_pParticleSystem->GetTargetAxis(),
				m_pParticleSystem->GetTargetPosition(),
				m_pParticleSystem->GetTargetScale() * m_pParticleSystem->GetScale());
		else
			return v3Pos;
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_v3CustomPos;
	}
	else
	{
		IEmitter *pEmitter(pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return TransformPoint(v3Pos, GetAxis(), V3_ZERO, GetScale());

		return TransformPoint(v3Pos, pEmitter->GetLastAxis(), pEmitter->GetLastPos(), pEmitter->GetLastScale());
	}
}


/*====================
  IEmitter::GetAxis
  ====================*/
CAxis	IEmitter::GetAxis(IEmitter *pOwner)
{
	if (pOwner == OWNER_SOURCE)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetSourceAxis();
		else
			return CAxis(0.0f, 0.0f, 0.0f);

	}
	else if (pOwner == OWNER_TARGET)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetTargetAxis();
		else
			return CAxis(0.0f, 0.0f, 0.0f);
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_aCustomAxis;
	}
	else
	{
		IEmitter *pEmitter(pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return CAxis(0.0f, 0.0f, 0.0f);

		return pEmitter->GetLastAxis();
	}
}


/*====================
  IEmitter::GetScale
  ====================*/
float	IEmitter::GetScale(IEmitter *pOwner)
{
	if (pOwner == OWNER_SOURCE)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetSourceScale() * m_pParticleSystem->GetScale();
		else
			return m_pParticleSystem->GetScale();
	}
	else if (pOwner == OWNER_TARGET)
	{
		if (m_pParticleSystem->GetSpace() == WORLD_SPACE)
			return m_pParticleSystem->GetTargetScale() * m_pParticleSystem->GetScale();
		else
			return m_pParticleSystem->GetScale();
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return m_fCustomScale;
	}
	else
	{
		IEmitter *pEmitter(pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return m_pParticleSystem->GetScale();

		return pEmitter->GetLastScale();
	}
}


/*====================
  IEmitter::GetBonePosition
  ====================*/
CVec3f	IEmitter::GetBonePosition(uint uiTime, IEmitter *pOwner, const tstring &sBone)
{
	if (sBone.empty())
		return V3_ZERO;

	if (pOwner == OWNER_SOURCE)
	{
		return m_pParticleSystem->GetSourceBonePosition(sBone, uiTime);
	}
	else if (pOwner == OWNER_TARGET)
	{
		return m_pParticleSystem->GetTargetBonePosition(sBone, uiTime);
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		return V3_ZERO;
	}
	else
	{
		IEmitter *pEmitter(pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
			return V3_ZERO;
		
		CSkeleton *pSkeleton(pEmitter->GetCustomSkeleton());

		if (pSkeleton != NULL)
			return m_pParticleSystem->GetCustomBonePosition(pSkeleton, sBone, uiTime);
		else
			return V3_ZERO;
	}
}


/*====================
  IEmitter::GetBoneAxisPos
  ====================*/
void	IEmitter::GetBoneAxisPos(uint uiTime, IEmitter *pOwner, const tstring &sBone, CAxis &aOutAxis, CVec3f &v3OutPos)
{
	if (sBone.empty())
	{
		aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
		v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
		return;
	}

	if (pOwner == OWNER_SOURCE)
	{
		m_pParticleSystem->GetSourceBoneAxisPos(sBone, uiTime, aOutAxis, v3OutPos);
	}
	else if (pOwner == OWNER_TARGET)
	{
		m_pParticleSystem->GetTargetBoneAxisPos(sBone, uiTime, aOutAxis, v3OutPos);
	}
	else if (m_pOwner == OWNER_CUSTOM)
	{
		aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
		v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
		return;
	}
	else
	{
		IEmitter *pEmitter(pOwner);

		if (pEmitter == NULL || !pEmitter->IsActive())
		{
			aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
			v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
			return;
		}
		
		CSkeleton *pSkeleton(pEmitter->GetCustomSkeleton());

		if (pSkeleton != NULL)
		{
			m_pParticleSystem->GetCustomBoneAxisPos(pSkeleton, sBone, uiTime, aOutAxis, v3OutPos);
		}
		else
		{
			aOutAxis = CAxis(0.0f, 0.0f, 0.0f);
			v3OutPos = CVec3f(0.0f, 0.0f, 0.0f);
			return;
		}
	}
}


/*====================
  IEmitter::GetSkeleton
  ====================*/
CSkeleton*	IEmitter::GetSkeleton()
{
	if (m_pOwner == OWNER_SOURCE)
		return m_pParticleSystem->GetSourceSkeleton();
	else if (m_pOwner == OWNER_TARGET)
		return m_pParticleSystem->GetTargetSkeleton();
	else
		return NULL;
}


/*====================
  IEmitter::GetModel
  ====================*/
CModel*	IEmitter::GetModel()
{
	if (m_pOwner == OWNER_SOURCE)
		return m_pParticleSystem->GetSourceModel();
	else if (m_pOwner == OWNER_TARGET)
		return m_pParticleSystem->GetTargetModel();
	else
		return NULL;
}


/*====================
  IEmitter::UpdatePaused
  ====================*/
void	IEmitter::ResumeFromPause(uint uiMilliseconds)
{
	uint uiPauseDuration(uiMilliseconds - m_uiPauseBegin);
	m_uiStartTime += uiPauseDuration;
	m_uiLastUpdateTime = uiMilliseconds;
	m_uiPauseBegin = 0;
}


/*====================
  IEmitter::UpdatePaused
  ====================*/
void	IEmitter::UpdatePaused(uint uiMilliseconds)
{
	if (m_uiPauseBegin == 0)
		m_uiPauseBegin = uiMilliseconds;
}


/*====================
  IEmitter::OnDelete
  ====================*/
void	IEmitter::OnDelete(IEmitter *pEmitter)
{
	if (m_pOwner == pEmitter)
		m_pOwner = NULL;
}


/*====================
  IEmitter::UpdateNextEmitter
  ====================*/
void	IEmitter::UpdateNextEmitter(uint uiMilliseconds, ParticleTraceFn_t pfnTrace)
{
	if (m_pNextEmitter == NULL)
		return;

	m_pNextEmitter->SetCustomPos(m_v3CustomPos);
	m_pNextEmitter->SetCustomAxis(m_aCustomAxis);
	m_pNextEmitter->SetCustomScale(m_fCustomScale);
	m_pNextEmitter->SetCustomVisibility(m_bCustomVisibility);

	if (!m_pNextEmitter->Update(uiMilliseconds, pfnTrace))
	{
		IEmitter *pNextEmitter(m_pNextEmitter->GetNextEmitter());
		m_pNextEmitter->SetNextEmitter(NULL);
		K2_DELETE(m_pNextEmitter);
		m_pNextEmitter = pNextEmitter;
	}
}
