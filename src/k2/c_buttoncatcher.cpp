// (C)2007 S2 Games
// c_buttoncatcher.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_buttoncatcher.h"
#include "c_interface.h"
#include "c_widgetstyle.h"
#include "c_uitextureregistry.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
#include "c_input.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CButtonCatcher::CButtonCatcher
  ====================*/
CButtonCatcher::CButtonCatcher(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_BUTTONCATCHER, style),
m_eLastButton(BUTTON_INVALID),
m_iLastModifier(0),
m_bImpulse(style.GetPropertyBool(_T("impulse"), false))
{
    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)

    SetEventCommand(WEVENT_BUTTON, style.GetProperty(_T("onbutton")));
}


/*====================
  CButtonCatcher::MouseDown
  ====================*/
void    CButtonCatcher::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (m_bImpulse)
    {
        UpdateButtonName(button);
        DO_EVENT_PARAM(WEVENT_BUTTON, m_sLastButtonName)
    }
    else
    {
        m_sLastButtonName = Input.GetBindString(button, 0);
        DO_EVENT_PARAM(WEVENT_BUTTON, m_sLastButtonName)
    }
}


/*====================
  CButtonCatcher::UpdateButtonName
  ====================*/
void    CButtonCatcher::UpdateButtonName(EButton button)
{
    int iModifier(0);
    if (Input.IsAltDown())
        iModifier |= BIND_MOD_ALT;
    if (Input.IsCtrlDown())
        iModifier |= BIND_MOD_CTRL;
    if (Input.IsShiftDown())
        iModifier |= BIND_MOD_SHIFT;
#ifdef __APPLE__
    if (Input.IsCommandDown())
        iModifier |= BIND_MOD_CMD;
#endif

    m_eLastButton = button;
    m_iLastModifier = iModifier;
    m_sLastButtonName = Input.GetBindString(button, iModifier);
}


/*====================
  CButtonCatcher::ButtonDown
  ====================*/
bool    CButtonCatcher::ButtonDown(EButton button)
{
    if (m_bImpulse)
    {
        if (button == BUTTON_ALT || button == BUTTON_CTRL || button == BUTTON_SHIFT
#ifdef __APPLE__
            || button == BUTTON_CMD
#endif
            )
        {
            UpdateButtonName(BUTTON_UNSET);
            DO_EVENT_PARAM_RETURN(WEVENT_CHANGE, m_sLastButtonName, true)
            return true;
        }
        else
        {
            UpdateButtonName(button);
            DO_EVENT_PARAM_RETURN(WEVENT_BUTTON, m_sLastButtonName, true)
            return true;
        }
    }
    else
    {
        m_sLastButtonName = Input.GetBindString(button, 0);
        DO_EVENT_PARAM_RETURN(WEVENT_BUTTON, m_sLastButtonName, true)
        return true;
    }
}


/*====================
  CButtonCatcher::ButtonUp
  ====================*/
bool    CButtonCatcher::ButtonUp(EButton button)
{
    UpdateButtonName(m_eLastButton);
    DO_EVENT_PARAM_RETURN(WEVENT_CHANGE, m_sLastButtonName, true)
    return true;
}


/*====================
  CButtonCatcher::DoEvent
  ====================*/
void    CButtonCatcher::DoEvent(EWidgetEvent eEvent, const tstring &sParam)
{
    IWidget::DoEvent(eEvent, sParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;
    
    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (eEvent == WEVENT_SHOW)
        m_pInterface->SetActiveWidget(this);
    else if (eEvent == WEVENT_HIDE)
    {
        if (m_pInterface->GetActiveWidget() == this)
            m_pInterface->SetActiveWidget(NULL);
    }
}


/*====================
  CButtonCatcher::DoEvent
  ====================*/
void    CButtonCatcher::DoEvent(EWidgetEvent eEvent, const tsvector &vParam)
{
    IWidget::DoEvent(eEvent, vParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;
    
    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (eEvent == WEVENT_SHOW)
        m_pInterface->SetActiveWidget(this);
    else if (eEvent == WEVENT_HIDE)
    {
        if (m_pInterface->GetActiveWidget() == this)
            m_pInterface->SetActiveWidget(NULL);
    }
}

    
