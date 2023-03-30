// (C)2005 S2 Games
// c_scrollbar.h
//
//=============================================================================
#ifndef __C_SCROLLBAR_H__
#define __C_SCROLLBAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

class CSlider;
class CButton;

enum EScrollbarOrient
{
    SCROLLBAR_ORIENT_VERTICAL,
    SCROLLBAR_ORIENT_HORIZONTAL
};

//=============================================================================
// CScrollbar
//=============================================================================
class CScrollbar : public IWidget
{
protected:
    CSlider*    m_pSlider;
    CButton*    m_pButtonMin;
    CButton*    m_pButtonMax;

    float       m_fIncrement;

    tstring     m_sTextureSet;

    // event handlers
    tstring     m_sOnChange;

    virtual void    DoChange();

public:
    virtual ~CScrollbar();
    CScrollbar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    tstring GetValue() const;

    float   GetValueFloat();

    void    SetValue(float fValue);
    void    SetValue(const tstring &sValue)                     { SetValue(AtoF(sValue)); }
    void    SetHandleSize(float fPercent);
    void    SetMinValue(float fMinValue);
    void    SetMaxValue(float fMaxValue);

    float   GetMaxValue();

    virtual void    MouseDown(EButton button, const CVec2f &v2CursorPos);

    void    MinButtonCommand();
    void    MaxButtonCommand();
    void    SliderChange();

    void    RecalculateChildSize();
};
//=============================================================================

#endif //__C_SCROLLBAR_H__
