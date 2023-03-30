// (C)2005 S2 Games
// c_scrollbar_button.h
//
//=============================================================================
#ifndef __C_SCROLLBAR_BUTTON_H__
#define __C_SCROLLBAR_BUTTON_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"

#include "c_button.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
enum EScrollbarButton
{
    SCROLLBAR_BUTTON_MIN = 0,
    SCROLLBAR_BUTTON_MAX
};
//=============================================================================

//=============================================================================
// CScrollbarButton
//=============================================================================
class CScrollbarButton : public CButton
{
private:
    EScrollbarButton    m_eType;

    uint                m_uiNextRepeat;
    uint                m_uiRepeatInterval;

public:
    ~CScrollbarButton() {}
    CScrollbarButton(CInterface *pInterface, IWidget *pParent, EScrollbarButton eType, const CWidgetStyle& style);

    void    Frame(uint uiFrameLength, bool bProcessFrame);
    void    MouseDown(EButton button, const CVec2f &v2CursorPos);
};
//=============================================================================

#endif // __C_SCROLLBAR_BUTTON_H__
