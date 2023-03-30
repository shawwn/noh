// (C)2005 S2 Games
// c_sliderhandle.h
//
//=============================================================================
#ifndef __C_SLIDERHANDLE_H__
#define __C_SLIDERHANDLE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "c_slider.h"

#include "../k2/c_input.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvar<int> ui_sliderSnap;
//=============================================================================

//=============================================================================
// CSliderHandle
//=============================================================================
class CSliderHandle : public IWidget
{
protected:
    bool    m_bIsSliding;

    CVec2f  m_vHandleConstraints;
    CVec2f  m_vLastPos;

    CVec2f  m_vecCursorStartPos;

    int     m_iNumSteps;

public:
    ~CSliderHandle()    {}
    CSliderHandle(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);


    void        MouseDown(EButton button, const CVec2f &v2CursorPos);
    void        MouseUp(EButton button, const CVec2f &v2CursorPos);

    void        BeginSlide(const CVec2f &v2CursorPos);
    void        EndSlide();
    bool        ProcessInputCursor(const CVec2f &v2CursorPos);
    void        DoSlide(const CVec2f &v2CursorPos);

    void        SetConstraints(float fUpper, float fLower)  { m_vHandleConstraints.Set(fUpper, fLower); }

    inline void SetHandleSize (float fPercent);
    inline void SetValue(float fValue);
    void        SetValue(const tstring &sValue)             { SetValue(AtoF(sValue)); }
    inline void SetPosition(float fValue);

    void        SetNumSteps(int iSteps)                     { m_iNumSteps = iSteps; }
};
//=============================================================================



/*====================
 CSliderHandle::SetHandleSize
 ====================*/
void    CSliderHandle::SetHandleSize(float fPercent)
{
    if (fPercent > 100.0f)
        fPercent = 100.0f;
    else if (fPercent < 0.0f)
        fPercent = 0.0f;

    if (m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL))
        m_recArea.SetSizeY(MAX(m_recArea.GetWidth(), ceilf((m_vHandleConstraints[1] - m_vHandleConstraints[0]) * (fPercent / 100.0f))));
    else
        m_recArea.SetSizeX(MAX(m_recArea.GetHeight(), ceilf((m_vHandleConstraints[1] - m_vHandleConstraints[0]) * (fPercent / 100.0f))));
}


/*====================
  CSliderHandle::SetValue
  ====================*/
void    CSliderHandle::SetValue(float fValue)
{
    if (m_bIsSliding)
        return;

    if (m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL))
    {
        float fNewYPos = LERP(fValue, m_vHandleConstraints[0], m_vHandleConstraints[1] - m_recArea.GetHeight());

        Move(m_recArea.left, fNewYPos);

        static_cast<CSlider *>(m_pParent)->SetSliderVar(fValue);
    }
    else
    {
        float fNewXPos = LERP(fValue, m_vHandleConstraints[0], m_vHandleConstraints[1] - m_recArea.GetWidth());

        Move(fNewXPos, m_recArea.top);

        static_cast<CSlider *>(m_pParent)->SetSliderVar(fValue);
    }
}


/*====================
  CSliderHandle::SetPosition
  ====================*/
void    CSliderHandle::SetPosition(float fValue)
{
    if (m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL))
    {
        float fNewYPos = LERP(fValue, m_vHandleConstraints[0], m_vHandleConstraints[1] - m_recArea.GetHeight());

        Move(m_recArea.left, fNewYPos);
    }
    else
    {
        float fNewXPos = LERP(fValue, m_vHandleConstraints[0], m_vHandleConstraints[1] - m_recArea.GetWidth());

        Move(fNewXPos, m_recArea.top);
    }
}

#endif //__C_SLIDERHANDLE_H__
