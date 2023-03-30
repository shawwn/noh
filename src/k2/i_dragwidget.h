// (C)2005 S2 Games
// i_dragwidget.h
//
//=============================================================================
#ifndef __I_DRAGWIDGET_H__
#define __I_DRAGWIDGET_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_uimanager.h"
#include "i_widget.h"
#include "c_interface.h"
//=============================================================================

//=============================================================================
// IDragWidget
//=============================================================================
class IDragWidget : public IWidget
{
protected:
	bool	m_bIsDragging;

	CVec2f	m_vecLastDrag;

public:
	virtual ~IDragWidget();
	IDragWidget(CInterface *pInterface, IWidget *pParent, EWidgetType eType, const CWidgetStyle& style);

	SUB_WIDGET_ACCESSOR(IDragWidget, DragWidget)

	void		SnapToParent(float &fNewXPos, float &fNewYPos, CVec2f &v2CursorPos, int iSnapAt);
	void		SnapToGrid(float &fNewXPos, float &fNewYPos, CVec2f &v2CursorPos, int iGridSquares, int iSnapAt);

	inline void	BeginGrab(const CVec2f &v2CursorPos);
	inline void	EndGrab();

	void		DoDrag(const CVec2f &v2CursorPos);

	bool		IsDragging()	{ return m_bIsDragging; }

	bool		ProcessInputCursor(const CVec2f &v2CursorPos);
	bool		ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue);

	void		SetCanGrab(bool bCanGrab);
};
//=============================================================================

/*====================
  IDragWidget::BeginGrab
  ====================*/
void IDragWidget::BeginGrab(const CVec2f &v2CursorPos)
{
	m_bIsDragging = true;
	m_vecLastDrag = v2CursorPos;

	tsvector vsParams;

	vsParams.push_back(XtoA(GetAbsolutePos().x));
	vsParams.push_back(XtoA(GetAbsolutePos().y));

	DoEvent(WEVENT_STARTDRAG, vsParams);
}

/*====================
  IDragWidget::EndGrab
  ====================*/
void IDragWidget::EndGrab()
{
	CVec2f v2Pos(GetAbsolutePos());
	m_pInterface->CheckSnapTo(v2Pos, this);

	tsvector vsParams;

	vsParams.push_back(XtoA(v2Pos.x));
	vsParams.push_back(XtoA(v2Pos.y));

	DoEvent(WEVENT_ENDDRAG, vsParams);

	m_bIsDragging = false;
	m_vecLastDrag.Set(-1, -1);
}

#endif //__I_DRAGWIDGET_H__
