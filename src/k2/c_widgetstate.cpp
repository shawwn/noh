// (C)2007 S2 Games
// c_widgetstate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_widgetstate.h"
#include "c_interface.h"
#include "c_widgetstyle.h"
#include "c_label.h"
#include "i_listwidget.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CWidgetState::CWidgetState
  ====================*/
CWidgetState::CWidgetState(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_STATE, style),
m_sStateName(style.GetProperty(_T("statename")))
{
	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)
}


/*====================
  CWidgetState::RenderWidget
  ====================*/
void	CWidgetState::RenderWidget(const CVec2f &vOrigin, float fFade)
{
	if (!HasFlags(WFLAG_VISIBLE))
		return;

	// Render children
	//for (WidgetPointerVector_cit it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
	//	(*it)->Render(vOrigin + m_recArea.lt(), WIDGET_RENDER_ALL, fFade * m_fFadeCurrent);
}
