// (C)2006 S2 Games
// c_minimap.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_minimap.h"
#include "c_interface.h"
#include "c_uitextureregistry.h"
#include "c_uiscript.h"
#include "c_uicmd.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
#include "c_draw2d.h"
#include "c_buffer.h"
#include "c_texture.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
#if 0
CVAR_BOOL   (ui_minimapDrawing,     false);
#else
const bool ui_minimapDrawing(false);
#endif
CVAR_BOOL   (ui_minimapPing,        false);
CVAR_INT    (ui_minimapFilter,      -1);
CVAR_INT    (ui_minimapDrawTime,    15000);
//=============================================================================

/*====================
  CMinimap::~CMinimap
  ====================*/
CMinimap::~CMinimap()
{
    if (m_bDragging || m_bDraggingRight)
    {
        Input.SetCursorConstrained(CURSOR_UI, BOOL_NOT_SET);
        Input.SetCursorConstraint(CURSOR_UI, CRectf(0.0f, 0.0f, 0.0f, 0.0f));
    }
}


/*====================
  CMinimap::CMinimap
  ====================*/
CMinimap::CMinimap(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_MAP, style),
m_fWorldOffsetX(0.0f),
m_fWorldOffsetY(0.0f),
m_fWorldPercentX(1.0f),
m_fWorldPercentY(1.0f),
m_bDragging(false),
m_bDraggingRight(false),
m_bNoPing(style.GetPropertyBool(_T("noping"), false)),
m_sOutlineSize(style.GetProperty(_T("outlinesize"), _T("1"))),
m_fHoverSize(style.GetPropertyFloat(_T("hoversize"), 1.0f) + 1.0f),
m_uiHoverUnit(-1),
m_bFlop(false)
{
    m_hFogofWarTexture = g_ResourceManager.Register(K2_NEW(ctx_Widgets,  CTexture)(_T("$fogofwar"), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);

    m_fOutlineSize = GetSizeFromString(m_sOutlineSize, GetWidth(), GetHeight());

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)

    SetFlags(WFLAG_INTERACTIVE);
    SetFlagsRecursive(WFLAG_PROCESS_CURSOR);
}


/*====================
  CMinimap::GetCursorInWorld
  ====================*/
CVec2f  CMinimap::GetCursorInWorld(const CVec2f &v2CursorPos)
{
    CVec2f v2WorldCursor(
        ILERP(v2CursorPos.x, m_recArea.left, m_recArea.right),
        ILERP(v2CursorPos.y, m_recArea.bottom, m_recArea.top)
        );

    if (m_bFlop)
    {
        v2WorldCursor.x = 1.0f - v2WorldCursor.x;
        v2WorldCursor.y = 1.0f - v2WorldCursor.y;
    }

    v2WorldCursor.x /= m_fWorldPercentX;
    v2WorldCursor.x -= m_fWorldOffsetX;
    v2WorldCursor.y /= m_fWorldPercentY;
    v2WorldCursor.y -= m_fWorldOffsetY;

    return v2WorldCursor;
}


/*====================
  CMinimap::ButtonDown
  ====================*/
bool    CMinimap::ButtonDown(EButton button)
{
    switch (button)
    {
    case BUTTON_ENTER:
        break;

    default:
        break;
    }

    return true;
}


/*====================
  CMinimap::Enable
  ====================*/
void    CMinimap::Enable()
{
    IWidget::Enable();
}


/*====================
  CMinimap::Disable
  ====================*/
void    CMinimap::Disable()
{
    IWidget::Disable();
}


/*====================
  CMinimap::Rollover
  ====================*/
void    CMinimap::Rollover()
{
    IWidget::Rollover();
}


/*====================
  CMinimap::Rolloff
  ====================*/
void    CMinimap::Rolloff()
{
    IWidget::Rolloff();
}


/*====================
  CMinimap::MouseDown
  ====================*/
void    CMinimap::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    CVec2f v2WorldCursor(GetCursorInWorld(v2CursorPos));

    if (button == BUTTON_MOUSEL)
    {
        m_bDragging = true;

        if (!ui_minimapDrawing)
        {
            for (MinimapButtonVector::iterator it(m_vButtons.begin()); it != m_vButtons.end(); ++it)
            {
                CRectf rect
                (
                    m_recArea.left + it->recArea.left,
                    m_recArea.top + it->recArea.top,
                    m_recArea.left + it->recArea.left + it->recArea.GetWidth(),
                    m_recArea.top + it->recArea.top + it->recArea.GetHeight()
                );

                if (rect.AltContains(v2CursorPos))
                {
                    Console.Execute(_TS("MinimapButton ") + XtoA(it->uiIndex));
                    m_bDragging = false;
                    return;
                }
            }

            tsvector vParams;
            vParams.push_back(XtoA(v2WorldCursor.x));
            vParams.push_back(XtoA(v2WorldCursor.y));
            DO_EVENT_PARAM(WEVENT_CLICK, vParams)
        }
        
        if (m_bDragging)
        {
            CVec2f v2ScreenPos(GetAbsolutePos());
            Input.SetCursorConstrained(CURSOR_UI, BOOL_TRUE);
            Input.SetCursorConstraint(CURSOR_UI, CRectf(v2ScreenPos.x, v2ScreenPos.y, m_recArea.GetWidth() + v2ScreenPos.x, m_recArea.GetHeight() + v2ScreenPos.y));
        }
    }
    else if (button == BUTTON_MOUSER)
    {
        m_bDraggingRight = true;

        tsvector vParams;
        vParams.push_back(XtoA(v2WorldCursor.x));
        vParams.push_back(XtoA(v2WorldCursor.y));
        DO_EVENT_PARAM(WEVENT_RIGHTCLICK, vParams)

        if (m_bDraggingRight)
        {
            CVec2f v2ScreenPos(GetAbsolutePos());
            Input.SetCursorConstrained(CURSOR_UI, BOOL_TRUE);
            Input.SetCursorConstraint(CURSOR_UI, CRectf(v2ScreenPos.x, v2ScreenPos.y, m_recArea.GetWidth() + v2ScreenPos.x, m_recArea.GetHeight() + v2ScreenPos.y));
        }
    }
}


/*====================
  CMinimap::MouseUp
  ====================*/
void    CMinimap::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    Input.SetCursorConstrained(CURSOR_UI, BOOL_NOT_SET);
    Input.SetCursorConstraint(CURSOR_UI, CRectf(0.0f, 0.0f, 0.0f, 0.0f));
    m_bDragging = false;
    m_bDraggingRight = false;

    if (m_pInterface->GetActiveWidget() == this)
        m_pInterface->SetActiveWidget(nullptr);
}

void        Hide();


/*====================
  CMinimap::ProcessInputCursor
  ====================*/
bool    CMinimap::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    m_uiHoverUnit = -1;

    for (vector<SMinimapIcon>::reverse_iterator itIcon(m_vIcons.rbegin()); itIcon != m_vIcons.rend(); ++itIcon)
    {
        SMinimapIcon sIcon(*itIcon);

        if (sIcon.uiUnitIndex == -1)
            continue;

        if (sIcon.recArea.Contains(v2CursorPos - m_recArea.lt()))
        {
            m_uiHoverUnit = sIcon.uiUnitIndex;
            break;
        }
    }

    if (m_bDragging || m_bDraggingRight)
    {
        if (!ui_minimapDrawing)
        {
            CVec2f v2WorldCursor(GetCursorInWorld(v2CursorPos));

            tsvector vParams;
            vParams.push_back(XtoA(v2WorldCursor.x));
            vParams.push_back(XtoA(v2WorldCursor.y));

            if (m_bDragging)
                DO_EVENT_PARAM_RETURN(WEVENT_CLICK, vParams, false)

            if (m_bDraggingRight)
                DO_EVENT_PARAM_RETURN(WEVENT_RIGHTCLICK, vParams, false)
        }
    }

    return false;
}


/*====================
  CMinimap::GetMinimapHoverUnit
  ====================*/
uint    CMinimap::GetMinimapHoverUnit()
{
    return m_uiHoverUnit;
}


/*====================
  CMinimap::Frame
  ====================*/
void    CMinimap::Frame(uint uiFrameLength, bool bProcessFrame)
{
    if (!HasFlags(WFLAG_ENABLED))
        return;

    if (ui_minimapDrawing)
    {
        m_uiResetCursor = 1;
        //Input.SetCursor(CURSOR_UI, _T("draw.tga"), CVec2i(5, 26));

        if (m_bDragging || m_bDraggingRight)
        {
            CRectf recAreaAbs(m_recArea);
            recAreaAbs.MoveTo(GetAbsolutePos());

            CVec2f v2CursorPos(Input.GetCursorPos());

            float fX(ILERP(v2CursorPos.x, recAreaAbs.left, recAreaAbs.right));
            float fY(ILERP(v2CursorPos.y, recAreaAbs.top, recAreaAbs.bottom));

            Console.Execute(_TS("MinimapDraw ") + XtoA(fX) + _T(" ") + XtoA(fY) + _T(" ") + XtoA(ui_minimapFilter));
        }
    }
    else
    {
        if (m_uiResetCursor)
        {
            //Input.SetCursor(CURSOR_UI, _T("arrow.tga"), CVec2i(2, 2));
            m_uiResetCursor = 0;
        }
    }

    while (m_deqDrawing.size() > 0 && m_deqDrawing.front().uiTime + ui_minimapDrawTime < Host.GetTime())
        m_deqDrawing.pop_front();

    DO_EVENT(WEVENT_FRAME)

    // Recursively call children frame functions
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        (*it)->Frame(uiFrameLength, bProcessFrame);
}


/*====================
  CMinimap::RenderWidget
  ====================*/
void    CMinimap::RenderWidget(const CVec2f &vOrigin, float fFade)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return;

    if (m_recPadding.GetWidth())
    {
        Draw2D.SetColor(CVec4f(0.0f, 0.0f, 0.0f, 1.0f));
        Draw2D.Rect(vOrigin.x - m_recPadding.left, vOrigin.y, m_recPadding.left, m_recArea.GetHeight(), 0);
        Draw2D.Rect(vOrigin.x + m_recArea.GetWidth(), vOrigin.y, m_recPadding.left, m_recArea.GetHeight(), 0);
    }

    if (m_recPadding.GetHeight())
    {
        Draw2D.SetColor(CVec4f(0.0f, 0.0f, 0.0f, 1.0f));
        Draw2D.Rect(vOrigin.x, vOrigin.y - m_recPadding.top, m_recArea.GetWidth(), m_recPadding.top, 0);
        Draw2D.Rect(vOrigin.x, vOrigin.y + m_recArea.GetHeight(), m_recArea.GetWidth(), m_recPadding.top, 0);
    }

    IWidget::RenderWidget(vOrigin, fFade);

    // Fog of War
    Draw2D.SetColor(CVec4f(0.0f, 0.0f, 0.0f, 1.0f));

    if (m_bFlop)
    {
        Draw2D.Rect
        (
            vOrigin.x + m_recArea.GetWidth(),
            vOrigin.y + m_recArea.GetHeight(),
            -m_recArea.GetWidth(),
            -m_recArea.GetHeight(),
            m_fCropS0, m_fCropT0, m_fCropS1, m_fCropT1,
            m_hFogofWarTexture,
            GUI_FOG
        );
    }
    else
    {
        Draw2D.Rect
        (
            vOrigin.x,
            vOrigin.y,
            m_recArea.GetWidth(),
            m_recArea.GetHeight(),
            m_fCropS0, m_fCropT0, m_fCropS1, m_fCropT1,
            m_hFogofWarTexture,
            GUI_FOG
        );
    }

    CVec2f v2LocalOrigin(vOrigin);

    CVec4f v4OldColor(Draw2D.GetCurrentColor());

    for (MinimapIconVector::iterator it(m_vIcons.begin()); it != m_vIcons.end(); ++it)
    {
        switch (it->eType)
        {
        case MINIMAP_ICON:
            if (m_uiHoverUnit != -1 && it->uiUnitIndex == m_uiHoverUnit)
            {
                Draw2D.SetColor(it->v4Color);
                Draw2D.Rect
                (
                    INT_FLOOR(v2LocalOrigin.x + (it->recHoverArea.left)),
                    INT_FLOOR(v2LocalOrigin.y + (it->recHoverArea.top)),
                    INT_FLOOR(it->recHoverArea.GetWidth()),
                    INT_FLOOR(it->recHoverArea.GetHeight()),
                    it->hTexture
                );
            }
            else
            {
                Draw2D.SetColor(it->v4Color);
                Draw2D.Rect
                (
                    INT_FLOOR(v2LocalOrigin.x + (it->recArea.left)),
                    INT_FLOOR(v2LocalOrigin.y + (it->recArea.top)),
                    INT_FLOOR(it->recArea.GetWidth()),
                    INT_FLOOR(it->recArea.GetHeight()),
                    it->hTexture
                );
            }
            break;
        case MINIMAP_ICON_OUTLINE:
            if (m_uiHoverUnit != -1 && it->uiUnitIndex == m_uiHoverUnit)
            {
                Draw2D.SetColor(it->v4Color);
                Draw2D.Rect
                (
                    INT_FLOOR(v2LocalOrigin.x + it->recHoverArea.left),
                    INT_FLOOR(v2LocalOrigin.y + it->recHoverArea.top),
                    INT_FLOOR(it->recHoverArea.GetWidth()),
                    INT_FLOOR(it->recHoverArea.GetHeight()),
                    it->hTexture
                );
            }
            else
            {
                Draw2D.SetColor(it->v4Color);
                Draw2D.Rect
                (
                    INT_FLOOR(v2LocalOrigin.x + it->recArea.left),
                    INT_FLOOR(v2LocalOrigin.y + it->recArea.top),
                    INT_FLOOR(it->recArea.GetWidth()),
                    INT_FLOOR(it->recArea.GetHeight()),
                    it->hTexture
                );
            }
            break;
        case MINIMAP_LINE:
            Draw2D.Line
            (
                v2LocalOrigin + it->recArea.lt(),
                v2LocalOrigin + it->recArea.rb(),
                it->v4Color,
                it->v4Color2
            );
            break;
        case MINIMAP_QUAD:
            Draw2D.SetColor(it->v4Color);
            Draw2D.Quad
            (
                v2LocalOrigin + it->recArea.lt(),
                v2LocalOrigin + it->recArea.rb(),
                v2LocalOrigin + it->v4Color2.xy(),
                v2LocalOrigin + CVec2f(it->v4Color2.z,it->v4Color2.w)
            );
            break;
        }
    }

    for (MinimapButtonVector::iterator it(m_vButtons.begin()); it != m_vButtons.end(); ++it)
    {
        CRectf rect
        (
            v2LocalOrigin.x + it->recArea.left,
            v2LocalOrigin.y + it->recArea.top,
            v2LocalOrigin.x + it->recArea.left + it->recArea.GetWidth(),
            v2LocalOrigin.y + it->recArea.top + it->recArea.GetHeight()
        );

        Draw2D.SetColor(it->v4Color);
        
        if (rect.AltContains(Input.GetCursorPos()))
        {
            Draw2D.SetColor(it->v4RolloverColor);
            float fXDelta((it->v2RolloverSize.x - rect.GetWidth()));
            float fYDelta((it->v2RolloverSize.y - rect.GetHeight()));
            rect.Stretch(fXDelta, fYDelta);
            rect.Shift(-fXDelta / 2.0f, -fYDelta / 2.0f);
        }

        Draw2D.Rect(rect, it->hTexture);
    }

    // Minimap drawing
    for (MinimapDrawDeque::iterator it(m_deqDrawing.begin()); it != m_deqDrawing.end() && it + 1 != m_deqDrawing.end(); ++it)
    {
        MinimapDrawDeque::iterator it2(it + 1);
        Draw2D.Line
        (
            v2LocalOrigin + it->v2Pos + CVec2f(1.0f, 1.0f),
            v2LocalOrigin + it2->v2Pos + CVec2f(1.0f, 1.0f),
            CVec4f(0.0f, 0.0f, 0.0f, 1.0f - CLAMP(ILERP(int(Host.GetTime()), int(it->uiTime + (ui_minimapDrawTime * 0.9f)), int(it->uiTime + ui_minimapDrawTime)), 0.0f, 1.0f)),
            CVec4f(0.0f, 0.0f, 0.0f, 1.0f - CLAMP(ILERP(int(Host.GetTime()), int(it2->uiTime + (ui_minimapDrawTime * 0.9f)), int(it2->uiTime + ui_minimapDrawTime)), 0.0f, 1.0f))
        );
    }

    for (MinimapDrawDeque::iterator it(m_deqDrawing.begin()); it != m_deqDrawing.end() && it + 1 != m_deqDrawing.end(); ++it)
    {
        MinimapDrawDeque::iterator it2(it + 1);
        Draw2D.Line
        (
            v2LocalOrigin + it->v2Pos,
            v2LocalOrigin + it2->v2Pos,
            CVec4f(it->v3Color, 1.0f - CLAMP(ILERP(int(Host.GetTime()), int(it->uiTime + (ui_minimapDrawTime * 0.9f)), int(it->uiTime + ui_minimapDrawTime)), 0.0f, 1.0f)),
            CVec4f(it2->v3Color, 1.0f - CLAMP(ILERP(int(Host.GetTime()), int(it2->uiTime + (ui_minimapDrawTime * 0.9f)), int(it2->uiTime + ui_minimapDrawTime)), 0.0f, 1.0f))
        );
    }

    Draw2D.SetColor(v4OldColor);
}


/*====================
  CMinimap::Execute
  ====================*/
void    CMinimap::Execute(const tstring &sCmd, IBuffer &buffer)
{
    if (!IsAbsoluteVisible())
        return;

    if (sCmd == _T("clear"))
    {
        m_vIcons.clear();
        m_vButtons.clear();
    }
    else if (sCmd == _T("icon"))
    {
        if (buffer.GetLength() != 40)
            return;

        buffer.Rewind();

        SMinimapIcon mmIcon;

        mmIcon.eType = MINIMAP_ICON;

        float fX(buffer.ReadFloat());
        float fY(buffer.ReadFloat());
        float fWidth(ROUND(buffer.ReadFloat() * m_recArea.GetWidth()));
        float fHeight(ROUND(buffer.ReadFloat() * m_recArea.GetWidth())); // Always square

        float fHoverWidth(fWidth * m_fHoverSize);
        float fHoverHeight(fHeight * m_fHoverSize);

        float fMinimapX((fX + m_fWorldOffsetX) * m_fWorldPercentX);
        float fMinimapY((fY + m_fWorldOffsetY) * m_fWorldPercentY);

        mmIcon.recArea = CRectf
        (
            ROUND(fMinimapX * m_recArea.GetWidth() - (fWidth * 0.5f)),
            ROUND(fMinimapY * m_recArea.GetHeight() - (fHeight * 0.5f)),
            ROUND(fMinimapX * m_recArea.GetWidth() + (fWidth * 0.5f)),
            ROUND(fMinimapY * m_recArea.GetHeight() + (fHeight * 0.5f))
        );

        mmIcon.recHoverArea = CRectf
        (
            ROUND(fMinimapX * m_recArea.GetWidth() - (fHoverWidth * 0.5f)),
            ROUND(fMinimapY * m_recArea.GetHeight() - (fHoverHeight * 0.5f)),
            ROUND(fMinimapX * m_recArea.GetWidth() + (fHoverWidth * 0.5f)),
            ROUND(fMinimapY * m_recArea.GetHeight() + (fHoverHeight * 0.5f))
        );

        mmIcon.v4Color[R] = buffer.ReadFloat();
        mmIcon.v4Color[G] = buffer.ReadFloat();
        mmIcon.v4Color[B] = buffer.ReadFloat();
        mmIcon.v4Color[A] = buffer.ReadFloat();

        mmIcon.hTexture = buffer.ReadInt();

        mmIcon.uiUnitIndex = buffer.ReadInt();

        m_vIcons.push_back(mmIcon);
    }
    else if (sCmd == _T("outline"))
    {
        if (buffer.GetLength() != 24)
            return;

        buffer.Rewind();

        SMinimapIcon mmIcon;

        mmIcon.eType = MINIMAP_ICON_OUTLINE;

        float fX(buffer.ReadFloat());
        float fY(buffer.ReadFloat());
        float fWidth(ROUND(buffer.ReadFloat() * m_recArea.GetWidth()));
        float fHeight(ROUND(buffer.ReadFloat() * m_recArea.GetWidth())); // Always square

        //m_fOutlineSize
        float fHoverWidth(fWidth * m_fHoverSize);
        float fHoverHeight(fHeight * m_fHoverSize);

        float fMinimapX((fX + m_fWorldOffsetX) * m_fWorldPercentX);
        float fMinimapY((fY + m_fWorldOffsetY) * m_fWorldPercentY);

        mmIcon.recArea = CRectf
        (
            ROUND(fMinimapX * m_recArea.GetWidth() - (fWidth * 0.5f) - INT_CEIL(m_fOutlineSize)),
            ROUND(fMinimapY * m_recArea.GetHeight() - (fHeight * 0.5f) - INT_CEIL(m_fOutlineSize)),
            ROUND(fMinimapX * m_recArea.GetWidth() + (fWidth * 0.5f) + INT_CEIL(m_fOutlineSize)),
            ROUND(fMinimapY * m_recArea.GetHeight() + (fHeight * 0.5f) + INT_CEIL(m_fOutlineSize))
        ); 

        mmIcon.recHoverArea = CRectf
        (
            ROUND(fMinimapX * m_recArea.GetWidth() - (fHoverWidth * 0.5f) - INT_CEIL(m_fOutlineSize)),
            ROUND(fMinimapY * m_recArea.GetHeight() - (fHoverHeight * 0.5f) - INT_CEIL(m_fOutlineSize)),
            ROUND(fMinimapX * m_recArea.GetWidth() + (fHoverWidth * 0.5f) + INT_CEIL(m_fOutlineSize)),
            ROUND(fMinimapY * m_recArea.GetHeight() + (fHoverHeight * 0.5f) + INT_CEIL(m_fOutlineSize))
        );

        mmIcon.v4Color[R] = 0.0f;
        mmIcon.v4Color[G] = 0.0f;
        mmIcon.v4Color[B] = 0.0f;
        mmIcon.v4Color[A] = 1.0f;

        mmIcon.hTexture = buffer.ReadInt();

        mmIcon.uiUnitIndex = buffer.ReadInt();

        m_vIcons.push_back(mmIcon);
    }
    else if (sCmd == _T("button"))
    {
        SMinimapButton mmButton;

        if (buffer.GetLength() != 64)
            return;
        buffer.Rewind();

        float fX(buffer.ReadFloat());
        float fY(buffer.ReadFloat());
        float fWidth(buffer.ReadFloat());
        float fHeight(buffer.ReadFloat());

        float fMinimapX((fX+m_fWorldOffsetX)*m_fWorldPercentX);
        float fMinimapY((fY+m_fWorldOffsetY)*m_fWorldPercentY);

        mmButton.recArea = CRectf
        (
            ROUND(fMinimapX * m_recArea.GetWidth() - (fWidth * .5f)),
            ROUND(fMinimapY * m_recArea.GetHeight() - (fHeight * .5f)),
            ROUND(fMinimapX * m_recArea.GetWidth() + (fWidth * .5f)),
            ROUND(fMinimapY * m_recArea.GetHeight() + (fHeight * .5f))
        );

        mmButton.v4Color[R] = buffer.ReadFloat();
        mmButton.v4Color[G] = buffer.ReadFloat();
        mmButton.v4Color[B] = buffer.ReadFloat();
        mmButton.v4Color[A] = buffer.ReadFloat();

        mmButton.v2RolloverSize.x = buffer.ReadFloat();
        mmButton.v2RolloverSize.y = buffer.ReadFloat();

        mmButton.v4RolloverColor[R] = buffer.ReadFloat();
        mmButton.v4RolloverColor[G] = buffer.ReadFloat();
        mmButton.v4RolloverColor[B] = buffer.ReadFloat();
        mmButton.v4RolloverColor[A] = buffer.ReadFloat();

        mmButton.hTexture = buffer.ReadInt();
        mmButton.uiIndex = buffer.ReadInt();

        m_vButtons.push_back(mmButton);
    }
    else if (sCmd == _T("line"))
    {
        SMinimapIcon mmLine;

        if (buffer.GetLength() != 48)
            return;
        buffer.Rewind();

        mmLine.eType = MINIMAP_LINE;

        float fL(buffer.ReadFloat());
        float fT(buffer.ReadFloat());
        float fR(buffer.ReadFloat());
        float fB(buffer.ReadFloat());

        float fMinimapL((fL+m_fWorldOffsetX)*m_fWorldPercentX);
        float fMinimapT((fT+m_fWorldOffsetY)*m_fWorldPercentY);
        float fMinimapR((fR+m_fWorldOffsetX)*m_fWorldPercentX);
        float fMinimapB((fB+m_fWorldOffsetY)*m_fWorldPercentY);

        mmLine.recArea = CRectf
        (
            ROUND(fMinimapL * m_recArea.GetWidth()),
            ROUND(fMinimapT * m_recArea.GetHeight()),
            ROUND(fMinimapR * m_recArea.GetWidth()),
            ROUND(fMinimapB * m_recArea.GetHeight())
        );

        CPlane plPlanes[4] =
        {
            CPlane(1.0f, 0.0f, 0.0, m_recArea.GetWidth()),
            CPlane(0.0f, 1.0f, 0.0, m_recArea.GetHeight()),
            CPlane(-1.0f, 0.0f, 0.0, 0.0f),
            CPlane(0.0f, -1.0f, 0.0, 0.0f)
        };

        bool    bValid(true);
        CVec3f  p1(mmLine.recArea.left, mmLine.recArea.top, 0.0f);
        CVec3f  p2(mmLine.recArea.right, mmLine.recArea.bottom, 0.0f);

        for (int i(0); i < 4 && bValid; ++i)
        {
            if (!M_ClipLine(plPlanes[i], p1, p2))
                bValid = false;
        }

        if (bValid)
        {
            mmLine.recArea.lt() = p1.xy();
            mmLine.recArea.rb() = p2.xy();

            mmLine.v4Color[R] = buffer.ReadFloat();
            mmLine.v4Color[G] = buffer.ReadFloat();
            mmLine.v4Color[B] = buffer.ReadFloat();
            mmLine.v4Color[A] = buffer.ReadFloat();
            mmLine.v4Color2[R] = buffer.ReadFloat();
            mmLine.v4Color2[G] = buffer.ReadFloat();
            mmLine.v4Color2[B] = buffer.ReadFloat();
            mmLine.v4Color2[A] = buffer.ReadFloat();
            
            m_vIcons.push_back(mmLine);
        }
    }
    else if (sCmd == _T("quad"))
    {
        SMinimapIcon mmQuad;

        if (buffer.GetLength() != 48)
            return;
        buffer.Rewind();

        mmQuad.eType = MINIMAP_QUAD;

        float TLx(buffer.ReadFloat());
        float TLy(buffer.ReadFloat());
        float TRx(buffer.ReadFloat());
        float TRy(buffer.ReadFloat());

        float fMinimapTLx((TLx+m_fWorldOffsetX)*m_fWorldPercentX);
        float fMinimapTLy((TLy+m_fWorldOffsetY)*m_fWorldPercentY);
        float fMinimapTRx((TRx+m_fWorldOffsetX)*m_fWorldPercentX);
        float fMinimapTRy((TRy+m_fWorldOffsetY)*m_fWorldPercentY);

        // stuffing first half of quad in recArea
        mmQuad.recArea = CRectf
        (
            ROUND(fMinimapTLx * m_recArea.GetWidth()),
            ROUND(fMinimapTLy * m_recArea.GetHeight()),
            ROUND(fMinimapTRx * m_recArea.GetWidth()),
            ROUND(fMinimapTRy * m_recArea.GetHeight())
        );
        
        mmQuad.recArea.Normalize();
        mmQuad.recArea = mmQuad.recArea & m_recArea;

        // stuffing second half of quad in color2
        mmQuad.v4Color2[0] = ROUND( buffer.ReadFloat() * m_recArea.GetWidth() );
        mmQuad.v4Color2[1] = ROUND( buffer.ReadFloat() * m_recArea.GetHeight() );
        mmQuad.v4Color2[2] = ROUND( buffer.ReadFloat() * m_recArea.GetWidth() );
        mmQuad.v4Color2[3] = ROUND( buffer.ReadFloat() * m_recArea.GetHeight() );

        mmQuad.v4Color[R] = buffer.ReadFloat();
        mmQuad.v4Color[G] = buffer.ReadFloat();
        mmQuad.v4Color[B] = buffer.ReadFloat();
        mmQuad.v4Color[A] = buffer.ReadFloat();
        
        m_vIcons.push_back(mmQuad);
    }
    else if (sCmd == _T("draw"))
    {
        if (buffer.GetLength() != 20)
            return;
        buffer.Rewind();

        float fX(buffer.ReadFloat());
        float fY(buffer.ReadFloat());
        CVec3f v3Color;
        v3Color[0] = (buffer.ReadFloat());
        v3Color[1] = (buffer.ReadFloat());
        v3Color[2] = (buffer.ReadFloat());

        SMinimapLineVertex sVert;
        sVert.v2Pos = CVec2f(ROUND(fX * m_recArea.GetWidth()), ROUND(fY * m_recArea.GetHeight()));
        sVert.uiTime = Host.GetTime();
        sVert.v3Color = v3Color;

        m_deqDrawing.push_back(sVert);
    }
    else if (sCmd == _T("padding")) // percent of the minimap not rendered. Params: Top, Right, Bottom, Left
    {
        if (buffer.GetLength() != 16)
            return;
        buffer.Rewind();

        float fT(buffer.ReadFloat());
        float fR(buffer.ReadFloat());
        float fB(buffer.ReadFloat());
        float fL(buffer.ReadFloat());

        if (m_bFlop)
            SWAP(fT, fB);

        if (HasFlags(WFLAG_CROPTEXTURE))
            m_recArea -= m_recPadding;
        else
            SetFlags(WFLAG_CROPTEXTURE);
        m_recPadding = CRectf(0.0f, 0.0f, 0.0f, 0.0f);

        m_fCropS0 = CLAMP(fL, 0.0f, 1.0f);
        m_fCropT0 = CLAMP(fB, 0.0f, 1.0f);
        m_fCropS1 = CLAMP(1.0f - fR, .1f, 1.0f);
        m_fCropT1 = CLAMP(1.0f - fT, .1f, 1.0f);

        float fWidth(m_fCropS1 - m_fCropS0);
        float fHeight(m_fCropT1 - m_fCropT0);

        m_fWorldOffsetX = -fL;
        m_fWorldPercentX = (1.0f)/fWidth;
        m_fWorldOffsetY = -fT;
        m_fWorldPercentY = (1.0f)/fHeight;

        if (fWidth > fHeight)
        {
            float fRatioLost(1.0f - fHeight/fWidth);

            // Calc dimension
            m_recPadding.top = (m_recArea.GetHeight()) * fRatioLost * .5f;
            m_recPadding.bottom = -m_recPadding.top;
        }
        else if (fHeight > fWidth)
        {
            float fRatioLost(1.0f - fWidth/fHeight);

            // Calc dimension
            m_recPadding.left = (m_recArea.right-m_recArea.left) * fRatioLost * .5f;
            m_recPadding.right = -m_recPadding.left;
        }

        // Modify dimension
        m_recArea += m_recPadding;
    }
    else if (sCmd == _T("flop"))
    {
        if (buffer.GetLength() != 4)
            return;
        buffer.Rewind();

        int iFlop(buffer.ReadInt());

        m_bFlop = iFlop != 0;
    }
    else if (sCmd == _T("breakdrag"))
    {
        m_bDragging = false;
        m_bDraggingRight = false;
    }
}


/*====================
  CMinimap::GetMinimapDrawX
  ====================*/
float   CMinimap::GetMinimapDrawX(float fFraction) const
{
    float fMinimapX((fFraction + m_fWorldOffsetX) * m_fWorldPercentX);

    CVec2f v2ParentPos(m_pParent != nullptr ? m_pParent->GetAbsolutePos() : V2_ZERO);

    return v2ParentPos.x + LERP(fMinimapX, m_recArea.left, m_recArea.right);
}


/*====================
  CMinimap::GetMinimapDrawY
  ====================*/
float   CMinimap::GetMinimapDrawY(float fFraction) const
{
    float fMinimapY((fFraction + m_fWorldOffsetY) * m_fWorldPercentY);

    CVec2f v2ParentPos(m_pParent != nullptr ? m_pParent->GetAbsolutePos() : V2_ZERO);

    return v2ParentPos.y + LERP(fMinimapY, m_recArea.top, m_recArea.bottom);
}


/*====================
  CMinimap::RecalculateSize
  ====================*/
void    CMinimap::RecalculateSize()
{
    m_fOutlineSize = GetSizeFromString(m_sOutlineSize, GetWidth(), GetHeight());

    IWidget::RecalculateSize();
}


/*--------------------
  BreakDrag
  --------------------*/
UI_VOID_CMD(BreakDrag, 0)
{
    if (pThis == nullptr ||
        pThis->GetType() != WIDGET_MAP)
        return;

    static_cast<CMinimap *>(pThis)->SetDragging(false);
    static_cast<CMinimap *>(pThis)->SetDraggingRight(false);
}


/*--------------------
  GetMinimapDrawX
  --------------------*/
UI_CMD(GetMinimapDrawX, 2)
{
    if (pThis == nullptr || pThis->GetInterface() == nullptr)
        return _CTS("0");

    tstring sWidgetName(vArgList[0]->Evaluate());
    IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
    if (pWidget == nullptr)
    {
        Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
        return _CTS("0");
    }

    if (pWidget->GetType() != WIDGET_MAP)
        return _CTS("0");
    else
        return XtoA(static_cast<CMinimap *>(pWidget)->GetMinimapDrawX(AtoF(vArgList[1]->Evaluate())));
}


/*--------------------
  GetMinimapDrawY
  --------------------*/
UI_CMD(GetMinimapDrawY, 2)
{
    if (pThis == nullptr || pThis->GetInterface() == nullptr)
        return _CTS("0");

    tstring sWidgetName(vArgList[0]->Evaluate());
    IWidget *pWidget(pThis->GetInterface()->GetWidget(sWidgetName));
    if (pWidget == nullptr)
    {
        Console << _T("Widget ") << SingleQuoteStr(sWidgetName) << _T(" not found") << newl;
        return _CTS("0");
    }

    if (pWidget->GetType() != WIDGET_MAP)
        return _CTS("0");
    else
        return XtoA(static_cast<CMinimap *>(pWidget)->GetMinimapDrawY(AtoF(vArgList[1]->Evaluate())));
}
