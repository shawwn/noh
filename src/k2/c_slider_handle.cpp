// (C)2005 S2 Games
// c_slider_handle.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_slider_handle.h"
#include "c_interface.h"
#include "c_panel.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"

#include "../k2/c_cmd.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_INT(ui_sliderSnap, 128);
//=============================================================================

/*====================
  CSliderHandle::CSliderHandle
  ====================*/
CSliderHandle::CSliderHandle(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_SLIDERHANDLE, style),
m_bIsSliding(false),
m_vHandleConstraints(-1.0f, -1.0f),
m_vLastPos(-1.0f, -1.0f),
m_vecCursorStartPos(-1.0f, -1.0f),
m_iNumSteps(style.GetPropertyInt(_T("steps")))
{
    SetFlags(WFLAG_INTERACTIVE);
    SetFlagsRecursive(WFLAG_PROCESS_CURSOR);

    SetEventCommand(WEVENT_REFRESH, style.GetProperty(_T("onrefresh")));
}


/*====================
  CSliderHandle::ProcessInputCursor
  ====================*/
bool    CSliderHandle::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    if (m_bIsSliding)
    {
        DoSlide(v2CursorPos);
        return true;
    }

    return false;
}


/*====================
  CSliderHandle::MouseDown
  ====================*/
void    CSliderHandle::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    switch (button)
    {
    case BUTTON_MOUSEL:
        m_pInterface->SetExclusiveWidget(this);
        BeginSlide(v2CursorPos);
        break;
    case BUTTON_WHEELDOWN:
    case BUTTON_WHEELUP:
        m_pParent->MouseDown(button, v2CursorPos);
        break;
    default:
        break;
    }
}


/*====================
  CSliderHandle::MouseUp
  ====================*/
void    CSliderHandle::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    switch (button)
    {
    case BUTTON_MOUSEL:
        EndSlide();
        break;
    default:
        break;
    }
}


/*====================
  CSliderHandle::BeginSlide
  ====================*/
void    CSliderHandle::BeginSlide(const CVec2f &v2CursorPos)
{
    m_bIsSliding = true;

    m_vLastPos = m_recArea.lt();
    m_vecCursorStartPos = v2CursorPos;

    m_pParent->DoEvent(WEVENT_STARTDRAG);
}


/*====================
  CSliderHandle::EndSlide
  ====================*/
void     CSliderHandle::EndSlide()
{
    if (m_bIsSliding)
    {
        m_bIsSliding = false;
        m_vLastPos.Set(-1.0f, -1.0f);
        m_pInterface->SetExclusiveWidget(nullptr);
        m_pInterface->SetActiveWidget(nullptr);

        if (m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL))
        {
            float fNewYPos(m_recArea.top);

            // Keep slider handle within slot constraints
            if (fNewYPos < m_vHandleConstraints[0])
                fNewYPos = m_vHandleConstraints[0];

            if (fNewYPos + m_recArea.GetHeight() >= m_vHandleConstraints[1])
                fNewYPos = m_vHandleConstraints[1] - m_recArea.GetHeight();

            // Check for steps
            if (m_iNumSteps != 0 && fNewYPos != m_vHandleConstraints[1] - m_recArea.GetHeight())
            {
                float fLength((m_vHandleConstraints[1] - m_recArea.GetHeight()) - m_vHandleConstraints[0]);
                float fIncrement(fLength / m_iNumSteps);
                float fProgress(fmod(fNewYPos - m_vHandleConstraints[0], fIncrement));
                if (fProgress / fIncrement < 0.5f)
                    fNewYPos -= fProgress;
                else
                    fNewYPos += (fIncrement - fProgress);
            }

            Move(m_recArea.left, fNewYPos);

            // set cvar depending on the slider value
            static_cast<CSlider *>(m_pParent)->SetSliderVar((fNewYPos - m_vHandleConstraints[0]) /
                (m_vHandleConstraints[1] - m_vHandleConstraints[0] - m_recArea.GetHeight()));

            m_pParent->DoEvent(WEVENT_SLIDE);
        }
        else
        {
            float fNewXPos(m_recArea.left);

            // keep slider handle within slot constraints
            if (fNewXPos < m_vHandleConstraints[0])
                fNewXPos = m_vHandleConstraints[0];

            if (fNewXPos + m_recArea.GetWidth() >= m_vHandleConstraints[1])
                fNewXPos = m_vHandleConstraints[1] - m_recArea.GetWidth();

            if (m_iNumSteps != 0 && fNewXPos != m_vHandleConstraints[1] - m_recArea.GetWidth())
            {
                float fLength((m_vHandleConstraints[1] - m_recArea.GetWidth()) - m_vHandleConstraints[0]);
                float fIncrement(fLength / m_iNumSteps);
                float fProgress(fmod(fNewXPos - m_vHandleConstraints[0], fIncrement));
                if (fProgress / fIncrement < 0.5f)
                    fNewXPos -= fProgress;
                else
                    fNewXPos += (fIncrement - fProgress);
            }

            Move(fNewXPos, m_recArea.top);

            // set cvar depending on the slider value
            static_cast<CSlider *>(m_pParent)->SetSliderVar((fNewXPos - m_vHandleConstraints[0]) /
                (m_vHandleConstraints[1] - m_vHandleConstraints[0] - m_recArea.GetWidth()));

            m_pParent->DoEvent(WEVENT_SLIDE);
        }

        m_pParent->DoEvent(WEVENT_ENDDRAG);
    }
}


/*====================
  CSliderHandle::DoSlide
  ====================*/
void    CSliderHandle::DoSlide(const CVec2f &v2CursorPos)
{
    CVec2f v2OldPos(m_recArea.lt());

    // move the slider back to last known position and end dragging if user strays too far from slider
    if ((m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL) &&
            (v2CursorPos.x < m_recArea.left - ui_sliderSnap ||
            v2CursorPos.x > m_recArea.right + ui_sliderSnap)) ||
        (!m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL) &&
            (v2CursorPos.y < m_recArea.top - ui_sliderSnap ||
            v2CursorPos.y > m_recArea.bottom + ui_sliderSnap)))
    {
        Move(m_vLastPos.x, m_vLastPos.y);

        if (m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL))
        {
            static_cast<CSlider*>(m_pParent)->SetSliderVar(m_vLastPos.y - m_vHandleConstraints[0],
                m_vHandleConstraints[1] - m_vHandleConstraints[0] - m_recArea.GetHeight());
        }
        else
        {
            static_cast<CSlider*>(m_pParent)->SetSliderVar(m_vLastPos.x - m_vHandleConstraints[0],
                m_vHandleConstraints[1] - m_vHandleConstraints[0] - m_recArea.GetWidth());
        }

        m_pParent->DoEvent(WEVENT_SLIDE);
    }
    else if (m_pParent->HasFlags(WFLAG_ORIENT_VERTICAL))
    {
        float fNewYPos(m_vLastPos.y + (v2CursorPos.y - m_vecCursorStartPos.y));

        // Keep slider handle within slot constraints
        if (fNewYPos < m_vHandleConstraints[0])
            fNewYPos = m_vHandleConstraints[0];

        if (fNewYPos + m_recArea.GetHeight() >= m_vHandleConstraints[1])
            fNewYPos = m_vHandleConstraints[1] - m_recArea.GetHeight();

        // Check for steps
        if (m_iNumSteps != 0 && fNewYPos != m_vHandleConstraints[1] - m_recArea.GetHeight())
        {
            float fLength((m_vHandleConstraints[1] - m_recArea.GetHeight()) - m_vHandleConstraints[0]);
            float fIncrement(fLength / m_iNumSteps);
            float fProgress(fmod(fNewYPos - m_vHandleConstraints[0], fIncrement));
            if (fProgress / fIncrement < 0.5f)
                fNewYPos -= fProgress;
            else
                fNewYPos += (fIncrement - fProgress);
        }

        Move(m_recArea.left, fNewYPos);

        // set cvar depending on the slider value
        static_cast<CSlider *>(m_pParent)->SetSliderVar((fNewYPos - m_vHandleConstraints[0]) /
            (m_vHandleConstraints[1] - m_vHandleConstraints[0] - m_recArea.GetHeight()));

        m_pParent->DoEvent(WEVENT_SLIDE);
    }
    else
    {
        float fNewXPos(m_vLastPos.x + (v2CursorPos.x - m_vecCursorStartPos.x));

        // keep slider handle within slot constraints
        if (fNewXPos < m_vHandleConstraints[0])
            fNewXPos = m_vHandleConstraints[0];

        if (fNewXPos + m_recArea.GetWidth() >= m_vHandleConstraints[1])
            fNewXPos = m_vHandleConstraints[1] - m_recArea.GetWidth();

        if (m_iNumSteps != 0 && fNewXPos != m_vHandleConstraints[1] - m_recArea.GetWidth())
        {
            float fLength((m_vHandleConstraints[1] - m_recArea.GetWidth()) - m_vHandleConstraints[0]);
            float fIncrement(fLength / m_iNumSteps);
            float fProgress(fmod(fNewXPos - m_vHandleConstraints[0], fIncrement));
            if (fProgress / fIncrement < 0.5f)
                fNewXPos -= fProgress;
            else
                fNewXPos += (fIncrement - fProgress);
        }

        Move(fNewXPos, m_recArea.top);

        // set cvar depending on the slider value
        static_cast<CSlider *>(m_pParent)->SetSliderVar((fNewXPos - m_vHandleConstraints[0]) /
            (m_vHandleConstraints[1] - m_vHandleConstraints[0] - m_recArea.GetWidth()));

        m_pParent->DoEvent(WEVENT_SLIDE);
    }

    if (m_recArea.lt() != v2OldPos)
        static_cast<CSlider *>(m_pParent)->DoChange();
}
