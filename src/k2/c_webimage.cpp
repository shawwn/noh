// (C)2010 S2 Games
// c_webimage.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_webimage.h"

#include "c_widgetstyle.h"
#include "c_uitextureregistry.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
#include "c_texture.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CWebImage::~CWebImage
  ====================*/
CWebImage::~CWebImage()
{
	if (!m_sURL.empty() && m_eState == WEBIMAGE_FINISHED)
		UITextureRegistry.ReleaseDownloadedTexture(m_sURL);
}


/*====================
  CWebImage::CWebImage
  ====================*/
CWebImage::CWebImage(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_WEBIMAGE, style),
m_eState(WEBIMAGE_BLANK)
{
	// Color
	if (!style.HasProperty(_CTS("color")))
		SetColor(WHITE);

	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)

	if (style.HasProperty(_CTS("textureurl")))
		SetTextureURL(style.GetProperty(_CTS("textureurl")));
}


/*====================
  CWebImage::SetTextureURL
  ====================*/
void	CWebImage::SetTextureURL(const tstring &sURL)
{
	tstring sCleanURL(FileManager.SanitizePath(sURL));

	size_t zPos(sCleanURL.find(_T("://")));
	if (zPos == tstring::npos)
	{
		if (!m_sURL.empty() && m_eState == WEBIMAGE_FINISHED)
			UITextureRegistry.ReleaseDownloadedTexture(m_sURL);

		m_sURL.clear();

		m_hTexture[0] = g_ResourceManager.GetWhiteTexture();

		return;
	}

	if (sCleanURL == m_sURL)
		return;

	if (!m_sURL.empty() && m_eState == WEBIMAGE_FINISHED)
		UITextureRegistry.ReleaseDownloadedTexture(m_sURL);

	m_sURL = sCleanURL;

	m_eState = WEBIMAGE_DOWNLOADING;

	UITextureRegistry.StartDownload(sCleanURL);

	if (UITextureRegistry.IsDownloaded(sCleanURL))
	{
		m_eState = WEBIMAGE_FINISHED;

		m_hTexture[0] = UITextureRegistry.GetDownloadedTexture(sCleanURL);
	}
}


/*====================
  CWebImage::Frame
  ====================*/
void	CWebImage::Frame(uint uiFrameLength, bool bProcessFrame)
{
	IWidget::Frame(uiFrameLength, bProcessFrame);

	if (m_eState == WEBIMAGE_DOWNLOADING)
	{
		if (UITextureRegistry.IsDownloaded(m_sURL))
		{
			m_eState = WEBIMAGE_FINISHED;

			m_hTexture[0] = UITextureRegistry.GetDownloadedTexture(m_sURL);
		}
	}
}


/*====================
  CWebImage::MouseDown
  ====================*/
void	CWebImage::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
	if (button == BUTTON_MOUSEL)
	{
		DO_EVENT(WEVENT_MOUSELDOWN)
		DO_EVENT(WEVENT_CLICK)
		
	}
	else if (button == BUTTON_MOUSER)
	{
		DO_EVENT(WEVENT_MOUSERDOWN)
		DO_EVENT(WEVENT_RIGHTCLICK)
	}
}


/*====================
  CWebImage::MouseUp
  ====================*/
void	CWebImage::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
	if (button == BUTTON_MOUSEL)
	{
		DO_EVENT(WEVENT_MOUSELUP)
	}
	else if (button == BUTTON_MOUSER)
	{
		DO_EVENT(WEVENT_MOUSERUP)
	}
}


/*====================
  CWebImage::SetTexture
  ====================*/
void	CWebImage::SetTexture(const tstring &sTexture)
{
	SetTextureURL(TSNULL);

	IWidget::SetTexture(sTexture);
}

void	CWebImage::SetTexture(const tstring &sTexture, const tstring &sSuffix)
{
	SetTextureURL(TSNULL);

	IWidget::SetTexture(sTexture, sSuffix);
}


/*--------------------
  SetTextureURL
  --------------------*/
UI_VOID_CMD(SetTextureURL, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_WEBIMAGE)
		return;

	static_cast<CWebImage *>(pThis)->SetTextureURL(vArgList[0]->Evaluate());
}

