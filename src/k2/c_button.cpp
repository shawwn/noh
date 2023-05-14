// (C)2007 S2 Games
// c_button.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_button.h"
#include "c_interface.h"
#include "c_uitextureregistry.h"
#include "c_uiscript.h"
#include "c_uicmd.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
#include "c_widgetstate.h"
//=============================================================================

/*====================
  CButton::CButton
  ====================*/
CButton::~CButton()
{
    for (uint uiState(0); uiState < m_uiNumStates; ++uiState)
    {
        for (uint uiGroup(0); uiGroup < NUM_UI_BUTTON_TEXTURES; ++uiGroup)
            SAFE_DELETE(m_vStateGroups[uiState][uiGroup]);
    }
}


/*====================
  CButton::CButton
  ====================*/
CButton::CButton(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_BUTTON, style, false),
m_bPressed(false),
m_uiState(0),
m_refCvar(style.GetProperty(_T("cvar"))),
m_uiNumStates(MAX(style.GetPropertyInt(_T("numstates"), 1), 1)),
m_vStateGroups(m_uiNumStates, ButtonStateGroupVector(NUM_UI_BUTTON_TEXTURES, nullptr)),
m_eActiveTexture(UI_BUTTON_TEXTURE_UP)
{
    SetFlags(WFLAG_PASSIVE_CHILDREN);

    if (style.GetPropertyBool(_T("interactive"), true))
        SetFlags(WFLAG_INTERACTIVE);

    tstring sFlags(style.GetProperty(_T("textureflags"), _T("udox")));
    tstring sBaseTextureName(m_sTextureName.empty() ? TSNULL : m_sTextureName[0]);
    ButtonTextureVector textures(NUM_UI_BUTTON_TEXTURES, INVALID_RESOURCE);
    for (uint ui(0); ui < m_uiNumStates; ++ui)
    {
        m_vTextureSets.push_back(textures);
        ProcessFlags(sFlags, ui);

        if (!m_sTextureName.empty())
            m_sTextureName[0] = sBaseTextureName;
    }

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)

    if (!m_refCvar.IsIgnored())
        m_uiState = CLAMP(m_refCvar.GetInt(), 0, int(m_uiNumStates - 1));

    if (HasFlags(WFLAG_ENABLED))
        SetActiveTexture(UI_BUTTON_TEXTURE_UP);
    else
        SetActiveTexture(UI_BUTTON_TEXTURE_DISABLED);
}


/*====================
  CButton::ProcessFlags
  ====================*/
void    CButton::ProcessFlags(const tstring &sTextureFlags, uint uiState)
{
    // Remember the base texture name
    tstring sBaseTextureName(m_sTextureName.empty() ? TSNULL : m_sTextureName[0]);
    if (sBaseTextureName.empty())
        return;

    for (uint uiFlag(0); uiFlag < sTextureFlags.size(); ++uiFlag)
    {
        // Determine texture index of this flag, if any
        uint uiIndex(0);
        for (; uiIndex < NUM_UI_BUTTON_TEXTURES; ++uiIndex)
        {
            if (g_acButtonTextureFlags[uiIndex] == sTextureFlags[uiFlag])
                break;
        }
        if (uiIndex == NUM_UI_BUTTON_TEXTURES)
            continue;

        // Try to load the texture
        tstring sSuffix(g_asButtonTextureSuffixes[uiIndex]);
        if (uiState != 0)
            sSuffix += XtoA(uiState + 1);
        SetTexture(sBaseTextureName, sSuffix);
        m_vTextureSets[uiState][uiIndex] = m_hTexture[0];
    }

    // If the "up" texture didn't load, try the plain base texture name
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] == INVALID_RESOURCE &&
        FileManager.Exists(sBaseTextureName))
    {
        SetTexture(sBaseTextureName);
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] = m_hTexture[0];
    }

    // Try to set missing textures to something valid
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] == INVALID_RESOURCE)
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] = m_vTextureSets[uiState][UI_BUTTON_TEXTURE_OVER];
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] == INVALID_RESOURCE)
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] = m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DOWN];
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] == INVALID_RESOURCE)
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP] = m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DISABLED];
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_OVER] == INVALID_RESOURCE)
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_OVER] = m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP];
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DOWN] == INVALID_RESOURCE)
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DOWN] = m_vTextureSets[uiState][UI_BUTTON_TEXTURE_OVER];
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DOWNOFF] == INVALID_RESOURCE)
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DOWNOFF] = m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP];
    if (m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DISABLED] == INVALID_RESOURCE)
        m_vTextureSets[uiState][UI_BUTTON_TEXTURE_DISABLED] = m_vTextureSets[uiState][UI_BUTTON_TEXTURE_UP];

    SetActiveTexture(UI_BUTTON_TEXTURE_UP);
}


/*====================
  CButton::ButtonDown
  ====================*/
bool    CButton::ButtonDown(EButton button)
{
    switch (button)
    {
    case BUTTON_ENTER:
        {
            m_pInterface->SetExclusiveWidget(nullptr);
            m_pInterface->SetActiveWidget(nullptr);

            uint uiState(m_uiState + 1 >= m_uiNumStates ? 0 : m_uiState + 1);
            SetState(uiState);
            SetActiveTexture(UI_BUTTON_TEXTURE_UP);
            DO_EVENT_RETURN(WEVENT_CLICK, true)
        }
        break;

    default:
        break;
    }

    return true;
}


/*====================
  CButton::Enable
  ====================*/
void    CButton::Enable()
{
    if (!HasFlags(WFLAG_ENABLED))
        SetActiveTexture(UI_BUTTON_TEXTURE_UP);

    IWidget::Enable();
    
}


/*====================
  CButton::Disable
  ====================*/
void    CButton::Disable()
{
    if (HasFlags(WFLAG_ENABLED))
        SetActiveTexture(UI_BUTTON_TEXTURE_DISABLED);

    IWidget::Disable(); 
}


/*====================
  CButton::Rollover
  ====================*/
void    CButton::Rollover()
{
    IWidget::Rollover();

    if (m_bPressed)
        SetActiveTexture(UI_BUTTON_TEXTURE_DOWN);
    else
        SetActiveTexture(UI_BUTTON_TEXTURE_OVER);
}


/*====================
  CButton::Rolloff
  ====================*/
void    CButton::Rolloff()
{
    IWidget::Rolloff();

    if (m_bPressed)
        SetActiveTexture(UI_BUTTON_TEXTURE_DOWNOFF);
    else
        SetActiveTexture(UI_BUTTON_TEXTURE_UP);
}


/*====================
  CButton::MouseDown
  ====================*/
void    CButton::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button != BUTTON_MOUSEL && button != BUTTON_MOUSER)
        return;

    m_pInterface->SetExclusiveWidget(this);

    m_v2LastCursorPos.Set(v2CursorPos.x, v2CursorPos.y);
    SetActiveTexture(UI_BUTTON_TEXTURE_DOWN);
    m_bPressed = true;

    if (button == BUTTON_MOUSEL)
    {
        DO_EVENT(WEVENT_MOUSELDOWN);
    }
    else
    {
        DO_EVENT(WEVENT_MOUSERDOWN);
    }
}


/*====================
  CButton::MouseUp
  ====================*/
void    CButton::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    if (button != BUTTON_MOUSEL && button != BUTTON_MOUSER)
        return;

    if (!m_bPressed)
        return;

    m_pInterface->SetExclusiveWidget(nullptr);
    m_pInterface->SetActiveWidget(nullptr);

    m_v2LastCursorPos.Set(-1, -1);

    m_bPressed = false;

    if (m_recArea.AltContains(v2CursorPos))
    {
        uint uiPrevState(GetState());

        if (button == BUTTON_MOUSEL)
        {
            uint uiState(m_uiState + 1 >= m_uiNumStates ? 0 : m_uiState + 1);
            SetState(uiState);

            DO_EVENT_PARAM(WEVENT_CLICK, XtoA(uiPrevState))
        }
        else if (button == BUTTON_MOUSER)
            DO_EVENT_PARAM(WEVENT_RIGHTCLICK, XtoA(uiPrevState))

        if (HasFlags(WFLAG_ENABLED))
            SetActiveTexture(UI_BUTTON_TEXTURE_OVER);
    }
    else
    {
        SetActiveTexture(UI_BUTTON_TEXTURE_UP);
    }
}


/*====================
  CButton::Frame
  ====================*/
void    CButton::Frame(uint uiFrameLength, bool bProcessFrame)
{
    if (!HasFlags(WFLAG_ENABLED))
        return;

    if (!m_refCvar.IsIgnored())
    {
        uint uiValue(CLAMP(m_refCvar.GetInt(), 0, int(m_uiNumStates - 1)));

        if (uiValue != m_uiState)
        {
            m_uiState = uiValue;
            SetActiveTexture(UI_BUTTON_TEXTURE_UP);
        }
    }

    IWidget::Frame(uiFrameLength, bProcessFrame);
}


/*====================
  CButton::Render
  ====================*/
void    CButton::Render(const CVec2f &vOrigin, int iFlag, float fFade)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return;

    if ((HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_TOP)) ||
        (!HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_BOTTOM)))
    {
        IWidget::RenderWidget(vOrigin + m_recArea.lt(), fFade * m_fFadeCurrent);

        CWidgetState *pState(m_vStateGroups[m_uiState][m_eActiveTexture]);

        if (m_eActiveTexture == UI_BUTTON_TEXTURE_UP)
        {
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_OVER];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_DOWN];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_DISABLED];
        }
        else if (m_eActiveTexture == UI_BUTTON_TEXTURE_OVER)
        {
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_UP];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_DOWN];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_DISABLED];
        }
        else if (m_eActiveTexture == UI_BUTTON_TEXTURE_DOWN)
        {
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_OVER];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_UP];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_DISABLED];
        }
        else if (m_eActiveTexture == UI_BUTTON_TEXTURE_DOWNOFF)
        {
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_UP];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_OVER];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_DISABLED];
        }
        else if (m_eActiveTexture == UI_BUTTON_TEXTURE_DISABLED)
        {
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_UP];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_DOWN];
            if (!pState) pState = m_vStateGroups[m_uiState][UI_BUTTON_TEXTURE_OVER];
        }

        if (pState)
            pState->Render(vOrigin + m_recArea.lt(), WIDGET_RENDER_ALL, fFade * m_fFadeCurrent);
    }

    // Render children
    for (WidgetPointerVector_cit it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
        (*it)->Render(vOrigin + m_recArea.lt(), iFlag, fFade * m_fFadeCurrent);
}


/*====================
  CButton::AddWidgetState
  ====================*/
bool    CButton::AddWidgetState(CWidgetState *pState)
{
    for (uint uiState(0); uiState < m_uiNumStates; ++uiState)
    {
        for (uint uiGroup(0); uiGroup < NUM_UI_BUTTON_TEXTURES; ++uiGroup)
        {
            if ((uiState == 0 && pState->GetStateName() == g_asButtonStateGroupNames[uiGroup]) ||
                pState->GetStateName() == g_asButtonStateGroupNames[uiGroup] + XtoA(uiState + 1))
            {
                m_vStateGroups[uiState][uiGroup] = pState;
                return true;
            }
        }
    }

    // Delete this widget state if we don't end up using it
    SAFE_DELETE(pState);
    return false;
}


/*====================
  CButton::SetState
  ====================*/
void    CButton::SetState(uint uiState)
{
    uiState = MIN(uiState, m_uiNumStates - 1);

    if (uiState == m_uiState)
        return;

    if (!m_refCvar.IsIgnored())
    {
        if (m_refCvar.GetInt() != uiState)
            m_refCvar.Set(int(uiState));
    }

    m_uiState = uiState;
}


/*====================
  CButton::DoEvent
  ====================*/
void    CButton::DoEvent(EWidgetEvent eEvent, const tstring &sParam)
{
    IWidget::DoEvent(eEvent, sParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;
    
    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (WIDGET_EVENT_RECURSIVE[eEvent])
    {
        for (uint uiState(0); uiState < m_uiNumStates; ++uiState)
        {
            for (uint uiGroup(0); uiGroup < NUM_UI_BUTTON_TEXTURES; ++uiGroup)
            {
                if (m_vStateGroups[uiState][uiGroup])
                {
                    m_vStateGroups[uiState][uiGroup]->DoEvent(eEvent, sParam);

                    if (m_uiFlags & WFLAG_RELEASED)
                        return;
                }
            }
        }
    }
}


/*====================
  CButton::DoEvent
  ====================*/
void    CButton::DoEvent(EWidgetEvent eEvent, const tsvector &vParam)
{
    IWidget::DoEvent(eEvent, vParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;
    
    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (WIDGET_EVENT_RECURSIVE[eEvent])
    {
        for (uint uiState(0); uiState < m_uiNumStates; ++uiState)
        {
            for (uint uiGroup(0); uiGroup < NUM_UI_BUTTON_TEXTURES; ++uiGroup)
            {
                if (m_vStateGroups[uiState][uiGroup])
                {
                    m_vStateGroups[uiState][uiGroup]->DoEvent(eEvent, vParam);

                    if (HasFlags(WFLAG_RELEASED))
                        return;
                }
            }
        }
    }
}


/*--------------------
  SetButtonState
  --------------------*/
UI_VOID_CMD(SetButtonState, 1)
{
    if (pThis->GetType() != WIDGET_BUTTON)
        return;

    static_cast<CButton*>(pThis)->SetState(vArgList[0]->EvaluateInteger());
}


/*--------------------
  GetButtonState
  --------------------*/
UI_CMD(GetButtonState, 0)
{
    if (pThis->GetType() != WIDGET_BUTTON)
        return 0;

    return XtoA(static_cast<CButton*>(pThis)->GetState());
}



/*====================
  CButton::RecalculateSize
  ====================*/
void    CButton::RecalculateSize()
{
    IWidget::RecalculateSize();
    
    for (VButtonStateGroupVector_it itGroup(m_vStateGroups.begin()), itGroupEnd(m_vStateGroups.end()); itGroup != itGroupEnd; ++itGroup)
    {
        for (ButtonStateGroupVector_it itState(itGroup->begin()), itStateEnd(itGroup->end()); itState != itStateEnd; ++itState)
        {
            if (*itState != nullptr)
                (*itState)->RecalculateSize();
        }
    }
}


/*====================
  CButton::RecalculatePosition
  ====================*/
void    CButton::RecalculatePosition()
{
    IWidget::RecalculatePosition();
    for (VButtonStateGroupVector::iterator itStateGroup(m_vStateGroups.begin()); itStateGroup != m_vStateGroups.end(); ++itStateGroup)
    {
        for (ButtonStateGroupVector::iterator itState(itStateGroup->begin()); itState != itStateGroup->end(); ++itState)
        {
            if (*itState != nullptr)
                (*itState)->RecalculatePosition();
        }
    }
}
