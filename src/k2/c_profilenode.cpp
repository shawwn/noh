// (C)2005 S2 Games
// c_profilenode.cpp
//
// Data node for CProfileManager
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_profilenode.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
//=============================================================================


/*====================
  CProfileNode::CProfileNode
  ====================*/
CProfileNode::CProfileNode(const TCHAR *szName, eProfileType eType, CProfileNode *pParent) :
m_szName(szName),
m_eType(eType),
m_llStartTime(0),
m_llTotalTime(0),
m_iFrameCalls(0),
m_iFrameRecursiveCalls(0),
m_llFrameTime(0),
m_iWorkingFrameCalls(0),
m_iWorkingFrameRecursiveCalls(0),
m_llWorkingFrameTime(0),
m_iMaxFrameCalls(0),
m_iMaxFrameRecursiveCalls(0),
m_llMaxFrameTime(0),
m_iTotalCalls(0),
m_iFrame(0),
m_pParent(pParent)
{
}


/*====================
  CProfileNode::~CProfileNode
  ====================*/
CProfileNode::~CProfileNode()
{
	bool bProfile(g_bProfile);

	g_bProfile = false;
	ProfileVector::iterator itEnd(m_vChildren.end());
	for (ProfileVector::iterator it(m_vChildren.begin()); it != itEnd; ++it)
		K2_DELETE(*it);
	g_bProfile = bProfile;
}


/*====================
  CProfileNode::ResetFrame
  ====================*/
void	CProfileNode::ResetFrame(eProfileType eType)
{
	if (m_eType == eType)
	{
		m_iFrameCalls = m_iWorkingFrameCalls;
		m_iFrameRecursiveCalls = m_iWorkingFrameRecursiveCalls;
		m_llFrameTime = m_llWorkingFrameTime;
		
		m_iWorkingFrameCalls = 0;
		m_llWorkingFrameTime = 0;
		m_iWorkingFrameRecursiveCalls = 0;

		m_iMaxFrameCalls = MAX(m_iMaxFrameCalls, m_iFrameCalls);
		m_llMaxFrameTime = MAX(m_llMaxFrameTime, m_llFrameTime);
		m_iMaxFrameRecursiveCalls = MAX(m_iMaxFrameRecursiveCalls, m_iFrameRecursiveCalls);

		++m_iFrame;
	}

	ProfileVector::iterator itEnd(m_vChildren.end());
	for (ProfileVector::iterator it(m_vChildren.begin()); it != itEnd; ++it)
		(*it)->ResetFrame(eType);
}


/*====================
  CProfileNode::ResetMax
  ====================*/
void	CProfileNode::ResetMax()
{
	m_iMaxFrameCalls = 0;
	m_llMaxFrameTime = 0;
	m_iMaxFrameRecursiveCalls = 0;

	ProfileVector::iterator itEnd(m_vChildren.end());
	for (ProfileVector::iterator it(m_vChildren.begin()); it != itEnd; ++it)
		(*it)->ResetMax();
}
