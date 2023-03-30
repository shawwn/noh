// (C)2005 S2 Games
// c_swatch.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_swatch.h"
#include "c_widgetstyle.h"
//=============================================================================

/*====================
  CSwatch::~CSwatch
  ====================*/
CSwatch::~CSwatch()
{
}


/*====================
  CSwatch::CSwatch
  ====================*/
CSwatch::CSwatch(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
CPanel(pInterface, pParent, style),
m_refRedCvar(style.GetProperty(_T("redcvar"))),
m_refGreenCvar(style.GetProperty(_T("greencvar"))),
m_refBlueCvar(style.GetProperty(_T("bluecvar"))),
m_refAlphaCvar(style.GetProperty(_T("alphacvar"))),
m_refColorCvar(style.GetProperty(_T("cvar")))
{
	if (m_refColorCvar.IsIgnored() &&
		(m_refRedCvar.IsIgnored() ||
		m_refGreenCvar.IsIgnored() ||
		m_refBlueCvar.IsIgnored()))
		SetTexture(_T("$red_checker"));

	UnsetFlags(WFLAG_NO_DRAW);
}


/*====================
  CSwatch::Frame
  ====================*/
void	CSwatch::Frame(uint uiFrameLength, bool bProcessFrame)
{
	if (!bProcessFrame || !HasFlags(WFLAG_ENABLED))
		return;

	DO_EVENT(WEVENT_FRAME)

	if (HasFlags(WFLAG_VISIBLE))
	{
		if (!m_refColorCvar.IsIgnored())
		{
			m_v4Color = m_refColorCvar.GetVec4();
			if (m_refColorCvar.GetType() == CT_VEC3)
				m_v4Color[A] = 1.0f;
		}
		else
		{
			if (!m_refRedCvar.IsIgnored())
				m_v4Color[R] = m_refRedCvar.GetFloat();
			if (!m_refGreenCvar.IsIgnored())
				m_v4Color[G] = m_refGreenCvar.GetFloat();
			if (!m_refBlueCvar.IsIgnored())
				m_v4Color[B] = m_refBlueCvar.GetFloat();
			if (!m_refAlphaCvar.IsIgnored())
				m_v4Color[A] = m_refAlphaCvar.GetFloat();
		}
	}

	// Recursively call children frame functions
	for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
		(*it)->Frame(uiFrameLength, bProcessFrame);
}
