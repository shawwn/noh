// (C)2005 S2 Games
// c_label.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_label.h"
#include "c_interface.h"
#include "c_uicmd.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
#include "c_cmd.h"
#include "c_draw2d.h"
#include "c_fontmap.h"
#include "c_uiscript.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
CVAR_BOOLF(	ui_translateLabels,	true,	CVAR_SAVECONFIG);
//=============================================================================

/*====================
  CLabel::CLabel
  ====================*/
CLabel::CLabel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_LABEL, style),
m_sText(style.GetProperty(_T("content"))),
m_hFontMap(g_ResourceManager.LookUpName(style.GetProperty(_T("font"), _T("system_medium")), RES_FONTMAP)),
m_iDrawFlags(0),
m_bElipse(style.GetPropertyBool(_T("elipse"), false)),
m_bShadow(style.GetPropertyBool(_T("shadow"), false)),
m_bWrap(style.GetPropertyBool(_T("wrap"), false)),
m_iPrecision(style.GetPropertyInt(_T("precision"), -1)),
m_v4ShadowColor(GetColorFromString(style.GetProperty(_T("shadowcolor"), _T("#000000")))),
m_fShadowOffsetX(style.GetPropertyFloat(_T("shadowoffsetx"), 0.0f)),
m_fShadowOffsetY(style.GetPropertyFloat(_T("shadowoffsety"), 0.0f)),
m_fShadowOffset(style.GetPropertyFloat(_T("shadowoffset"), 1.0f)),
m_bOutline(style.GetPropertyBool(_T("outline"), false)),
m_fOutlineOffset(style.GetPropertyFloat(_T("outlineoffset"), 1.0f)),
m_v4OutlineColor(GetColorFromString(style.GetProperty(_T("outlinecolor"), _T("#000000")))),
m_bFitX(style.GetPropertyBool(_T("fitx"), false)),//!style.HasProperty(_T("width")) && style.GetPropertyBool(_T("fitx"))),
m_bFitY(style.GetPropertyBool(_T("fity"), false)),//!style.HasProperty(_T("height")) && style.GetPropertyBool(_T("fity"))),
m_sFitXPadding(style.GetProperty(_T("fitxpadding"), _CWS("0"))),
m_sFitYPadding(style.GetProperty(_T("fitypadding"), _CWS("0"))),
m_sFitXMax(style.GetProperty(_T("fitxmax"), _CWS("0"))),
m_sRenderText(_T("")),
m_bLineRet(false)
{
	if (m_bOutline)
	{
		m_v4ShadowColor = m_v4OutlineColor;
		m_fShadowOffset = m_fOutlineOffset;
	}

	if (m_fShadowOffsetX == 0.0f && m_fShadowOffset)
	{
		m_fShadowOffsetX = m_fShadowOffset;
		m_fShadowOffsetY = m_fShadowOffset;
	}

	if (ui_translateLabels)
		m_sText = UIManager.Translate(m_sText);

	if (m_hFontMap == INVALID_RESOURCE)
		m_hFontMap = g_ResourceManager.LookUpName(_T("system_medium"), RES_FONTMAP);

	// Text Alignment
	// Horizontal
	const tstring sLeft(_T("left"));
	const tstring &sTextAlign(style.GetProperty(_T("textalign"), sLeft));
	if (sTextAlign == _T("center"))
		m_iDrawFlags |= DRAW_STRING_CENTER;
	else if (sTextAlign == _T("right"))
		m_iDrawFlags |= DRAW_STRING_RIGHT;

	// Vertical
	const tstring sTop(_T("top"));
	const tstring &sTextVAlign(style.GetProperty(_T("textvalign"), sTop));
	if (sTextVAlign == _T("center"))
		m_iDrawFlags |= DRAW_STRING_VCENTER;
	else if (sTextVAlign == _T("bottom"))
		m_iDrawFlags |= DRAW_STRING_BOTTOM;

	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)

	// Text fitting
	m_sRenderText = StripColorCodes(m_sText);

	if (m_iPrecision >= 0 && !m_sText.empty() && !(m_bFitX || m_bFitY))
		m_sText = XtoA(AtoF(m_sText), 0, 0, m_iPrecision);

	m_bLineRet = false;
	if(m_sText.find_first_of(_T("\n")) != -1)
		m_bLineRet = true;

	RecalculateText();
	RecalculateSize();
}

/*====================
  CLabel::RecalculateText
  ====================*/
void	CLabel::RecalculateText()
{
	CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFontMap));
	if(pFontMap == NULL)
		return; 

	bool bResize(false);
	if(m_bFitX)
	{
		float fBiggestWidth(BiggestStringWidth(m_sRenderText, pFontMap, GetWidth()));
		if(m_fBiggestWidth != fBiggestWidth)
		{
			m_fBiggestWidth = fBiggestWidth;
			bResize = true;
		}
	}

	float fWrapCount(WrapStringCount(m_sRenderText, pFontMap, GetWidth(), m_bWrap, &m_vLineWrap, m_iDrawFlags, &m_vLineCentering));
	if(m_bFitY)
	{
		if(m_fWrapCount != fWrapCount)
		{
			m_fWrapCount = fWrapCount;
			bResize = true;
		}
	}
	
	if(bResize)
		RecalculateSize();
}

/*====================
  CLabel::RecalculateSize
  ====================*/
void	CLabel::RecalculateSize()
{
	float fOldWidth(GetWidth());
	float fOldHeight(GetHeight());

	float fWidth(GetWidth());
	float fHeight(GetHeight());
	float fMaxWidth(GetSizeFromString(m_sFitXMax, fWidth, fHeight));
	CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFontMap));
	float fMaxTextHeight(0.0f);

	if(pFontMap)
		fMaxTextHeight = pFontMap->GetMaxHeight();

	if (m_bFitX)
	{
		fWidth = m_fBiggestWidth;
		if (fMaxWidth > 0.0f && fWidth > fMaxWidth)
		{
			fWidth = fMaxWidth;
		}
		SetWidth(fWidth + ROUND(GetSizeFromString(m_sFitXPadding, fWidth, fHeight)));
	}
	else
	{
		SetWidth(GetSizeFromString(m_sWidth, GetParentWidth(), GetParentHeight()));
		fWidth = GetWidth();
	}

	if (m_bFitY)
	{
		fHeight = m_fWrapCount * fMaxTextHeight;
		SetHeight(fHeight + ROUND(GetSizeFromString(m_sFitYPadding, fMaxTextHeight, fWidth)));
	}
	else
	{
		SetHeight(GetSizeFromString(m_sHeight, GetParentHeight(), GetParentWidth()));
	}
	
	if(!(m_pParent && m_pParent->HasFlags(WFLAG_REGROW)) || !HasFlags(WFLAG_REGROW))
		RecalculatePosition();

	if (GetWidth() != fOldWidth || GetHeight() != fOldHeight)
		RecalculateChildSize();

	if (m_pParent != NULL && m_pParent->HasFlags(WFLAG_GROW_WITH_CHILDREN))
		m_pParent->RecalculateSize();
}

/*====================
  CLabel::NullSize
  ====================*/
void	CLabel::NullSize()
{
	m_vLineWrap.clear();
	m_vLineCentering.clear();

	if(!(m_bFitX || m_bFitY))
		return;

	float fWidth(GetWidth());
	float fHeight(GetHeight());
	float fMaxWidth(GetSizeFromString(m_sFitXMax, fWidth, fHeight));
	CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFontMap));
	float fMaxTextHeight(0.0f);

	if(pFontMap)
		fMaxTextHeight = pFontMap->GetMaxHeight();

	float fBiggestWidth(1.0f);

	if (m_bFitX)
	{
		fWidth = fBiggestWidth;
		if (fMaxWidth > 0.0f && fWidth > fMaxWidth)
		{
			fWidth = fMaxWidth;
		}
		SetWidth(fWidth + ROUND(GetSizeFromString(m_sFitXPadding, fWidth, fHeight)));
	}

	if (m_bFitY)
	{
		fHeight = fMaxTextHeight;
		SetHeight(fHeight + ROUND(GetSizeFromString(m_sFitYPadding, fMaxTextHeight, fWidth)));
	}
}

/*====================
  CLabel::RenderWidget
  ====================*/
void	CLabel::RenderWidget(const CVec2f &v2Origin, float fFade)
{
	if (!HasFlags(WFLAG_VISIBLE) || m_sText.empty())
		return;

	float fX(ROUND(v2Origin.x));
	float fY(ROUND(v2Origin.y));
	float fWidth(m_recArea.GetWidth());
	if (fWidth == 0.0f)
		fWidth = Draw2D.GetScreenW() - fX;
	float fHeight(m_recArea.GetHeight());
	if (fHeight == 0.0f)
		fHeight = Draw2D.GetScreenH() - fY;

	int iFlags(m_iDrawFlags);

	// Keep the alpha the same between shadow and text
	const CVec4f &v4Color(fFade == 1.0f ? m_v4Color : GetFadedColor(m_v4Color, fFade));
	const CVec4f &v4ShadowColor(fFade == 1.0f ? m_v4ShadowColor : GetFadedColor(m_v4ShadowColor, fFade));

	if (m_bWrap || m_bLineRet)
	{
		Draw2D.SetColor(v4Color);
		Draw2D.String(fX, fY, fWidth, fHeight, m_sText, m_hFontMap, m_vLineWrap, m_vLineCentering, iFlags, m_bShadow, m_bOutline, m_fShadowOffsetX, m_fShadowOffsetY, v4ShadowColor);
	}
	else
	{
		Draw2D.SetColor(v4Color);
		Draw2D.String(fX, fY, fWidth, fHeight, m_sText, m_hFontMap, m_vLineWrap, m_vLineCentering, iFlags, m_bShadow, m_bOutline, m_fShadowOffsetX, m_fShadowOffsetY, v4ShadowColor);
	}
}


/*====================
  CLabel::SetText
  ====================*/
void	CLabel::SetText(const tstring &sStr)
{
	if (sStr.size() == 0)
	{
		m_sText = sStr;
		m_sRenderText = m_sText;
		NullSize();
		return;
	}

	m_sText = sStr;

	m_sRenderText = StripColorCodes(m_sText);

	if (m_iPrecision >= 0 && !m_sText.empty() && !(m_bFitX || m_bFitY))
		m_sText = XtoA(AtoF(m_sText), 0, 0, m_iPrecision);

	m_bLineRet = false;
	if (m_sText.find_first_of(_T("\n")) != -1)
		m_bLineRet = true;

	if (m_bWrap || m_bFitX || m_bFitY || m_bLineRet)
		RecalculateText();
	else
	{
		m_vLineCentering.clear();
		m_vLineWrap.clear();

		CFontMap *pFontMap(g_ResourceManager.GetFontMap(m_hFontMap));
		if(pFontMap == NULL)
			return; 

		float fBiggestWidth(pFontMap->GetStringWidth(m_sRenderText));

		m_vLineCentering.push_back(fBiggestWidth);

		if (m_fBiggestWidth != fBiggestWidth)
			m_fBiggestWidth = fBiggestWidth;

		m_fWrapCount = 1;
	}

}


/*====================
  CLabel::SetFont
  ====================*/
void	CLabel::SetFont(const tstring &sFont)
{
	m_hFontMap = g_ResourceManager.LookUpName(sFont, RES_FONTMAP);
}


/*--------------------
  SetText
  --------------------*/
UI_VOID_CMD(SetText, 1)
{
	if (pThis == NULL ||
		pThis->GetType() != WIDGET_LABEL)
		return;

	static_cast<CLabel*>(pThis)->SetText(vArgList[0]->Evaluate());
}


/*--------------------
  ClearText
  --------------------*/
UI_VOID_CMD(ClearText, 0)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_LABEL)
		return;

	static_cast<CLabel*>(pThis)->SetText(_T(""));
}


/*--------------------
  GetTextWidth
  --------------------*/
UI_CMD(GetTextWidth, 0)
{
	if (pThis == NULL ||
		pThis->GetType() != WIDGET_LABEL)
		return TSNULL;

	CLabel *pLabel(static_cast<CLabel *>(pThis));

	// Retrieve the font map
	CFontMap *pFontMap(g_ResourceManager.GetFontMap(pLabel->GetFont()));
	if (pFontMap == NULL)
		return TSNULL;

	return XtoA(pFontMap->GetStringWidth(pLabel->GetText()));
}


/*--------------------
  SetFont
  --------------------*/
UI_VOID_CMD(SetFont, 1)
{
	if (pThis == NULL ||
		pThis->GetType() != WIDGET_LABEL)
		return;

	static_cast<CLabel*>(pThis)->SetFont(vArgList[0]->Evaluate());
}




