// (C)2005 S2 Games
// c_floater.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_floater.h"

#include "c_widgetstyle.h"
#include "c_uitextureregistry.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
#include "c_interface.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CFloater::CFloater
  ====================*/
CFloater::CFloater(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_FLOATER, style),
	//MikeG
m_bLockToParent(style.GetPropertyBool(_CWS("locktoparent"), false)), 
m_bKeepOnScren(style.GetPropertyBool(_CWS("keeponscreen"), true))
{
	GetParent()->BringChildToFront(this);//m_vBringToFront.push_back(this);
	SetFlags(WFLAG_PROCESS_CURSOR | WFLAG_NO_CLICK | WFLAG_PASSIVE_CHILDREN);
}


/*====================
  CFloater::ProcessInputCursor
  ====================*/
bool	CFloater::ProcessInputCursor(const CVec2f &v2CursorPos)
{
	return IWidget::ProcessInputCursor(v2CursorPos);
}

	//MikeG added so floaters stick to the mouse
/*====================
  CFloater::Render
  ====================*/
void	CFloater::Render(const CVec2f &vOrigin, int iFlag, float fFade)
{
	if (IsDead())
		return;

	if (!HasFlags(WFLAG_VISIBLE))
		return;

	CVec2f v2LocalOrigin(vOrigin +  m_recArea.lt());
	float fLocalFade(fFade * m_fFadeCurrent);

	// Render this widget
	if ((HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_TOP)) ||
		(!HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_BOTTOM)))
		RenderWidget( m_recArea.lt(), fLocalFade);

	// Render children
	for (WidgetPointerVector_cit it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
		(*it)->Render(v2LocalOrigin, iFlag, fLocalFade);
}


/*====================
  CFloater::Frame
  ====================*/
void	CFloater::Frame(uint uiFrameLength, bool bProcessFrame)
{
	if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
		return;

	const CVec2f &v2CursorPos(m_pInterface->GetCursorPos());

		//MikeG Made some changes here so floaters werent offset by just there parent
	if (GetAlign() == ALIGN_CENTER)
		SetX(v2CursorPos.x  + GetPositionFromString(GetBaseX(), Draw2D.GetScreenW(), Draw2D.GetScreenH()) - ROUND(GetWidth() / 2.0f));
	else if (GetAlign() == ALIGN_RIGHT)
		SetX(v2CursorPos.x  + GetPositionFromString(GetBaseX(), Draw2D.GetScreenW(), Draw2D.GetScreenH()) - GetWidth());
	else
		SetX(v2CursorPos.x  + GetPositionFromString(GetBaseX(), Draw2D.GetScreenW(), Draw2D.GetScreenH()));

	if (GetVAlign() == ALIGN_CENTER)
		SetY(v2CursorPos.y  - GetPositionFromString(GetBaseY(), Draw2D.GetScreenW(), Draw2D.GetScreenH()) - ROUND(GetHeight() / 2.0f));
	else if (GetVAlign() == ALIGN_BOTTOM)
		SetY(v2CursorPos.y  - GetPositionFromString(GetBaseY(), Draw2D.GetScreenW(), Draw2D.GetScreenH()) - GetHeight());
	else
		SetY(v2CursorPos.y  - GetPositionFromString(GetBaseY(), Draw2D.GetScreenW(), Draw2D.GetScreenH()));

	IWidget::Frame(uiFrameLength, bProcessFrame);

		//Set them to be lockable by the parent
	if (m_bLockToParent && m_pParent)
	{
		IWidget *FamilyFinder = 0;
		CVec2f TVec = CVec2f(0, 0);
		FamilyFinder = m_pParent->GetParent();
		while(FamilyFinder)
		{
			TVec += FamilyFinder->GetRect().lt();
			FamilyFinder = FamilyFinder->GetParent();
		}

		if (m_recArea.top < m_pParent->GetRect().top + TVec.y)
			m_recArea.MoveToY(m_pParent->GetRect().top + TVec.y);
		if (m_recArea.left < m_pParent->GetRect().left + TVec.x)
			m_recArea.MoveToX(m_pParent->GetRect().left + TVec.x);
		if (m_recArea.bottom > m_pParent->GetRect().bottom + TVec.y)
			m_recArea.MoveToY(m_pParent->GetRect().bottom + TVec.y - m_recArea.GetHeight());
		if (m_recArea.right > m_pParent->GetRect().right + TVec.x)
			m_recArea.MoveToX(m_pParent->GetRect().right + TVec.x - m_recArea.GetWidth());

	}
	if (m_bKeepOnScren)
	{
		if (m_recArea.top < 0.0f)
			m_recArea.MoveToY(0.0f);
		if (m_recArea.left < 0.0f)
			m_recArea.MoveToX(0.0f);
		if (m_recArea.bottom > Draw2D.GetScreenH())
			m_recArea.MoveToY(Draw2D.GetScreenH() - m_recArea.GetHeight());
		if (m_recArea.right > Draw2D.GetScreenW())
			m_recArea.MoveToX(Draw2D.GetScreenW() - m_recArea.GetWidth());
	}
}


/*====================
  CFloater::RecalculatePosition
  ====================*/
void	CFloater::RecalculatePosition()
{
}
