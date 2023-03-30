// (C)2007 S2 Games
// c_buttoncatcher.h
//
//=============================================================================
#ifndef __C_BUTTONCATCHER_H__
#define __C_BUTTONCATCHER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// CButtonCatcher
//=============================================================================
class CButtonCatcher : public IWidget
{
	EButton		m_eLastButton;
	int			m_iLastModifier;
	tstring		m_sLastButtonName;

	bool		m_bImpulse;

	void	UpdateButtonName(EButton button);

public:
	~CButtonCatcher()	{}
	K2_API CButtonCatcher(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

	tstring	GetValue() const			{ return m_sLastButtonName; }

	virtual void	MouseDown(EButton button, const CVec2f &v2CursorPos);
	virtual bool	ButtonDown(EButton button);
	virtual bool	ButtonUp(EButton button);

	virtual void	DoEvent(EWidgetEvent eEvent, const tstring &sParam = TSNULL);
	virtual void	DoEvent(EWidgetEvent eEvent, const tsvector &vParam);
};
//=============================================================================
#endif // __C_BUTTONCATCHER_H__
