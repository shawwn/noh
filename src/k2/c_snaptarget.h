// (C)2005 S2 Games
// c_snaptarget.h
//
//=============================================================================
#ifndef __C_SNAPTARGET_H__
#define __C_SNAPTARGET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// CSnapTarget
//=============================================================================
class CSnapTarget : public IWidget
{
protected:

	uint		m_uiSnapDistance;
	IWidget*	m_pSnapTarget;
	IWidget*	m_pDragTarget;

public:
	~CSnapTarget()	{}
	CSnapTarget(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

	tstring	GetValue() const									{ if (m_pSnapTarget) return m_pSnapTarget->GetName(); return _T("(UNDEFINED)"); }

	void	Render(const CVec2f &vOrigin, int iFlag, float fFade);

	bool	CheckSnapTargets(CVec2f &v2Pos, IWidget *pWidget);
	bool	CheckSnapTo(CVec2f &v2Pos, IWidget *pWidget);

	void	ClearSnapTarget();

	void	WidgetLost(IWidget *pWidget);
};
//=============================================================================

#endif // __C_SNAPTARGET_H__
