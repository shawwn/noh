// (C)2004 S2 Games
// c_combobox.h
//
//=============================================================================
#ifndef __C_COMBOBOX_H__
#define __C_COMBOBOX_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_listwidget.h"
#include "c_cvarreference.h"
#include "c_combobox_listbox.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CImage;
class CListItem;

const tstring g_asComboBoxStateNames[] =
{
    _T("up"),
    _T("down"),
    _T("over"),
    _T("disabled")
};

enum EComboBoxState
{
    COMBOBOX_UP,
    COMBOBOX_DOWN,
    COMBOBOX_OVER,
    COMBOBOX_DISABLED,

    NUM_COMBOBOXSTATES
};
//=============================================================================

//=============================================================================
// CComboBox
//=============================================================================
class CComboBox : public IListWidget
{
protected:
    CListItem*          m_pActiveListItem;
    CComboBoxListBox*   m_pListBox;

    CWidgetState*       m_apWidgetStates[NUM_COMBOBOXSTATES];
    EComboBoxState      m_eWidgetState;

    bool    m_bSingleClick;
    bool    m_bLeft;
    int     m_iMaxListHeight;

    CCvarReference  m_refCvar;
    tstring m_sCvar;

    void    UpdateCvar();

public:
    ~CComboBox();
    CComboBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

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

    void        SetActiveListItem(CListItem *pListItem, bool bEvent);
    tstring     GetValue() const;
    CListItem*  GetSelectedListItem() const;

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

    virtual CListItem*  GetItem(uint uiItem);
    virtual CListItem*  GetItemByValue(const tstring &sValue);
    virtual uint        GetItemIndex(CListItem *pItem);
};
//=============================================================================

#endif //__C_COMBOBOX_H__
