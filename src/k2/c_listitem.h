// (C)2005 S2 Games
// c_listitem.h
//
//=============================================================================
#ifndef __C_LISTITEM_H__
#define __C_LISTITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
#include "i_listwidget.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint LISTITEM_SELECT					(BIT(0));
const uint LISTITEM_COMMAND					(BIT(1));
const uint LISTITEM_DRAW_COLOR_UNDER		(BIT(2));
const uint LISTITEM_USE_BACKGROUND_IMAGE	(BIT(3));
const uint LISTITEM_DRAW_COLORS				(BIT(4));
//=============================================================================

//=============================================================================
// CListItem
//=============================================================================
class CListItem : public IWidget
{
protected:
	TCHAR		m_cHotkey;
	uint		m_uiListItemFlags;
	tstring		m_sValue;
	tsvector	m_vSortIndex;
	tstring		m_sCondition;

	CVec4f		m_v4LastColor;
	CVec4f		m_v4NextColor;
	CVec4f		m_v4LastBorderColor;
	CVec4f		m_v4NextBorderColor;

	uint		m_uiTransitionStart;
	uint		m_uiTransitionTime;

public:
	~CListItem()	{}
	K2_API CListItem(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

	float			GetParentWidth() const					{ return static_cast<IListWidget*>(m_pParent)->GetListItemWidth(); }
	float			GetParentHeight() const					{ return static_cast<IListWidget*>(m_pParent)->GetListItemHeight(); }

	virtual CVec2f	GetAbsolutePos();

	void			SetValue(const tstring &sValue)			{ m_sValue = sValue; }
	void			SetText(tstring sText);
	void			SetSelect(bool bValue)					{ if (bValue) m_uiListItemFlags |= LISTITEM_SELECT; else m_uiListItemFlags &= ~LISTITEM_SELECT; }
	void			SetCommand(bool bValue)					{ if (bValue) m_uiListItemFlags |= LISTITEM_COMMAND; else m_uiListItemFlags &= ~LISTITEM_COMMAND; }
	virtual tstring	GetValue() const						{ return m_sValue; }
	tstring			GetText() const;
	TCHAR			GetHotkey() const						{ return m_cHotkey; }
	bool			GetSelect() const						{ return (m_uiListItemFlags & LISTITEM_SELECT) != 0; }
	bool			GetCommand() const						{ return (m_uiListItemFlags & LISTITEM_COMMAND) != 0; }
	bool			GetDrawColorUnder() const				{ return (m_uiListItemFlags & LISTITEM_DRAW_COLOR_UNDER) != 0; }
	bool			GetUseBackgroundImage() const			{ return (m_uiListItemFlags & LISTITEM_USE_BACKGROUND_IMAGE) != 0; }
	bool			GetDrawColors() const					{ return (m_uiListItemFlags & LISTITEM_DRAW_COLORS) != 0; }
	const tstring&	GetSortIndex(uint uiIndex = 0) const	{ if (m_vSortIndex.size() > uiIndex) return m_vSortIndex[uiIndex]; return TSNULL; }
	const tstring&	GetCondition() const					{ return m_sCondition; }

	void			SetNextColor(const CVec4f &v4Color, const CVec4f &v4BorderColor);
	void			SetLastColor(const CVec4f &v4Color, const CVec4f &v4BorderColor)		{ m_v4LastColor = v4Color; m_v4LastBorderColor = v4BorderColor; }
	void			SetTransitionTime(uint uiTime)			{ m_uiTransitionTime = uiTime; }
	void			SetDrawColorUnder(bool bValue)			{ if (bValue) m_uiListItemFlags |= LISTITEM_DRAW_COLOR_UNDER; else m_uiListItemFlags &= ~LISTITEM_DRAW_COLOR_UNDER; }
	void			SetUseBackgroundImage(bool bValue)		{ if (bValue) m_uiListItemFlags |= LISTITEM_USE_BACKGROUND_IMAGE; else m_uiListItemFlags &= ~LISTITEM_USE_BACKGROUND_IMAGE; }
	void			SetDrawColors(bool bValue)				{ if (bValue) m_uiListItemFlags |= LISTITEM_DRAW_COLORS; else m_uiListItemFlags &= ~LISTITEM_DRAW_COLORS; }

	void			RenderWidget(const CVec2f &vOrigin, float fFade)	{}
	void			Render(const CVec2f &vOrigin, int iFlag, float fFade, bool bHovered, bool bSelected, CWidgetState *pBackgroundState, CWidgetState *pHighlightState);
};
//=============================================================================

#endif //__C_LISTITEM_H__
