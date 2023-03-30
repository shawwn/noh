// (C)2005 S2 Games
// c_anim.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_anim.h"
#include "c_skeleton.h"
#include "c_k2model.h"
#include "c_clip.h"
#include "c_model.h"
#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CAnim::~CAnim
  ====================*/
CAnim::~CAnim()
{
	K2_DELETE_ARRAY(m_ppMotions);

	g_ResourceManager.RemoveResourceWatcher(this, m_hClip);
}


/*====================
  CAnim::CAnim
  ====================*/
CAnim::CAnim
(
	CK2Model *pModel,
	uint uiIndex,
	const tstring &sName,
	const tstring &sClip,
	int iStartFrame,
	int iNumFrames,
	int iLoopbackFrame,
	bool bLoop,
	float fFps,
	int iNumLoopFrames,
	int iBlendTime,
	bool bLock,
	int iMinStartFrame,
	int iMaxStartFrame,
	uint uiIgnoreFlags
) :
IResourceWatcher(),
m_uiIndex(uiIndex),
m_sName(sName),
m_ppMotions(NULL),
m_iStartFrame(iStartFrame),
m_iNumFrames(iNumFrames),
m_iLoopbackFrame(iLoopbackFrame),
m_bLoop(bLoop),
m_fFps(fFps),
m_iMSperFrame(MAX(INT_FLOOR(1000.0f / fFps), 1)),
m_iNumLoopFrames(iNumLoopFrames),
m_uiBlendTime(iBlendTime),
m_bLock(bLock),
m_iMinStartFrame(iMinStartFrame),
m_iMaxStartFrame(iMaxStartFrame),
m_pModel(pModel),
m_iStartFrameDef(iStartFrame),
m_iNumFramesDef(iNumFrames),
m_iLoopbackFrameDef(iLoopbackFrame),
m_iMinStartFrameDef(iMinStartFrame),
m_iMaxStartFrameDef(iMaxStartFrame)
{
	m_hClip = g_ResourceManager.Register(sClip, RES_CLIP, uiIgnoreFlags);

	g_ResourceManager.AddResourceWatcher(this, m_hClip);

	Rebuild(m_hClip);
}


/*====================
  CAnim::CheckEvents

  Check for events in the range [time0, time1)
  ====================*/
void	CAnim::CheckEvents(CSkeleton *pSkeleton, int iTime0, int iTime1, int iChannel)
{
	if (m_vFrameEvents.empty() || iTime0 >= iTime1)
		return;

	if (m_bLoop)
	{
		int iLoopTime(m_iNumFrames * m_iMSperFrame);
		int iLoopLength(m_iNumLoopFrames * m_iMSperFrame);

		int iStartLoop(MAX((iTime0 - iLoopTime < 0) ? 0 : (iTime0 - iLoopTime) / iLoopLength + 1, 0));
		int iEndLoop(MAX((iTime1 - iLoopTime < 0) ? 1 : (iTime1 - iLoopTime) / iLoopLength + 2, 0));

		if (iEndLoop - iStartLoop > 2)
		{
			iStartLoop = iEndLoop - 2;
			//Console.Warn << "Large number of loops in CAnim::CheckEvents" << newl;
		}

		for (int iLoop(iStartLoop); iLoop < iEndLoop; ++iLoop)
		{
			if (iLoop == 0)
			{
				for (vector<SAnimEvent>::iterator it(m_vFrameEvents.begin()); it != m_vFrameEvents.end(); ++it)
				{
					int	iFrameTime(it->iFrame * m_iMSperFrame);

					if (iFrameTime >= iTime0 && iFrameTime < iTime1)
						pSkeleton->AddEvent(it->sCommand, iFrameTime - iTime1);
				}
			}
			else
			{
				// Calculate time0 and time1 relative to the current loop
				int iLocalTime0((iTime0 - iLoopTime) - iLoopLength * (iLoop - 1));
				int iLocalTime1((iTime1 - iLoopTime) - iLoopLength * (iLoop - 1));

				for (vector<SAnimEvent>::iterator it(m_vFrameEvents.begin()); it != m_vFrameEvents.end(); ++it)
				{
					if (it->iFrame < (m_iLoopbackFrame - m_iStartFrame))
						continue;

					int	iFrameTime((it->iFrame - (m_iLoopbackFrame - m_iStartFrame)) * m_iMSperFrame);

					if (iFrameTime >= iLocalTime0 && iFrameTime < iLocalTime1)
						pSkeleton->AddEvent(it->sCommand, iFrameTime - iLocalTime1);
				}
			}
		}
	}
	else
	{
		for (vector<SAnimEvent>::iterator it(m_vFrameEvents.begin()); it != m_vFrameEvents.end(); ++it)
		{
			int	iFrameTime(it->iFrame * m_iMSperFrame);

			if (iFrameTime >= iTime0 && iFrameTime < iTime1)
				pSkeleton->AddEvent(it->sCommand, iFrameTime - iTime1);
		}
	}
}


/*====================
  CAnim::AddFrameEvent
  ====================*/
void	CAnim::AddFrameEvent(int iFrame, const tstring &sCommand)
{
	m_vFrameEvents.push_back(SAnimEvent(iFrame, sCommand));
}


/*====================
  CAnim::AddStartEvent
  ====================*/
void	CAnim::AddStartEvent(const tstring &sCommand)
{
	m_vStartEvents.push_back(sCommand);
}


/*====================
  CAnim::AddEndEvent
  ====================*/
void	CAnim::AddEndEvent(const tstring &sCommand)
{
	m_vEndEvents.push_back(sCommand);
}


/*====================
  CAnim::Rebuild
  ====================*/
void	CAnim::Rebuild(ResHandle hResource)
{
	PROFILE("CAnim::Rebuild");

	if (m_ppMotions)
	{
		K2_DELETE_ARRAY(m_ppMotions);
		m_ppMotions = NULL;
	}

	m_iStartFrame = m_iStartFrameDef;
	m_iNumFrames = m_iNumFramesDef;
	m_iLoopbackFrame = m_iLoopbackFrameDef;
	m_iMinStartFrame = m_iMinStartFrameDef;
	m_iMaxStartFrame = m_iMaxStartFrameDef;

	// Set up pointers to bone motions
	if (m_pModel->GetNumBones() > 0)
		m_ppMotions = K2_NEW_ARRAY(ctx_Models,  SBoneMotion*, m_pModel->GetNumBones());

	CClip *pClip(g_ResourceManager.GetClip(m_hClip));
	if (pClip == NULL)
	{
		for (uint n(0); n < m_pModel->GetNumBones(); ++n)
			m_ppMotions[n] = NULL;

		// Fix up the frames
		m_iStartFrame = 0;
		m_iNumFrames = 1;
		return;
	}

	for (uint n(0); n < m_pModel->GetNumBones(); ++n)
		m_ppMotions[n] = NULL;

	for (int i = 0; i < pClip->GetNumMotions(); ++i)
	{
		// Assign the bone to this motion
		uint uiBone(m_pModel->GetBoneIndex(pClip->GetBoneMotion(i)->sBoneName));

		if (uiBone != INVALID_BONE)
			m_ppMotions[uiBone] = pClip->GetBoneMotion(i);
	}

	// Fix up the frames
	if (m_iStartFrame == 0 && m_iNumFrames == 0)
		m_iNumFrames = pClip->GetNumFrames();
	else if (m_iNumFrames == -1)
		m_iNumFrames = pClip->GetNumFrames() - m_iStartFrame;
	else if (m_iStartFrame + m_iNumFrames > pClip->GetNumFrames())
		m_iNumFrames = pClip->GetNumFrames() - m_iStartFrame;
	
	if (m_iNumFrames <= 0)
		m_iNumFrames = 1;

	if (m_bLoop)
	{
		if (m_iLoopbackFrame == -1)
			m_iLoopbackFrame = m_iStartFrame + m_iNumFrames - 1;

		m_iLoopbackFrame = CLAMP(m_iLoopbackFrame, m_iStartFrame, m_iStartFrame + m_iNumFrames - 1);
		m_iNumLoopFrames = m_iStartFrame + m_iNumFrames - m_iLoopbackFrame;
	}

	if (m_iMinStartFrame == -1)
		m_iMinStartFrame = m_iStartFrame + m_iNumFrames - 1;

	m_iMinStartFrame = CLAMP(m_iMinStartFrame, m_iStartFrame, m_iStartFrame + m_iNumFrames - 1);

	if (m_iMaxStartFrame == -1)
		m_iMaxStartFrame = m_iStartFrame + m_iNumFrames - 1;

	m_iMaxStartFrame = CLAMP(m_iMaxStartFrame, m_iMinStartFrame, m_iStartFrame + m_iNumFrames - 1);
}


/*====================
  CAnim::ComputeAnimFrame

  computes loframe, hiframe, and lerp amounts based on a CAnim structure and a given time
  non looping animations will freeze on their last frame
  looping animations will loop back to the loopbackframe specified in the anim struct
  time is specified in milliseconds
  ====================*/
bool	CAnim::ComputeAnimFrame(int iTime, int &iLoFrame, int &iHiFrame, float &fLerpAmt, uint uiForceLength)
{
	int iMsPerFrame(m_iMSperFrame);
	if (uiForceLength != 0)
		iMsPerFrame = uiForceLength / m_iNumFrames;
	if (iMsPerFrame == 0)
		iMsPerFrame = 1;

	if (iTime < 0)
	{
		iLoFrame = 0;
		iHiFrame = 1;
		fLerpAmt = 0.0f;

		return true;
	}

	int iFrame(iTime / iMsPerFrame);

	if (iFrame >= m_iNumFrames - 1)
	{
		if (!m_bLoop)
		{
			iLoFrame = m_iStartFrame + m_iNumFrames - 1;
			iHiFrame = m_iStartFrame + m_iNumFrames - 1;
			fLerpAmt = 0.0f;
			return false;
		}
		else
		{
			iLoFrame = ((iFrame - (m_iLoopbackFrame - m_iStartFrame)) % m_iNumLoopFrames) + m_iLoopbackFrame;
			iHiFrame = ((iFrame - (m_iLoopbackFrame - m_iStartFrame) + 1) % m_iNumLoopFrames) + m_iLoopbackFrame;

			int iTimeLo(iFrame * iMsPerFrame);
			fLerpAmt = (iTime - iTimeLo) / float(iMsPerFrame);

			return true;
		}
	}

	iLoFrame = (iFrame % m_iNumFrames) + m_iStartFrame;
	iHiFrame = ((iFrame + 1) % m_iNumFrames) + m_iStartFrame;

	int iTimeLo(iFrame * iMsPerFrame);
	fLerpAmt = (iTime - iTimeLo) / float(iMsPerFrame);

	return true;
}

