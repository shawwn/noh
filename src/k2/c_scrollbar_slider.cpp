// (C)2005 S2 Games
// c_scrollbar_slider.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_scrollbar_slider.h"
#include "c_scrollbar.h"
//=============================================================================

/*====================
  CScrollbarSlider::~CScrollbarSlider
  ====================*/
CScrollbarSlider::~CScrollbarSlider()
{
}


/*====================
  CScrollbarSlider::CScrollbarSlider
  ====================*/
CScrollbarSlider::CScrollbarSlider(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
CSlider(pInterface, pParent, style)
{
}


/*====================
  CScrollbarSlider::DoChange
  ====================*/
void    CScrollbarSlider::DoChange()
{
    static_cast<CScrollbar *>(m_pParent)->SliderChange();

    CSlider::DoChange();
}


/*====================
  CScrollbarSlider::MouseDown
  ====================*/
void    CScrollbarSlider::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (m_pParent == NULL || m_pParent->GetType() != WIDGET_SCROLLBAR)
    {
        CSlider::MouseDown(button, v2CursorPos);
        return;
    }

    CScrollbar *pScrollbar(static_cast<CScrollbar*>(m_pParent));

    if (button == BUTTON_WHEELUP)
        pScrollbar->MinButtonCommand();
    else if (button == BUTTON_WHEELDOWN)
        pScrollbar->MaxButtonCommand();

    if (button == BUTTON_WHEELUP || button == BUTTON_WHEELDOWN)
        return CSlider::MouseDown(button, v2CursorPos);
}
