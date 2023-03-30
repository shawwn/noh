// (C)2010 S2 Games
// c_avatar.h
//
//=============================================================================
#ifndef __C_AVATAR_H__
#define __C_AVATAR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CAvatar
//=============================================================================
class CAvatar : public IWidget
{
private:

public:
	~CAvatar();
	K2_API CAvatar(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

	void	SetAvatar(const tstring &sURL);

	virtual void	MouseUp(EButton button, const CVec2f &v2CursorPos);
	virtual void	MouseDown(EButton button, const CVec2f &v2CursorPos);

	virtual void	SetTexture(const tstring &sTexture);
	virtual void	SetTexture(const tstring &sTexture, const tstring &sSuffix);
};
//=============================================================================

#endif //__C_AVATAR_H__
