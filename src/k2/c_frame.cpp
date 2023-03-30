// (C)2005 S2 Games
// c_frame.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_frame.h"
#include "c_interface.h"
#include "c_panel.h"
#include "c_frame_border.h"
#include "c_widgetstyle.h"
#include "c_uicmd.h"
#include "c_uitextureregistry.h"
#include "c_draw2d.h"
#include "c_texture.h"
#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CFrame::CFrame
  ====================*/
CFrame::CFrame(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_FRAME, style, false),
m_sBorderSize(style.GetProperty(_T("borderthickness"))),
m_v4BorderColor(style.HasProperty(_T("bordercolor")) ? GetColorFromString(style.GetProperty(_T("bordercolor"), _T("silver"))) : m_v4Color)
{
    m_uiFlags |= WFLAG_NO_DRAW;

    m_fBorderSize = GetSizeFromString(m_sBorderSize, pParent->GetWidth(), pParent->GetHeight());

    const tstring &sTextureName(style.GetProperty(_T("texture")));

    uint uiTextureFlags(0);
    if (style.GetPropertyBool(_T("nocompress"), false))
    {
        uiTextureFlags |= TEX_NO_COMPRESS;
    }

    if (!sTextureName.empty())
    {
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_bl")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_BOTTOM_LEFT]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_b")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_BOTTOM]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_br")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_BOTTOM_RIGHT]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_l")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_LEFT]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_c")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_CENTER]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_r")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_RIGHT]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_tl")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_TOP_LEFT]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_t")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_TOP]);
        UITextureRegistry.Register(Filename_AppendSuffix(sTextureName, _T("_tr")), uiTextureFlags, m_hBorderTextures[FRAME_PIECE_TOP_RIGHT]);
    }
    else
    {
        for (int i(0); i < 9; ++i)
            m_hBorderTextures[i] = g_ResourceManager.GetWhiteTexture();
    }

    // m_sTextureName isn't what we want in the base m_hTexture...
    m_hTexture[0] = g_ResourceManager.GetWhiteTexture();

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CFrame::SetColor
  ====================*/
void    CFrame::SetColor(const CVec4f &v4Color)
{
    m_v4Color = v4Color;
}


/*====================
  CFrame::SetBorderColor
  ====================*/
void    CFrame::SetBorderColor(const CVec4f &v4Color)
{
    m_v4BorderColor = v4Color;
}


/*====================
  CFrame::RecalculateSize
  ====================*/
void    CFrame::RecalculateSize()
{
    m_fBorderSize = GetSizeFromString(m_sBorderSize, GetParentWidth(), GetParentHeight());

    IWidget::RecalculateSize();
}


/*====================
  CFrame::RenderWidget
  ====================*/
void    CFrame::RenderWidget(const CVec2f &vOrigin, float fFade)
{
    int iRenderFlags(0);

    if (m_eRenderMode == WRENDER_ADDITIVE)
        iRenderFlags |= GUI_ADDITIVE;
    else if (m_eRenderMode == WRENDER_OVERLAY)
        iRenderFlags |= GUI_OVERLAY;
    else if (m_eRenderMode == WRENDER_GRAYSCALE)
        iRenderFlags |= GUI_GRAYSCALE;
    else if (m_eRenderMode == WRENDER_BLUR)
        iRenderFlags |= GUI_BLUR;

    IWidget::RenderWidget(vOrigin, fFade);

    const CVec4f &v4Color(fFade == 1.0f ? m_v4Color : GetFadedColor(m_v4Color, fFade));
    const CVec4f &v4BorderColor(fFade == 1.0f ? m_v4BorderColor : GetFadedColor(m_v4BorderColor, fFade));

    // Center
    if (v4Color[A] != 0.0f && m_hBorderTextures[FRAME_PIECE_CENTER] != INVALID_RESOURCE)
    {
        Draw2D.SetColor(v4Color);
        Draw2D.Rect
        (
            vOrigin.x + m_fBorderSize,
            vOrigin.y + m_fBorderSize,
            m_recArea.GetWidth() - m_fBorderSize * 2.0f,
            m_recArea.GetHeight() - m_fBorderSize * 2.0f,
            m_hBorderTextures[FRAME_PIECE_CENTER],
            iRenderFlags
        );
    }

    if (v4BorderColor[A] == 0.0f)
        return;

    Draw2D.SetColor(v4BorderColor);
    
    // Bottom Left
    if (m_hBorderTextures[FRAME_PIECE_BOTTOM_LEFT] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x,
            vOrigin.y + GetHeight() - m_fBorderSize,
            m_fBorderSize,
            m_fBorderSize,
            m_hBorderTextures[FRAME_PIECE_BOTTOM_LEFT],
            iRenderFlags
        );
    }

    // Bottom
    if (m_hBorderTextures[FRAME_PIECE_BOTTOM] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x + m_fBorderSize,
            vOrigin.y + GetHeight() - m_fBorderSize,
            GetWidth() - m_fBorderSize * 2.0f,
            m_fBorderSize,
            m_hBorderTextures[FRAME_PIECE_BOTTOM],
            iRenderFlags
        );
    }

    // Bottom Right
    if (m_hBorderTextures[FRAME_PIECE_BOTTOM_RIGHT] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x + GetWidth() - m_fBorderSize,
            vOrigin.y + GetHeight() - m_fBorderSize,
            m_fBorderSize,
            m_fBorderSize,
            m_hBorderTextures[FRAME_PIECE_BOTTOM_RIGHT],
            iRenderFlags
        );
    }

    // Left
    if (m_hBorderTextures[FRAME_PIECE_LEFT] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x,
            vOrigin.y + m_fBorderSize,
            m_fBorderSize,
            GetHeight() - m_fBorderSize * 2.0f,
            m_hBorderTextures[FRAME_PIECE_LEFT],
            iRenderFlags
        );
    }

    // Right
    if (m_hBorderTextures[FRAME_PIECE_RIGHT] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x + GetWidth() - m_fBorderSize,
            vOrigin.y + m_fBorderSize,
            m_fBorderSize,
            GetHeight() - m_fBorderSize * 2.0f,
            m_hBorderTextures[FRAME_PIECE_RIGHT],
            iRenderFlags
        );
    }

    // Top Left
    if (m_hBorderTextures[FRAME_PIECE_TOP_LEFT] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x,
            vOrigin.y,
            m_fBorderSize,
            m_fBorderSize,
            m_hBorderTextures[FRAME_PIECE_TOP_LEFT],
            iRenderFlags
        );
    }

    // Top
    if (m_hBorderTextures[FRAME_PIECE_TOP] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x + m_fBorderSize,
            vOrigin.y,
            GetWidth() - m_fBorderSize * 2.0f,
            m_fBorderSize,
            m_hBorderTextures[FRAME_PIECE_TOP],
            iRenderFlags
        );
    }

    // Top Left
    if (m_hBorderTextures[FRAME_PIECE_TOP_RIGHT] != INVALID_RESOURCE)
    {
        Draw2D.Rect
        (
            vOrigin.x + GetWidth() - m_fBorderSize,
            vOrigin.y,
            m_fBorderSize,
            m_fBorderSize,
            m_hBorderTextures[FRAME_PIECE_TOP_RIGHT],
            iRenderFlags
        );
    }
}


/*--------------------
  SetBorderColor
  --------------------*/
UI_VOID_CMD(SetBorderColor, 1)
{
    if (pThis->GetType() != WIDGET_FRAME)
        return;

    CFrame *pFrame(static_cast<CFrame*>(pThis));

    if (vArgList.size() == 4)
        pFrame->SetBorderColor(CVec4f(AtoF(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate()), AtoF(vArgList[2]->Evaluate()), AtoF(vArgList[3]->Evaluate())));
    else if (vArgList.size() == 3)
        pFrame->SetBorderColor(CVec4f(AtoF(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate()), AtoF(vArgList[2]->Evaluate()), 1.0f));
    else
        pFrame->SetBorderColor(GetColorFromString(vArgList[0]->Evaluate()));
}
