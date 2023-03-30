// (C)2006 S2 Games
// c_slider.h
//
//=============================================================================
#ifndef __C_SLIDER_H__
#define __C_SLIDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CConsoleElement;
class CPanel;
class CSliderHandle;
class ICvar;
//=============================================================================

//=============================================================================
// CSlider
//=============================================================================
class CSlider : public IWidget
{
    friend class CSliderHandle;

protected:
    CPanel*         m_pSlot;
    CSliderHandle*  m_pHandle;

    float           m_fValue;
    float           m_fMinValue;
    float           m_fMaxValue;

    float           m_fSlotWidth;
    float           m_fSlotHeight;
    float           m_fHandleWidth;
    float           m_fHandleHeight;

    tstring         m_sSlotWidth;
    tstring         m_sSlotHeight;
    tstring         m_sHandleWidth;
    tstring         m_sHandleHeight;

    float           m_fStep;

    tstring         m_sCvar;
    CCvarReference  m_refCvar;

    // event handlers
    tstring     m_sOnChange;

    virtual void    DoChange();

    void    SetSliderVar(float fPos, float fUnits)
    {
        float fValue = LERP(fPos / fUnits, m_fMinValue, m_fMaxValue);

        m_fValue = fValue;

        if (!m_refCvar.IsIgnored() && !FLOAT_EQUALS(m_refCvar.GetFloat(), fValue))
            m_refCvar.Set(fValue);
    }

    void    SetSliderVar(float fLerp)
    {
        float fValue = LERP(fLerp, m_fMinValue, m_fMaxValue);

        m_fValue = fValue;

        if (!m_refCvar.IsIgnored() && !FLOAT_EQUALS(m_refCvar.GetFloat(), fValue))
            m_refCvar.Set(fValue);
    }

public:
    virtual ~CSlider();
    K2_API CSlider(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    CPanel*         GetSlot() const                                     { return m_pSlot; }
    CSliderHandle*  GetHandle() const                                   { return m_pHandle; }

    void            SetMinValue(float fMinValue);
    void            SetMaxValue(float fMaxValue);

    float           GetMinValue() const                                 { return m_fMinValue; }
    float           GetMaxValue() const                                 { return m_fMaxValue; }
    float           GetStep() const                                     { return m_fStep; }

    void            MouseDown(EButton button, const CVec2f &v2CursorPos);
    bool            ButtonUp(EButton button);

    void            SetSliderHandleSize(float fPercent);

    void            SetValue(float fValue);
    void            SetValue(const tstring &sValue)                     { SetValue(AtoF(sValue)); }
    tstring         GetValue() const                                    { return XtoA(m_fValue); }
    float           GetValueFloat()                                     { return m_fValue; }

    void            Frame(uint uiFrameLength, bool bProcessFrame);

    void            RecalculateChildSize();
};
//=============================================================================

#endif // __C_SLIDER_H__
