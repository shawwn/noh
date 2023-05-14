// (C)2009 S2 Games
// c_menu.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_menu.h"

#include "c_interface.h"
#include "c_menu_listbox.h"
#include "c_image.h"
#include "c_listitem.h"
#include "c_uiscript.h"
#include "c_widgetstyle.h"
#include "c_widgetstate.h"
#include "c_uicmd.h"
#include "c_xmlnode.h"
#include "c_widgettemplate.h"
#include "c_uimanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================


/*====================
  CMenu::CMenu
  ====================*/
CMenu::~CMenu()
{
    SAFE_DELETE(m_pListBox);
}


/*====================
  CMenu::CMenu
  ====================*/
CMenu::CMenu(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IListWidget(pInterface, pParent, WIDGET_MENU, style),
m_pListBox(nullptr),
m_bSingleClick(false),
m_bLeft(false),
m_iMaxListHeight(style.GetPropertyInt(_T("maxlistheight"), -1)),
m_bExclusive(true)
{
    if (style.GetPropertyBool(_T("interactive"), true))
        SetFlags(WFLAG_INTERACTIVE);

    m_sListPaddingY = style.GetProperty(_T("listpaddingy"), _T("0"));
    m_fListPaddingY = GetSizeFromString(m_sListPaddingY, GetParentHeight(), GetParentWidth());

    CWidgetStyle styleCopy(style);
    styleCopy.RemoveProperty(_T("name"));
    styleCopy.RemoveProperty(_T("group"));
    styleCopy.RemoveProperty(_T("onselect"));
    styleCopy.RemoveProperty(_T("onframe"));
    styleCopy.RemoveProperty(_T("ontrigger"));
    styleCopy.RemoveProperty(_T("onshow"));
    styleCopy.RemoveProperty(_T("onhide"));
    styleCopy.RemoveProperty(_T("onenable"));
    styleCopy.RemoveProperty(_T("ondisable"));
    styleCopy.RemoveProperty(_T("onchange"));
    styleCopy.RemoveProperty(_T("onslide"));
    styleCopy.RemoveProperty(_T("onselect"));
    styleCopy.RemoveProperty(_T("onclick"));
    styleCopy.RemoveProperty(_T("ondoubleclick"));
    styleCopy.RemoveProperty(_T("onrightclick"));
    styleCopy.RemoveProperty(_T("onfocus"));
    styleCopy.RemoveProperty(_T("onlosefocus"));
    styleCopy.RemoveProperty(_T("onload"));

    styleCopy.RemoveProperty(_T("align"));
    styleCopy.RemoveProperty(_T("valign"));
    styleCopy.RemoveProperty(_T("cvar"));
    styleCopy.RemoveProperty(_T("form"));
    styleCopy.RemoveProperty(_T("data"));
    styleCopy.RemoveProperty(_T("watch"));
    styleCopy.RemoveProperty(_T("ontrigger"));

    for (int i(0); i < 10; ++i)
    {
        styleCopy.RemoveProperty(_T("watch") + XtoA(i));
        styleCopy.RemoveProperty(_T("ontrigger") + XtoA(i));
    }

    styleCopy.SetProperty(_T("visible"), false);
    styleCopy.SetProperty(_T("x"), style.GetProperty(_T("listx"), _T("0")));
    styleCopy.SetProperty(_T("y"), style.GetProperty(_T("listy"), _T("0")));
    styleCopy.SetProperty(_T("width"), style.GetProperty(_T("listwidth"), style.GetProperty(_T("itemwidth"), _T("100%"))));
    styleCopy.SetProperty(_T("itemwidth"),style.GetProperty(_T("itemwidth"), _T("100%")));
    styleCopy.SetProperty(_T("texture"), style.GetProperty(_T("listtexture")));
    styleCopy.SetProperty(_T("color"), style.GetProperty(_T("listcolor"), style.GetProperty(_T("color"))));
    styleCopy.SetProperty(_T("noclick"), style.GetPropertyBool(_T("noclick"), false));
    styleCopy.SetProperty(_T("select"), false);
    styleCopy.SetProperty(_T("hoverselect"), true);
    styleCopy.SetProperty(_T("clearselection"), true);

    if (!styleCopy.HasProperty(_T("itemheight")))
    {
        styleCopy.SetProperty(_T("itemheight"), _T("100%"));
        styleCopy.SetProperty(_T("height"), _T("0"));
    }
    else
    {
        styleCopy.SetProperty(_T("height"), _T("0"));
    }

    styleCopy.SetProperty(_T("name"), m_sName, _T("_listbox"));
    m_pListBox = K2_NEW(ctx_Widgets,  CMenuListBox)(m_pInterface, this, styleCopy);
    AddChild(m_pListBox);

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CMenu::Open
  ====================*/
void    CMenu::Open(bool bMidClick)
{
    if (m_pListBox->GetNumListitems() == 0)
        return;

    if (m_bExclusive)
        m_pInterface->SetExclusiveWidget(this);
    m_pInterface->SetActiveWidget(this);
    m_pListBox->Show();
    m_pListBox->SetMidClick(false);
    m_bSingleClick = bMidClick;
    m_bLeft = true;
    m_pListBox->SetSelectedItem(-1, false);
}


/*====================
  CMenu::Close
  ====================*/
void    CMenu::Close()
{
    if (!m_pListBox->HasFlags(WFLAG_VISIBLE))
        return;

    SetFocus(false);
    if (m_bExclusive)
        m_pInterface->SetExclusiveWidget(nullptr);
    g_pUIManager->RefreshCursor();
    m_pListBox->Hide();
    m_bSingleClick = false;
}


/*====================
  CMenu::ButtonDown
  ====================*/
bool    CMenu::ButtonDown(EButton button)
{
    switch (button)
    {
    case BUTTON_ESC:
        if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        {
            SetFocus(false);
            if (m_bExclusive)
                m_pInterface->SetExclusiveWidget(nullptr);
            g_pUIManager->RefreshCursor();
            m_pListBox->Hide();
            m_bSingleClick = false;
            return true;
        }
        break;

    case BUTTON_ENTER:
        if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        {
            SetFocus(false);
            if (m_bExclusive)
                m_pInterface->SetExclusiveWidget(nullptr);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
            SelectItem(m_pListBox->GetSelectedListItem(), true);
            m_bSingleClick = false;
            return true;
        }
        break;

    default:
        break;
    }

    if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        return m_pListBox->ButtonDown(button);
    else
        return true;
}


/*====================
  CMenu::MouseDown
  ====================*/
bool    CMenu::UseMouseDown() const
{
    return m_pListBox->HasFlags(WFLAG_VISIBLE);
}


/*====================
  CMenu::MouseDown
  ====================*/
void    CMenu::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (!m_bSingleClick)
    {
        if (!m_pListBox->GetRect().AltContains(v2CursorPos - m_recArea.lt()) && !m_recArea.AltContains(v2CursorPos))
        {
            SetFocus(false);
            if (m_bExclusive)
                m_pInterface->SetExclusiveWidget(nullptr);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
        }
        else if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        {
            m_pListBox->SetMidClick(true);
        }
        else if (m_pListBox->GetNumListitems() > 0)
        {
            if (m_bExclusive)
                m_pInterface->SetExclusiveWidget(this);
            m_pListBox->Show();
            m_pListBox->SetMidClick(true);
            m_bSingleClick = true;
            m_bLeft = false;
            m_pListBox->SetSelectedItem(-1, false);
        }
    }
    else
    {
        // Is this even possible?
        if (m_recArea.AltContains(v2CursorPos))
        {
            SetFocus(false);
            if (m_bExclusive)
                m_pInterface->SetExclusiveWidget(nullptr);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
        }

        m_bSingleClick = false;
    }
}


/*====================
  CMenu::MouseUp
  ====================*/
void    CMenu::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    if (m_bSingleClick)
    {
        m_bSingleClick = false;
    }
    else if (m_pListBox->HasFlags(WFLAG_VISIBLE) && m_recArea.AltContains(v2CursorPos))
    {
        SetFocus(false);
        if (m_bExclusive)
            m_pInterface->SetExclusiveWidget(nullptr);
        UIManager.RefreshCursor();
        m_pListBox->Hide();
        m_bSingleClick = false;
    }
    else if (m_pListBox->HasFlags(WFLAG_VISIBLE) && !m_pListBox->Contains(v2CursorPos - m_recArea.lt()))
    {
        SetFocus(false);
        if (m_bExclusive)
            m_pInterface->SetExclusiveWidget(nullptr);
        UIManager.RefreshCursor();
        m_pListBox->Hide();
        m_bSingleClick = false;
    }

    m_pListBox->SetMidClick(false);
}


/*====================
  CMenu::ProcessInputCursor
  ====================*/
bool    CMenu::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    // Let children handle the input directly
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        if ((*it)->HasFlags(WFLAG_PROCESS_CURSOR) && (*it)->ProcessInputCursor(v2CursorPos - m_recArea.lt()))
            return true;

    return false;
}


/*====================
  CMenu::Enable
  ====================*/
void    CMenu::Enable()
{
    IWidget::Enable();
}


/*====================
  CMenu::Disable
  ====================*/
void    CMenu::Disable()
{
    IWidget::Disable(); 
}


/*====================
  CMenu::Rollover
  ====================*/
void    CMenu::Rollover()
{
    IWidget::Rollover();
}


/*====================
  CMenu::Rolloff
  ====================*/
void    CMenu::Rolloff()
{
    IWidget::Rolloff();

    m_bLeft = true;
}


/*====================
  CMenu::AddListItem
  ====================*/
void    CMenu::AddListItem(CListItem *pListItem, const bool bReverseSort)
{
    if (!bReverseSort)
        m_pListBox->AddListItem(pListItem);
    else
        m_pListBox->AddListItem(pListItem, true);   

    if (m_iMaxListHeight == -1)
    {
        m_pListBox->SetBaseHeight(XtoA(m_pListBox->GetNumListitems() * GetListItemHeight() + m_fListPaddingY));
        m_pListBox->RecalculateSize();
    }
    else
    {
        m_pListBox->SetBaseHeight(XtoA(MIN(int(m_pListBox->GetNumListitems()), m_iMaxListHeight) * GetListItemHeight() + m_fListPaddingY));
        m_pListBox->RecalculateSize();
    }
}


/*====================
  CMenu::CreateNewListItemFromTemplate
  ====================*/
void    CMenu::CreateNewListItemFromTemplate(const tstring &sName, const tstring &sValue, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        // Lookup the template
        CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
        if (pTemplate == nullptr)
            EX_ERROR(_T("Could not retrieve template named: ") + sName);

        // Create new listitem
        CXMLNode::PropertyMap mapProperties(mapParams);
        mapProperties[_T("value")] = sValue;
        CWidgetStyle style(m_pInterface, mapProperties);
        style.SetProperty(_T("width"), GetBaseListItemWidth());
        style.SetProperty(_T("height"), GetBaseListItemHeight());
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, m_pListBox, style));

        CWidgetStyle styleInstance(m_pInterface, mapParams);
        pTemplate->Instantiate(pNewListItem, styleInstance);
        AddListItem(pNewListItem);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CMenu::CreateNewListItemFromTemplate() - "), NO_THROW);
    }
}


/*====================
  CMenu::CreateNewListItemFromTemplateWithSort
  ====================*/
void    CMenu::CreateNewListItemFromTemplateWithSort(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        // Lookup the template
        CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
        if (pTemplate == nullptr)
            EX_ERROR(_T("Could not retrieve template named: ") + sName);

        // Create new listitem
        CXMLNode::PropertyMap mapProperties(mapParams);
        mapProperties[_T("value")] = sValue;
        CWidgetStyle style(m_pInterface, mapProperties);
        style.SetProperty(_T("width"), GetBaseListItemWidth());
        style.SetProperty(_T("height"), GetBaseListItemHeight());
        style.SetProperty(_T("sortindex"), sSort);
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, m_pListBox, style));

        CWidgetStyle styleInstance(m_pInterface, mapParams);
        pTemplate->Instantiate(pNewListItem, styleInstance);
        AddListItem(pNewListItem);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CMenu::CreateNewListItemFromTemplate() - "), NO_THROW);
    }
}


/*====================
  CMenu::CreateNewListItemFromTemplateWithSortReversed
  ====================*/
void    CMenu::CreateNewListItemFromTemplateWithSortReversed(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        // Lookup the template
        CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
        if (pTemplate == nullptr)
            EX_ERROR(_T("Could not retrieve template named: ") + sName);

        // Create new listitem
        CXMLNode::PropertyMap mapProperties(mapParams);
        mapProperties[_T("value")] = sValue;
        CWidgetStyle style(m_pInterface, mapProperties);
        style.SetProperty(_T("width"), GetBaseListItemWidth());
        style.SetProperty(_T("height"), GetBaseListItemHeight());
        style.SetProperty(_T("sortindex"), sSort);
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, m_pListBox, style));

        CWidgetStyle styleInstance(m_pInterface, mapParams);
        pTemplate->Instantiate(pNewListItem, styleInstance);
        AddListItem(pNewListItem, true);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CMenu::CreateNewListItemFromTemplate() - "), NO_THROW);
    }
}


/*====================
  CMenu::ResizeListTemplate
  ====================*/
void    CMenu::ResizeListTemplate(const tstring &sName, uint uiSize, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        if (m_pListBox->GetNumListitems() < uiSize)
        {
            // Lookup the template
            CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
            if (pTemplate == nullptr)
                EX_ERROR(_T("Could not retrieve template named: ") + sName);

            // Add items
            while (m_pListBox->GetNumListitems() < uiSize)
            {
                // Create new listitem
                CXMLNode::PropertyMap mapProperties(mapParams);
                mapProperties[_T("value")] = XtoA(m_pListBox->GetNumListitems());
                CWidgetStyle style(m_pInterface, mapProperties);
                style.SetProperty(_T("width"), GetBaseListItemWidth());
                style.SetProperty(_T("height"), GetBaseListItemHeight());
                CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, m_pListBox, style));

                CWidgetStyle styleInstance(m_pInterface, mapParams);
                styleInstance.SetProperty(_T("index"), int(m_pListBox->GetNumListitems()));
                pTemplate->Instantiate(pNewListItem, styleInstance);
                AddListItem(pNewListItem);
            }
        }
        else if (m_pListBox->GetNumListitems() > uiSize)
        {
            // Delete items from back
            while (m_pListBox->GetNumListitems() > uiSize)
                m_pListBox->RemoveListItem(m_pListBox->GetItem(m_pListBox->GetNumListitems() - 1));
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CMenu::ResizeListTemplate() - "), NO_THROW);
    }
}


/*====================
  CMenu::RemoveListItem
  ====================*/
void    CMenu::RemoveListItem(CListItem *pListItem)
{
    m_pListBox->RemoveListItem(pListItem);
}


/*====================
  CMenu::ClearList
  ====================*/
void    CMenu::ClearList()
{
    m_pListBox->ClearList();
}


/*====================
  CMenu::SortList
  ====================*/
void    CMenu::SortList()
{
    m_pListBox->SortList();
}


/*====================
  CMenu::GetNumListitems
  ====================*/
uint    CMenu::GetNumListitems() const
{
    return m_pListBox->GetNumListitems();
}


/*====================
  CMenu::GetBaseListItemWidth
  ====================*/
const tstring&  CMenu::GetBaseListItemWidth() const
{
    return m_pListBox->GetBaseListItemWidth();
}


/*====================
  CMenu::GetBaseListItemHeight
  ====================*/
const tstring&  CMenu::GetBaseListItemHeight() const
{
    return m_pListBox->GetBaseListItemHeight();
}


/*====================
  CMenu::GetListItemWidth
  ====================*/
float   CMenu::GetListItemWidth() const
{
    return m_pListBox->GetListItemWidth();
}


/*====================
  CMenu::GetListItemHeight
  ====================*/
float   CMenu::GetListItemHeight() const
{
    return m_pListBox->GetListItemHeight();
}


/*====================
  CMenu::GetListItemValue
  ====================*/
tstring CMenu::GetListItemValue(uint uItem)
{
    return m_pListBox->GetListItemValue(uItem);
}


/*====================
  CMenu::SetListItemText
  ====================*/
void    CMenu::SetListItemText(uint uItem, tstring sValue)
{
    return m_pListBox->SetListItemText(uItem, sValue);
}


/*====================
  CMenu::Render
  ====================*/
void    CMenu::Render(const CVec2f &vOrigin, int iFlag, float fFade)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return;

    IListWidget::Render(vOrigin, iFlag, fFade);
}


/*====================
  CMenu::Frame
  ====================*/
void    CMenu::Frame(uint uiFrameLength, bool bProcessFrame)
{
    //UnsetFlags(WFLAG_RENDER_TOP);

    if (!HasFlags(WFLAG_ENABLED))
        return;

    //if (m_pListBox->HasFlags(WFLAG_VISIBLE))
    //  SetFlags(WFLAG_RENDER_TOP);

    CRectf recAbsArea(m_pListBox->GetRect());
    recAbsArea.MoveTo(GetAbsolutePos());

    CRectf recArea(m_pListBox->GetRect());
    recArea.MoveTo(V2_ZERO);

    if (recAbsArea.right >= Draw2D.GetScreenW())
        recArea.ShiftX(-recArea.GetWidth());
    if (recAbsArea.bottom >= Draw2D.GetScreenH())
        recArea.ShiftY(-recArea.GetHeight());
        
    m_pListBox->SetRect(recArea);

    DO_EVENT(WEVENT_FRAME)

    // Recursively call children frame functions
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        (*it)->Frame(uiFrameLength, bProcessFrame);
}


/*====================
  CMenu::SelectItem
  ====================*/
void    CMenu::SelectItem(CListItem *pListItem, bool bEvent)
{
    if (bEvent && pListItem != nullptr)
        pListItem->DoEvent(WEVENT_SELECT);

    m_bSingleClick = false;
    m_pListBox->Hide();
    SetFocus(false);
    m_pInterface->SetExclusiveWidget(nullptr);
    UIManager.RefreshCursor();

    if (pListItem != nullptr && pListItem->GetCommand())
        return;

    if (bEvent)
        DO_EVENT_PARAM(WEVENT_SELECT, pListItem != nullptr ? pListItem->GetValue() : TSNULL)
}


/*====================
  CMenu::SetSelectedItem
  ====================*/
void    CMenu::SetSelectedItem(int iSelectedItem, bool bEvent)
{
}


/*====================
  CMenu::SetSelectedItem
  ====================*/
void    CMenu::SetSelectedItem(const tstring &sValue, bool bEvent)
{
}


/*====================
  CMenu::AllocateWidgetState
  ====================*/
CWidgetState*   CMenu::AllocateWidgetState(const CWidgetStyle &style)
{
    const tstring &sName(style.GetProperty(_T("statename")));
    if (TStringCompare(sName, _T("listbg")) == 0)
        return K2_NEW(ctx_Widgets,  CWidgetState)(m_pInterface, m_pListBox, style);

    if (TStringCompare(sName, _T("itembg")) == 0 ||
        TStringCompare(sName, _T("itemhighlight")) == 0)
        return K2_NEW(ctx_Widgets,  CListWidgetState)(m_pInterface, m_pListBox, style);

    return K2_NEW(ctx_Widgets,  CWidgetState)(m_pInterface, this, style);
}


/*====================
  CMenu::AddWidgetState
  ====================*/
bool    CMenu::AddWidgetState(CWidgetState *pState)
{
    const tstring &sStateName(pState->GetStateName());
    if (TStringCompare(sStateName, _T("listbg")) == 0 || 
        TStringCompare(sStateName, _T("itemhighlight")) == 0 ||
        TStringCompare(sStateName, _T("itembg")) == 0)
    {
        return m_pListBox->AddWidgetState(pState);
    }

    // Delete this widget state if we don't end up using it
    SAFE_DELETE(pState);
    return false;
}


/*====================
  CMenu::WidgetLost
  ====================*/
void    CMenu::WidgetLost(IWidget *pWidget)
{
    if (pWidget == nullptr)
        return;

    if (m_pListBox == pWidget)
        m_pListBox = nullptr;

    IWidget::WidgetLost(pWidget);
}


/*====================
  CMenu::RecalculateChildSize
  ====================*/
void    CMenu::RecalculateChildSize()
{
    m_fListPaddingY = GetSizeFromString(m_sListPaddingY, GetParentHeight(), GetParentWidth());

    IListWidget::RecalculateChildSize();

    if (m_iMaxListHeight == -1)
    {
        m_pListBox->SetBaseHeight(XtoA(m_pListBox->GetNumListitems() * GetListItemHeight() + m_fListPaddingY));
        m_pListBox->RecalculateSize();
    }
    else
    {
        m_pListBox->SetBaseHeight(XtoA(MIN(int(m_pListBox->GetNumListitems()), m_iMaxListHeight) * GetListItemHeight() + m_fListPaddingY));
        m_pListBox->RecalculateSize();
    }
}


/*====================
  CMenu::UpdateMenuConditions
  ====================*/
void    CMenu::UpdateMenuConditions()
{
    m_pListBox->UpdateConditions();
    RecalculateChildSize();
}


/*====================
  CMenu::GetItem
  ====================*/
CListItem*  CMenu::GetItem(uint uiItem)
{
    return m_pListBox->GetItem(uiItem);
}


/*====================
  CMenu::GetItemByValue
  ====================*/
CListItem*  CMenu::GetItemByValue(const tstring &sValue)
{
    return m_pListBox->GetItemByValue(sValue);
}


/*====================
  CMenu::GetItemIndex
  ====================*/
uint    CMenu::GetItemIndex(CListItem *pItem)
{
    return m_pListBox->GetItemIndex(pItem);
}


/*--------------------
  MenuCmd
  --------------------*/
UI_VOID_CMD(MenuCmd, 1)
{
    if (pThis == nullptr ||
        pThis->GetType() != WIDGET_MENU)
        return;

    CMenu *pMenu(static_cast<CMenu *>(pThis));

    tstring sInput(vArgList[0]->Evaluate());

    if (sInput.empty())
        return;

    tsvector vTokens(TokenizeString(sInput, _T(' ')));

    if (vTokens.empty())
        return;

    tstring sCommand(vTokens[0]);

    vTokens.erase(vTokens.begin());
    sInput = ConcatinateArgs(vTokens);

    if (CompareNoCase(sCommand, _T("clear")) == 0)
    {
        pMenu->SetSelectedItem(-1, false);
        pMenu->ClearList();
    }
    else if (CompareNoCase(sCommand, _T("sort")) == 0)
    {
        pMenu->SortList();
    }
    else if (CompareNoCase(sCommand, _T("select")) == 0)
    {
        if (vArgList.size() > 1)
            pMenu->SetSelectedItem(AtoI(sInput), true);
    }
    else if (CompareNoCase(sCommand, _T("selectlast")) == 0)
    {
        pMenu->SetSelectedItem(pMenu->GetNumListitems() - 1, true);
    }
    else if (CompareNoCase(sCommand, _T("selectnone")) == 0)
    {
        pMenu->SetSelectedItem(-1, true);
    }
    //This command will search the list for a specified
    //value and change it's color to vTokenArgs[1]
    else if (CompareNoCase(sCommand, _T("setcolor")) == 0)
    {
        if (vTokens.size() >= 2)
        {
            for (uint uiLoop(0); uiLoop < pMenu->GetNumListitems(); uiLoop++)
            {
                tstring sValue(vTokens[0]);
                if (pMenu->GetListItemValue(uiLoop) == sValue)
                    pMenu->SetListItemText(uiLoop, vTokens[1] + StripColorCodes(sValue));
            }
        }
    }
#if 0
    else if (CompareNoCase(sCommand, _T("add")) == 0)
    {
        CXMLNode node;
        node.SetProperty(_T("content"), sInput);
        node.SetProperty(_T("textalign"), pMenu->GetItemAlign());
        node.SetProperty(_T("font"), pMenu->GetItemFont());
        node.SetProperty(_T("color"), pMenu->GetTextColor());

        node.SetProperty(_T("name"), StripColorCodes(sInput) + _T("_") + pMenu->GetName() + _T("_") + XtoA(pMenu->GetNumListitems()));

        node.SetProperty(_T("value"), StripColorCodes(sInput));

        node.SetProperty(_T("width"), pMenu->GetListItemWidth());
        node.SetProperty(_T("height"), pMenu->GetListItemHeight());

        CWidgetStyle style(pThis->GetInterface(), node);

        // Create new listitem
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(pThis->GetInterface(), pMenu, style));

        style.SetProperty(_T("name"), style.GetProperty(_T("name")) + _T("_label"));
        style.SetProperty(_T("width"), _T("100%"));
        style.SetProperty(_T("height"), _T("100%"));

        // Fill new listitem
        CLabel *pNewLabel(K2_NEW(ctx_Widgets,  CLabel)(pThis->GetInterface(), pNewListItem, style));
        pNewLabel->SetText(sInput);
        pNewListItem->AddChild(pNewLabel);
        
        pMenu->AddListItem(pNewListItem);
    }
    else if (CompareNoCase(sCommand, _T("remove")) == 0)
    {
        pMenu->RemoveListItemByName(sInput);
    }
#endif
}


/*--------------------
  OpenMenu
  --------------------*/
UI_VOID_CMD(OpenMenu, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_MENU)
        return;

    static_cast<CMenu *>(pThis)->Open(vArgList.size() > 0 ? AtoB(vArgList[0]->Evaluate()) : false);
}


/*--------------------
  CloseMenu
  --------------------*/
UI_VOID_CMD(CloseMenu, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_MENU)
        return;

    static_cast<CMenu *>(pThis)->Close();
}


/*--------------------
  UpdateMenuConditions
  --------------------*/
UI_VOID_CMD(UpdateMenuConditions, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_MENU)
        return;

    static_cast<CMenu *>(pThis)->UpdateMenuConditions();
}

