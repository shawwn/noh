// (C)2005 S2 Games
// i_dragwidget.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_dragwidget.h"
#include "c_interface.h"
#include "c_widgetstyle.h"

#include "../k2/c_rect.h"
#include "../k2/c_vec2.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_uicmd.h"
//=============================================================================

/*====================
  IDragWidget::~IDragWidget
  ====================*/
IDragWidget::~IDragWidget()
{
}


/*====================
  IDragWidget::IDragWidget
  ====================*/
IDragWidget::IDragWidget(CInterface *pInterface, IWidget *pParent, EWidgetType eType, const CWidgetStyle& style) :
IWidget(pInterface, pParent, eType, style),
m_bIsDragging(false),
m_vecLastDrag(1.0f, 1.0f)
{
	if (style.GetPropertyBool(_T("cangrab"), false))
	{
		SetFlags(WFLAG_CAN_GRAB);
		SetFlagsRecursive(WFLAG_PROCESS_CURSOR);
	}
}


/*====================
 IDragWidget::SnapToParent
 ====================*/
void	IDragWidget::SnapToParent(float &fNewXPos, float &fNewYPos, CVec2f &v2CursorPos, int iSnapAt)
{
	float fTmpX(fNewXPos);
	float fTmpY(fNewYPos);
	CRectf recParent(m_pParent->GetRect());

	// inside snapping

	// snap to inside right edge of widget
	if (fNewXPos + GetWidth() > recParent.GetWidth() - iSnapAt
	&& fNewXPos + GetWidth() <= recParent.GetWidth() + (iSnapAt >> 1))
		fNewXPos = recParent.GetWidth() - GetWidth();
	// snap to inside left edge of widget
	else if (fNewXPos < iSnapAt
		&& fNewXPos >= -(iSnapAt >> 1))
		fNewXPos = 0;

	// snap to inside top edge of widget
	if (fNewYPos < iSnapAt
		&& fNewYPos >= -(iSnapAt >> 1))
		fNewYPos = 0;
	// snap to inside bottom edge of widget
	else if (fNewYPos + GetHeight() > recParent.GetHeight() - iSnapAt
		&& fNewYPos + GetHeight() <= recParent.GetHeight() + (iSnapAt >> 1))
		fNewYPos = recParent.GetHeight() - GetHeight();

	// outside snapping

	// snap left edge to right outside edge of parent
	if (fNewXPos > recParent.GetWidth()
		&& fNewXPos <= recParent.GetWidth() + iSnapAt)
		fNewXPos = recParent.GetWidth();
	// snap right edge to left outside edge of parent
	else if (fNewXPos + GetWidth() < 0
		&& fNewXPos + GetWidth() >= -iSnapAt)
		fNewXPos = -GetWidth();
	// snap bottom edge to top outside edge of parent
	else if (fNewYPos + GetHeight() < 0
		&& fNewYPos + GetHeight() >= -iSnapAt)
		fNewYPos = -GetHeight();
	// snap top edge to bottom outside edge of parent
	else if (fNewYPos > recParent.GetHeight()
		&& fNewYPos <= recParent.GetHeight() + iSnapAt)
		fNewYPos = recParent.GetHeight();

	v2CursorPos.x += fNewXPos - fTmpX;
	v2CursorPos.y += fNewYPos - fTmpY;
}


/*====================
 IDragWidget::SnapToGrid

 FIXME: Widgets do a weird bouncy thing when they snap and i can't figure out what is going on
 ====================*/
void	IDragWidget::SnapToGrid(float &fNewXPos, float &fNewYPos, CVec2f &v2CursorPos, int iGridSquares, int iSnapAt)
{
	float fTmpX(fNewXPos);
	float fTmpY(fNewYPos);
	int iNumSquaresX(INT_FLOOR(Draw2D.GetScreenW() / iGridSquares));
	int iNumSquaresY(INT_FLOOR(Draw2D.GetScreenH() / iGridSquares));
	CVec2f v2Pos(GetAbsolutePos());

	// snap left edge to closest grid cel
	float fSnap = fmod(fNewXPos, static_cast<float>(iNumSquaresX)); // just do calculation once
	if (fSnap <= iSnapAt
		&& fSnap != 0)
		fNewXPos -= fSnap;

	// snap top edge to closest grid cel
	fSnap = fmod(fNewYPos, static_cast<float>(iNumSquaresY));
	if (fSnap <= iSnapAt
		&& fSnap != 0)
		fNewYPos -= fSnap;

	v2CursorPos.x += fNewXPos - fTmpX;
	v2CursorPos.y += fNewYPos - fTmpY;
}


/*====================
 IDragWidget::DoDrag
 ====================*/
void	IDragWidget::DoDrag(const CVec2f &v2CursorPos)
{
	CVec2f v2Pos(v2CursorPos);
	bool bFoundTarget;
	float fNewXPos = GetX() + (v2CursorPos.x - m_vecLastDrag.x);
	float fNewYPos = GetY() + (v2CursorPos.y - m_vecLastDrag.y);

	if (m_pInterface->SnapWidgetsToParent())
		SnapToParent(fNewXPos, fNewYPos, v2Pos, m_pInterface->GetParentSnapAt());

	if (m_pInterface->IsGridSnapEnabled())
		SnapToGrid(fNewXPos, fNewYPos, v2Pos, m_pInterface->GetNumGridSquares(), m_pInterface->GetGridSnapAt());

	CVec2f v2AbsPos(GetAbsolutePos());

	v2Pos.x += (v2AbsPos.x - GetX());
	v2Pos.y += (v2AbsPos.y - GetY());
	bFoundTarget = m_pInterface->CheckSnapTargets(v2Pos, this);
	v2Pos.x -= (v2AbsPos.x - GetX());
	v2Pos.y -= (v2AbsPos.y - GetY());

	if (bFoundTarget)
	{
		fNewXPos = v2Pos.x;
		fNewYPos = v2Pos.y;
	}

	Move(fNewXPos, fNewYPos);

	m_vecLastDrag = v2Pos;
}


/*====================
  IDragWidget::ProcessInputMouseButton
  ====================*/
bool	IDragWidget::ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue)
{
	if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
		return false;

	CVec2f v2LastCursorPos;

	if (HasSavedCursorPos())
		v2LastCursorPos = m_v2LastCursorPos;

	if (!HasFlags(WFLAG_PASSIVE_CHILDREN))
	{
		// Check children, relative to parent widget
		for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
		{
			if ((*it)->ProcessInputMouseButton(v2CursorPos - m_recArea.lt(), button, fValue))
				return true;
		}
	}

	if (m_eWidgetType == WIDGET_INTERFACE || (HasFlags(WFLAG_NO_CLICK) && !IsDragging()))
		return false;

	// Check this widget
	if (fValue == 1.0f) // down
	{
		if (m_recArea.AltContains(v2CursorPos) || (HasSavedCursorPos() && m_recArea.AltContains(v2LastCursorPos)))
		{
			// set active widget we click on so we know where to send input
			if (IsInteractive())
				m_pInterface->SetActiveWidget(this);

			switch (button)
			{
			case BUTTON_MOUSEL:
				if (!IsDragging())
				{
					MouseDown(BUTTON_MOUSEL, v2CursorPos);
					if (HasFlags(WFLAG_CAN_GRAB))
						BeginGrab(v2CursorPos);
				}
				break;
			
			case BUTTON_MOUSER:
			case BUTTON_MOUSEM:
			case BUTTON_MOUSEX1:
			case BUTTON_MOUSEX2:
			case BUTTON_WHEELUP:
			case BUTTON_WHEELDOWN:
			case BUTTON_WHEELLEFT:
			case BUTTON_WHEELRIGHT:
				MouseDown(button, v2CursorPos);
				break;

			default:
				break;
			}

			return true;
		}
	}
	else // up
	{
		bool bExclusive(m_pInterface && m_pInterface->GetExclusiveWidget() == this);

		switch (button)
		{
		case BUTTON_MOUSEL:
			MouseUp(BUTTON_MOUSEL, v2CursorPos);
			if (IsDragging())
				EndGrab();
			break;

		case BUTTON_MOUSER:
		case BUTTON_MOUSEM:
		case BUTTON_MOUSEX1:
		case BUTTON_MOUSEX2:
		case BUTTON_WHEELUP:
		case BUTTON_WHEELDOWN:
		case BUTTON_WHEELLEFT:
		case BUTTON_WHEELRIGHT:
			MouseUp(button, v2CursorPos);
			break;

		default:
			break;
		}

		return bExclusive; // never eat up's
	}

	return false;
}


/*====================
  IDragWidget::ProcessInputCursor
  ====================*/
bool	IDragWidget::ProcessInputCursor(const CVec2f &v2CursorPos)
{
	if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
		return false;

	if (IsDragging())
		DoDrag(v2CursorPos);

	return IWidget::ProcessInputCursor(v2CursorPos);
}


/*====================
  IDragWidget::SetCanGrab
  ====================*/
void	IDragWidget::SetCanGrab(bool bCanGrab)
{
	if (bCanGrab)
	{
		SetFlags(WFLAG_CAN_GRAB);
		SetFlagsRecursive(WFLAG_PROCESS_CURSOR);
	}
	else
	{
		UnsetFlags(WFLAG_CAN_GRAB);
	}
}


/*--------------------
  SetCanGrab
  --------------------*/
UI_VOID_CMD(SetCanGrab, 1)
{
	if (pThis == NULL ||
		!pThis->IsDragWidget())
		return;

	pThis->GetAsDragWidget()->SetCanGrab(AtoB(vArgList[0]->Evaluate()));
}

