// (C)2009 S2 Games
// c_menu.h
//
//=============================================================================
#ifndef __C_MENU_H__
#define __C_MENU_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_listwidget.h"
#include "c_cvarreference.h"
#include "c_menu_listbox.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CImage;
class CListItem;
//=============================================================================

//=============================================================================
// CMenu
//=============================================================================
class CMenu : public IListWidget
{
protected:
    CMenuListBox*       m_pListBox;

    bool    m_bSingleClick;
    bool    m_bLeft;
    bool    m_bExclusive;
    int     m_iMaxListHeight;

    tstring m_sListPaddingY;
    float   m_fListPaddingY;

public:
    ~CMenu();
    CMenu(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    virtual void        MouseDown(EButton button, const CVec2f &v2CursorPos);
    virtual void        MouseUp(EButton button, const CVec2f &v2CursorPos);

    virtual bool        ButtonDown(EButton button);
    virtual bool        ButtonUp(EButton button)                { return true; }

    virtual bool        Char(TCHAR c)                           { return false; }

    virtual void        Enable();
    virtual void        Disable();

    virtual void        Rollover();
    virtual void        Rolloff();

    virtual bool        ProcessInputCursor(const CVec2f &v2CursorPos);
    
    virtual void        Frame(uint uiFrameLength, bool bProcessFrame);
    virtual void        Render(const CVec2f &vOrigin, int iFlag, float fFade);

    void            SelectItem(CListItem *pListItem, bool bEvent);

    K2_API void     AddListItem(CListItem *pListItem, const bool bReverseSort = false);
    K2_API void     RemoveListItem(CListItem *pListItem);
    K2_API void     ShowListItem(const tstring &sValue) {}
    K2_API void     HideListItem(const tstring &sValue) {}
    K2_API void     ClearList();
    K2_API void     CreateNewListItemFromTemplate(const tstring &sName, const tstring &sValue, const CXMLNode::PropertyMap &mapParams);
    K2_API void     CreateNewListItemFromTemplateWithSort(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams);
    K2_API void     CreateNewListItemFromTemplateWithSortReversed(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams);
    K2_API void     ResizeListTemplate(const tstring &sName, uint uiSize, const CXMLNode::PropertyMap &mapParams);
    K2_API void     SortList();
    K2_API tstring  GetListItemValue(uint uItem);
    K2_API void     SetListItemText(uint uItem, tstring sValue);

    virtual uint            GetListDrawStartIndex() const               { return 0; /*TODO*/ }

    virtual const tstring&  GetBaseListItemWidth() const;
    virtual const tstring&  GetBaseListItemHeight() const;
    virtual float           GetListItemWidth() const;
    virtual float           GetListItemHeight() const;
    virtual uint            GetNumListitems() const;

    void            SetSelectedItem(int iSelectedItem, bool bEvent);
    void            SetSelectedItem(const tstring &sValue, bool bEvent);

    CWidgetState*   AllocateWidgetState(const CWidgetStyle &style);
    virtual bool    AddWidgetState(CWidgetState *pState);

    void            WidgetLost(IWidget *pWidget);

    virtual void    RecalculateChildSize();

    virtual IListWidget*    GetListWidget()                     { return m_pListBox; }

    void            Open(bool bMidClick);
    void            Close();
    void            UpdateMenuConditions();

    virtual bool        UseMouseDown() const;

    virtual CListItem*  GetItem(uint uiItem);
    virtual CListItem*  GetItemByValue(const tstring &sValue);
    virtual uint        GetItemIndex(CListItem *pItem);
};
//=============================================================================

#endif //__C_MENU_H__
