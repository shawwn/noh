// (C)2005 S2 Games
// c_listbox.h
//
//=============================================================================
#ifndef __C_LISTBOX_H__
#define __C_LISTBOX_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_listwidget.h"
#include "c_widgetstate.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CListItem;
class CListBoxScrollbar;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CListItem
//=============================================================================
class CListWidgetState : public CWidgetState
{
private:

public:
	~CListWidgetState()	{}
	CListWidgetState(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
	CWidgetState(pInterface, pParent, style)
	{
		assert(pParent->HasFlags(WFLAG_LIST));
	}

	CListWidgetState*	GetAsListWidgetState()	{ return this; }

	float	GetParentWidth() const	{ return static_cast<IListWidget*>(m_pParent)->GetListItemWidth(); }
	float	GetParentHeight() const	{ return static_cast<IListWidget*>(m_pParent)->GetListItemHeight(); }
};
//=============================================================================

//=============================================================================
// CListBox
//=============================================================================
class CListBox : public IListWidget
{
protected:
	typedef vector<CListItem*>			ListItemVector;
	typedef ListItemVector::iterator	ListItemVector_it;
	typedef ListItemVector::reverse_iterator	ListItemVector_rit;

	float	m_fHPadding;
	float	m_fVPadding;

	float	m_fScrollbarSize;

	int		m_iListDrawStart;
	int		m_iSelectedListItem;
	int		m_iHoverListItem;

	CVec4f	m_v4BackgroundFill;
	CVec4f	m_v4BackgroundBorder;
	CVec4f	m_v4HighlightFill;
	CVec4f	m_v4HighlightBorder;
	CVec4f	m_v4SelectedFill;
	CVec4f	m_v4SelectedBorder;

	tstring	m_sListItemImage;
	CVec4f	m_v4ItemImageColor;

	uint	m_uiColorTransitionTime;
	bool	m_bColorTransitions;

	bool	m_bAutoSize;
	bool	m_bHoverSelect;
	bool	m_bHoverHighlight;
	bool	m_bSelect;
	bool	m_bHighlightOver;
	bool	m_bExteriorScrollbars;
	bool	m_bScrollbarPlaceholder;
	bool	m_bUseScrollbars;

	uint	m_uiDoubleClickTime;
	EButton	m_eLastClickType;
	uint	m_uiLastClickTime;
	int		m_iLastClickItem;

	int		m_iLastListItem;
	bool	m_bMouseOver;
	bool	m_bThisTimeOutOfArea;

	ListItemVector	m_vItems;
	ListItemVector	m_vHiddenItems;

	CListBoxScrollbar*	m_pHScrollbar;
	CListBoxScrollbar*	m_pVScrollbar;

	tstring	m_sItemAlign;
	tstring	m_sTextColor;

	tstring	m_sListItemHeight;
	tstring	m_sListItemWidth;

	float	m_fListItemHeight;
	float	m_fListItemWidth;

	float	m_fListItemWidthFudge;
	float	m_fListItemHeightFudge;

	tstring	m_sListItemOffsetX;
	tstring	m_sListItemOffsetY;

	float	m_fListItemOffsetX;
	float	m_fListItemOffsetY;

	CCvarReference	m_refCvar;

	IWidget*		m_pHoverWidget;

	CWidgetState*		m_pBackground;
	CListWidgetState*	m_pItemBackground;
	CListWidgetState*	m_pItemHighlight;

	bool	m_bClearSelection;

	void	UpdateScrollbars();
	void	UpdateCvar();
	void	SetHoverWidget(IWidget *pWidget);
	
public:
	~CListBox();
	CListBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style);

	virtual tstring	GetValue() const;

	virtual void	Show(uint uiDuration = -1);

	virtual void	MouseDown(EButton button, const CVec2f &v2CursorPos);
	virtual void	MouseUp(EButton button, const CVec2f &v2CursorPos);

	virtual bool	ButtonDown(EButton button);
	virtual bool	ButtonUp(EButton button);

	virtual bool	Char(TCHAR c);

	virtual void	Render(const CVec2f &v2Origin, int iFlag, float fFade);

	virtual bool	ProcessInputCursor(const CVec2f &v2CursorPos);

	virtual const tstring&	GetBaseListItemWidth() const				{ return m_sListItemWidth; }
	virtual const tstring&	GetBaseListItemHeight() const				{ return m_sListItemHeight; }
	virtual float			GetListItemWidth() const					{ return m_fListItemWidth; }
	virtual float			GetListItemHeight() const					{ return m_fListItemHeight; }
	virtual uint			GetNumListitems() const						{ return uint(m_vItems.size()); }

	virtual uint			GetListDrawStartIndex() const				{ return (uint)MAX(0, m_iListDrawStart); }

	K2_API void		AddListItem(CListItem *pListItem, const bool bReverseSort = false);
	K2_API void		RemoveListItem(CListItem *pListItem);
	K2_API void		ShowListItem(const tstring &sValue);
	K2_API void		HideListItem(const tstring &sValue);
	K2_API void		RemoveListItemByName(const tstring &sName);
	K2_API void		CreateNewListItemFromTemplate(const tstring &sName, const tstring &sValue, const CXMLNode::PropertyMap &mapParams);
	K2_API void		CreateNewListItemFromTemplateWithSort(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams);
	K2_API void		CreateNewListItemFromTemplateWithSortReversed(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams);
	K2_API void		ResizeListTemplate(const tstring &sName, uint uiSize, const CXMLNode::PropertyMap &mapParams);


	CListItem*		GetSelectedListItem();
	CListItem*		GetHoverListItem();
	tstring			GetListItemValue(uint uItem);
	void			SetListItemText(uint uItem, tstring sValue);
	void			SetSelectedItem(int iSelectedListItem, bool bEvent);
	void			SetSelectedItem(const tstring &sValue, bool bEvent);
	void			SetSelectedItem(CListItem *pListItem, bool bEvent);
	void			SetHoverItem(int iHoveredListItem, bool bEvent);
	void			SetHoverItem(CListItem *pListItem, bool bEvent);
	CListItem*		GetItem(uint uiItem);
	CListItem*		GetItemByValue(const tstring &sValue);
	uint			GetItemIndex(CListItem *pItem);
	uint 			GetSelectedItem() 							{ return m_iSelectedListItem; }

	void	HorizontalScrollbarChange(float fNewValue);
	void	VerticalScrollbarChange(float fNewValue);

	K2_API void		SortList();
	K2_API void		SortListValue();
	K2_API void		SortListNumericValue();
	K2_API void		SortListSortIndex(int iIndex);
	K2_API void		SortListSortIndexNumeric(int iIndex);
	K2_API void		ClearList();

	virtual void	DoEvent(EWidgetEvent eEvent, const tstring &sParam = TSNULL);
	virtual void	DoEvent(EWidgetEvent eEvent, const tsvector &vParam);

	virtual CWidgetState*	AllocateWidgetState(const CWidgetStyle &style);
	virtual bool			AddWidgetState(CWidgetState *pState);

	void			WidgetLost(IWidget *pWidget);

	virtual void	RecalculateSize();
	virtual void	RecalculateChildSize();

	virtual IListWidget*	GetListWidget()						{ return this; }

	virtual void	Frame(uint uiFrameLength, bool bProcessFrame);

	virtual void	Purge();

	void			UpdateConditions();

	void			SetSelectedListItem(int iItem);
	void			SetHoverListItem(int iItem);
};
//=============================================================================

#endif // __C_LISTBOX_H__
