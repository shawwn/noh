// (C)2006 S2 Games
// c_slider.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_slider.h"
#include "c_slider_handle.h"
#include "c_interface.h"
#include "c_panel.h"
#include "c_uiscript.h"
#include "c_uicmd.h"
#include "c_widgetstyle.h"

#include "../k2/c_xmlnode.h"
//=============================================================================

/*====================
  CSlider::~CSlider
  ====================*/
CSlider::~CSlider()
{
    SAFE_DELETE(m_pSlot);
    SAFE_DELETE(m_pHandle);
}


/*====================
  CSlider::CSlider
  ====================*/
CSlider::CSlider(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_SLIDER, style, false),
m_pSlot(NULL),
m_pHandle(NULL),
m_fValue(style.GetPropertyFloat(_T("value"), 0.0f)),
m_fMinValue(style.GetPropertyFloat(_T("minvalue"), 0.0f)),
m_fMaxValue(style.GetPropertyFloat(_T("maxvalue"), 1.0f)),
m_sSlotWidth(style.GetProperty(_T("slotwidth"), _T("25%"))),
m_sSlotHeight(style.GetProperty(_T("slotheight"), _T("100%"))),
m_sHandleWidth(style.GetProperty(_T("handlewidth"), _T("100%"))),
m_sHandleHeight(style.GetProperty(_T("handleheight"), _T("5%"))),
m_fStep(style.GetPropertyFloat(_T("step"), 0.0f)),
m_sCvar(style.GetProperty(_T("cvar")))
{
    m_refCvar.Assign(m_sCvar);

    SetFlags(WFLAG_NO_DRAW | WFLAG_INTERACTIVE);

    // Slot
    m_fSlotWidth = GetSizeFromString(m_sSlotWidth, GetWidth(), GetHeight());
    m_fSlotHeight = GetSizeFromString(m_sSlotHeight, GetHeight(), GetWidth());

    float fSlotX(MAX(GetWidth() / 2.0f - m_fSlotWidth / 2.0f, 0.0f));
    float fSlotY(MAX(GetHeight() / 2.0f - m_fSlotHeight / 2.0f, 0.0f));

    CWidgetStyle styleCopy(style);
    styleCopy.RemoveProperty(_T("name"));
    styleCopy.RemoveProperty(_T("group"));
    styleCopy.RemoveProperty(_T("onselect"));
    styleCopy.RemoveProperty(_T("onframe"));
    styleCopy.RemoveProperty(_T("ontrigger"));
    styleCopy.RemoveProperty(_T("onshow"));
    styleCopy.RemoveProperty(_T("onhide"));
    styleCopy.RemoveProperty(_T("onenable"));
    styleCopy.RemoveProperty(_T("ondisable"));
    styleCopy.RemoveProperty(_T("onchange"));
    styleCopy.RemoveProperty(_T("onslide"));
    styleCopy.RemoveProperty(_T("onselect"));
    styleCopy.RemoveProperty(_T("onclick"));
    styleCopy.RemoveProperty(_T("ondoubleclick"));
    styleCopy.RemoveProperty(_T("onrightclick"));
    styleCopy.RemoveProperty(_T("onfocus"));
    styleCopy.RemoveProperty(_T("onlosefocus"));
    styleCopy.RemoveProperty(_T("visible"));
    styleCopy.RemoveProperty(_T("enable"));
    styleCopy.RemoveProperty(_T("onload"));
    styleCopy.RemoveProperty(_T("watch"));
    styleCopy.RemoveProperty(_T("ontrigger"));

    for (int i(0); i < 10; ++i)
    {
        styleCopy.RemoveProperty(_T("watch") + XtoA(i));
        styleCopy.RemoveProperty(_T("ontrigger") + XtoA(i));
    }

    styleCopy.RemoveProperty(_T("align"));
    styleCopy.RemoveProperty(_T("valign"));

    if (!GetName().empty())
        styleCopy.SetProperty(_T("name"), GetName() + _T("_slot"));
    styleCopy.RemoveProperty(_T("texture"));

    styleCopy.SetProperty(_T("x"), XtoA(fSlotX));
    styleCopy.SetProperty(_T("y"), XtoA(fSlotY));
    styleCopy.SetProperty(_T("width"), m_sSlotWidth);
    styleCopy.SetProperty(_T("height"), m_sSlotHeight);
    m_pSlot = K2_NEW(ctx_Widgets,  CPanel)(m_pInterface, this, styleCopy);
    m_pSlot->SetColor(style.GetProperty(_T("slotcolor")));
    m_pSlot->SetTexture(m_sTextureName.empty() ? TSNULL : m_sTextureName[0], _T("_slot"));
    m_pSlot->UnsetFlags(WFLAG_NO_DRAW);
    m_pSlot->SetFlags(WFLAG_NO_CLICK);
    AddChild(m_pSlot);

    // Handle
    m_fHandleWidth = GetSizeFromString(m_sHandleWidth, GetWidth(), GetHeight());
    m_fHandleHeight = GetSizeFromString(m_sHandleHeight, GetHeight(), GetWidth());

    float fHandleX(0.0f);
    float fHandleY(0.0f);

    if (HasFlags(WFLAG_ORIENT_VERTICAL))
    {
        fHandleX = GetWidth() / 2.0f - m_fHandleWidth / 2.0f;

        if (!m_refCvar.IsIgnored())
            fHandleY = ILERP(m_refCvar.GetFloat(), m_fMinValue, m_fMaxValue) * (m_fSlotHeight - m_fHandleHeight) + m_pSlot->GetY();
        else
            fHandleY = ILERP(m_fValue, m_fMinValue, m_fMaxValue) * (m_fSlotHeight - m_fHandleHeight) + m_pSlot->GetY();
    }
    else
    {
        fHandleY = GetHeight() / 2.0f - m_fHandleHeight / 2.0f;

        if (!m_refCvar.IsIgnored())
            fHandleX = ILERP(m_refCvar.GetFloat(), m_fMinValue, m_fMaxValue) * (m_fSlotWidth - m_fHandleWidth) + m_pSlot->GetX();
        else
            fHandleX = ILERP(m_fValue, m_fMinValue, m_fMaxValue) * (m_fSlotWidth - m_fHandleWidth) + m_pSlot->GetX();
    }

    if (!GetName().empty())
        styleCopy.SetProperty(_T("name"), this->GetName() + _T("_handle"));

    styleCopy.SetProperty(_T("x"), XtoA(fHandleX));
    styleCopy.SetProperty(_T("y"), XtoA(fHandleY));
    styleCopy.SetProperty(_T("width"), m_sHandleWidth);
    styleCopy.SetProperty(_T("height"), m_sHandleHeight);
    m_pHandle = K2_NEW(ctx_Widgets,  CSliderHandle)(m_pInterface, this, styleCopy);
    m_pHandle->SetColor(style.GetProperty(_T("handlecolor")));
    m_pHandle->UnsetFlags(WFLAG_NO_DRAW);
    if (HasFlags(WFLAG_ORIENT_VERTICAL))
    {
        m_pHandle->SetTexture(m_sTextureName.empty() ? TSNULL : m_sTextureName[0], _T("_vhandle"));
        m_pHandle->SetConstraints(fSlotY, fSlotY + m_fSlotHeight);
    }
    else
    {
        m_pHandle->SetTexture(m_sTextureName.empty() ? TSNULL : m_sTextureName[0], _T("_handle"));
        m_pHandle->SetConstraints(fSlotX, fSlotX + m_fSlotWidth);
    }

    if (m_fStep > 0.0f)
        m_pHandle->SetNumSteps(INT_CEIL(fabs(m_fMaxValue - m_fMinValue) / m_fStep));

    AddChild(m_pHandle);

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CSlider::ButtonUp
  ====================*/
bool    CSlider::ButtonUp(EButton button)
{
    return true;
}


/*====================
  CSlider::MouseDown
  ====================*/
void    CSlider::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    switch (button)
    {
    case BUTTON_MOUSEL:
        m_pInterface->SetExclusiveWidget(m_pHandle);
        m_pHandle->BeginSlide(CVec2f(m_pHandle->GetRect().GetMid()));
        m_pHandle->DoSlide(v2CursorPos - m_recArea.lt());
        break;
    case BUTTON_MOUSER:
        m_pInterface->SetExclusiveWidget(m_pHandle);
        m_pHandle->BeginSlide(CVec2f(m_pHandle->GetRect().GetMid()));
        m_pHandle->DoSlide(v2CursorPos - m_recArea.lt());
        break;
    }
}


/*====================
  CSlider::SetSliderHandleSize
  ====================*/
void    CSlider::SetSliderHandleSize(float fPercent)
{
    if (m_pHandle)
        m_pHandle->SetHandleSize(fPercent);
}


/*====================
  CSlider::DoChange
  ====================*/
void    CSlider::DoChange()
{
    DO_EVENT(WEVENT_CHANGE)
}


/*====================
  CSlider::SetValue
  ====================*/
void    CSlider::SetValue(float fValue)
{
    if (!FLOAT_EQUALS(m_fValue, fValue))
    {
        m_fValue = fValue;

        if (m_pHandle)
            m_pHandle->SetValue(CLAMP(ILERP(fValue, m_fMinValue, m_fMaxValue), 0.0f, 1.0f));

        DoChange();
    }
}


/*====================
  CSlider::SetMinValue
  ====================*/
void    CSlider::SetMinValue(float fMinValue)
{
    if (m_fMinValue != fMinValue)
    {
        m_fMinValue = fMinValue;

        if (m_pHandle)
        {
            if (m_fStep > 0.0f)
                m_pHandle->SetNumSteps(INT_CEIL(fabs(m_fMaxValue - m_fMinValue) / m_fStep));
            m_pHandle->SetValue(ILERP(m_fValue, m_fMinValue, m_fMaxValue));
        }

        DoChange();
    }
}


/*====================
  CSlider::SetMaxValue
  ====================*/
void    CSlider::SetMaxValue(float fMaxValue)
{
    if (m_fMaxValue != fMaxValue)
    {
        m_fMaxValue = fMaxValue;

        if (m_pHandle)
        {
            if (m_fStep > 0.0f)
                m_pHandle->SetNumSteps(INT_CEIL(fabs(m_fMaxValue - m_fMinValue) / m_fStep));
            m_pHandle->SetValue(ILERP(m_fValue, m_fMinValue, m_fMaxValue));
        }

        DoChange();
    }
}


/*====================
  CSlider::Frame
  ====================*/
void    CSlider::Frame(uint uiFrameLength, bool bProcessFrame)
{
    if (!HasFlags(WFLAG_ENABLED))
        return;

    if (!m_refCvar.IsValid() && !m_sCvar.empty())
        m_refCvar.Assign(m_sCvar);

    if (m_fMinValue <= m_fMaxValue)
    {
        if (!m_refCvar.IsIgnored() && !FLOAT_EQUALS(m_refCvar.GetFloat(), m_fValue))
        {
            if (m_refCvar.GetFloat() >= m_fMinValue && m_refCvar.GetFloat() <= m_fMaxValue)
            {
                SetValue(m_refCvar.GetFloat());
            }
            else
            {
                if (m_pHandle)
                    m_pHandle->SetPosition(1.0f);
            }
        }
    }
    else
    {
        if (!m_refCvar.IsIgnored() && !FLOAT_EQUALS(m_refCvar.GetFloat(), m_fValue))
        {
            if (m_refCvar.GetFloat() >= m_fMaxValue && m_refCvar.GetFloat() <= m_fMinValue)
            {
                SetValue(m_refCvar.GetFloat());
            }
            else
            {
                if (m_pHandle)
                    m_pHandle->SetPosition(1.0f);
            }
        }
    }

    if (m_fMinValue <= m_fMaxValue)
        m_fValue = CLAMP(m_fValue, m_fMinValue, m_fMaxValue);
    else
        m_fValue = CLAMP(m_fValue, m_fMaxValue, m_fMinValue);

    DO_EVENT(WEVENT_FRAME)

    // Recursively call children frame functions
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        (*it)->Frame(uiFrameLength, bProcessFrame);
}


/*====================
  CSlider::RecalculateChildSize
  ====================*/
void    CSlider::RecalculateChildSize()
{
    float fWidth(GetWidth());
    float fHeight(GetHeight());

    // Slot
    m_fSlotWidth = GetSizeFromString(m_sSlotWidth, fWidth, fHeight);
    m_fSlotHeight = GetSizeFromString(m_sSlotHeight, fHeight, fWidth);

    m_pSlot->SetBaseX(XtoA(MAX(GetWidth() / 2.0f - m_fSlotWidth / 2.0f, 0.0f)));

    // Handle
    m_fHandleWidth = GetSizeFromString(m_sHandleWidth, fWidth, fHeight);
    m_fHandleHeight = GetSizeFromString(m_sHandleHeight, fHeight, fWidth);
    
    m_pHandle->SetBaseWidth(XtoA(m_fHandleWidth));
    m_pHandle->SetBaseHeight(XtoA(m_fHandleHeight));

    IWidget::RecalculateChildSize();

    float fHandleX(0.0f);
    float fHandleY(0.0f);
    if (HasFlags(WFLAG_ORIENT_VERTICAL))
    {
        fHandleX = (fWidth / 2) - (m_fHandleWidth / 2);

        if (!m_refCvar.IsIgnored())
            fHandleY = ILERP(m_refCvar.GetFloat(), m_fMinValue, m_fMaxValue) * (m_fSlotHeight - m_fHandleHeight) + m_pSlot->GetY();
        else
            fHandleY = ILERP(m_fValue, m_fMinValue, m_fMaxValue) * (m_fSlotHeight - m_fHandleHeight) + m_pSlot->GetY();

        m_pHandle->SetConstraints(0.0f, fHeight);
    }
    else
    {
        fHandleY = (fHeight / 2) - (m_fHandleHeight / 2);

        if (!m_refCvar.IsIgnored())
            fHandleX = ILERP(m_refCvar.GetFloat(), m_fMinValue, m_fMaxValue) * (m_fSlotWidth - m_fHandleWidth) + m_pSlot->GetX();
        else
            fHandleX = ILERP(m_fValue, m_fMinValue, m_fMaxValue) * (m_fSlotWidth - m_fHandleWidth) + m_pSlot->GetX();

        m_pHandle->SetConstraints(0.0f, fWidth);
    }

    m_pHandle->SetX(fHandleX);
    m_pHandle->SetY(fHandleY);
}


/*--------------------
  SetMinValue
  --------------------*/
UI_VOID_CMD(SetMinValue, 1)
{
    if (pThis == NULL ||
        pThis->GetType() != WIDGET_SLIDER)
        return;

    static_cast<CSlider*>(pThis)->SetMinValue(AtoF(vArgList[0]->Evaluate()));
}


/*--------------------
  SetMaxValue
  --------------------*/
UI_VOID_CMD(SetMaxValue, 1)
{
    if (pThis == NULL ||
        pThis->GetType() != WIDGET_SLIDER)
        return;

    static_cast<CSlider*>(pThis)->SetMaxValue(AtoF(vArgList[0]->Evaluate()));
}


