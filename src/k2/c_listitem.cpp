// (C)2005 S2 Games
// c_listitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_listitem.h"
#include "c_widgetstate.h"
#include "c_interface.h"
#include "c_widgetstyle.h"
#include "c_label.h"
#include "i_listwidget.h"
#include "c_uicmd.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CListItem::CListItem
  ====================*/
CListItem::CListItem(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_LISTITEM, style),
m_uiListItemFlags(0),
m_sValue(style.GetProperty(_T("value"))),
m_sCondition(style.GetProperty(_T("condition"))),

m_v4LastColor(CLEAR),
m_v4NextColor(CLEAR),
m_v4LastBorderColor(CLEAR),
m_v4NextBorderColor(CLEAR),

m_uiTransitionStart(0), 
m_uiTransitionTime(INVALID_TIME)
{
    assert(pParent && pParent->HasFlags(WFLAG_LIST));

    SetSelect(style.GetPropertyBool(_T("select"), true));
    SetCommand(style.GetPropertyBool(_T("command"), false));

    SetDrawColorUnder(true);
    SetDrawColors(true);

    RecalculateSize();

    const tstring &sHotkey(style.GetProperty(_T("hotkey")));

    m_cHotkey = sHotkey.empty() ? 0 : sHotkey[0];

    if (style.HasProperty(_T("sortindex")))
        m_vSortIndex = TokenizeString(style.GetProperty(_T("sortindex")), _T(','));

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CListItem::GetAbsolutePos
  ====================*/
CVec2f  CListItem::GetAbsolutePos()
{
    assert(m_pParent != NULL && m_pParent->HasFlags(WFLAG_LIST));
    if (m_pParent == NULL || !m_pParent->HasFlags(WFLAG_LIST))
        return CVec2f(0.0f, 0.0f);

    IListWidget* pParentList((IListWidget*)m_pParent);
    uint uiCount(pParentList->GetNumListitems());

    switch (pParentList->GetWrapMode())
    {
    case LISTBOX_WRAP_COLUMN:
    case LISTBOX_WRAP_NONE:
        {
            float fY(0.0f);
            for (uint ui(pParentList->GetListDrawStartIndex()); ui < uiCount; ++ui)
            {
                CListItem* pItem(pParentList->GetItem(ui));
                if (pItem == this)
                {
                    return IWidget::GetAbsolutePos() + CVec2f(0.0f, fY);
                }
                else
                {
                    fY += pItem->GetHeight();
                }
            }
        }
        break;
    case LISTBOX_WRAP_ROW:
        {
            float fX(0.0f);
            for (uint ui(pParentList->GetListDrawStartIndex()); ui < uiCount; ++ui)
            {
                CListItem* pItem(pParentList->GetItem(ui));
                if (pItem == this)
                {
                    return IWidget::GetAbsolutePos() + CVec2f(fX, 0.0f);
                }
                else
                {
                    fX += pItem->GetWidth();
                }
            }
        }
        break;
    }

    // Failure
    assert(false);
    return IWidget::GetAbsolutePos();
}


/*====================
  CListItem::SetText
  ====================*/
void    CListItem::SetText(tstring sText)
{
    //Set the text of all children who have text...
    for (WidgetPointerVector_cit it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
    {
        if ((*it)->GetType() == WIDGET_LABEL)
            static_cast<CLabel *>(*it)->SetText(sText);
    }
}


/*====================
  CListItem::GetText
  ====================*/
tstring CListItem::GetText() const
{
    //Find the first child with text and return it...
    for (WidgetPointerVector_cit it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
    {
        if ((*it)->GetType() == WIDGET_LABEL)
            return static_cast<CLabel *>(*it)->GetText();
    }

    return _T("");
}


/*====================
  CListItem::SetNextColor
  ====================*/
void    CListItem::SetNextColor(const CVec4f &v4Color, const CVec4f &v4BorderColor)
{
    uint uiTransitionTime = (K2System.Milliseconds() - m_uiTransitionStart);

    float fCompletion(0.0f);

    if (m_uiTransitionTime != INVALID_TIME && m_uiTransitionTime != 0)
        fCompletion = CLAMP(float(uiTransitionTime) / float(m_uiTransitionTime), 0.0f, 1.0f);

    m_v4LastColor = LERP(fCompletion, m_v4LastColor, m_v4NextColor);
    m_v4NextColor = v4Color;

    m_v4LastBorderColor = LERP(fCompletion, m_v4LastBorderColor, m_v4NextBorderColor);
    m_v4NextBorderColor = v4BorderColor;

    m_uiTransitionStart = K2System.Milliseconds();
}


/*====================
  CListItem::Render
  ====================*/
void    CListItem::Render(const CVec2f &vOrigin, int iFlag, float fFade, bool bHovered, bool bSelected, CWidgetState *pBackgroundState, CWidgetState *pHighlightState)
{
    if (IsDead())
        return;

    if (!HasFlags(WFLAG_VISIBLE))
        return;

    CVec2f v2LocalOrigin(vOrigin + m_recArea.lt());
    float fLocalFade(fFade * m_fFadeCurrent);
    CRectf rect(vOrigin, vOrigin + (m_recArea.rb() - m_recArea.lt()));

    uint uiTransitionTime = (K2System.Milliseconds() - m_uiTransitionStart);
    float fCompletion(0.0f);

    if (m_uiTransitionTime != INVALID_TIME && m_uiTransitionTime != 0)
        fCompletion = CLAMP(float(uiTransitionTime) / float(m_uiTransitionTime), 0.0f, 1.0f);

    if (GetDrawColors() && GetUseBackgroundImage())
    {
        CVec4f v4Color(GetColor());

        v4Color[A] *= fLocalFade;

        Draw2D.SetColor(v4Color);
        Draw2D.Rect(vOrigin.x, vOrigin.y, m_recArea.GetWidth(), m_recArea.GetHeight(), m_hTexture[0]);
    }

    if (GetDrawColors())
    {
        if (pBackgroundState != NULL)
            pBackgroundState->Render(vOrigin, iFlag, fLocalFade);

        // Draw back color
        if (GetDrawColorUnder())
        {
            CVec4f v4BorderColor(LERP(fCompletion, m_v4LastBorderColor, m_v4NextBorderColor));
            CVec4f v4Color(LERP(fCompletion, m_v4LastColor, m_v4NextColor));

            v4BorderColor[A] *= fLocalFade;
            v4Color[A] *= fLocalFade;

            Draw2D.FilledRect(rect, 1.0f, v4BorderColor, v4Color);
        }

        if ((bHovered || bSelected) && pHighlightState != NULL)
            pHighlightState->Render(vOrigin, iFlag, fLocalFade);
    }

    // Render this widget
    if ((HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_TOP)) ||
        (!HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_BOTTOM)))
        RenderWidget(v2LocalOrigin, fLocalFade);

    // Render children
    for (WidgetPointerVector_cit it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
        (*it)->Render(v2LocalOrigin, iFlag, fLocalFade);

    // Draw fore color
    if (GetDrawColors() && !GetDrawColorUnder())
    {
        CVec4f v4BorderColor(LERP(fCompletion, m_v4LastBorderColor, m_v4NextBorderColor));
        CVec4f v4Color(LERP(fCompletion, m_v4LastColor, m_v4NextColor));

        v4BorderColor[A] *= fLocalFade;
        v4Color[A] *= fLocalFade;

        Draw2D.FilledRect(rect, 1.0f, v4BorderColor, v4Color);
    }
}


/*--------------------
  SetAllowSelect
  --------------------*/
UI_VOID_CMD(SetAllowSelect, 1)
{
    if (pThis == NULL || pThis->GetType() != WIDGET_LISTITEM)
        return;

    CListItem *pListItem(static_cast<CListItem*>(pThis));

    pListItem->SetSelect(AtoB(vArgList[0]->Evaluate()));
}
