// (C)2005 S2 Games
// i_widget.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_widget.h"
#include "c_interface.h"
#include "c_uitextureregistry.h"
#include "c_uiscript.h"
#include "c_uitrigger.h"
#include "c_uitriggerregistry.h"
#include "c_uicmd.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
#include "c_widgetstate.h"
#include "c_widgetreference.h"
#include "c_uiform.h"
#include "c_resourcemanager.h"

#include "../k2/c_cmd.h"
#include "../k2/c_filemanager.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
int IWidget::s_iNumWidgets(0);
//=============================================================================

//=============================================================================
// Private Helper Functions
//=============================================================================

/*====================
  AtoAlign
  ====================*/
static EAlignment       AtoAlign(const tstring &s)
{
    if (s == _T("center"))
        return ALIGN_CENTER;
    if (s == _T("right"))
        return ALIGN_RIGHT;
    return ALIGN_LEFT;
}


/*====================
  AtoVAlign
  ====================*/
static EAlignment       AtoVAlign(const tstring &s)
{
    if (s == _T("center"))
        return ALIGN_CENTER;
    if (s == _T("bottom"))
        return ALIGN_BOTTOM;
    return ALIGN_TOP;
}


/*====================
  AlignToA
  ====================*/
static tstring      AlignToA(EAlignment eAlignment)
{
    if (eAlignment == ALIGN_CENTER)
        return _T("center");
    if (eAlignment == ALIGN_RIGHT)
        return _T("right");
    return _T("left");
}


/*====================
  VAlignToA
  ====================*/
static tstring      VAlignToA(EAlignment eVAlignment)
{
    if (eVAlignment == ALIGN_CENTER)
        return _T("center");
    if (eVAlignment == ALIGN_BOTTOM)
        return _T("bottom");
    return _T("top");
}

//=============================================================================

/*====================
  IWidget::IWidget
  ====================*/
IWidget::IWidget(CInterface *pInterface, IWidget *pParent, EWidgetType widgetType, const CWidgetStyle &style, bool bLoadTextures) :
m_pParent(pParent),
m_pInterface(pInterface),
m_sName(style.GetProperty(_CWS("name"))),
m_sResourceContext(Trim(style.GetProperty(_CWS("resourcecontext")))),
m_uiID(INVALID_INDEX),
m_fUOffset(0.0f),
m_fVOffset(0.0f),
m_fUScale(1.0f),
m_fVScale(1.0f),
m_fUSpeed(0.0f),
m_fVSpeed(0.0f),
m_fRotation(0.0f),
m_fPadding(0.0f),
m_fSpacing(0.0f),
m_uiFlags(0),
m_uiTabOrder(style.GetPropertyInt(_CWS("taborder"), -1)),
m_eWidgetType(widgetType),
m_eFloat(WFLOAT_NONE),
m_eAdhere(WFLOAT_NONE),
m_eAlignment(ALIGN_LEFT),
m_eVAlignment(ALIGN_TOP),
#pragma warning(push)
#pragma warning(disable:4355)
m_refStickyTarget(this),
#pragma warning(pop)
m_fLastWidth(0),
m_fLastHeight(0),
m_sBaseX(style.GetProperty(_CWS("x"))),
m_sBaseY(style.GetProperty(_CWS("y"))),

m_fCropS0(0.0f), m_fCropS1(1.0f),
m_fCropT0(0.0f), m_fCropT1(1.0f),
m_fStartX(0.0f),
m_fStartY(0.0f),
m_fStartRotation(0.0f),
m_fStartWidth(0.0f),
m_fStartHeight(0.0f),
m_fTargetX(0.0f),
m_fTargetY(0.0f),
m_fTargetRotation(0.0f),
m_fTargetWidth(0.0f),
m_fTargetHeight(0.0f),
m_uiTargetTimeWidth(INVALID_TIME),
m_uiMoveStartTimeWidth(INVALID_TIME),
m_uiTargetTimeHeight(INVALID_TIME),
m_uiMoveStartTimeHeight(INVALID_TIME),
m_uiTargetTimeRotation(INVALID_TIME),
m_uiMoveStartTimeRotation(INVALID_TIME),
m_uiTargetTimeX(INVALID_TIME),
m_uiTargetTimeY(INVALID_TIME),
m_uiMoveStartTimeX(INVALID_TIME),
m_uiMoveStartTimeY(INVALID_TIME),
m_iDirectionHeight(0),
m_iDirectionWidth(0),
m_uiHideTime(-1),
m_uiWakeTime(-1),
#pragma warning(push)
#pragma warning(disable:4355)
m_refFloatTarget(this),
m_refAdhereTarget(this),
#pragma warning(pop)
m_pLerp(0),
m_fFadeCurrent(1.0f),
m_fFadeStart(1.0f),
m_fFadeEnd(1.0f),
m_uiFadeStartTime(INVALID_TIME),
m_uiFadeEndTime(INVALID_TIME),
m_v2LastCursorPos(-1, -1),
m_vvWatched(10),
m_bMouseOut(true)
{
    PROFILE("IWidget::IWidget");

    // Flags
    #define SET_WIDGET_FLAG(name, flag, def)    if (style.GetPropertyBool(_CWS(#name), def)) SetFlags(WFLAG_##flag)
    SET_WIDGET_FLAG(visible, VISIBLE, true);
    SET_WIDGET_FLAG(enabled, ENABLED, true);
    SET_WIDGET_FLAG(noclick, NO_CLICK, false);
    SET_WIDGET_FLAG(grow, GROW_WITH_CHILDREN, false);
    SET_WIDGET_FLAG(regrow, REGROW, false);
    SET_WIDGET_FLAG(growinvis, GROW_WITH_INVIS, true);
    SET_WIDGET_FLAG(passivechildren, PASSIVE_CHILDREN, false);
    SET_WIDGET_FLAG(vertical, ORIENT_VERTICAL, false);
    SET_WIDGET_FLAG(cangrab, CAN_GRAB, false);
    SET_WIDGET_FLAG(blockinput, BLOCK_INPUT, false);
    SET_WIDGET_FLAG(reverse, REVERSE, false);
    SET_WIDGET_FLAG(hflip, HFLIP, false);
    SET_WIDGET_FLAG(vflip, VFLIP, false);
    SET_WIDGET_FLAG(sticky, STICKY, false);
    SET_WIDGET_FLAG(stickysizing, STICKYSIZING, false);
    SET_WIDGET_FLAG(stickytoinvis, STICKYTOINVIS, true);
    SET_WIDGET_FLAG(resizeparentwidth, RESIZE_PARENT_WIDTH, false);
    SET_WIDGET_FLAG(resizeparentheight, RESIZE_PARENT_HEIGHT, false);
    SET_WIDGET_FLAG(utile, UTILE, false);
    SET_WIDGET_FLAG(vtile, VTILE, false);
    #undef SET_WIDGET_FLAG

    if (!HasFlags(WFLAG_VISIBLE))
        m_fFadeCurrent = 0.0f;

    // Hotkey
    const tstring &sHotkey(style.GetProperty(_CWS("hotkey")));
    m_eHotkey = sHotkey.empty() ? BUTTON_INVALID : Input.MakeEButton(sHotkey);

    // Floating
    const tstring &sFloating(LowerString(style.GetProperty(_CWS("float"))));
    if (TStringCompare(sFloating, _T("right")) == 0)
        m_eFloat = WFLOAT_RIGHT;
    else if (TStringCompare(sFloating, _T("bottom")) == 0)
        m_eFloat = WFLOAT_BOTTOM;

    // Alignment
    const tstring &sAlignment(LowerString(style.GetProperty(_CWS("align"))));
    if (TStringCompare(sAlignment, _T("center")) == 0)
        m_eAlignment = ALIGN_CENTER;
    else if (TStringCompare(sAlignment, _T("right")) == 0)
        m_eAlignment = ALIGN_RIGHT;

    const tstring &sVAlignment(LowerString(style.GetProperty(_CWS("valign"))));
    if (TStringCompare(sVAlignment, _T("center")) == 0)
        m_eVAlignment = ALIGN_CENTER;
    else if (TStringCompare(sVAlignment, _T("bottom")) == 0)
        m_eVAlignment = ALIGN_BOTTOM;

    m_sWidth = style.GetProperty(_CWS("width"), HasFlags(WFLAG_GROW_WITH_CHILDREN) ? _CWS("0") : _CWS("100%"));
    m_sHeight = style.GetProperty(_CWS("height"), HasFlags(WFLAG_GROW_WITH_CHILDREN) ? _CWS("0") : _CWS("100%"));

    m_sMarginH = style.GetProperty(_CWS("hmargin"), _CWS("0"));
    m_sMarginV = style.GetProperty(_CWS("vmargin"), _CWS("0"));

    // Size
    float fParentWidth((m_pParent == NULL) ? Draw2D.GetScreenW() : m_pParent->GetWidth());
    float fParentHeight((m_pParent == NULL) ? Draw2D.GetScreenH() : m_pParent->GetHeight());

    float fMarginH(ROUND(GetPositionFromString(m_sMarginH, fParentWidth, fParentHeight)));
    float fMarginV(ROUND(GetPositionFromString(m_sMarginV, fParentHeight, fParentWidth)));

    SetWidth(ROUND(GetSizeFromString(m_sWidth, fParentWidth, fParentHeight)) - fMarginH * 2.0f);
    SetHeight(ROUND(GetSizeFromString(m_sHeight, fParentHeight, fParentWidth)) - fMarginV * 2.0f);
    if (m_eFloat == WFLOAT_RIGHT)
        m_fPadding = ROUND(GetSizeFromString(style.GetProperty(_CWS("padding")), fParentWidth, fParentHeight));
    else if (m_eFloat == WFLOAT_BOTTOM)
        m_fPadding = ROUND(GetSizeFromString(style.GetProperty(_CWS("padding")), fParentHeight, fParentWidth));

    // Adherence
    const tstring &sAdherence(LowerString(style.GetProperty(_CWS("adhere"))));
    if (TStringCompare(sAdherence, _T("right")) == 0)
        m_eAdhere = WFLOAT_RIGHT;
    else if (TStringCompare(sAdherence, _T("bottom")) == 0)
        m_eAdhere = WFLOAT_BOTTOM;

    SetRotation(style.GetPropertyFloat(_CWS("rotation"), 0.0f));

    if (m_eAdhere == WFLOAT_RIGHT)
        m_fSpacing = ROUND(GetSizeFromString(style.GetProperty(_CWS("spacing")), fParentWidth, fParentHeight));
    else if (m_eAdhere == WFLOAT_BOTTOM)
        m_fSpacing = ROUND(GetSizeFromString(style.GetProperty(_CWS("spacing")), fParentHeight, fParentWidth));
    else if (m_pParent != NULL)
        m_fSpacing = m_pParent->m_fPadding;

    // Position
    if (m_sBaseX.empty() &&
        m_sBaseY.empty() &&
        (m_eAdhere != WFLOAT_NONE || (m_pParent != NULL && m_pParent->m_eFloat != WFLOAT_NONE && (m_pParent->HasFloatTarget() || m_pInterface->IsAnchored()))))
    {
        CVec2f v2Pos(GetFloatPosition(m_fSpacing));
        SetX(v2Pos.x);
        SetY(v2Pos.y);
    }
    else
    {
        if (m_pParent != NULL)
        {
            float fBaseX(0.0f);
            float fBaseY(0.0f);

            if (m_eAlignment == ALIGN_CENTER)
                fBaseX = (m_pParent->GetWidth() / 2.0f) - (GetWidth() / 2.0f);
            else if (m_eAlignment == ALIGN_RIGHT)
                fBaseX = m_pParent->GetWidth() - GetWidth();

            if (m_eVAlignment == ALIGN_CENTER)
                fBaseY = (m_pParent->GetHeight() / 2.0f) - (GetHeight() / 2.0f);
            else if (m_eVAlignment == ALIGN_BOTTOM)
                fBaseY = m_pParent->GetHeight() - GetHeight();

            SetX(ROUND(fBaseX + GetPositionFromString(m_sBaseX, m_pParent->GetWidth(), m_pParent->GetHeight())) + fMarginH);
            SetY(ROUND(fBaseY + GetPositionFromString(m_sBaseY, m_pParent->GetHeight(), m_pParent->GetWidth())) + fMarginV);
        }
        else
        {
            SetX(ROUND(GetPositionFromString(m_sBaseX, Draw2D.GetScreenW(), Draw2D.GetScreenH())) + fMarginH);
            SetY(ROUND(GetPositionFromString(m_sBaseY, Draw2D.GetScreenH(), Draw2D.GetScreenW())) + fMarginV);
        }
    }

    // Color and texture
    const tstring &sTexture(style.GetProperty(_CWS("texture")));
    SetColor(style.GetProperty(_CWS("color"), sTexture.empty() ? _CWS("gray") : _CWS("white")));

    // Check if we need to load the base texture. If not,
    // set the texture name properly for reference.
    if (!sTexture.empty())
        m_sTextureName.push_back(sTexture);

    m_hTexture[0] = INVALID_RESOURCE;
    for (int i(1); i < MAX_WIDGET_TEXTURES; ++i)
    {
        m_hTexture[i] = INVALID_RESOURCE;
        tstring sProperty(_CWS("texture") + XtoA(i));
        if (!style.HasProperty(sProperty))
            continue;

        uint uiElems(i + 1);
        if (uiElems > m_sTextureName.size())
            m_sTextureName.resize(uiElems, TSNULL);

        m_sTextureName[i] = style.GetProperty(sProperty);
    }

    if (bLoadTextures)
        LoadTextures();

    // Render mode
    SetRenderMode(style.GetProperty(_CWS("rendermode")));

    // Texture coordinates
    m_fUOffset = GetTextureOffsetFromString(style.GetProperty(_CWS("uoffset"), _CWS("0")), X);
    m_fVOffset = GetTextureOffsetFromString(style.GetProperty(_CWS("voffset"), _CWS("0")), Y);
    m_fUScale = GetTextureScaleFromString(style.GetProperty(_CWS("uscale"), _CWS("1")), X);
    m_fVScale = GetTextureScaleFromString(style.GetProperty(_CWS("vscale"), _CWS("1")), Y);
    m_fUSpeed = GetTextureOffsetFromString(style.GetProperty(_CWS("uspeed"), _CWS("0")), X);
    m_fVSpeed = GetTextureOffsetFromString(style.GetProperty(_CWS("vspeed"), _CWS("0")), Y);

    // Watchers
    const tstring &sWatch(style.GetProperty(_CWS("watch")));
    if (!sWatch.empty())
    {
        tsvector vWatchNames;
        if (SplitArgs(sWatch, vWatchNames) > 0)
        {
            for (tsvector::iterator it(vWatchNames.begin()); it != vWatchNames.end(); ++it)
            {
                CUIWatcher *pNewWatcher(K2_NEW(ctx_Widgets,  CUIWatcher)(this, *it));
                if (pNewWatcher != NULL)
                    m_vWatched.push_back(pNewWatcher);
            }
        }
    }

    // More Watchers
    for (int i(0); i <= 9; ++i)
    {
        const tstring &sWatch(style.GetProperty(_CWS("watch") + XtoA(i)));
        if (!sWatch.empty())
        {
            tsvector    vWatchNames;

            if (SplitArgs(sWatch, vWatchNames) > 0)
            {
                for (tsvector::iterator it = vWatchNames.begin(); it != vWatchNames.end(); ++it)
                {
                    CUIWatcher *pNewWatcher(K2_NEW(ctx_Widgets,  CUIWatcher)(this, *it, i));
                    if (pNewWatcher != NULL)
                        m_vvWatched[i].push_back(pNewWatcher);
                }
            }
        }
    }

    // Events
    SetEventCommand(WEVENT_FRAME, style.GetProperty(_CWS("onframe")));
    SetEventCommand(WEVENT_TRIGGER, style.GetProperty(_CWS("ontrigger")));
    SetEventCommand(WEVENT_SHOW, style.GetProperty(_CWS("onshow")));
    SetEventCommand(WEVENT_HIDE, style.GetProperty(_CWS("onhide")));
    SetEventCommand(WEVENT_ENABLE, style.GetProperty(_CWS("onenable")));
    SetEventCommand(WEVENT_DISABLE, style.GetProperty(_CWS("ondisable")));
    SetEventCommand(WEVENT_CHANGE, style.GetProperty(_CWS("onchange")));
    SetEventCommand(WEVENT_SLIDE, style.GetProperty(_CWS("onslide")));
    SetEventCommand(WEVENT_SNAP, style.GetProperty(_CWS("onsnap")));
    SetEventCommand(WEVENT_STARTDRAG, style.GetProperty(_CWS("onstartdrag")));
    SetEventCommand(WEVENT_ENDDRAG, style.GetProperty(_CWS("onenddrag")));
    SetEventCommand(WEVENT_SELECT, style.GetProperty(_CWS("onselect")));
    SetEventCommand(WEVENT_HOTKEY, style.GetProperty(_CWS("onhotkey")));
    SetEventCommand(WEVENT_CLICK, style.GetProperty(_CWS("onclick")));
    SetEventCommand(WEVENT_DOUBLECLICK, style.GetProperty(_CWS("ondoubleclick")));
    SetEventCommand(WEVENT_RIGHTCLICK, style.GetProperty(_CWS("onrightclick")));
    SetEventCommand(WEVENT_FOCUS, style.GetProperty(_CWS("onfocus")));
    SetEventCommand(WEVENT_LOSEFOCUS, style.GetProperty(_CWS("onlosefocus")));
    SetEventCommand(WEVENT_LOAD, style.GetProperty(_CWS("onload")));
    SetEventCommand(WEVENT_MOUSEOVER, style.GetProperty(_CWS("onmouseover")));
    SetEventCommand(WEVENT_MOUSEOUT, style.GetProperty(_CWS("onmouseout")));
    SetEventCommand(WEVENT_REFRESH, style.GetProperty(_CWS("onrefresh")));
    SetEventCommand(WEVENT_WAKE, style.GetProperty(_CWS("onwake")));
    SetEventCommand(WEVENT_MOUSELDOWN, style.GetProperty(_CWS("onmouseldown")));
    SetEventCommand(WEVENT_MOUSELUP, style.GetProperty(_CWS("onmouselup")));
    SetEventCommand(WEVENT_MOUSERDOWN, style.GetProperty(_CWS("onmouserdown")));
    SetEventCommand(WEVENT_MOUSERUP, style.GetProperty(_CWS("onmouserup")));
    SetEventCommand(WEVENT_RELOAD, style.GetProperty(_CWS("onreload")));
    SetEventCommand(WEVENT_KEYDOWN, style.GetProperty(_CWS("onkeydown")));
    SetEventCommand(WEVENT_KEYUP, style.GetProperty(_CWS("onkeyup")));
    SetEventCommand(WEVENT_INSTANTIATE, style.GetProperty(_CWS("oninstantiate")));
    SetEventCommand(WEVENT_TAB, style.GetProperty(_CWS("ontab")));
    SetEventCommand(WEVENT_OPEN, style.GetProperty(_CWS("onopen")));
    SetEventCommand(WEVENT_CLOSE, style.GetProperty(_CWS("onclose")));

    for (int i(0); i <= 9; ++i)
        SetEventCommand(EWidgetEvent(WEVENT_TRIGGER0 + i), style.GetProperty(_TS("ontrigger") + XtoA(i)));

    SetEventCommand(WEVENT_EVENT, style.GetProperty(_CWS("onevent")));
    for (int i(0); i <= 9; ++i)
        SetEventCommand(EWidgetEvent(WEVENT_EVENT0 + i), style.GetProperty(_TS("onevent") + XtoA(i)));

    if (style.HasProperty(_CWS("sleeptime")))
        m_uiWakeTime = Host.GetTime() + style.GetPropertyInt(_CWS("sleeptime"));

    if (pInterface)
    {
        pInterface->AddWidget(this);
        
        // Add widget to tab ordering
        if (m_uiTabOrder != uint(-1))
            pInterface->AddWidgetTabOrder(this, m_uiTabOrder);
    }

    // Grouping
    if (style.HasProperty(_CWS("group")))
        SetGroup(style.GetProperty(_CWS("group")));

    // Register with a form
    const tstring &sForm(style.GetProperty(_CWS("form")));
    const tstring &sData(style.GetProperty(_CWS("data")));
    if (!sForm.empty() && !sData.empty())
    {
        CUIForm *pForm(pInterface->GetForm(sForm));
        if (pForm != NULL)
            pForm->AddWidgetVariable(sData, this);
    }

    ++s_iNumWidgets;
}


/*====================
  IWidget::~IWidget
  ====================*/
IWidget::~IWidget()
{

    if(m_pLerp)
        SAFE_DELETE(m_pLerp);

    // Invalidate references to this widget
    if (!m_vReferences.empty())
    {
        ReferenceVector vReferences(m_vReferences);
        for (ReferenceVector_it itReference(vReferences.begin()), itEnd(vReferences.end()); itReference != itEnd; ++itReference)
            (*itReference)->Invalidate();
    }

    // Delete all of it it's children
    if (!m_vChildren.empty())
    {
        WidgetPointerVector vChildren(m_vChildren);
        for (WidgetPointerVector_it itChild(vChildren.begin()); itChild != vChildren.end(); ++itChild)
        {
            SAFE_DELETE(*itChild);
        }
    }

    // Notify parent
    if (m_pParent != NULL)
    {
        if (m_pParent->GetFloatTarget() == this)
            m_pParent->SetFloatTarget(m_refStickyTarget);
        
        if (m_pParent->GetAdhereTarget() == this)
            m_pParent->SetAdhereTarget(m_refStickyTarget);

        m_pParent->WidgetLost(this);

        WidgetPointerVector_it itFind(find(m_pParent->m_vChildren.begin(), m_pParent->m_vChildren.end(), this));

        if (itFind != m_pParent->m_vChildren.end())
            m_pParent->m_vChildren.erase(itFind);
    }

    // Notify all watchers that this widget has become invalid
    for (WatcherVector_it itWatched(m_vWatched.begin()), itEnd(m_vWatched.end()); itWatched != itEnd; ++itWatched)
        SAFE_DELETE(*itWatched);

    for (int i(0); i <= 9; ++i)
    {
        for (WatcherVector_it itWatched(m_vvWatched[i].begin()), itEnd(m_vvWatched[i].end()); itWatched != itEnd; ++itWatched)
            SAFE_DELETE(*itWatched);
    }

    --s_iNumWidgets;
}


/*====================
  IWidget::UpdateChildAlignment

  This function updates alignment for all children passed pUpdateFrom in the list.
  ====================*/
void    IWidget::UpdateChildAlignment(IWidget *pUpdateFrom)
{
    bool bFound(false);

    WidgetPointerVector_it itEnd(m_vChildren.end());
    for (WidgetPointerVector_it itChild(m_vChildren.begin()); itChild != itEnd; ++itChild)
    {
        if (!bFound && (*itChild) != pUpdateFrom)
            continue;

        if (!bFound)
        {
            // We're currently looking at the "update from" location.
            // All children from here on need their alignment updated.
            bFound = true;
            continue;
        }

        (*itChild)->RealignToSticky();
    }
}


/*====================
  IWidget::LostReference
  ====================*/
void    IWidget::LostReference(IWidget *pWidget)
{
    if (pWidget == NULL)
        return;

    bool bRecalculatePos(false);

    if (m_refStickyTarget.GetTarget() == pWidget)
    {
        bRecalculatePos = true;
        m_refStickyTarget = pWidget->GetStickyTarget();
    }

    if (m_refAdhereTarget.GetTarget() == pWidget)
    {
        WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend());

        for (; it != itEnd; ++it)
        {
            if (*it == pWidget)
                continue;

            m_refAdhereTarget = *it;
            break;
        }

        if (it == itEnd)
            m_refAdhereTarget = NULL;
    }

    if (m_refFloatTarget.GetTarget() == pWidget)
    {
        WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend());

        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        {
            if (*it == pWidget || (*it)->m_eAdhere != WFLOAT_NONE)
                continue;

            m_refFloatTarget = *it;
            break;
        }

        if (it == itEnd)
            m_refFloatTarget = NULL;
    }

    if (bRecalculatePos)
    {
        RealignToSticky();

        if (m_pParent != NULL)
            m_pParent->UpdateChildAlignment(this);
    }
}


/*====================
  IWidget::RealignToSticky

  This function realigns the widget to it's sticky target.
  Used when it loses it's original sticky target and has to realign.
  ====================*/
void    IWidget::RealignToSticky()
{
    if (!m_refStickyTarget.IsValid())
    {
        SetX(0.0f);
        SetY(0.0f);
    }
    else
    {
        // Determine widget to float from and direction of floating
        EWidgetFloat eFloat(WFLOAT_NONE);
        if (m_eAdhere != WFLOAT_NONE)
            eFloat = m_eAdhere;
        else if (m_pParent != NULL && m_pParent->m_eFloat != WFLOAT_NONE)
            eFloat = m_pParent->m_eFloat;

        if (eFloat == WFLOAT_BOTTOM)
        {
            float fX(floor(m_refStickyTarget->GetX()));
            if (m_eAlignment == ALIGN_CENTER)
                fX += ROUND((m_refStickyTarget->GetWidth() / 2.0f) - (GetWidth() / 2.0f));
            if (m_eAlignment == ALIGN_RIGHT)
                fX += m_refStickyTarget->GetWidth() - GetWidth();

            SetX(fX);
            SetY(ROUND(m_refStickyTarget->GetY() + m_refStickyTarget->GetHeight() + m_fSpacing));
        }
        else if (eFloat == WFLOAT_RIGHT)
        {
            float fY(floor(m_refStickyTarget->GetY()));
            if (m_eVAlignment == ALIGN_CENTER)
                fY += ROUND((m_refStickyTarget->GetHeight() / 2.0f) - (GetHeight() / 2.0f));
            if (m_eVAlignment == ALIGN_BOTTOM)
                fY += m_refStickyTarget->GetHeight() - GetHeight();

            SetX(ROUND(m_refStickyTarget->GetX() + m_refStickyTarget->GetWidth() + m_fSpacing));
            SetY(fY);
        }
        else
        {
            SetX(0.0f);
            SetY(0.0f);
        }
    }
}


/*====================
  IWidget::LoseChildren
  ====================*/
void    IWidget::LoseChildren()
{
    for (WidgetPointerVector_it it(m_vChildren.begin()); it != m_vChildren.end(); it++)
    {
        WidgetLost(*it);
        (*it)->LoseChildren();
    }

    m_vChildren.clear();
}


/*====================
  IWidget::WidgetLost
  ====================*/
void    IWidget::WidgetLost(IWidget *pWidget)
{
    if (pWidget == NULL)
        return;

    if (m_refStickyTarget.GetTarget() == pWidget)
        m_refStickyTarget.Invalidate();

    if (m_refAdhereTarget.GetTarget() == pWidget)
        m_refAdhereTarget.Invalidate();

    if (m_refFloatTarget.GetTarget() == pWidget)
        m_refFloatTarget.Invalidate();

    if (m_pInterface == pWidget)
        m_pInterface = NULL;

    if (m_pParent != NULL)
        m_pParent->WidgetLost(pWidget);
}


/*====================
  IWidget::BringChildToFront
  ====================*/
void   IWidget::BringChildToFront(IWidget *pWidget)
{
    m_vBringToFront.push_back(pWidget);
}

/*====================
  IWidget::AddChildWidget
  ====================*/
void   IWidget::AddChildWidget(IWidget *pWidget)
{
    m_vAddChild.push_back(pWidget);
}

/*====================
  IWidget::SetChildNewParent
  ====================*/
void   IWidget::SetChildNewParent(IWidget *pChild, IWidget *pParent)
{
    m_vMoveChild.push_back(WidgetPair(pChild, pParent));
}


/*====================
  IWidget::SetStickyTarget
  ====================*/
void    IWidget::SetStickyTarget(const CWidgetReference &ref)
{
    m_refStickyTarget = ref;
}


/*====================
  IWidget::GetFloatPosition
  ====================*/
CVec2f  IWidget::GetFloatPosition(float fSpacing)
{
    // Check for an overriding <anchor> tag
    if (m_pInterface != NULL && m_pInterface->IsAnchored())
        return m_pInterface->GetAnchorPosition();

    if (m_pParent == NULL)
        return V2_ZERO;

    // Determine widget to float from and direction of floating
    EWidgetFloat eFloat(WFLOAT_NONE);
    if (m_eAdhere != WFLOAT_NONE)
    {
        eFloat = m_eAdhere;

        if (m_pParent->GetAdhereTarget() != NULL)
            m_refStickyTarget = m_pParent->GetAdhereTarget();
    }
    else if (m_pParent->m_eFloat != WFLOAT_NONE)
    {
        eFloat = m_pParent->m_eFloat;

        if (m_pParent->GetFloatTarget() != NULL)
            m_refStickyTarget = m_pParent->GetFloatTarget();
    }

    if (!m_refStickyTarget.IsValid())
        return V2_ZERO;

    if (eFloat == WFLOAT_BOTTOM)
    {
        float fX(floor(m_refStickyTarget->GetX()));
        if (m_eAlignment == ALIGN_CENTER)
            fX += ROUND((m_refStickyTarget->GetWidth() / 2.0f) - (GetWidth() / 2.0f));
        if (m_eAlignment == ALIGN_RIGHT)
            fX += m_refStickyTarget->GetWidth() - GetWidth();
        return CVec2f(fX, ROUND(m_refStickyTarget->GetY() + m_refStickyTarget->GetHeight() + fSpacing));
    }
    
    if (eFloat == WFLOAT_RIGHT)
    {
        float fY(floor(m_refStickyTarget->GetY()));
        if (m_eVAlignment == ALIGN_CENTER)
            fY += ROUND((m_refStickyTarget->GetHeight() / 2.0f) - (GetHeight() / 2.0f));
        if (m_eVAlignment == ALIGN_BOTTOM)
            fY += m_refStickyTarget->GetHeight() - GetHeight();
        return CVec2f(ROUND(m_refStickyTarget->GetX() + m_refStickyTarget->GetWidth() + fSpacing), fY);
    }

    return V2_ZERO;
}

// MikeG Took out the extra uneeded code
/*====================
  IWidget::RecalculatePosition
  ====================*/
void    IWidget::RecalculatePosition()
{
    if (!(m_sBaseX.empty() &&
        m_sBaseY.empty() &&
        (m_eAdhere != WFLOAT_NONE || (m_pParent != NULL && m_pParent->m_eFloat != WFLOAT_NONE && (m_pParent->HasFloatTarget()/* || m_pInterface->IsAnchored()*/)))))
    {
        if (m_pParent != NULL)
        {
            float fBaseX(0.0f);
            float fBaseY(0.0f);

            if (m_eAlignment == ALIGN_CENTER)
                fBaseX = (m_pParent->GetWidth() / 2.0f) - (GetWidth() / 2.0f);
            else if (m_eAlignment == ALIGN_RIGHT)
                fBaseX = m_pParent->GetWidth() - GetWidth();

            if (m_eVAlignment == ALIGN_CENTER)
                fBaseY = (m_pParent->GetHeight() / 2.0f) - (GetHeight() / 2.0f);
            else if (m_eVAlignment == ALIGN_BOTTOM)
                fBaseY = m_pParent->GetHeight() - GetHeight();

            SetX(ROUND(fBaseX + GetPositionFromString(m_sBaseX, m_pParent->GetWidth(), m_pParent->GetHeight())));
            SetY(ROUND(fBaseY + GetPositionFromString(m_sBaseY, m_pParent->GetHeight(), m_pParent->GetWidth())));
        }
        else
        {
            SetX(ROUND(GetPositionFromString(m_sBaseX, Draw2D.GetScreenW(), Draw2D.GetScreenH())));
            SetY(ROUND(GetPositionFromString(m_sBaseY, Draw2D.GetScreenH(), Draw2D.GetScreenW())));
        }
    }
}


/*====================
  IWidget::RecalculateChildSize
  ====================*/
void    IWidget::RecalculateChildSize()
{
    for (WidgetPointerVector_cit itChild(m_vChildren.begin()), itEnd(m_vChildren.end()); itChild != itEnd; ++itChild)
    {
        (*itChild)->RecalculateSize();

        if (HasFlags(WFLAG_GROW_WITH_CHILDREN) && (HasFlags(WFLAG_GROW_WITH_INVIS) || (*itChild)->HasFlags(WFLAG_VISIBLE)))
        {
            CRectf rec((*itChild)->GetRect());
            rec.Shift(m_recArea.left, m_recArea.top);
            m_recArea |= rec;

            RecalculatePosition();
            for (WidgetPointerVector_cit it(m_vChildren.begin()); it != itChild; ++it)
                (*it)->RecalculateSize();
        }
    }
}


/*====================
  IWidget::RecalculateSize
  ====================*/
void    IWidget::RecalculateSize()
{
    if (HasFlags(WFLAG_RESIZING))
        return;

    SetFlags(WFLAG_RESIZING);

    float fOldWidth(GetWidth());
    float fOldHeight(GetHeight());

    float fParentWidth(GetParentWidth());
    float fParentHeight(GetParentHeight());

    float fMarginH(ROUND(GetPositionFromString(m_sMarginH, fParentWidth, fParentHeight)));
    float fMarginV(ROUND(GetPositionFromString(m_sMarginV, fParentHeight, fParentWidth)));

    SetWidth(ROUND(GetSizeFromString(m_sWidth, fParentWidth, fParentHeight)) + fMarginH);
    SetHeight(ROUND(GetSizeFromString(m_sHeight, fParentHeight, fParentWidth)) + fMarginV);

    RecalculatePosition();

    if (GetWidth() != fOldWidth || GetHeight() != fOldHeight || HasFlags(WFLAG_GROW_WITH_CHILDREN))
        RecalculateChildSize();

    bool bSticky(false);
    if (HasFlags(WFLAG_REGROW))
    {
        CVec2f v2Pos(0.0f, 0.0f);
        if (m_vChildren.size())
        {
            m_refFloatTarget = 0;
            for (WidgetPointerVector_cit itChild(m_vChildren.begin()), itEnd(m_vChildren.end()); itChild != itEnd; ++itChild)
            {
                if (((*itChild)->HasFlags(WFLAG_REGROW) || m_eFloat != WFLOAT_NONE) && (*itChild)->HasFlags(WFLAG_VISIBLE) )//((*itChild)->HasFlags(WFLAG_REGROW) || m_eFloat != WFLOAT_NONE) && 
                {
                    if (m_refFloatTarget.GetTarget())
                    {
                        v2Pos = (*itChild)->GetFloatPosition((*itChild)->m_fSpacing)+(*itChild)->m_fPadding;
                        (*itChild)->SetX(v2Pos.x);
                        (*itChild)->SetY(v2Pos.y);
                        if ((*itChild)->HasFlags(WFLAG_STICKY))
                        {
                            if(HasFlags(WFLAG_GROW_WITH_CHILDREN))
                            bSticky = true;
                        }

                        if (HasFlags(WFLAG_GROW_WITH_CHILDREN) && (*itChild)->HasFlags(WFLAG_VISIBLE))
                        {
                            CRectf rec((*itChild)->GetRect());
                            rec.rb()+m_fSpacing;
                            rec.Shift(m_recArea.left, m_recArea.top);
                            m_recArea |= rec;   
                        }
                    }
                    else if ((*itChild)->HasFlags(WFLAG_STICKY))
                    {
                        (*itChild)->ApplyStickiness();
                        bSticky = true;
                    }
                    m_refFloatTarget = (*itChild);
                }
            }
        }
    }
    
    if (bSticky)
    {
        RecalculatePosition();

        if (GetWidth() != fOldWidth || GetHeight() != fOldHeight || HasFlags(WFLAG_GROW_WITH_CHILDREN))
            RecalculateChildSize();
    }

    UnsetFlags(WFLAG_RESIZING);

    if (m_pParent != NULL && m_pParent->HasFlags(WFLAG_GROW_WITH_CHILDREN))
    {
        m_pParent->RecalculateSize();
        RecalculatePosition();
    }
}


/*====================
  IWidget::GetStickyRect
  ====================*/
CRectf  IWidget::GetStickyRect()
{
    IWidget *pTarget(m_refStickyTarget.GetTarget());
    // No target, use parents top left
    if (pTarget == NULL)
        return CRectf(0.0f, 0.0f, 0.0f, 0.0f);

    if (pTarget->HasFlags(WFLAG_VISIBLE) || HasFlags(WFLAG_STICKYTOINVIS))
        return pTarget->GetRect();

    return pTarget->GetStickyRect();
}


/*====================
  IWidget::ApplyStickiness
  ====================*/
void    IWidget::ApplyStickiness()
{
    if (!HasFlags(WFLAG_STICKY))
        return;

    float fParentWidth;
    float fParentHeight;
    if (m_pParent != NULL)
    {
        fParentWidth = m_pParent->GetWidth();
        fParentHeight = m_pParent->GetHeight();
    }
    else
    {
        fParentWidth = Draw2D.GetScreenW();
        fParentHeight = Draw2D.GetScreenH();
    }

    if (m_eAdhere != WFLOAT_NONE && m_refStickyTarget.IsValid())
    {
        CRectf rec(GetStickyRect());

        if (m_eAdhere == WFLOAT_RIGHT)
        {
            SetX(rec.right + m_fSpacing);
            SetY(rec.top);

            if (HasFlags(WFLAG_STICKYSIZING))
            {
                SetWidth(GetSizeFromString(m_sWidth, fParentWidth - GetX(), fParentHeight - GetY()));

                if (GetWidth() != m_fLastWidth)
                {
                    RecalculateChildSize();
                    m_fLastWidth = GetWidth();
                }
            }
        }
        else if (m_eAdhere == WFLOAT_BOTTOM)
        {
            SetX(rec.left);
            SetY(rec.bottom + m_fSpacing);

            if (HasFlags(WFLAG_STICKYSIZING))
            {
                SetHeight(GetSizeFromString(m_sHeight, fParentHeight - GetY(), fParentWidth - GetX()));

                if (GetHeight() != m_fLastHeight)
                {
                    RecalculateChildSize();
                    m_fLastHeight = GetHeight();
                }
            }
        }
        return;
    }

    if (m_pParent == NULL)
        return;

    if (m_pParent->m_eFloat != WFLOAT_NONE)
    {
        CRectf rec(GetStickyRect());

        if (m_pParent->m_eFloat == WFLOAT_RIGHT)
        {
            SetX(rec.right + m_fSpacing);
            SetY(rec.top);

            if (HasFlags(WFLAG_STICKYSIZING))
            {
                SetWidth(GetSizeFromString(m_sWidth, fParentWidth - GetX(), fParentHeight - GetY()));

                if (GetWidth() != m_fLastWidth)
                {
                    RecalculateChildSize();
                    m_fLastWidth = GetWidth();
                }
            }
        }
        else if (m_pParent->m_eFloat == WFLOAT_BOTTOM)
        {
            SetX(rec.left);
            SetY(rec.bottom + m_fSpacing);

            if (HasFlags(WFLAG_STICKYSIZING))
            {
                SetHeight(GetSizeFromString(m_sHeight, fParentHeight - GetY(), fParentWidth - GetX()));

                if (GetHeight() != m_fLastHeight)
                {
                    RecalculateChildSize();
                    m_fLastHeight = GetHeight();
                }
            }
        }
        return;
    }
}


/*====================
  IWidget::ResizeParent
  ====================*/
void    IWidget::ResizeParent()
{
    if (!HasFlags(WFLAG_VISIBLE) || m_pParent == NULL)
        return;

    bool bRecalculate(false);

    if (HasFlags(WFLAG_RESIZE_PARENT_WIDTH))
    {
        m_pParent->SetBaseWidth(XtoA(GetWidth()));
        bRecalculate = true;
    }

    if (HasFlags(WFLAG_RESIZE_PARENT_HEIGHT))
    {
        m_pParent->SetBaseHeight(XtoA(GetHeight()));
        bRecalculate = true;
    }

    if (bRecalculate)
        m_pParent->RecalculateSize();
}


/*====================
  IWidget::GetResourceContext
  ====================*/
tstring     IWidget::GetResourceContext() const
{
    if (m_sResourceContext.empty())
        return _T("global");
    else
        return _TS("ui:") + m_sResourceContext;
}


/*====================
  IWidget::GetPositionFromString
  ====================*/
float   IWidget::GetPositionFromString(const tstring &sPos, float fParentSize, float fParentSize2)
{
    if (sPos.empty())
        return 0.0f;

    if (sPos[sPos.length() - 1] == _T('%'))
        return ROUND(PercentageToScreen(fParentSize, PtoF(sPos)));
    else if (sPos[sPos.length() - 1] == _T('@'))
        return ROUND(PercentageToScreen(fParentSize2, P2toF(sPos)));
    else if (sPos[sPos.length() - 1] == _T('w'))
        return ROUND(PercentageToScreen(Draw2D.GetScreenW(), AtoF(sPos.substr(0, sPos.length() - 1))));
    else if (sPos[sPos.length() - 1] == _T('h'))
        return ROUND(PercentageToScreen(Draw2D.GetScreenH(), AtoF(sPos.substr(0, sPos.length() - 1))));
    else if (sPos[sPos.length() - 1] == _T('a'))
        return ROUND(Draw2D.GetScreenW() / Draw2D.GetScreenH() * AtoF(sPos.substr(0, sPos.length() - 1)));
    else
        return ROUND(AtoF(sPos));
}


/*====================
  IWidget::GetTextureOffsetFromString
  ====================*/
float   IWidget::GetTextureOffsetFromString(const tstring &sOffset, EVectorComponent eMajorAxis)
{
    assert(eMajorAxis == X || eMajorAxis == Y);

    if (sOffset.empty())
        return 0.0f;

    float fWidgetValue(eMajorAxis == X ? GetWidth() : GetHeight());

    if (sOffset.length() > 1)
    {
        if (sOffset[sOffset.length() - 1] == _T('%'))
            return PercentageToScreen(GetAbsolutePos()[eMajorAxis] / (eMajorAxis == X ? Draw2D.GetScreenW() : Draw2D.GetScreenH()), PtoF(sOffset));
        else if (sOffset[sOffset.length() - 1] == _T('@'))
            return PercentageToScreen(GetAbsolutePos()[eMajorAxis] / (eMajorAxis == X ? Draw2D.GetScreenH() : Draw2D.GetScreenW()), P2toF(sOffset));
        else if (sOffset[sOffset.length() - 1] == _T('p'))
        {
            if (fWidgetValue != 0.0f)
                return AtoF(sOffset.substr(0, sOffset.length() - 1)) / fWidgetValue;
            else
                return 0.0f;
        }
    }

    return AtoF(sOffset);
}


/*====================
  IWidget::GetTextureScaleFromString
  ====================*/
float   IWidget::GetTextureScaleFromString(const tstring &sScale, EVectorComponent eMajorAxis)
{
    assert(eMajorAxis == X || eMajorAxis == Y);

    if (sScale.empty())
        return 0.0f;

    float fWidgetValue(eMajorAxis == X ? GetWidth() : GetHeight());

    if (sScale.length() > 1)
    {
        if (sScale[sScale.length() - 1] == _T('%'))
            return PercentageToScreen(fWidgetValue / (eMajorAxis == X ? Draw2D.GetScreenW() : Draw2D.GetScreenH()), PtoF(sScale));
        else if (sScale[sScale.length() - 1] == _T('@'))
            return PercentageToScreen(fWidgetValue / (eMajorAxis == X ? Draw2D.GetScreenH() : Draw2D.GetScreenW()), P2toF(sScale));
        else if (sScale[sScale.length() - 1] == _T('p'))
        {
            float fScale(AtoF(sScale.substr(0, sScale.length() - 1)));

            if (fScale != 0.0f)
                return fWidgetValue / fScale;
            else
                return 0.0f;
        }
    }

    float fScale(AtoF(sScale));

    if (fScale != 0.0f)
        return 1.0f / fScale;
    else
        return 0.0f;
}


/*====================
  IWidget::GetSizeFromString
  ====================*/
float   IWidget::GetSizeFromString(const tstring &sSize, float fParentSize, float fParentSize2)
{
    if (sSize.empty())
        return 0.0f;

    float fSize(0.0f);

    if (sSize[sSize.length() - 1] == _T('%'))
        fSize = ROUND(PercentageToScreen(fParentSize, PtoF(sSize)));
    else if (sSize[sSize.length() - 1] == _T('@'))
        fSize = ROUND(PercentageToScreen(fParentSize2, P2toF(sSize)));
    else if (sSize[sSize.length() - 1] == _T('w'))
        fSize = ROUND(PercentageToScreen(Draw2D.GetScreenW(), AtoF(sSize.substr(0, sSize.length() - 1))));
    else if (sSize[sSize.length() - 1] == _T('h'))
        fSize = ROUND(PercentageToScreen(Draw2D.GetScreenH(), AtoF(sSize.substr(0, sSize.length() - 1))));
    else if (sSize[sSize.length() - 1] == _T('a'))
        return ROUND(Draw2D.GetScreenW() / Draw2D.GetScreenH() * AtoF(sSize.substr(0, sSize.length() - 1)));
    else
        fSize = ROUND(AtoF(sSize));

    if (sSize[0] == _T('+') || fSize < 0.0f)
        return fParentSize + fSize;
    else
        return fSize;
}


/*====================
  IWidget::SetWatch
  ====================*/
void    IWidget::SetWatch(const tstring &sWatch)
{
    // Notify all watchers that this widget has become invalid
    for (WatcherVector_it itWatched(m_vWatched.begin()), itEnd(m_vWatched.end()); itWatched != itEnd; ++itWatched)
        SAFE_DELETE(*itWatched);
    m_vWatched.clear();

    // Watchers
    if (sWatch.empty())
        return;

    tsvector vWatchNames;
    if (SplitArgs(sWatch, vWatchNames) > 0)
    {
        for (tsvector::iterator it(vWatchNames.begin()); it != vWatchNames.end(); ++it)
        {
            CUIWatcher *pNewWatcher(K2_NEW(ctx_Widgets,  CUIWatcher)(this, *it));
            if (pNewWatcher != NULL)
                m_vWatched.push_back(pNewWatcher);
        }
    }
}

/*====================
  IWidget::StopWatching
  ====================*/
void    IWidget::StopWatching(CUITrigger *pTrigger)
{
    //m_vWatched.remove(pTrigger);
}


/*====================
  IWidget::Show
  ====================*/
void    IWidget::Show(uint uiDuration)
{
    if (m_uiFadeEndTime == INVALID_TIME)
        m_fFadeCurrent = 1.0f;

    if (!HasFlags(WFLAG_VISIBLE) || uiDuration != uint(-1))
    {
        if (!HasFlags(WFLAG_VISIBLE))
        {
            SetFlags(WFLAG_VISIBLE);
            DO_EVENT(WEVENT_SHOW)
        }

        if (uiDuration == uint(-1))
            m_uiHideTime = -1;
        else
            m_uiHideTime = Host.GetTime() + uiDuration;
    }

    if(m_pParent && m_pParent->HasFlags(WFLAG_REGROW))
    {
        CVec2f v2OldPos = m_pParent->GetPos();
        m_pParent->RecalculateSize();
        m_pParent->SetPos(v2OldPos);
    }

}


/*====================
  IWidget::Hide
  ====================*/
void    IWidget::Hide()
{
    // Use WFLAG_WASVISIBLE to prevent possible infinite recursion
    if (HasFlags(WFLAG_WASVISIBLE))
        return;

    SetFlags(WFLAG_WASVISIBLE);
    DO_EVENT(WEVENT_HIDE)
    UnsetFlags(WFLAG_VISIBLE);
    UnsetFlags(WFLAG_WASVISIBLE);
    m_fFadeCurrent = 0.0f;

    if (m_pInterface->GetActiveWidget() == this)
        m_pInterface->SetActiveWidget(NULL);
    if (m_pInterface->GetExclusiveWidget() == this)
        m_pInterface->SetExclusiveWidget(NULL);

    if(m_pParent && m_pParent->HasFlags(WFLAG_REGROW))
    {
        CVec2f v2OldPos = m_pParent->GetPos();
        m_pParent->RecalculateSize();
        m_pParent->SetPos(v2OldPos);
    }
}


/*====================
  IWidget::Enable
  ====================*/
void    IWidget::Enable()
{
    DO_EVENT(WEVENT_ENABLE)
    SetFlags(WFLAG_ENABLED);
}


/*====================
  IWidget::Disable
  ====================*/
void    IWidget::Disable()
{
    DO_EVENT(WEVENT_DISABLE)
    UnsetFlags(WFLAG_ENABLED);

    if (m_pInterface->GetActiveWidget() == this)
        m_pInterface->SetActiveWidget(NULL);
    if (m_pInterface->GetExclusiveWidget() == this)
        m_pInterface->SetExclusiveWidget(NULL);
}


/*====================
  IWidget::SetSleepTimer
  ====================*/
void    IWidget::SetSleepTimer(uint uiDuration)
{
    m_uiWakeTime = Host.GetTime() + uiDuration;
}


/*====================
  IWidget::ClearWakeEvents
  ====================*/
void    IWidget::ClearWakeEvents()
{
    m_uiWakeTime = -1;
    m_vPendingWakeEvents.clear();
    ClearEventCommand(WEVENT_WAKE);
}


/*====================
  IWidget::PushWakeEvent
  ====================*/
void    IWidget::PushWakeEvent(uint uiDuration, const tstring &sCommand)
{
    m_vPendingWakeEvents.push_back(WakeEvent(uiDuration, sCommand));
}


/*====================
  IWidget::Move
  ====================*/
void    IWidget::Move(float fX, float fY)
{
    m_recArea.MoveTo(fX, fY);
}


/*====================
  IWidget::SlideY
  ====================*/
void    IWidget::SlideY(float fY, uint uiTime)
{
    m_uiMoveStartTimeY = Host.GetTime();
    m_uiTargetTimeY = m_uiMoveStartTimeY + uiTime;
    m_fStartY = GetY();
    m_fTargetY = fY;
}

/*====================
  IWidget::SlideX
  ====================*/
void    IWidget::SlideX(float fX, uint uiTime)
{
    m_uiMoveStartTimeX = Host.GetTime();
    m_uiTargetTimeX = m_uiMoveStartTimeX + uiTime;
    m_fStartX = GetX();
    m_fTargetX = fX;
}


/*====================
  IWidget::Rotate
  ====================*/
void    IWidget::Rotate(float fRotation, uint uiTime, bool bRecurse)
{
    m_uiMoveStartTimeRotation = Host.GetTime();
    m_uiTargetTimeRotation = m_uiMoveStartTimeRotation + uiTime;
    m_fStartRotation = GetRotation();
    m_fTargetRotation = fRotation;

    if (bRecurse)
    {
        for (WidgetPointerVector_cit it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
            (*it)->Rotate(fRotation, uiTime, bRecurse);
    }
}


/*====================
  IWidget::ScaleWidth
  ====================*/
void    IWidget::ScaleWidth(float fWidth, uint uiTime, int iDirection, bool bRecurse)
{
    if(m_uiMoveStartTimeWidth != INVALID_TIME)
    {
        if(Host.GetTime() > m_uiTargetTimeWidth)
        {
            SetWidth(m_fTargetWidth);
            RecalculateChildSize();
            m_uiMoveStartTimeWidth = INVALID_TIME;
        }
        else
        {
            float fCurrentTimedSize = 0;
            float fTimeLenght = (m_uiTargetTimeWidth - m_uiMoveStartTimeWidth);
            float fCurrentTime = (Host.GetTime() - m_uiMoveStartTimeWidth);
            float fDistance = (m_fTargetWidth - m_fStartWidth);
            
            fCurrentTimedSize = (fDistance * (fCurrentTime / fTimeLenght))+m_fStartWidth;
            if(fDistance > 0 && iDirection < 0)
                fCurrentTimedSize = m_fTargetWidth;
            else if(fDistance < 0 && iDirection > 0)
                fCurrentTimedSize = m_fTargetWidth;

            SetWidth(fCurrentTimedSize);
            RecalculateChildSize();
        }
    }
        
    m_fStartWidth = GetWidth();
    m_uiMoveStartTimeWidth = Host.GetTime();
    m_uiTargetTimeWidth = m_uiMoveStartTimeWidth + uiTime;
    m_fTargetWidth = fWidth;
    m_iDirectionWidth = iDirection;

    if (bRecurse)
    {
        for (WidgetPointerVector_cit it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
            (*it)->ScaleWidth(fWidth, uiTime, iDirection, bRecurse);
    }
}

/*====================
  IWidget::ScaleHeight
  ====================*/
void    IWidget::ScaleHeight(float fHeight, uint uiTime, int iDirection, bool bRecurse)
{
    if(m_uiMoveStartTimeHeight != INVALID_TIME)
    {
        if(Host.GetTime() > m_uiTargetTimeHeight)
        {
            SetHeight(m_fTargetHeight);
            RecalculateChildSize();
            m_uiMoveStartTimeHeight = INVALID_TIME;
        }
        else
        {
            float fCurrentTimedSize = 0;
            float fTimeLenght = (m_uiTargetTimeHeight - m_uiMoveStartTimeHeight);
            float fCurrentTime = (Host.GetTime() - m_uiMoveStartTimeHeight);
            float fDistance = (m_fTargetHeight - m_fStartHeight);
            
            fCurrentTimedSize = (fDistance * (fCurrentTime / fTimeLenght))+m_fStartHeight;
            if(fDistance > 0 && iDirection < 0)
                fCurrentTimedSize = m_fTargetHeight;
            else if(fDistance < 0 && iDirection > 0)
                fCurrentTimedSize = m_fTargetHeight;

            SetHeight(fCurrentTimedSize);
            RecalculateChildSize();
        }
    }
    
    m_fStartHeight = GetHeight();
    m_uiMoveStartTimeHeight = Host.GetTime();
    m_uiTargetTimeHeight = m_uiMoveStartTimeHeight + uiTime;
    m_fTargetHeight = fHeight;
    m_iDirectionHeight = iDirection;

    if (bRecurse)
    {
        for (WidgetPointerVector_cit it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
            (*it)->ScaleHeight(fHeight, uiTime, iDirection, bRecurse);
    }
}

/*====================
  IWidget::GetFadedColor
  ====================*/
CVec4f  IWidget::GetFadedColor(const CVec4f &v4ColorIn, float fFade) const
{
    CVec4f v4ColorOut(v4ColorIn);
    if (m_eRenderMode == WRENDER_ADDITIVE)
        v4ColorOut *= fFade;
    else if (m_eRenderMode == WRENDER_OVERLAY)
        v4ColorOut.xyz() = ((v4ColorOut.xyz() - 0.5f) * fFade) + 0.5f;
    else
        v4ColorOut[A] *= fFade;

    return v4ColorOut;
}


/*====================
  IWidget::FadeOut
  ====================*/
void    IWidget::FadeOut(uint uiTime)
{
    m_fFadeStart = m_fFadeCurrent;
    m_fFadeEnd = 0.0f;
    m_uiFadeStartTime = Host.GetTime();
    m_uiFadeEndTime = m_uiFadeStartTime + uiTime;
}


/*====================
  IWidget::FadeIn
  ====================*/
void    IWidget::FadeIn(uint uiTime)
{
    m_fFadeStart = m_fFadeCurrent;
    m_fFadeEnd = 1.0f;
    m_uiFadeStartTime = Host.GetTime();
    m_uiFadeEndTime = m_uiFadeStartTime + uiTime;
}


/*====================
  IWidget::Fade
  ====================*/
void    IWidget::Fade(float fStart, float fEnd, uint uiTime)
{
    m_fFadeStart = fStart;
    m_fFadeEnd = fEnd;
    m_uiFadeStartTime = Host.GetTime();
    m_uiFadeEndTime = m_uiFadeStartTime + uiTime;
}


/*====================
  IWidget::LoadTextures
  ====================*/
void    IWidget::LoadTextures()
{
    if (GetType() == WIDGET_ANIMATEDIMAGE)
        return;

    if (m_sTextureName.empty())
    {
        m_hTexture[0] = g_ResourceManager.GetWhiteTexture();
        return;
    }

    K2_WITH_RESOURCE_SCOPE(GetResourceContext())
    {
        tsvector_it it(m_sTextureName.begin());
        uint ui(0);
        for (; it != m_sTextureName.end() && ui < MAX_WIDGET_TEXTURES; ++it, ++ui)
            UITextureRegistry.Register(*it, 0, m_hTexture[ui]);
    }
}


/*====================
  IWidget::RenderWidget
  ====================*/
void    IWidget::RenderWidget(const CVec2f &vOrigin, float fFade)
{
    if (HasFlags(WFLAG_NO_DRAW))
        return;

    int iRenderFlags(0);

    if (m_eRenderMode == WRENDER_ADDITIVE)
        iRenderFlags |= GUI_ADDITIVE;
    else if (m_eRenderMode == WRENDER_OVERLAY)
        iRenderFlags |= GUI_OVERLAY;
    else if (m_eRenderMode == WRENDER_GRAYSCALE)
        iRenderFlags |= GUI_GRAYSCALE;
    else if (m_eRenderMode == WRENDER_BLUR)
        iRenderFlags |= GUI_BLUR;
    
    if (HasFlags(WFLAG_UTILE))
        iRenderFlags |= GUI_TILE_U;
    if (HasFlags(WFLAG_VTILE))
        iRenderFlags |= GUI_TILE_V;

    const CVec4f &v4Color(fFade == 1.0f ? m_v4Color : GetFadedColor(m_v4Color, fFade));

    if (m_eRenderMode == WRENDER_ADDITIVE && v4Color[R] == 0.0f && v4Color[G] == 0.0f && v4Color[B] == 0.0f)
        return;
    else if (m_eRenderMode == WRENDER_OVERLAY && v4Color[R] == 0.5f && v4Color[G] == 0.5f && v4Color[B] == 0.5f)
        return;
    else if (v4Color[A] == 0.0f)
        return;

    Draw2D.SetColor(v4Color);
    for (int i(0); i < MAX_WIDGET_TEXTURES; ++i)
    {
        if (i > 0 && m_hTexture[i] == INVALID_RESOURCE)
            continue;

        if (m_fRotation == 0.0f)
        {
            if (m_hTexture[i] == INVALID_RESOURCE)
            {
                Draw2D.Rect(vOrigin.x, vOrigin.y, m_recArea.GetWidth(), m_recArea.GetHeight(), iRenderFlags);
            }
            else
            {
                float fS0(m_fUOffset), fS1(m_fUOffset + m_fUScale);
                if (fS0 < 0.0f)
                {
                    fS0 -= m_fUScale;
                    fS1 -= m_fUScale;
                }
                float fT0(1.0f - m_fVOffset - m_fVScale), fT1(1.0f -m_fVOffset);

                if (HasFlags(WFLAG_CROPTEXTURE))
                {
                    fS0 = m_fCropS0;
                    fS1 = m_fCropS1;
                    fT0 = m_fCropT0;
                    fT1 = m_fCropT1;
                }

                if (HasFlags(WFLAG_HFLIP))
                {
                    float fTmp(fS0);

                    fS0 = fS1;
                    fS1 = fTmp;
                }

                if (HasFlags(WFLAG_VFLIP))
                {
                    float fTmp(fT0);

                    fT0 = fT1;
                    fT1 = fTmp;
                }

                Draw2D.Rect(vOrigin.x, vOrigin.y, m_recArea.GetWidth(), m_recArea.GetHeight(), fS0, fT0, fS1, fT1, m_hTexture[i], iRenderFlags);
            }
        }
        else
        {
            float fHalfWidth(m_recArea.GetWidth() * 0.5f);
            float fHalfHeight(m_recArea.GetHeight() * 0.5f);
            CVec2f v2Center(vOrigin.x + fHalfWidth, vOrigin.y + fHalfHeight);

            float fS0(m_fUOffset), fS1(m_fUScale);
            float fT0(m_fVOffset), fT1(m_fVScale);
            if (HasFlags(WFLAG_HFLIP))
            {
                float fTmp(fS0);

                fS0 = fS1;
                fS1 = fTmp;
            }

            if (HasFlags(WFLAG_VFLIP))
            {
                float fTmp(fT0);

                fT0 = fT1;
                fT1 = fTmp;
            }

            CVec2f tL(-fHalfWidth, -fHalfHeight);
            CVec2f tR(fHalfWidth, -fHalfHeight);
            CVec2f bL(-fHalfWidth, fHalfHeight);
            CVec2f bR(fHalfWidth, fHalfHeight);
            tL.Rotate(m_fRotation);
            tR.Rotate(m_fRotation);
            bL.Rotate(m_fRotation);
            bR.Rotate(m_fRotation);
            if (m_hTexture[i] == INVALID_RESOURCE)
            {
                Draw2D.Quad(tL + v2Center, bL + v2Center, bR + v2Center, tR + v2Center, iRenderFlags);
            }
            else
            {
                Draw2D.Quad(tL + v2Center, bL + v2Center, bR + v2Center, tR + v2Center,
                            CVec2f(fS0, fT1), CVec2f(fS0, fT0), CVec2f(fS1, fT0), CVec2f(fS1, fT1),
                            m_hTexture[i], iRenderFlags);
            }
        }
    }
}


/*====================
  IWidget::Render
  ====================*/
void    IWidget::Render(const CVec2f &vOrigin, int iFlag, float fFade)
{
    if (IsDead())
        return;

    if (!HasFlags(WFLAG_VISIBLE))
        return;

    CVec2f v2LocalOrigin(vOrigin + m_recArea.lt());
    float fLocalFade(fFade * m_fFadeCurrent);

    // Render this widget
    if ((HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_TOP)) ||
        (!HasFlags(WFLAG_RENDER_TOP) && (iFlag & WIDGET_RENDER_BOTTOM)))
        RenderWidget(v2LocalOrigin, fLocalFade);

    // Render children
    for (WidgetPointerVector_cit it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
        (*it)->Render(v2LocalOrigin, iFlag, fLocalFade);
}


/*====================
  IWidget::ProcessInputMouseButton
  ====================*/
bool    IWidget::ProcessInputMouseButton(const CVec2f &v2CursorPos, EButton button, float fValue)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    CVec2f v2LastCursorPos;

    if (HasSavedCursorPos())
        v2LastCursorPos = m_v2LastCursorPos;

    if (!HasFlags(WFLAG_PASSIVE_CHILDREN))
    {
        // Check children, relative to parent widget
        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        {
            IWidget* pWidget(*it);
            if (pWidget->ProcessInputMouseButton(v2CursorPos - m_recArea.lt(), button, fValue))
                return true;
        }
    }

    if (m_eWidgetType == WIDGET_INTERFACE || HasFlags(WFLAG_NO_CLICK))
        return false;

    // Check this widget
    if (fValue == 1.0f) // down
    {
        if (m_recArea.AltContains(v2CursorPos) || (HasSavedCursorPos() && m_recArea.AltContains(v2LastCursorPos)) || m_pInterface->GetExclusiveWidget() == this)
        {
            // set active widget we click on so we know where to send input
            if (IsInteractive())
                m_pInterface->SetActiveWidget(this);

            switch (button)
            {
            case BUTTON_MOUSEL:
            case BUTTON_MOUSER:
            case BUTTON_MOUSEM:
            case BUTTON_MOUSEX1:
            case BUTTON_MOUSEX2:
            case BUTTON_WHEELUP:
            case BUTTON_WHEELDOWN:
            case BUTTON_WHEELLEFT:
            case BUTTON_WHEELRIGHT:
                MouseDown(button, v2CursorPos);
                break;

            default:
                break;
            }

            return UseMouseDown();
        }
    }
    else // up
    {
        bool bExclusive(m_pInterface && m_pInterface->GetExclusiveWidget() == this);

        switch (button)
        {
        case BUTTON_MOUSEL:
        case BUTTON_MOUSER:
        case BUTTON_MOUSEM:
        case BUTTON_MOUSEX1:
        case BUTTON_MOUSEX2:
        case BUTTON_WHEELUP:
        case BUTTON_WHEELDOWN:
        case BUTTON_WHEELLEFT:
        case BUTTON_WHEELRIGHT:
            MouseUp(button, v2CursorPos);
            break;

        default:
            break;
        }

        return bExclusive; // only eat exclusive up's
    }

    return false;
}


/*====================
  IWidget::ProcessInputAxis
  ====================*/
bool    IWidget::ProcessInputAxis(EAxis axis, float fValue)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    if (!HasFlags(WFLAG_PASSIVE_CHILDREN))
    {
        // Let children handle the input directly
        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        {
            if ((*it)->HasFlags(WFLAG_PROCESS_AXIS) && (*it)->ProcessInputAxis(axis, fValue))
                return true;
        }
    }

    return HasFlags(WFLAG_BLOCK_INPUT);
}

/*====================
  IWidget::GetMinimapHoverUnit
  ====================*/
uint    IWidget::GetMinimapHoverUnit()
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return uint(-1);

    if (!HasFlags(WFLAG_PASSIVE_CHILDREN))
    {
        // Find the minimap and see if theres a Unit hovering.
        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        {
            uint uiHoverUnit((*it)->GetMinimapHoverUnit());
            if (uiHoverUnit != uint(-1))
                return uiHoverUnit;
        }
    }

    return uint(-1);
}

/*====================
  IWidget::ProcessInputCursor
  ====================*/
bool    IWidget::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    if (!HasFlags(WFLAG_PASSIVE_CHILDREN))
    {
        // Let children handle the input directly
        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        {
            if ((*it)->HasFlags(WFLAG_PROCESS_CURSOR) && (*it)->ProcessInputCursor(v2CursorPos - m_recArea.lt()))
                return true;
        }
    }

    return (HasFlags(WFLAG_BLOCK_INPUT) && Contains(v2CursorPos));
}


/*====================
  IWidget::ButtonDown
  ====================*/
bool    IWidget::ButtonDown(EButton button)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;
    
    return HasFlags(WFLAG_BLOCK_INPUT);
}


/*====================
  IWidget::ProcessHotKeys
  ====================*/
bool    IWidget::ProcessHotKeys(EButton eButton)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return false;
    
    bool bUsed(false);
    for (WidgetPointerVector_it it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
    {
        if ((*it)->ProcessHotKeys(eButton))
        {
            bUsed = true;
            break;
        }
    }

    if (bUsed)
        return true;

    if (m_eHotkey != BUTTON_INVALID && eButton == m_eHotkey)
    {
        DO_EVENT_RETURN(WEVENT_HOTKEY, true)
        return true;
    }

    return false;
}

/*====================
  IWidget::Frame
  ====================*/
void    IWidget::Frame(uint uiFrameLength, bool bProcessFrame)
{
    if (IsDead())
        return;

    if(m_pLerp)
        m_pLerp->Update();

    uint uiTime(Host.GetTime());

    // Motion
    bool bResized(false);

    if (m_uiMoveStartTimeX != INVALID_TIME)
    {
        float fLerp(M_SmoothStepN(CLAMP(float(uiTime - m_uiMoveStartTimeX) / float(m_uiTargetTimeX - m_uiMoveStartTimeX), 0.0f, 1.0f)));
        SetX(ROUND(LERP(fLerp, m_fStartX, m_fTargetX)));
        if (Host.GetTime() >= m_uiTargetTimeX)
            m_uiMoveStartTimeX = INVALID_TIME;
    }

    if (m_uiMoveStartTimeY != INVALID_TIME)
    {
        float fLerp(M_SmoothStepN(CLAMP(float(uiTime - m_uiMoveStartTimeY) / float(m_uiTargetTimeY - m_uiMoveStartTimeY), 0.0f, 1.0f)));
        SetY(ROUND(LERP(fLerp, m_fStartY, m_fTargetY)));
        if (Host.GetTime() >= m_uiTargetTimeY)
            m_uiMoveStartTimeY = INVALID_TIME;
    }

    if (m_uiMoveStartTimeRotation != INVALID_TIME)
    {   
        float fLerp(M_SmoothStepN(CLAMP(float(uiTime - m_uiMoveStartTimeRotation) / float(m_uiTargetTimeRotation - m_uiMoveStartTimeRotation), 0.0f, 1.0f)));
        SetRotation(LERP(fLerp, m_fStartRotation, m_fTargetRotation));
        if (Host.GetTime() >= m_uiTargetTimeRotation)
            m_uiMoveStartTimeRotation = INVALID_TIME;
    }

    if(m_uiMoveStartTimeHeight != INVALID_TIME)
    {
        if(Host.GetTime() > m_uiTargetTimeHeight)
        {
            SetHeight(m_fTargetHeight);
            bResized = true;
            m_uiMoveStartTimeHeight = INVALID_TIME;
        }
        else
        {
            float fCurrentTimedSize = 0;
            float fTimeLenght = (m_uiTargetTimeHeight - m_uiMoveStartTimeHeight);
            float fCurrentTime = (Host.GetTime() - m_uiMoveStartTimeHeight);
            float fDistance = (m_fTargetHeight - m_fStartHeight);
            
            fCurrentTimedSize = (fDistance * (fCurrentTime / fTimeLenght))+m_fStartHeight;
            if(fDistance > 0 && m_iDirectionHeight < 0)
                fCurrentTimedSize = m_fTargetHeight;
            else if(fDistance < 0 && m_iDirectionHeight > 0)
                fCurrentTimedSize = m_fTargetHeight;

            SetHeight(fCurrentTimedSize);
            bResized = true;
        }
    }

    if(m_uiMoveStartTimeWidth != INVALID_TIME)
    {
        if(Host.GetTime() > m_uiTargetTimeWidth)
        {
            SetWidth(m_fTargetWidth);
            bResized = true;
            m_uiMoveStartTimeWidth = INVALID_TIME;
        }
        else
        {
            float fCurrentTimedSize = 0;
            float fTimeLenght = (m_uiTargetTimeWidth - m_uiMoveStartTimeWidth);
            float fCurrentTime = (Host.GetTime() - m_uiMoveStartTimeWidth);
            float fDistance = (m_fTargetWidth - m_fStartWidth);
            
            fCurrentTimedSize = (fDistance * (fCurrentTime / fTimeLenght))+m_fStartWidth;
            if(fDistance > 0 && m_iDirectionWidth < 0)
                fCurrentTimedSize = m_fTargetWidth;
            else if(fDistance < 0 && m_iDirectionWidth > 0)
                fCurrentTimedSize = m_fTargetWidth;

            SetWidth(fCurrentTimedSize);
            bResized = true;
        }
    }

    if (bResized)
    {
        RecalculateChildSize();
        RecalculatePosition();
    }

    // Fading
    if (m_uiFadeEndTime != INVALID_TIME)
    {
        m_fFadeCurrent = m_fFadeStart + M_SmoothStepN(float(uiTime - m_uiFadeStartTime) / float(m_uiFadeEndTime - m_uiFadeStartTime)) * (m_fFadeEnd - m_fFadeStart);

        if (Host.GetTime() >= m_uiFadeEndTime)
        {
            m_uiFadeEndTime = INVALID_TIME;
            if (!m_WidgetEvents.empty() && !m_WidgetEvents[WEVENT_FADED].empty())
            {
                UIScript.Evaluate(this, m_WidgetEvents[WEVENT_FADED]);
                ClearEventCommand(WEVENT_FADED);
            }
        }

        if (m_fFadeCurrent == 0.0f && HasFlags(WFLAG_VISIBLE))
            Hide();

        if (m_fFadeCurrent > 0.0f && !HasFlags(WFLAG_VISIBLE))
            Show();
    }

    if (m_fUSpeed != 0.0f || m_fVSpeed != 0.0f)
    {
        float fSeconds(MsToSec(uiFrameLength));
        m_fUOffset += m_fUScale * fSeconds * m_fUSpeed;
        m_fUOffset -= floor(m_fUOffset);
        m_fVOffset += m_fVScale * fSeconds * m_fVSpeed;
        m_fVOffset -= floor(m_fVOffset);
    }

    while (uiTime >= m_uiWakeTime)
    {
        uint uiPrevWake(m_uiWakeTime);
        m_uiWakeTime = -1;
        DO_EVENT(WEVENT_WAKE);

        if (m_vPendingWakeEvents.empty())
            break;

        const WakeEvent&    cEvent(m_vPendingWakeEvents.front());
        uint                uiDelay(cEvent.first);
        const tstring&      sCommand(cEvent.second);
        assert(uiDelay != INVALID_TIME);

        m_uiWakeTime = uiPrevWake + uiDelay;
        SetEventCommand(WEVENT_WAKE, sCommand);
        m_vPendingWakeEvents.erase(m_vPendingWakeEvents.begin());
    }

    if (uiTime >= m_uiHideTime)
        Hide();

    ApplyStickiness();

    ResizeParent();

    bProcessFrame = bProcessFrame && HasFlags(WFLAG_VISIBLE) && HasFlags(WFLAG_ENABLED);

    if (bProcessFrame)
    {
        DO_EVENT(WEVENT_FRAME);

        // Makes widgets in m_vAddChild a child of the interface and brings them to the front of the interface
        if (!m_vAddChild.empty())
        {
            for (WidgetPointerVector_it itFront(m_vAddChild.begin()); itFront != m_vAddChild.end(); ++itFront)
            {
                IWidget* pChild(*itFront);
                AddChild(pChild);
                pChild->SetParent(this);
                pChild->RecalculateSize();
            }
            m_vAddChild.clear();
        }

        // Gives the widgets in m_vMoveChild a new Parent then brings them to the front of there new parent
        if (!m_vMoveChild.empty())
        {
            for (WidgetPointerVectorNewParent_it itFront(m_vMoveChild.begin()); itFront != m_vMoveChild.end(); ++itFront)
            {
                IWidget* pNewParent(itFront->second);
                IWidget* pChild(itFront->first);
                if (RemoveChild(pChild))
                {
                    pNewParent->AddChildWidget(pChild);
                    pChild->SetParent(0); // Omg the Child is lost with no parent
                }
            }
            m_vMoveChild.clear();
        }

        // Bring widgets in m_vBringToFront to the front of child ordering
        if (!m_vBringToFront.empty())
        {
            for (WidgetPointerVector_it itFront(m_vBringToFront.begin()); itFront != m_vBringToFront.end(); ++itFront)
            {
                if (RemoveChild(*itFront))
                    m_vChildren.push_back(*itFront);
            }
            m_vBringToFront.clear();
        }

        // Recursively call children frame functions
        for (WidgetPointerVector_it it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
            (*it)->Frame(uiFrameLength, bProcessFrame);
    }
}


/*====================
  IWidget::Purge
  ====================*/
void    IWidget::Purge()
{
    if (!NeedsPurge())
        return;

    WidgetPointerVector vChildren(m_vChildren);
    for (WidgetPointerVector_rit it(vChildren.rbegin()), itEnd(vChildren.rend()); it != itEnd; ++it)
    {
        if ((*it)->IsDead())
        {
            SAFE_DELETE(*it);
            continue;
        }

        (*it)->Purge();
    }

    UnsetFlags(WFLAG_NEEDSPURGE);
}


/*====================
  IWidget::GetAbsolutePos
  ====================*/
CVec2f  IWidget::GetAbsolutePos()
{
    if (m_pParent)
        return m_recArea.lt() + m_pParent->GetAbsolutePos();
    else
        return CVec2f(0.0f, 0.0f);
}


/*====================
  IWidget::GetWidget
  ====================*/
IWidget*    IWidget::GetWidget(const CVec2f &v2Pos, bool bClick)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return NULL;

    if (!bClick || !HasFlags(WFLAG_PASSIVE_CHILDREN))
    {
        for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        {
            IWidget *pWidget((*it)->GetWidget(v2Pos - m_recArea.lt(), bClick));

            if (pWidget)
                return pWidget;
        }
    }

    if (HasFlags(WFLAG_NO_CLICK) && bClick)
        return NULL;

    if (m_recArea.AltContains(v2Pos))
        return this;
    else
        return NULL;
}


/*====================
  IWidget::Contains
  ====================*/
bool    IWidget::Contains(const CVec2f &v2Pos)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return false;

    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
    {
        if ((*it)->Contains(v2Pos - m_recArea.lt()))
            return true;
    }

    if (m_recArea.AltContains(v2Pos))
        return true;
    else
        return false;
}


/*====================
  IWidget::SetTexture
  ====================*/
void    IWidget::SetTexture(const tstring &sTexture)
{
    PROFILE("IWidget::SetTexture");

    if (sTexture.find(IMAGELIST_SEPERATOR) == tstring::npos)
    {
        if (m_sTextureName.size() <= 0)
            m_sTextureName.push_back(sTexture);
        else
            m_sTextureName[0] = sTexture;

        if (m_sTextureName[0].empty())
            m_hTexture[0] = g_ResourceManager.GetWhiteTexture();
        else
        {
            K2_WITH_RESOURCE_SCOPE(GetResourceContext())
                UITextureRegistry.Register(m_sTextureName[0], 0, m_hTexture[0]);
        }

        for (uint ui(1); ui < MAX_WIDGET_TEXTURES; ++ui)
        {
            if (m_sTextureName.size() > ui)
                m_sTextureName[ui].clear();
            
            m_hTexture[ui] = INVALID_RESOURCE;
        }
    }
    else
    {
        tsvector vImageList(TokenizeString(sTexture, IMAGELIST_SEPERATOR));

        uint ui(0);
        for (; ui < uint(vImageList.size()) && ui < MAX_WIDGET_TEXTURES; ++ui)
        {
            if (m_sTextureName.size() <= ui)
                m_sTextureName.resize(ui + 1, TSNULL);

            m_sTextureName[ui] = vImageList[ui];

            if (m_sTextureName[ui].empty())
                m_hTexture[ui] = g_ResourceManager.GetWhiteTexture();
            else
            {
                K2_WITH_RESOURCE_SCOPE(GetResourceContext())
                    UITextureRegistry.Register(m_sTextureName[ui], 0, m_hTexture[ui]);
            }
        }

        for (; ui < MAX_WIDGET_TEXTURES; ++ui)
        {
            if (m_sTextureName.size() > ui)
                m_sTextureName[ui].clear();

            m_hTexture[ui] = INVALID_RESOURCE;
        }
    }
}

void    IWidget::SetTexture(const tstring &sTexture, const tstring &sSuffix)
{
    if (m_sTextureName.size() <= 0)
        m_sTextureName.push_back(Filename_StripExtension(sTexture));
    else
        m_sTextureName[0] = Filename_StripExtension(sTexture);

    if (m_sTextureName[0].empty())
    {
        m_hTexture[0] = g_ResourceManager.GetWhiteTexture();

        for (uint ui(1); ui < MAX_WIDGET_TEXTURES; ++ui)
        {
            if (m_sTextureName.size() > ui)
                m_sTextureName[ui].clear();

            m_hTexture[ui] = INVALID_RESOURCE;
        }
        return;
    }

    m_sTextureName[0] += sSuffix + _T(".") + Filename_GetExtension(sTexture);

    K2_WITH_RESOURCE_SCOPE(GetResourceContext())
        UITextureRegistry.Register(m_sTextureName[0], 0, m_hTexture[0]);

    for (uint ui(1); ui < MAX_WIDGET_TEXTURES; ++ui)
    {
        if (m_sTextureName.size() > ui)
            m_sTextureName[ui].clear();

        m_hTexture[ui] = INVALID_RESOURCE;
    }
}


/*====================
  IWidget::GetTexture
  ====================*/
const tstring&  IWidget::GetTexture(uint uiIdx)
{
    assert(uiIdx < MAX_WIDGET_TEXTURES);
    if (uiIdx >= MAX_WIDGET_TEXTURES)
        return TSNULL;

    if (uiIdx >= m_sTextureName.size())
        return TSNULL;

    return m_sTextureName[uiIdx];
}


/*====================
  IWidget::SetLerp
  ====================*/
void IWidget::SetLerp(tstring sName, float fTarget, uint iTime, uint iType , int iStyle)
{
    if(!m_pLerp)
    {
        m_pLerp = K2_NEW(ctx_Lerps,  CLerpFloat)(sName, fTarget, iTime, iType, iStyle);
    }
    else
    {
        m_pLerp->Reset(fTarget, iTime, iType, iStyle);
    }
}

/*====================
  IWidget::AddChild
  ====================*/
void    IWidget::AddChild(IWidget *pChild)
{
    if (pChild == NULL)
        return;

    if (HasFlags(WFLAG_REVERSE))
        m_vChildren.insert(m_vChildren.begin(), pChild);
    else
        m_vChildren.push_back(pChild);

    if (HasFlags(WFLAG_GROW_WITH_CHILDREN))
        RecalculateSize();

    m_refAdhereTarget = pChild;
    if (pChild->m_eAdhere == WFLOAT_NONE)
        m_refFloatTarget = pChild;
}

/*====================
  IWidget::RemoveChild
  ====================*/
bool    IWidget::RemoveChild(IWidget *pChild)
{
    for (WidgetPointerVector_it it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
    {
        if (*it == pChild)
        {
            m_vChildren.erase(it);
            return true;
        }
    }
    return false;
}


/*====================
  IWidget::SetGroup
  ====================*/
void    IWidget::SetGroup(const tstring &sGroupName)
{
    m_pInterface->RemoveWidgetFromGroup(this);

    m_sGroupName = sGroupName;
    if (!m_sGroupName.empty())
        m_pInterface->AddWidgetToGroup(this);
}


/*====================
  IWidget::SetEventCommand
  ====================*/
void    IWidget::SetEventCommand(EWidgetEvent eEvent, const tstring &sCommand)
{
    if (eEvent < 0 || eEvent >= NUM_WEVENTS)
        return;

    if (sCommand.empty() && m_WidgetEvents.empty())
        return;

    if (m_WidgetEvents.empty())
        m_WidgetEvents.resize(NUM_WEVENTS, TSNULL);

    m_WidgetEvents[eEvent] = sCommand;
}


/*====================
  IWidget::ClearEventCommand
  ====================*/
void    IWidget::ClearEventCommand(EWidgetEvent eEvent)
{
    if (eEvent < 0 || eEvent >= NUM_WEVENTS)
        return;

    if (m_WidgetEvents.empty())
        return;

    m_WidgetEvents[eEvent].clear();
}


/*====================
  IWidget::DoEvent
  ====================*/
void    IWidget::DoEvent(EWidgetEvent eEvent, const tstring &sParam)
{
    if (eEvent < 0 || eEvent >= NUM_WEVENTS)
        return;

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;

    if(eEvent == WEVENT_MOUSEOUT)
    {
        if(m_bMouseOut == true)
            return;
        else
            m_bMouseOut = true;
    }
    else if(eEvent == WEVENT_MOUSEOVER)
    {
        if(m_bMouseOut == false)
            return;
        else
            m_bMouseOut = false;
    }

    if (eEvent == WEVENT_HIDE || eEvent == WEVENT_DISABLE)
    {
        if (m_pInterface->GetActiveWidget() == this)
            m_pInterface->SetActiveWidget(NULL);
        if (m_pInterface->GetExclusiveWidget() == this)
            m_pInterface->SetExclusiveWidget(NULL);
    }

    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (!m_WidgetEvents.empty() && !m_WidgetEvents[eEvent].empty())
    {
        tsvector vParam(1);
        vParam[0] = sParam;
        UIScript.Evaluate(this, m_WidgetEvents[eEvent], vParam);
    }

    if (HasFlags(WFLAG_RELEASED))
        return;

    if (WIDGET_EVENT_RECURSIVE[eEvent])
    {
        for (WidgetPointerVector_it it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
            (*it)->DoEvent(eEvent, sParam);
    }
}


/*====================
  IWidget::DoEvent
  ====================*/
void    IWidget::DoEvent(EWidgetEvent eEvent, const tsvector &vParam)
{
    if (eEvent < 0 || eEvent >= NUM_WEVENTS)
        return;

    if(eEvent == WEVENT_MOUSEOUT)
    {
        if(m_bMouseOut == true)
            return;
        else
            m_bMouseOut = true;
    }
    else if(eEvent == WEVENT_MOUSEOVER)
    {
        if(m_bMouseOut == false)
            return;
        else
            m_bMouseOut = false;
    }

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;

    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (!m_WidgetEvents.empty() && !m_WidgetEvents[eEvent].empty())
    {
        UIScript.Evaluate(this, m_WidgetEvents[eEvent], vParam);
    }

    if (HasFlags(WFLAG_RELEASED))
        return;

    if (WIDGET_EVENT_RECURSIVE[eEvent])
    {
        for (WidgetPointerVector_it it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
            (*it)->DoEvent(eEvent, vParam);
    }
}


/*====================
  IWidget::IsAbsoluteVisible
  ====================*/
bool    IWidget::IsAbsoluteVisible()
{
    if (m_pParent == NULL)
    {
        CInterface *pInterface(GetInterface());

        if (pInterface != NULL && pInterface != UIManager.GetActiveInterface() && pInterface != UIManager.GetSavedActiveInterface() && !UIManager.IsOverlayInterface(pInterface))
            return false;

        return HasFlags(WFLAG_VISIBLE);
    }
    else
        return HasFlags(WFLAG_VISIBLE) && m_pParent->IsAbsoluteVisible();
}


/*====================
  IWidget::IsAbsoluteEnabled
  ====================*/
bool    IWidget::IsAbsoluteEnabled()
{
    if (m_pParent == NULL)
        return HasFlags(WFLAG_ENABLED);
    else
        return HasFlags(WFLAG_ENABLED) && m_pParent->IsAbsoluteEnabled();
}


/*====================
  IWidget::Execute
  ====================*/
void    IWidget::Execute(const tstring &sScript)
{
    UIScript.Evaluate(this, sScript);
}


/*====================
  IWidget::SetRenderMode
  ====================*/
void    IWidget::SetRenderMode(const tstring &sRenderMode)
{
    if (sRenderMode == _T("additive"))
        m_eRenderMode = WRENDER_ADDITIVE;
    else if (sRenderMode == _T("overlay"))
        m_eRenderMode = WRENDER_OVERLAY;
    else if (sRenderMode == _T("grayscale"))
        m_eRenderMode = WRENDER_GRAYSCALE;
    else if (sRenderMode == _T("blur"))
        m_eRenderMode = WRENDER_BLUR;
    else
        m_eRenderMode = WRENDER_NORMAL;
}


/*====================
  IWidget::CheckSnapTargets
  ====================*/
bool    IWidget::CheckSnapTargets(CVec2f &v2Pos, IWidget *pWidget)
{
    WidgetPointerVector_it it(m_vChildren.begin());
    WidgetPointerVector_it itEnd(m_vChildren.end());
    bool bFoundTarget(false);

    while (it != itEnd && !bFoundTarget)
    {
        if (*it == pWidget)
        {
            ++it;
            continue;
        }

        bFoundTarget = (*it)->CheckSnapTargets(v2Pos, pWidget);
        ++it;
    }

    return bFoundTarget;
}


/*====================
  IWidget::CheckSnapTo
  ====================*/
bool    IWidget::CheckSnapTo(CVec2f &v2Pos, IWidget *pWidget)
{
    WidgetPointerVector_it it(m_vChildren.begin());
    WidgetPointerVector_it itEnd(m_vChildren.end());

    bool bFoundTarget(false);

    while (it != itEnd && !bFoundTarget)
    {
        if (*it == pWidget)
        {
            ++it;
            continue;
        }

        bFoundTarget = (*it)->CheckSnapTo(v2Pos, pWidget);
        ++it;
    }

    return bFoundTarget;
}


/*====================
  IWidget::AllocateWidgetState
  ====================*/
CWidgetState*   IWidget::AllocateWidgetState(const CWidgetStyle &style)
{
    return K2_NEW(ctx_Widgets,  CWidgetState)(m_pInterface, this, style);
}


/*====================
  IWidget::AddWidgetState
  ====================*/
bool    IWidget::AddWidgetState(CWidgetState *pState)
{
    SAFE_DELETE(pState); // HA!
    return false;
}


/*====================
  IWidget::RequestPurge
  ====================*/
void    IWidget::RequestPurge()
{
    if (m_pParent)
        m_pParent->RequestPurge();

    SetFlags(WFLAG_NEEDSPURGE);
}


/*====================
  IWidget::ChildHasFocus
  ====================*/
bool    IWidget::ChildHasFocus()
{
    if (m_pInterface == NULL)
        return false;

    IWidget *pActive(m_pInterface->GetActiveWidget());

    for (WidgetPointerVector_it itChild(m_vChildren.begin()), itEnd(m_vChildren.end()); itChild != itEnd; ++itChild)
    {
        if (*itChild != pActive && !((*itChild)->ChildHasFocus()))
            continue;

        return true;
    }

    return false;
}


/*====================
  IWidget::GetAbsoluteFractionX
  ====================*/
float   IWidget::GetAbsoluteFractionX(float fFraction) const
{
    CVec2f v2ParentPos(m_pParent != NULL ? m_pParent->GetAbsolutePos() : V2_ZERO);

    return v2ParentPos.x + LERP(fFraction, m_recArea.left, m_recArea.right);
}


/*====================
  IWidget::GetAbsoluteFractionY
  ====================*/
float   IWidget::GetAbsoluteFractionY(float fFraction) const
{
    CVec2f v2ParentPos(m_pParent != NULL ? m_pParent->GetAbsolutePos() : V2_ZERO);

    return v2ParentPos.y + LERP(fFraction, m_recArea.top, m_recArea.bottom);
}


/*====================
  IWidget::SetPassiveChildren
  ====================*/
void    IWidget::SetPassiveChildren(bool bPassiveChildren)
{
    if (bPassiveChildren)
        SetFlags(WFLAG_PASSIVE_CHILDREN);
    else
        UnsetFlags(WFLAG_PASSIVE_CHILDREN);
}


/*====================
  IWidget::SetNoClick
  ====================*/
void    IWidget::SetNoClick(bool bNoClick)
{
    if (bNoClick)
        SetFlags(WFLAG_NO_CLICK);
    else
        UnsetFlags(WFLAG_NO_CLICK);
}


/*====================
  IWidget::SetFocus
  ====================*/
void    IWidget::SetFocus(bool bFocus)
{
    if (bFocus)
    {
        m_pInterface->SetActiveWidget(this);
    }
    else
    {
        if (m_pInterface->GetActiveWidget() == this)
            m_pInterface->SetActiveWidget(NULL);
    }
}


/*====================
  IWidget::HasFocus
  ====================*/
bool    IWidget::HasFocus() const
{
    return m_pInterface->GetActiveWidget() == this;
}


/*====================
  IWidget::DeleteChildren
  ====================*/
void    IWidget::DeleteChildren()
{
    vector<IWidget*> vChildren(m_vChildren);
    for (vector<IWidget*>::iterator itChild(vChildren.begin()), itEnd(vChildren.end()); itChild != itEnd; ++itChild)
        SAFE_DELETE(*itChild);
}


/*--------------------
  GetTexture
  --------------------*/
UI_CMD(GetTexture, 0)
{
    PROFILE("GetTexture");

    if (pThis == NULL)
        return TSNULL;

    int iIdx(0);
    if (vArgList.size() >= 1)
        iIdx = AtoI(vArgList[0]->Evaluate());

    if (iIdx < 0 || iIdx > MAX_WIDGET_TEXTURES)
    {
        Console.Warn << _T("GetTexture(") << iIdx << _T(") - the index is out of range.  Must be greater than 0 and less than ") << MAX_WIDGET_TEXTURES << newl;
        return TSNULL;
    }

    return pThis->GetTexture(INT_SIZE(iIdx));
}


/*--------------------
  SetTexture
  --------------------*/
UI_VOID_CMD(SetTexture, 1)
{
    PROFILE("SetTexture");

    if (pThis == NULL)
        return;

    pThis->SetTexture(vArgList[0]->Evaluate());
}


/*--------------------
  precacheSetTexture
  --------------------*/
CMD_PRECACHE(SetTexture)
{
    assert(!"does anything call CMD_PRECACHE for SetTexture?");

    if (vArgList.size() < 1)
        return false;

    ResHandle hTexture(INVALID_RESOURCE);

    UITextureRegistry.Register(vArgList[0], 0, hTexture);

    return true;
}


/*--------------------
  SetColor
  --------------------*/
UI_VOID_CMD(SetColor, 1)
{
    if (!pThis)
        return;

    if (vArgList.size() == 4)
        pThis->SetColor(CVec4f(AtoF(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate()), AtoF(vArgList[2]->Evaluate()), AtoF(vArgList[3]->Evaluate())));
    else if (vArgList.size() == 3)
        pThis->SetColor(CVec4f(AtoF(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate()), AtoF(vArgList[2]->Evaluate()), 1.0f));
    else
        pThis->SetColor(vArgList[0]->Evaluate());
}


/*--------------------
  GetColorFromPosition
  --------------------*/
UI_CMD(GetColorFromPosition, 1)
{
    if (vArgList.size() < 1)
        return _T("white");
        
    tstring sColor;
    
    switch (AtoI(vArgList[0]->Evaluate()))
    {
        case 0:
            sColor = _T("pcblue");
            break;
        case 1:
            sColor = _T("pcteal");
            break;
        case 2:
            sColor = _T("pcpurple");
            break;
        case 3:
            sColor = _T("pcyellow");
            break;
        case 4:
            sColor = _T("pcorange");
            break;
        case 5:
            sColor = _T("pcpink");
            break;
        case 6:
            sColor = _T("pcgray");
            break;
        case 7:
            sColor = _T("pclightblue");
            break;
        case 8:
            sColor = _T("pcdarkgreen");
            break;
        case 9:
            sColor = _T("pcbrown");
            break;
        default:
            sColor = _T("white");
            break;
    }
    
    return sColor;
}


/*--------------------
  SetVisible
  --------------------*/
UI_VOID_CMD(SetVisible, 1)
{
    if (!pThis)
        return;

    uint uiDuration(-1);
    if (vArgList.size() > 1)
        uiDuration = AtoI(vArgList[1]->Evaluate());

    if (AtoB(vArgList[0]->Evaluate()))
        pThis->Show(uiDuration);
    else
        pThis->Hide();
}


/*--------------------
  IsVisible
  --------------------*/
UI_CMD(IsVisible, 0)
{
    if (pThis == NULL)
        return _T("0");

    if (!vArgList.empty())
    {
        if (pThis->GetInterface() == NULL)
            return _T("0");

        IWidget *pTarget(pThis->GetInterface()->GetWidget(vArgList[0]->Evaluate()));

        if (pTarget == NULL)
            return _T("0");

        return XtoA(pTarget->IsAbsoluteVisible(), true);
    }

    return XtoA(pThis->IsAbsoluteVisible(), true);
}

/*--------------------
  IsEnabled
  --------------------*/
UI_CMD(IsEnabled, 0)
{
    if (pThis == NULL)
        return _T("0");

    if (!vArgList.empty())
    {
        if (pThis->GetInterface() == NULL)
            return _T("0");

        IWidget *pTarget(pThis->GetInterface()->GetWidget(vArgList[0]->Evaluate()));

        if (pTarget == NULL)
            return _T("0");

        return XtoA(pTarget->IsAbsoluteEnabled(), true);
    }

    return XtoA(pThis->IsAbsoluteEnabled(), true);
}

/*--------------------
  SetEnabled
  --------------------*/
UI_VOID_CMD(SetEnabled, 1)
{
    if (!pThis)
        return;

    if (AtoB(vArgList[0]->Evaluate()))
        pThis->Enable();
    else
        pThis->Disable();
}


/*--------------------
  SetFocus
  --------------------*/
UI_VOID_CMD(SetFocus, 1)
{
    if (!pThis)
        return;

    CInterface *pInterface(pThis->GetInterface());

    if (AtoB(vArgList[0]->Evaluate()))
    {
        pInterface->SetActiveWidget(pThis);
    }
    else
    {
        if (pInterface->GetActiveWidget() == pThis)
            pInterface->SetActiveWidget(NULL);
    }
}


/*--------------------
  HasFocus
  --------------------*/
UI_CMD(HasFocus, 0)
{
    if (!pThis)
        return _T("0");

    CInterface *pInterface(pThis->GetInterface());

    if (!pInterface)
        return _T("0");

    return XtoA(pInterface->GetActiveWidget() == pThis, true);
}


/*--------------------
  SetDefaultFocus
  --------------------*/
UI_VOID_CMD(SetDefaultFocus, 1)
{
    if (!pThis)
        return;

    CInterface *pInterface(pThis->GetInterface());

    if (AtoB(vArgList[0]->Evaluate()))
    {
        pInterface->SetDefaultActiveWidget(pThis);
    }
    else
    {
        if (pInterface->GetDefaultActiveWidget() == pThis)
            pInterface->SetDefaultActiveWidget(NULL);
    }
}



/*--------------------
  BreakExclusive
  --------------------*/
UI_VOID_CMD(BreakExclusive, 0)
{
    if (!pThis)
        return;

    CInterface *pInterface(pThis->GetInterface());

    if (pInterface->GetExclusiveWidget() == pThis)
        pInterface->SetExclusiveWidget(NULL);
}


/*--------------------
  SetWidth
  --------------------*/
UI_VOID_CMD(SetWidth, 1)
{
    if (!pThis)
        return;

    pThis->SetBaseWidth(vArgList[0]->Evaluate());
    pThis->RecalculateSize();
}


/*--------------------
  SetHeight
  --------------------*/
UI_VOID_CMD(SetHeight, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetBaseHeight(vArgList[0]->Evaluate());
    pThis->RecalculateSize();
}


/*--------------------
  SetUOffset
  --------------------*/
UI_VOID_CMD(SetUOffset, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetUOffset(pThis->GetTextureOffsetFromString(vArgList[0]->Evaluate(), X));
}


/*--------------------
  SetVOffset
  --------------------*/
UI_VOID_CMD(SetVOffset, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetVOffset(pThis->GetTextureOffsetFromString(vArgList[0]->Evaluate(), Y));
}


/*--------------------
  SetUScale
  --------------------*/
UI_VOID_CMD(SetUScale, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetUScale(pThis->GetTextureScaleFromString(vArgList[0]->Evaluate(), X));
}


/*--------------------
  SetVScale
  --------------------*/
UI_VOID_CMD(SetVScale, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetVScale(pThis->GetTextureScaleFromString(vArgList[0]->Evaluate(), Y));
}


/*--------------------
  SetUSpeed
  --------------------*/
UI_VOID_CMD(SetUSpeed, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetUSpeed(pThis->GetTextureOffsetFromString(vArgList[0]->Evaluate(), X));
}


/*--------------------
  SetVSpeed
  --------------------*/
UI_VOID_CMD(SetVSpeed, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetVSpeed(pThis->GetTextureOffsetFromString(vArgList[0]->Evaluate(), Y));
}


/*--------------------
  GetWidth
  --------------------*/
UI_CMD(GetWidth, 0)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetWidth());
}

/*--------------------
  GetHeight
  --------------------*/
UI_CMD(GetHeight, 0)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetHeight());
}


/*--------------------
  GetX
  --------------------*/
UI_CMD(GetX, 0)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetX());
}


/*--------------------
  GetParentX
  --------------------*/
UI_CMD(GetParentX, 0)
{
    if (!pThis || !pThis->GetParent())
        return XtoA(0.0f);

    IWidget* pParent(pThis->GetParent());
    return XtoA(pParent->GetX());
}


/*--------------------
  SetX
  --------------------*/
UI_VOID_CMD(SetX, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetBaseX(vArgList[0]->Evaluate());
    pThis->RecalculatePosition();
}


/*--------------------
  GetAbsoluteX
  --------------------*/
UI_CMD(GetAbsoluteX, 0)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetAbsolutePos().x);
}


/*--------------------
  GetParentAbsoluteX
  --------------------*/
UI_CMD(GetParentAbsoluteX, 0)
{
    if (!pThis || !pThis->GetParent())
        return XtoA(0.0f);

    IWidget* pParent(pThis->GetParent());
    return XtoA(pParent->GetAbsolutePos().x);
}


/*--------------------
  SetAbsoluteX
  --------------------*/
UI_VOID_CMD(SetAbsoluteX, 1)
{
    if (pThis == NULL)
        return;

    float fPos(AtoF(vArgList[0]->Evaluate()));

    fPos = (fPos - pThis->GetAbsolutePos().x) + pThis->GetX();

    pThis->SetX(fPos);
}


/*--------------------
  GetAbsoluteFractionX
  --------------------*/
UI_CMD(GetAbsoluteFractionX, 1)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetAbsoluteFractionX(AtoF(vArgList[0]->Evaluate())));
}


/*--------------------
  SetY
  --------------------*/
UI_VOID_CMD(SetY, 1)
{
    if (!pThis)
        return;

    pThis->SetBaseY(vArgList[0]->Evaluate());
    pThis->RecalculatePosition();
}


/*--------------------
  GetY
  --------------------*/
UI_CMD(GetY, 0)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetY());
}


/*--------------------
  GetParentY
  --------------------*/
UI_CMD(GetParentY, 0)
{
    if (!pThis || !pThis->GetParent())
        return XtoA(0.0f);

    IWidget* pParent(pThis->GetParent());
    return XtoA(pParent->GetY());
}


/*--------------------
  GetAbsoluteY
  --------------------*/
UI_CMD(GetAbsoluteY, 0)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetAbsolutePos().y);
}


/*--------------------
  GetParentAbsoluteY
  --------------------*/
UI_CMD(GetParentAbsoluteY, 0)
{
    if (!pThis || !pThis->GetParent())
        return XtoA(0.0f);

    IWidget* pParent(pThis->GetParent());
    return XtoA(pParent->GetAbsolutePos().y);
}

/*--------------------
  SetAbsoluteY
  --------------------*/
UI_VOID_CMD(SetAbsoluteY, 1)
{
    if (pThis == NULL)
        return;

    float fPos(AtoF(vArgList[0]->Evaluate()));

    fPos = (fPos - pThis->GetAbsolutePos().y) + pThis->GetY();

    pThis->SetY(fPos);
}


/*--------------------
  GetAbsoluteFractionY
  --------------------*/
UI_CMD(GetAbsoluteFractionY, 1)
{
    if (!pThis)
        return XtoA(0.0f);

    return XtoA(pThis->GetAbsoluteFractionY(AtoF(vArgList[0]->Evaluate())));
}


/*--------------------
  MoveToCursor
  --------------------*/
UI_VOID_CMD(MoveToCursor, 0)
{
    if (pThis == NULL)
        return;

    CVec2f v2Pos(Input.GetCursorPos());

    v2Pos = (v2Pos - pThis->GetAbsolutePos()) + pThis->GetPos();

    pThis->SetX(v2Pos.x);
    pThis->SetY(v2Pos.y);
}


/*--------------------
  Slide
  --------------------*/
UI_VOID_CMD(Slide, 3)
{
    if (pThis == NULL)
        return;

    float fParentWidth(Draw2D.GetScreenW());
    float fParentHeight(Draw2D.GetScreenH());
    float fBaseX(0.0f);
    float fBaseY(0.0f);
    IWidget *pParent(pThis->GetParent());
    if (pParent != NULL)
    {
        fParentWidth = pParent->GetWidth();
        fParentHeight = pParent->GetHeight();

        if (pThis->GetAlign() == ALIGN_CENTER)
            fBaseX = (pParent->GetWidth() / 2.0f) - (pThis->GetWidth() / 2.0f);
        else if (pThis->GetAlign() == ALIGN_RIGHT)
            fBaseX = pParent->GetWidth() - pThis->GetWidth();

        if (pThis->GetVAlign() == ALIGN_CENTER)
            fBaseY = (pParent->GetHeight() / 2.0f) - (pThis->GetHeight() / 2.0f);
        else if (pThis->GetVAlign() == ALIGN_RIGHT)
            fBaseY = pParent->GetHeight() - pThis->GetHeight();
    }

    tstring sTargetX(vArgList[0]->Evaluate());
    float fX(0.0f);
    if (sTargetX[0] == _T('+'))
        fX = pThis->GetX() + fBaseX + pThis->GetPositionFromString(sTargetX.substr(1), fParentWidth, fParentHeight);
    else
        fX = fBaseX + pThis->GetPositionFromString(sTargetX, fParentWidth, fParentHeight);

    tstring sTargetY(vArgList[1]->Evaluate());
    float fY(0.0f);
    if (sTargetY[0] == _T('+'))
        fY = pThis->GetY() + fBaseY + pThis->GetPositionFromString(sTargetY.substr(1), fParentHeight, fParentWidth);
    else
        fY = fBaseY + pThis->GetPositionFromString(sTargetY, fParentHeight, fParentWidth);

    pThis->SlideX(fX, AtoI(vArgList[2]->Evaluate()));
    pThis->SlideY(fY, AtoI(vArgList[2]->Evaluate()));
}


/*--------------------
  SlideX
  --------------------*/
UI_VOID_CMD(SlideX, 2)
{
    if (pThis == NULL)
        return;

    float fParentWidth(Draw2D.GetScreenW());
    float fParentHeight(Draw2D.GetScreenH());
    float fBaseX(0.0f);
    IWidget *pParent(pThis->GetParent());
    if (pParent != NULL)
    {
        fParentWidth = pParent->GetWidth();
        fParentHeight = pParent->GetHeight();

        if (pThis->GetAlign() == ALIGN_CENTER)
            fBaseX = (pParent->GetWidth() / 2.0f) - (pThis->GetWidth() / 2.0f);
        else if (pThis->GetAlign() == ALIGN_RIGHT)
            fBaseX = pParent->GetWidth() - pThis->GetWidth();
    }

    tstring sTarget(vArgList[0]->Evaluate());
    float fX(0.0f);
    if (sTarget[0] == _T('+'))
        fX = pThis->GetX() + fBaseX + pThis->GetPositionFromString(sTarget.substr(1), fParentWidth, fParentHeight);
    else
        fX = fBaseX + pThis->GetPositionFromString(sTarget, fParentWidth, fParentHeight);

    pThis->SlideX(fX, AtoI(vArgList[1]->Evaluate()));
}


/*--------------------
  SlideY
  --------------------*/
UI_VOID_CMD(SlideY, 2)
{
    if (pThis == NULL)
        return;

    float fParentWidth(Draw2D.GetScreenW());
    float fParentHeight(Draw2D.GetScreenH());
    float fBaseY(0.0f);
    IWidget *pParent(pThis->GetParent());
    if (pParent != NULL)
    {
        fParentWidth = pParent->GetWidth();
        fParentHeight = pParent->GetHeight();

        if (pThis->GetVAlign() == ALIGN_CENTER)
            fBaseY = (pParent->GetHeight() / 2.0f) - (pThis->GetHeight() / 2.0f);
        else if (pThis->GetVAlign() == ALIGN_RIGHT)
            fBaseY = pParent->GetHeight() - pThis->GetHeight();
    }

    tstring sTarget(vArgList[0]->Evaluate());
    float fY(0.0f);
    if (sTarget[0] == _T('+'))
        fY = pThis->GetY() + fBaseY + pThis->GetPositionFromString(sTarget.substr(1), fParentHeight, fParentWidth);
    else
        fY = fBaseY + pThis->GetPositionFromString(sTarget, fParentHeight, fParentWidth);

    pThis->SlideY(fY, AtoI(vArgList[1]->Evaluate()));
}


/*--------------------
  Rotate
  --------------------*/
UI_VOID_CMD(Rotate, 2)
{
    if (pThis == NULL)
        return;

    tstring sTarget(vArgList[0]->Evaluate());
    float fRotation(0.0f);
    if (sTarget[0] == _T('+'))
        fRotation = pThis->GetRotation() + AtoF(sTarget.substr(1));
    else
        fRotation = AtoF(sTarget);

    bool bRecurse(false);
    if (vArgList.size() > 2)
        bRecurse = AtoB(vArgList[2]->Evaluate());

    pThis->Rotate(fRotation, AtoI(vArgList[1]->Evaluate()), bRecurse);
}


/*--------------------
  Scale
  --------------------*/
UI_VOID_CMD(Scale, 3)
{
    if (pThis == NULL)
        return;

    float fParentWidth(Draw2D.GetScreenW());
    float fParentHeight(Draw2D.GetScreenH());
    IWidget *pParent(pThis->GetParent());
    if (pParent != NULL)
    {
        fParentWidth = pParent->GetWidth();
        fParentHeight = pParent->GetHeight();
    }

    tstring sTargetWidth(vArgList[0]->Evaluate());
    float fWidth(0.0f);
    if (sTargetWidth[0] == _T('+'))
        fWidth = pThis->GetWidth() + pThis->GetSizeFromString(sTargetWidth.substr(1), fParentWidth, fParentHeight);
    else
        fWidth = pThis->GetSizeFromString(sTargetWidth, fParentWidth, fParentHeight);

    tstring sTargetHeight(vArgList[1]->Evaluate());
    float fHeight(0.0f);
    if (sTargetHeight[0] == _T('+'))
        fHeight = pThis->GetHeight() + pThis->GetSizeFromString(sTargetHeight.substr(1), fParentHeight, fParentWidth);
    else
        fHeight = pThis->GetSizeFromString(sTargetHeight, fParentHeight, fParentWidth);

    int iDirection = 0;
    if(vArgList.size() > 2)
    {
        iDirection = AtoI(vArgList[2]->Evaluate());
    }

    bool bRecurse(false);
    if (vArgList.size() > 3)
        bRecurse = AtoB(vArgList[3]->Evaluate());

    pThis->ScaleWidth(fWidth, AtoI(vArgList[2]->Evaluate()), iDirection, bRecurse);
    pThis->ScaleHeight(fHeight, AtoI(vArgList[2]->Evaluate()), iDirection, bRecurse);
}


/*--------------------
  ScaleWidth
  --------------------*/
UI_VOID_CMD(ScaleWidth, 2)
{
    if (pThis == NULL)
        return;

    float fParentWidth(Draw2D.GetScreenW());
    float fParentHeight(Draw2D.GetScreenH());
    IWidget *pParent(pThis->GetParent());
    if (pParent != NULL)
    {
        fParentWidth = pParent->GetWidth();
        fParentHeight = pParent->GetHeight();
    }

    tstring sTargetWidth(vArgList[0]->Evaluate());
    float fWidth(0.0f);
    if (sTargetWidth[0] == _T('+'))
        fWidth = pThis->GetWidth() + pThis->GetSizeFromString(sTargetWidth.substr(1), fParentWidth, fParentHeight);
    else
        fWidth = pThis->GetSizeFromString(sTargetWidth, fParentWidth, fParentHeight);

    int iDirection = 0;
    if(vArgList.size() > 2)
    {
        iDirection = AtoI(vArgList[2]->Evaluate());
    }

    bool bRecurse(false);
    if (vArgList.size() > 3)
        bRecurse = AtoB(vArgList[3]->Evaluate());

    pThis->ScaleWidth(fWidth, AtoI(vArgList[1]->Evaluate()), iDirection, bRecurse);
}


/*--------------------
  ScaleHeight
  --------------------*/
UI_VOID_CMD(ScaleHeight, 2)
{
    if (pThis == NULL)
        return;

    float fParentWidth(Draw2D.GetScreenW());
    float fParentHeight(Draw2D.GetScreenH());
    IWidget *pParent(pThis->GetParent());
    if (pParent != NULL)
    {
        fParentWidth = pParent->GetWidth();
        fParentHeight = pParent->GetHeight();
    }

    tstring sTargetHeight(vArgList[0]->Evaluate());
    float fHeight(0.0f);
    if (sTargetHeight[0] == _T('+'))
        fHeight = pThis->GetHeight() + pThis->GetSizeFromString(sTargetHeight.substr(1), fParentHeight, fParentWidth);
    else
        fHeight = pThis->GetSizeFromString(sTargetHeight, fParentHeight, fParentWidth);

    int iDirection = 0;
    if(vArgList.size() > 2)
    {
        iDirection = AtoI(vArgList[2]->Evaluate());
    }

    bool bRecurse(false);
    if (vArgList.size() > 3)
        bRecurse = AtoB(vArgList[3]->Evaluate());

    pThis->ScaleHeight(fHeight, AtoI(vArgList[1]->Evaluate()), iDirection, bRecurse);
}


/*--------------------
  FadeOut
  --------------------*/
UI_VOID_CMD(FadeOut, 1)
{
    if (pThis == NULL)
        return;

    pThis->FadeOut(vArgList[0]->EvaluateInteger());

    pThis->ClearEventCommand(WEVENT_FADED);
    if (vArgList.size() >= 2)
        pThis->SetEventCommand(WEVENT_FADED, vArgList[1]->Evaluate());
}


/*--------------------
  FadeIn
  --------------------*/
UI_VOID_CMD(FadeIn, 1)
{
    if (pThis == NULL)
        return;

    pThis->FadeIn(vArgList[0]->EvaluateInteger());

    pThis->ClearEventCommand(WEVENT_FADED);
    if (vArgList.size() >= 2)
        pThis->SetEventCommand(WEVENT_FADED, vArgList[1]->Evaluate());
}


/*--------------------
  Fade
  --------------------*/
UI_VOID_CMD(Fade, 3)
{
    if (pThis == NULL)
        return;

    pThis->Fade(AtoF(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate()), vArgList[2]->EvaluateInteger());

    pThis->ClearEventCommand(WEVENT_FADED);
    if (vArgList.size() >= 4)
        pThis->SetEventCommand(WEVENT_FADED, vArgList[3]->Evaluate());
}


/*--------------------
  SetValue
  --------------------*/
UI_VOID_CMD(SetValue, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetValue(vArgList[0]->Evaluate());
}


/*--------------------
  GetValue
  --------------------*/
UI_CMD(GetValue, 0)
{
    if (pThis == NULL)
        return _T("");

    return pThis->GetValue();
}


/*--------------------
  SetRenderMode
  --------------------*/
UI_VOID_CMD(SetRenderMode, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetRenderMode(vArgList[0]->Evaluate());
}


/*--------------------
  SetRotation
  --------------------*/
UI_VOID_CMD(SetRotation, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetRotation(AtoF(vArgList[0]->Evaluate()));
}

/*--------------------
  Sin
  --------------------*/
UI_CMD(Sin, 1)
{
    return XtoA(sin(AtoF(vArgList[0]->Evaluate())));
}

/*--------------------
  Cos
  --------------------*/
UI_CMD(Cos, 1)
{
    return XtoA(cos(AtoF(vArgList[0]->Evaluate())));
}

/*--------------------
  Tan
  --------------------*/
UI_CMD(Tan, 1)
{
    return XtoA(tan(AtoF(vArgList[0]->Evaluate())));
}


/*--------------------
  SetWatch
  --------------------*/
UI_VOID_CMD(SetWatch, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetWatch(vArgList[0]->Evaluate());
}


/*--------------------
  SetOntrigger
  --------------------*/
UI_VOID_CMD(SetOntrigger, 1)
{
    if (pThis == NULL)
        return;

    if (vArgList.size() == 1)
    {
        pThis->SetEventCommand(WEVENT_TRIGGER, vArgList[0]->Evaluate());
        return;
    }

    int iIdx(vArgList[0]->EvaluateInteger());
    if (iIdx < 0 || iIdx > 9)
        return;

    pThis->SetEventCommand(EWidgetEvent(WEVENT_TRIGGER0 + iIdx), vArgList[1]->Evaluate());
}


/*--------------------
  SetOnclick
  --------------------*/
UI_VOID_CMD(SetOnclick, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetEventCommand(WEVENT_CLICK, vArgList[0]->Evaluate());
}


/*--------------------
  SetOnrightclick
  --------------------*/
UI_VOID_CMD(SetOnrightclick, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetEventCommand(WEVENT_RIGHTCLICK, vArgList[0]->Evaluate());
}


/*--------------------
  SetOnmouseover
  --------------------*/
UI_VOID_CMD(SetOnmouseover, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetEventCommand(WEVENT_MOUSEOVER, vArgList[0]->Evaluate());
}


/*--------------------
  SetOnmouseout
  --------------------*/
UI_VOID_CMD(SetOnmouseout, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetEventCommand(WEVENT_MOUSEOUT, vArgList[0]->Evaluate());
}


/*--------------------
  PrintNumWidgets
  --------------------*/
CMD(PrintNumWidgets)
{
    Console << IWidget::GetNumWidgets() << _T(" widgets") << newl;
    return true;
}


/*--------------------
  DestroyWidget
  --------------------*/
UI_VOID_CMD(DestroyWidget, 0)
{
    CInterface *pInterface(pThis->GetInterface());
    if (pInterface == NULL)
        return;

    IWidget *pWidget(pThis);
    if (vArgList.size() > 0)
        pWidget = pInterface->GetWidget(vArgList[0]->Evaluate());

    if (pWidget == NULL)
        return;

    pWidget->RequestPurge();
    pWidget->Kill();
}


/*--------------------
  SleepWidget
  --------------------*/
UI_VOID_CMD(SleepWidget, 1)
{
    pThis->ClearWakeEvents();
    pThis->SetSleepTimer(AtoI(vArgList[0]->Evaluate()));
    if (vArgList.size() > 1)
        pThis->SetEventCommand(WEVENT_WAKE, vArgList[1]->Evaluate());

    for (size_t uiArgIdx(2); uiArgIdx + 1 < vArgList.size(); uiArgIdx += 2)
    {
        int iDuration(AtoI(vArgList[uiArgIdx]->Evaluate()));
        if (iDuration < 0)
            return;

        pThis->PushWakeEvent(iDuration, vArgList[uiArgIdx + 1]->Evaluate());
    }
}


/*--------------------
  DoEvent
  --------------------*/
UI_VOID_CMD(DoEvent, 0)
{
    if (vArgList.empty())
    {
        pThis->DoEvent(WEVENT_EVENT);
        return;
    }

    int iIdx(vArgList[0]->EvaluateInteger());
    if (iIdx < 0 || iIdx > 9)
        return;

    pThis->DoEvent(EWidgetEvent(WEVENT_EVENT0 + iIdx));
}


/*--------------------
    BringToFront
    --------------------*/
UI_VOID_CMD(BringToFront, 0)
{
    if (pThis == NULL)
        return;

    if (pThis->GetParent() == NULL)
        return;

    pThis->GetParent()->BringChildToFront(pThis);
}


/*--------------------
  SetParent
  --------------------*/
UI_VOID_CMD(SetParent, 1)
{
    if (pThis == NULL)
        return;

    if (pThis->GetParent() == NULL)
        return;

    IWidget *pNewParent(pThis->GetInterface()->GetWidget(vArgList[0]->Evaluate()));

    if(pNewParent && pThis != pNewParent)
    {
        pThis->GetParent()->SetChildNewParent(pThis, pNewParent);
    }
}


/*--------------------
  GetParent
  --------------------*/
UI_CMD(GetParent, 0)
{
    if (pThis == NULL)
        return TSNULL;

    IWidget* pParent(pThis->GetParent());
    if (!pParent)
        return TSNULL;

    return pParent->GetName();
}


/*--------------------
  ForceToFront
  --------------------*/
UI_VOID_CMD(ForceToFront, 0)
{
    if (pThis == NULL)
        return;

    if (pThis->GetInterface() == NULL)
        return;

    if (pThis->GetParent() == NULL)
        return;

    IWidget *pNewParent(pThis->GetInterface());

    if(pNewParent && pThis != pNewParent)
    {
        pThis->GetParent()->SetChildNewParent(pThis, pNewParent);
    }
}

/*--------------------
  Clear
  --------------------*/
UI_VOID_CMD(Clear, 0)
{
    pThis->Clear();
}


/*--------------------
  GetWidthFromString
  --------------------*/
UI_CMD(GetWidthFromString, 1)
{
    IWidget *pWidget(pThis != NULL ? pThis->GetParent() : NULL);

    if (pWidget != NULL)
        return XtoA(IWidget::GetSizeFromString(vArgList[0]->Evaluate(), pWidget->GetWidth(), pWidget->GetHeight()));
    else
        return XtoA(IWidget::GetSizeFromString(vArgList[0]->Evaluate(), Draw2D.GetScreenW(), Draw2D.GetScreenH()));
}


/*--------------------
  GetHeightFromString
  --------------------*/
UI_CMD(GetHeightFromString, 1)
{
    IWidget *pWidget(pThis != NULL ? pThis->GetParent() : NULL);

    if (pWidget != NULL)
        return XtoA(IWidget::GetSizeFromString(vArgList[0]->Evaluate(), pWidget->GetHeight(), pWidget->GetWidth()));
    else
        return XtoA(IWidget::GetSizeFromString(vArgList[0]->Evaluate(), Draw2D.GetScreenH(), Draw2D.GetScreenW()));
}


/*--------------------
  GetXFromString
  --------------------*/
UI_CMD(GetXFromString, 1)
{
    IWidget *pWidget(pThis != NULL ? pThis->GetParent() : NULL);

    if (pWidget != NULL)
        return XtoA(IWidget::GetPositionFromString(vArgList[0]->Evaluate(), pWidget->GetWidth(), pWidget->GetHeight()));
    else
        return XtoA(IWidget::GetPositionFromString(vArgList[0]->Evaluate(), Draw2D.GetScreenW(), Draw2D.GetScreenH()));
}


/*--------------------
  GetYFromString
  --------------------*/
UI_CMD(GetYFromString, 1)
{
    IWidget *pWidget(pThis != NULL ? pThis->GetParent() : NULL);

    if (pWidget != NULL)
        return XtoA(IWidget::GetPositionFromString(vArgList[0]->Evaluate(), pWidget->GetHeight(), pWidget->GetWidth()));
    else
        return XtoA(IWidget::GetPositionFromString(vArgList[0]->Evaluate(), Draw2D.GetScreenH(), Draw2D.GetScreenW()));
}


/*--------------------
  ChildHasFocus
  --------------------*/
UI_CMD(ChildHasFocus, 0)
{
    if (pThis != NULL)
        return XtoA(pThis->ChildHasFocus(), true);

    return _T("0");
}


/*--------------------
  SetPassiveChildren
  --------------------*/
UI_VOID_CMD(SetPassiveChildren, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetPassiveChildren(AtoB(vArgList[0]->Evaluate()));
}


/*--------------------
  SetNoClick
  --------------------*/
UI_VOID_CMD(SetNoClick, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetNoClick(AtoB(vArgList[0]->Evaluate()));
}


/*--------------------
  GetAlign
  --------------------*/
UI_CMD(GetAlign, 0)
{
    if (pThis == NULL)
        return AlignToA(ALIGN_LEFT);

    return AlignToA(pThis->GetAlign());
}


/*--------------------
  GetVAlign
  --------------------*/
UI_CMD(GetVAlign, 0)
{
    if (pThis == NULL)
        return VAlignToA(ALIGN_TOP);

    return VAlignToA(pThis->GetVAlign());
}


/*--------------------
  SetAlign
  --------------------*/
UI_VOID_CMD(SetAlign, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetAlign(AtoAlign(vArgList[0]->Evaluate()));
    pThis->RecalculatePosition();
}


/*--------------------
  SetVAlign
  --------------------*/
UI_VOID_CMD(SetVAlign, 1)
{
    if (pThis == NULL)
        return;

    pThis->SetVAlign(AtoVAlign(vArgList[0]->Evaluate()));
    pThis->RecalculatePosition();
}


/*--------------------
  RecalculateSize
  --------------------*/
UI_VOID_CMD(RecalculateSize, 0)
{
    if (pThis == NULL)
        return;

    pThis->RecalculateSize();
}


/*--------------------
  IsVisibleExt
  --------------------*/
UI_CMD(IsVisibleExt, 2)
{
    CInterface *pInterface(UIManager.GetInterface(vArgList[0]->Evaluate()));

    if (pInterface == NULL)
        return _T("0");

    IWidget *pTarget(pInterface->GetWidget(vArgList[1]->Evaluate()));

    if (pTarget == NULL)
        return _T("0");

    return XtoA(pTarget->IsAbsoluteVisible(), true);
}


/*--------------------
  GetAbsoluteXExt
  --------------------*/
UI_CMD(GetAbsoluteXExt, 2)
{
    CInterface *pInterface(UIManager.GetInterface(vArgList[0]->Evaluate()));

    if (pInterface == NULL)
        return _T("0");

    IWidget *pTarget(pInterface->GetWidget(vArgList[1]->Evaluate()));

    if (pTarget == NULL)
        return _T("0");

    return XtoA(pTarget->GetAbsolutePos().x);
}


/*--------------------
  GetAbsoluteYExt
  --------------------*/
UI_CMD(GetAbsoluteYExt, 2)
{
    CInterface *pInterface(UIManager.GetInterface(vArgList[0]->Evaluate()));

    if (pInterface == NULL)
        return _T("0");

    IWidget *pTarget(pInterface->GetWidget(vArgList[1]->Evaluate()));

    if (pTarget == NULL)
        return _T("0");

    return XtoA(pTarget->GetAbsolutePos().y);
}


/*--------------------
  GetAbsoluteFractionXExt
  --------------------*/
UI_CMD(GetAbsoluteFractionXExt, 3)
{
    CInterface *pInterface(UIManager.GetInterface(vArgList[0]->Evaluate()));

    if (pInterface == NULL)
        return _T("0");

    IWidget *pTarget(pInterface->GetWidget(vArgList[1]->Evaluate()));

    if (pTarget == NULL)
        return _T("0");

    return XtoA(pThis->GetAbsoluteFractionX(AtoF(vArgList[2]->Evaluate())));
}


/*--------------------
  GetAbsoluteFractionYExt
  --------------------*/
UI_CMD(GetAbsoluteFractionYExt, 3)
{
    CInterface *pInterface(UIManager.GetInterface(vArgList[0]->Evaluate()));

    if (pInterface == NULL)
        return _T("0");

    IWidget *pTarget(pInterface->GetWidget(vArgList[1]->Evaluate()));

    if (pTarget == NULL)
        return _T("0");

    return XtoA(pThis->GetAbsoluteFractionY(AtoF(vArgList[2]->Evaluate())));
}
