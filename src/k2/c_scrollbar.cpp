// (C)2005 S2 Games
// c_scrollbar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_scrollbar.h"
#include "c_scrollbar_button.h"
#include "c_scrollbar_slider.h"
#include "c_panel.h"
#include "c_interface.h"
#include "c_uiscript.h"
#include "c_widgetstyle.h"
//=============================================================================

/*====================
  CScrollbar::~CScrollbar
  ====================*/
CScrollbar::~CScrollbar()
{
    SAFE_DELETE(m_pButtonMin);
    SAFE_DELETE(m_pSlider);
    SAFE_DELETE(m_pButtonMax);
}


/*====================
  CScrollbar::CScrollbar
  ====================*/
CScrollbar::CScrollbar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_SCROLLBAR, style, false),
m_pSlider(NULL),
m_pButtonMin(NULL),
m_pButtonMax(NULL),
m_fIncrement(1.0f)
{
    SetFlags(WFLAG_NO_DRAW | WFLAG_INTERACTIVE);

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

    styleCopy.RemoveProperty(_T("align"));
    styleCopy.RemoveProperty(_T("valign"));
    styleCopy.RemoveProperty(_T("form"));
    styleCopy.RemoveProperty(_T("data"));
    styleCopy.RemoveProperty(_T("watch"));
    styleCopy.RemoveProperty(_T("ontrigger"));

    for (int i(0); i < 10; ++i)
    {
        styleCopy.RemoveProperty(_T("watch") + XtoA(i));
        styleCopy.RemoveProperty(_T("ontrigger") + XtoA(i));
    }

    styleCopy.SetProperty(_T("slotwidth"), _T("100%"));
    styleCopy.SetProperty(_T("slotheight"), _T("100%"));
    styleCopy.SetProperty(_T("x"), 0.0f);
    styleCopy.SetProperty(_T("y"), 0.0f);

    if (HasFlags(WFLAG_ORIENT_VERTICAL))
    {
        styleCopy.SetProperty(_T("width"), GetBaseWidth());
        styleCopy.SetProperty(_T("height"), GetBaseWidth());
        styleCopy.SetProperty(_T("texture"), Filename_AppendSuffix(m_sTextureName[0], _T("_vmin")));
        styleCopy.SetProperty(_T("textureflags"), _T("udo"));
        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), GetName() + _T("_button_min"));
        m_pButtonMin = K2_NEW(ctx_Widgets,  CScrollbarButton)(m_pInterface, this, SCROLLBAR_BUTTON_MIN, styleCopy);
        AddChild(m_pButtonMin);

        styleCopy.SetProperty(_T("y"), GetBaseWidth());
        styleCopy.SetProperty(_T("height"), MAX(GetHeight() - GetWidth() * 2.0f, 0.0f));
        styleCopy.SetProperty(_T("texture"), Filename_AppendSuffix(m_sTextureName[0], _T("_vslider")));
        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), GetName() + _T("_slider"));
        m_pSlider = K2_NEW(ctx_Widgets,  CScrollbarSlider)(m_pInterface, this, styleCopy);
        AddChild(m_pSlider);

        styleCopy.SetProperty(_T("valign"), _T("bottom"));
        styleCopy.SetProperty(_T("y"), _T("0"));
        styleCopy.SetProperty(_T("height"), GetBaseWidth());
        styleCopy.SetProperty(_T("texture"), Filename_AppendSuffix(m_sTextureName[0], _T("_vmax")));
        styleCopy.SetProperty(_T("textureflags"), _T("udo"));
        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), this->GetName() + _T("_button_max"));
        m_pButtonMax = K2_NEW(ctx_Widgets,  CScrollbarButton)(m_pInterface, this, SCROLLBAR_BUTTON_MAX, styleCopy);
        AddChild(m_pButtonMax);
    }
    else
    {
        styleCopy.SetProperty(_T("width"), GetBaseHeight());
        styleCopy.SetProperty(_T("height"), GetBaseHeight());
        styleCopy.SetProperty(_T("texture"), Filename_AppendSuffix(m_sTextureName[0], _T("_hmin")));
        styleCopy.SetProperty(_T("textureflags"), _T("udo"));
        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), this->GetName() + _T("_button_min"));
        m_pButtonMin = K2_NEW(ctx_Widgets,  CScrollbarButton)(m_pInterface, this, SCROLLBAR_BUTTON_MIN, styleCopy);
        AddChild(m_pButtonMin);

        styleCopy.SetProperty(_T("x"), GetBaseHeight());
        styleCopy.SetProperty(_T("width"), MAX(GetWidth() - GetHeight() * 2.0f, 0.0f));
        styleCopy.SetProperty(_T("texture"), Filename_AppendSuffix(m_sTextureName[0], _T("_hslider")));
        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), this->GetName() + _T("_slider"));
        m_pSlider = K2_NEW(ctx_Widgets,  CScrollbarSlider)(m_pInterface, this, styleCopy);
        m_pSlider->GetSlot()->SetHeight(GetHeight());
        AddChild(m_pSlider);

        styleCopy.SetProperty(_T("align"), _T("right"));
        styleCopy.SetProperty(_T("x"), _T("0"));
        styleCopy.SetProperty(_T("width"), GetBaseHeight());
        styleCopy.SetProperty(_T("texture"), Filename_AppendSuffix(m_sTextureName[0], _T("_hmax")));
        styleCopy.SetProperty(_T("textureflags"), _T("udo"));
        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), this->GetName() + _T("_button_max"));
        m_pButtonMax = K2_NEW(ctx_Widgets,  CScrollbarButton)(m_pInterface, this, SCROLLBAR_BUTTON_MAX, styleCopy);
        AddChild(m_pButtonMax);
    }

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CScrollbar::GetValue
  ====================*/
tstring CScrollbar::GetValue() const
{
    return m_pSlider->GetValue();
}


/*====================
  CScrollbar::GetValueFloat
  ====================*/
float   CScrollbar::GetValueFloat()
{
    return m_pSlider->GetValueFloat();
}


/*====================
  CScrollbar::SetValue
  ====================*/
void    CScrollbar::SetValue(float fValue)
{
    return m_pSlider->SetValue(fValue);
}


/*====================
  CScrollbar::SetHandleSize
  ====================*/
void    CScrollbar::SetHandleSize(float fPercent)
{
    return m_pSlider->SetSliderHandleSize(fPercent);
}


/*====================
  CScrollbar::SetMinValue
  ====================*/
void    CScrollbar::SetMinValue(float fMinValue)
{
    return m_pSlider->SetMinValue(fMinValue);
}


/*====================
  CScrollbar::SetMaxValue
  ====================*/
void    CScrollbar::SetMaxValue(float fMaxValue)
{
    return m_pSlider->SetMaxValue(fMaxValue);
}


/*====================
  CScrollbar::GetMaxValue
  ====================*/
float   CScrollbar::GetMaxValue()
{
    return m_pSlider->GetMaxValue();
}


/*====================
  CScrollbar::MouseDown
  ====================*/
void    CScrollbar::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button == BUTTON_WHEELUP)
        MinButtonCommand();
    else if (button == BUTTON_WHEELDOWN)
        MaxButtonCommand();
    if (button != BUTTON_MOUSEL || !Contains(v2CursorPos))
        return;
}


/*====================
  CScrollbar::MinButtonCommand
  ====================*/
void    CScrollbar::MinButtonCommand()
{
    float fValue = m_pSlider->GetValueFloat();
    float fMinValue = m_pSlider->GetMinValue();

    if (fValue > fMinValue)
        fValue -= m_fIncrement;

    if (fValue < fMinValue)
        fValue = fMinValue;

    m_pSlider->SetValue(fValue);
}


/*====================
  CScrollbar::MaxButtonCommand
  ====================*/
void    CScrollbar::MaxButtonCommand()
{
    float fValue = m_pSlider->GetValueFloat();
    float fMaxValue = m_pSlider->GetMaxValue();

    if (fValue < fMaxValue)
        fValue += m_fIncrement;

    if (fValue > fMaxValue)
        fValue = fMaxValue;

    m_pSlider->SetValue(fValue);
}


/*====================
  CScrollbar::DoChange
  ====================*/
void    CScrollbar::DoChange()
{
    if (!m_sOnChange.empty())
        UIScript.Evaluate(this, m_sOnChange);
}


/*====================
  CScrollbar::SliderChange
  ====================*/
void    CScrollbar::SliderChange()
{
    DoChange();
}


/*====================
  CScrollbar::RecalculateChildSize
  ====================*/
void    CScrollbar::RecalculateChildSize()
{
    if (HasFlags(WFLAG_ORIENT_VERTICAL))
        m_pSlider->SetBaseHeight(XtoA(GetHeight() - GetWidth() * 2.0f));
    else
        m_pSlider->SetBaseWidth(XtoA(GetWidth() - GetHeight() * 2.0f));

    IWidget::RecalculateChildSize();
}
