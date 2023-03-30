// (C)2010 S2 Games
// c_avatar.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_avatar.h"

#include "c_widgetstyle.h"
#include "c_uitextureregistry.h"
#include "c_draw2d.h"
#include "c_uicmd.h"
#include "c_filehttp.h"
#include "c_texture.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CAvatar::~CAvatar
  ====================*/
CAvatar::~CAvatar()
{
}


/*====================
  CAvatar::CAvatar
  ====================*/
CAvatar::CAvatar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_AVATAR, style)
{
	// Color
	if (!style.HasProperty(_CTS("color")))
		SetColor(WHITE);

	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)

	if (style.HasProperty(_CTS("avatar")))
		SetAvatar(style.GetProperty(_CTS("avatar")));
}


/*====================
  CAvatar::SetAvatar
  ====================*/
void	CAvatar::SetAvatar(const tstring &sURL)
{
	uint uiCrc32(FileManager.GetCRC32((char *)sURL.c_str(), sURL.length() * sizeof(TCHAR)));

	IWidget::SetTexture((uiCrc32 & 1) == 0 ? _T("/npcs/chiprel/icon.tga") : _T("/heroes/dwarf_magi/icon.tga"));
}


/*====================
  CAvatar::MouseDown
  ====================*/
void	CAvatar::MouseDown(EButton button, const CVec2f &v2CursorPos)
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
  CAvatar::MouseUp
  ====================*/
void	CAvatar::MouseUp(EButton button, const CVec2f &v2CursorPos)
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
  CAvatar::SetTexture
  ====================*/
void	CAvatar::SetTexture(const tstring &sTexture)
{
	SetAvatar(TSNULL);

	IWidget::SetTexture(sTexture);
}

void	CAvatar::SetTexture(const tstring &sTexture, const tstring &sSuffix)
{
	SetAvatar(TSNULL);

	IWidget::SetTexture(sTexture, sSuffix);
}


/*--------------------
  SetAvatar
  --------------------*/
UI_VOID_CMD(SetAvatar, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_AVATAR)
		return;

	static_cast<CAvatar *>(pThis)->SetAvatar(vArgList[0]->Evaluate());
}

