// (C)2005 S2 Games
// c_scrollbar_slider.h
//
//=============================================================================
#ifndef __C_SCROLLBAR_SLIDER_H__
#define __C_SCROLLBAR_SLIDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"

#include "c_slider.h"
//=============================================================================

//=============================================================================
// CScrollbarSlider
//=============================================================================
class CScrollbarSlider : public CSlider
{
protected:
    void DoChange();

public:
    ~CScrollbarSlider();
    CScrollbarSlider(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    void    MouseDown(EButton button, const CVec2f &v2CursorPos);
};
//=============================================================================

#endif // __C_SCROLLBAR_SLIDER_H__
