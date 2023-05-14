// (C)2005 S2 Games
// c_snaptarget.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_snaptarget.h"
#include "c_interface.h"
#include "c_widgetstyle.h"
#include "c_uicmd.h"
//=============================================================================

/*====================
  CSnapTarget::CSnapTarget
  ====================*/
CSnapTarget::CSnapTarget(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_SNAPTARGET, style),
m_uiSnapDistance(style.GetPropertyInt(_T("snapdistance"), 0)),
m_pSnapTarget(nullptr),
m_pDragTarget(nullptr)
{
    if (!style.HasProperty(_T("texture")) && !style.HasProperty(_T("color")))
        SetFlags(WFLAG_NO_DRAW);

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CSnapTarget::CheckSnapTargets
  ====================*/
bool    CSnapTarget::CheckSnapTargets(CVec2f &v2Pos, IWidget *pWidget)
{
    CVec2f v2Area(GetAbsolutePos());

    if ((v2Pos.x <= v2Area.x + GetWidth() + m_uiSnapDistance) && 
        (v2Pos.x >= v2Area.x - m_uiSnapDistance) &&
        (v2Pos.y <= v2Area.y + GetHeight() + m_uiSnapDistance) &&
        (v2Pos.y >= v2Area.y - m_uiSnapDistance))
    {
        v2Pos.x = v2Area.x + (GetWidth() / 2) - (pWidget->GetWidth() / 2);
        v2Pos.y = v2Area.y + (GetHeight() / 2) - (pWidget->GetHeight() / 2);

        m_pDragTarget = pWidget;

        return true;
    }
    else
    {
        if (pWidget == m_pSnapTarget)
            m_pSnapTarget = nullptr;

        if (pWidget == m_pDragTarget)
            m_pDragTarget = nullptr;
    }

    return IWidget::CheckSnapTargets(v2Pos, pWidget);
}

/*====================
  CSnapTarget::CheckSnapTo
  ====================*/
bool    CSnapTarget::CheckSnapTo(CVec2f &v2Pos, IWidget *pWidget)
{
    CVec2f v2Area(GetAbsolutePos());

    if ((v2Pos.x <= v2Area.x + GetWidth() + m_uiSnapDistance) && 
        (v2Pos.x >= v2Area.x - m_uiSnapDistance) &&
        (v2Pos.y <= v2Area.y + GetHeight() + m_uiSnapDistance) &&
        (v2Pos.y >= v2Area.y - m_uiSnapDistance))
    {
        m_pDragTarget = nullptr;
        m_pSnapTarget = pWidget;
        DO_EVENT_PARAM_RETURN(WEVENT_SNAP, pWidget->GetName(), true)

        return true;
    }
    else if (pWidget == m_pSnapTarget)
        m_pSnapTarget = nullptr;

    return IWidget::CheckSnapTo(v2Pos, pWidget);
}


/*====================
  CSnapTarget::Render
  ====================*/
void    CSnapTarget::Render(const CVec2f &vOrigin, int iFlag, float fFade)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return;

    if ((HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_TOP)) ||
        (!HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_BOTTOM)))
        IWidget::Render(vOrigin, WIDGET_RENDER_ALL, fFade);

/*  if (!(iFlag & WIDGET_RENDER_TOP))
        return;*/

    // Always render the snapped target overtop of the widget
    if (m_pSnapTarget != nullptr)
        m_pSnapTarget->Render(m_pSnapTarget->GetAbsolutePos() - m_pSnapTarget->GetRect().lt(), WIDGET_RENDER_ALL, fFade * m_fFadeCurrent);

    // Always render the dragged target overtop of the widget and snapped widget
    if (m_pDragTarget != nullptr)
        m_pDragTarget->Render(m_pDragTarget->GetAbsolutePos() - m_pDragTarget->GetRect().lt(), WIDGET_RENDER_ALL, fFade * m_fFadeCurrent);
}


/*====================
  CSnapTarget::ClearSnapTarget
  ====================*/
void    CSnapTarget::ClearSnapTarget()
{
    m_pSnapTarget = nullptr;
    m_pDragTarget = nullptr;
}

/*====================
  CSnapTarget::WidgetLost
  ====================*/
void    CSnapTarget::WidgetLost(IWidget *pWidget)
{
    if (pWidget == nullptr)
        return;

    if (m_pSnapTarget == pWidget)
        m_pSnapTarget = nullptr;

    if (m_pDragTarget == pWidget)
        m_pSnapTarget = nullptr;

    IWidget::WidgetLost(pWidget);
}


/*--------------------
  ClearSnapTarget
  --------------------*/
UI_VOID_CMD(ClearSnapTarget, 0)
{
    if (!pThis || pThis->GetType() != WIDGET_SNAPTARGET)
        return;

    static_cast<CSnapTarget *>(pThis)->ClearSnapTarget();
}
