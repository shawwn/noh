// (C)2005 S2 Games
// i_listwidget.h
//
//=============================================================================
#ifndef __I_LISTWIDGET__
#define __I_LISTWIDGET__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CListItem;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EWrapMode
{
	LISTBOX_WRAP_NONE,
	LISTBOX_WRAP_COLUMN,
	LISTBOX_WRAP_ROW
};
//=============================================================================

//=============================================================================
// IListWidget
//=============================================================================
class IListWidget : public IWidget
{
private:
	tstring		m_sItemFont;
	tstring		m_sItemAlign;
	tstring		m_sTextColor;

protected:
	EWrapMode	m_eWrapMode;

public:
	virtual ~IListWidget()	{}
	IListWidget(CInterface *pInterface, IWidget *pParent, EWidgetType widgetType, const CWidgetStyle &style);

	virtual EWrapMode		GetWrapMode() const			{ return m_eWrapMode; }

	virtual const tstring&	GetBaseListItemWidth() const = 0;
	virtual const tstring&	GetBaseListItemHeight() const = 0;

	virtual float		GetListItemWidth() const = 0;
	virtual float		GetListItemHeight() const = 0;
	virtual uint		GetNumListitems() const = 0;

	virtual void		AddListItem(CListItem *pListItem, const bool bReverseSort = false) = 0;
	virtual void		RemoveListItem(CListItem *pListItem) = 0;
	virtual void		ShowListItem(const tstring &sValue) = 0;
	virtual void		HideListItem(const tstring &sValue) = 0;
	virtual void		CreateNewListItemFromTemplate(const tstring &sName, const tstring &sValue, const CXMLNode::PropertyMap &mapParams) = 0;
	virtual void		CreateNewListItemFromTemplateWithSort(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams) = 0;
	virtual void		CreateNewListItemFromTemplateWithSortReversed(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams) = 0;
	virtual void		ResizeListTemplate(const tstring &sName, uint uiSize, const CXMLNode::PropertyMap &mapParams) = 0;
	virtual void		ClearList() = 0;
	virtual void		Clear()				{ ClearList(); }

	virtual void			SetSelectedItem(int iSelectedItem, bool bEvent) = 0;
	virtual void			SetSelectedItem(const tstring &sValue, bool bEvent) = 0;
	virtual IListWidget*	GetListWidget() = 0;

	virtual CListItem*	GetItem(uint uiItem) = 0;
	virtual CListItem*	GetItemByValue(const tstring &sValue) = 0;
	virtual uint		GetItemIndex(CListItem *pItem) = 0;

	virtual uint		GetListDrawStartIndex() const=0;

	const tstring&	GetItemFont()		{ return m_sItemFont; }
	const tstring&	GetItemAlign()		{ return m_sItemAlign; }
	const tstring&	GetTextColor()		{ return m_sTextColor; }
};
//=============================================================================

#endif // __I_LISTWIDGET__
