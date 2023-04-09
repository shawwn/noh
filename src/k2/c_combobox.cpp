// (C)2006 S2 Games
// c_combobox.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_combobox.h"
#include "c_interface.h"
#include "c_combobox_listbox.h"
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
  CComboBox::CComboBox
  ====================*/
CComboBox::~CComboBox()
{
    SAFE_DELETE(m_pListBox);

    for (uint uiState(0); uiState < NUM_COMBOBOXSTATES; ++uiState)
        SAFE_DELETE(m_apWidgetStates[uiState]);
}


/*====================
  CComboBox::CComboBox
  ====================*/
CComboBox::CComboBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
IListWidget(pInterface, pParent, WIDGET_COMBOBOX, style),
m_pActiveListItem(NULL),
m_pListBox(NULL),
m_eWidgetState(COMBOBOX_UP),
m_bSingleClick(false),
m_bLeft(false),
m_iMaxListHeight(style.GetPropertyInt(_T("maxlistheight"), -1)),
m_sCvar(style.GetProperty(_T("cvar")))
{
    if (style.GetPropertyBool(_T("interactive"), true))
        SetFlags(WFLAG_INTERACTIVE);

    for (int i(0); i < NUM_COMBOBOXSTATES; ++i)
        m_apWidgetStates[i] = NULL;

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
    styleCopy.SetProperty(_T("x"), _T("0"));
    styleCopy.SetProperty(_T("y"), _T("100%"));
    styleCopy.SetProperty(_T("width"), style.GetProperty(_T("listwidth"), style.GetProperty(_T("itemwidth"), _T("100%"))));
    styleCopy.SetProperty(_T("itemwidth"),style.GetProperty(_T("itemwidth"), _T("100%")));
    styleCopy.SetProperty(_T("texture"), style.GetProperty(_T("listtexture")));
    styleCopy.SetProperty(_T("color"), style.GetProperty(_T("listcolor"), style.GetProperty(_T("color"))));
    styleCopy.SetProperty(_T("noclick"), style.GetPropertyBool(_T("noclick"), false));
    styleCopy.SetProperty(_T("select"), false);

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
    m_pListBox = K2_NEW(ctx_Widgets,  CComboBoxListBox)(m_pInterface, this, styleCopy);
    AddChild(m_pListBox);

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CComboBox::ButtonDown
  ====================*/
bool    CComboBox::ButtonDown(EButton button)
{
    switch (button)
    {
    case BUTTON_ESC:
        if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        {
            m_pInterface->SetActiveWidget(NULL);
            m_pInterface->SetExclusiveWidget(NULL);
            g_pUIManager->RefreshCursor();
            m_pListBox->Hide();
            m_bSingleClick = false;
            if (m_recArea.AltContains(Input.GetCursorPos() - (m_pParent != NULL ? m_pParent->GetAbsolutePos() : V2_ZERO)))
                m_eWidgetState = COMBOBOX_OVER;
            else
            {
                DoEvent(WEVENT_CLOSE);          
                m_eWidgetState = COMBOBOX_UP;
            }
            return true;
        }
        break;

    case BUTTON_ENTER:
        if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        {
            m_pInterface->SetActiveWidget(NULL);
            m_pInterface->SetExclusiveWidget(NULL);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
            SetActiveListItem(m_pListBox->GetSelectedListItem(), true);
            m_bSingleClick = false;
            if (m_recArea.AltContains(Input.GetCursorPos() - (m_pParent != NULL ? m_pParent->GetAbsolutePos() : V2_ZERO)))
                m_eWidgetState = COMBOBOX_OVER;
            else
            {
                m_eWidgetState = COMBOBOX_UP;
                DoEvent(WEVENT_CLOSE);
            }
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
  CComboBox::MouseDown
  ====================*/
void    CComboBox::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button != BUTTON_MOUSEL)
    {
        if (!m_pListBox->GetRect().AltContains(v2CursorPos - m_recArea.lt()))
        {
            m_pInterface->SetActiveWidget(NULL);
            m_pInterface->SetExclusiveWidget(NULL);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
            m_eWidgetState = COMBOBOX_UP;
            DoEvent(WEVENT_CLOSE);
        }
        else if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        {
            m_pListBox->MouseDown(button, v2CursorPos);
        }
        
        return;
    }

    if (!m_bSingleClick)
    {
        if (!m_pListBox->GetRect().AltContains(v2CursorPos - m_recArea.lt()) && !m_recArea.AltContains(v2CursorPos))
        {
            m_pInterface->SetActiveWidget(NULL);
            m_pInterface->SetExclusiveWidget(NULL);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
            m_eWidgetState = COMBOBOX_UP;
            DoEvent(WEVENT_CLOSE);
        }
        else if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        {
            m_pListBox->SetMidClick(true);
        }
        else if (m_pListBox->GetNumListitems() > 0)
        {
            m_pInterface->SetExclusiveWidget(this);
            m_pListBox->Show();
            m_pListBox->SetMidClick(true);
            m_bSingleClick = true;
            m_eWidgetState = COMBOBOX_DOWN;
            m_bLeft = false;
            m_pListBox->SetSelectedItem(m_pActiveListItem, false);
            DoEvent(WEVENT_OPEN);
        }
    }
    else
    {
        // Is this even possible?
        if (m_recArea.AltContains(v2CursorPos))
        {
            m_pInterface->SetActiveWidget(NULL);
            m_pInterface->SetExclusiveWidget(NULL);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
            m_eWidgetState = COMBOBOX_UP;
            DoEvent(WEVENT_CLOSE);
        }

        m_bSingleClick = false;
    }
}


/*====================
  CComboBox::MouseUp
  ====================*/
void    CComboBox::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    if (button != BUTTON_MOUSEL)
    {
        if (m_pListBox->HasFlags(WFLAG_VISIBLE))
            m_pListBox->MouseUp(button, v2CursorPos);
        
        return;
    }

    if (m_bSingleClick)
    {
        if (m_recArea.AltContains(v2CursorPos))
        {
            if (m_bLeft)
            {
                m_pInterface->SetActiveWidget(NULL);
                m_pInterface->SetExclusiveWidget(NULL);
                UIManager.RefreshCursor();
                m_pListBox->Hide();
                m_pListBox->SetSelectedItem(-1, true);
                m_eWidgetState = COMBOBOX_OVER;
            }
            
            m_bSingleClick = false;
        }
        else
        {
            m_pInterface->SetActiveWidget(NULL);
            m_pInterface->SetExclusiveWidget(NULL);
            UIManager.RefreshCursor();
            m_pListBox->Hide();
            m_pListBox->SetSelectedItem(-1, true);
            m_bSingleClick = false;
            m_eWidgetState = COMBOBOX_UP;
            DoEvent(WEVENT_CLOSE);
        }
    }
    else if (m_pListBox->HasFlags(WFLAG_VISIBLE) && m_recArea.AltContains(v2CursorPos))
    {
        m_pInterface->SetActiveWidget(NULL);
        m_pInterface->SetExclusiveWidget(NULL);
        UIManager.RefreshCursor();
        m_pListBox->Hide();
        m_bSingleClick = false;
        m_eWidgetState = COMBOBOX_OVER;
    }
    else if (m_pListBox->HasFlags(WFLAG_VISIBLE) && !m_pListBox->Contains(v2CursorPos - m_recArea.lt()))
    {
        m_pInterface->SetActiveWidget(NULL);
        m_pInterface->SetExclusiveWidget(NULL);
        UIManager.RefreshCursor();
        m_pListBox->Hide();
        m_bSingleClick = false;
        m_eWidgetState = COMBOBOX_UP;
        DoEvent(WEVENT_CLOSE);
    }

    m_pListBox->SetMidClick(false);
}


/*====================
  CComboBox::ProcessInputCursor
  ====================*/
bool    CComboBox::ProcessInputCursor(const CVec2f &v2CursorPos)
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
  CComboBox::Enable
  ====================*/
void    CComboBox::Enable()
{
    if (!HasFlags(WFLAG_ENABLED))
        m_eWidgetState = COMBOBOX_UP;

    IWidget::Enable();
    
}


/*====================
  CComboBox::Disable
  ====================*/
void    CComboBox::Disable()
{
    if (HasFlags(WFLAG_ENABLED))
        m_eWidgetState = COMBOBOX_DISABLED;

    IWidget::Disable(); 
}


/*====================
  CComboBox::Rollover
  ====================*/
void    CComboBox::Rollover()
{
    IWidget::Rollover();

    if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        m_eWidgetState = COMBOBOX_DOWN;
    else
        m_eWidgetState = COMBOBOX_OVER;
}


/*====================
  CComboBox::Rolloff
  ====================*/
void    CComboBox::Rolloff()
{
    IWidget::Rolloff();

    if (m_pListBox->HasFlags(WFLAG_VISIBLE))
        m_eWidgetState = COMBOBOX_DOWN;
    else
    {
        m_eWidgetState = COMBOBOX_UP;
        DoEvent(WEVENT_CLOSE);

    }

    m_bLeft = true;
}


/*====================
  CComboBox::GetValue
  ====================*/
tstring CComboBox::GetValue() const
{
    if (m_pActiveListItem)
        return m_pActiveListItem->GetValue();
    else
        return _T("");
}


/*====================
  CComboBox::GetSelectedListItem
  ====================*/
CListItem* CComboBox::GetSelectedListItem() const
{
    if (m_pActiveListItem)
        return m_pActiveListItem;
    else
        return NULL;
}


/*====================
  CComboBox::AddListItem
  ====================*/
void    CComboBox::AddListItem(CListItem *pListItem, const bool bReverseSort)
{
    if (!bReverseSort)
        m_pListBox->AddListItem(pListItem);
    else
        m_pListBox->AddListItem(pListItem, true);

    if (m_iMaxListHeight == -1)
    {
        m_pListBox->SetBaseHeight(XtoA(m_pListBox->GetNumListitems() * GetListItemHeight()));
        m_pListBox->RecalculateSize();
    }
    else
    {
        m_pListBox->SetBaseHeight(XtoA(MIN(int(m_pListBox->GetNumListitems()), m_iMaxListHeight) * GetListItemHeight()));
        m_pListBox->RecalculateSize();
    }
}


/*====================
  CComboBox::CreateNewListItemFromTemplate
  ====================*/
void    CComboBox::CreateNewListItemFromTemplate(const tstring &sName, const tstring &sValue, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        // Lookup the template
        CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
        if (pTemplate == NULL)
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
        ex.Process(_T("CComboBox::CreateNewListItemFromTemplate() - "), NO_THROW);
    }
}


/*====================
  CComboBox::CreateNewListItemFromTemplateWithSort
  ====================*/
void    CComboBox::CreateNewListItemFromTemplateWithSort(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        // Lookup the template
        CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
        if (pTemplate == NULL)
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
        ex.Process(_T("CComboBox::CreateNewListItemFromTemplateWithSort() - "), NO_THROW);
    }
}


/*====================
  CComboBox::CreateNewListItemFromTemplateWithSortReversed
  ====================*/
void    CComboBox::CreateNewListItemFromTemplateWithSortReversed(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        // Lookup the template
        CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
        if (pTemplate == NULL)
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
        ex.Process(_T("CComboBox::CreateNewListItemFromTemplateWithSortReversed() - "), NO_THROW);
    }
}


/*====================
  CComboBox::ResizeListTemplate
  ====================*/
void    CComboBox::ResizeListTemplate(const tstring &sName, uint uiSize, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        if (m_pListBox->GetNumListitems() < uiSize)
        {
            // Lookup the template
            CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
            if (pTemplate == NULL)
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
        ex.Process(_T("CComboBox::ResizeListTemplate() - "), NO_THROW);
    }
}


/*====================
  CComboBox::RemoveListItem
  ====================*/
void    CComboBox::RemoveListItem(CListItem *pListItem)
{
    m_pListBox->RemoveListItem(pListItem);
}


/*====================
  CComboBox::ClearList
  ====================*/
void    CComboBox::ClearList()
{
    m_pListBox->ClearList();
    m_pActiveListItem = NULL;
}


/*====================
  CComboBox::SortList
  ====================*/
void    CComboBox::SortList()
{
    m_pListBox->SortList();
}


/*====================
  CComboBox::GetNumListitems
  ====================*/
uint    CComboBox::GetNumListitems() const
{
    return m_pListBox->GetNumListitems();
}


/*====================
  CComboBox::GetBaseListItemWidth
  ====================*/
const tstring&  CComboBox::GetBaseListItemWidth() const
{
    return m_pListBox->GetBaseListItemWidth();
}


/*====================
  CComboBox::GetBaseListItemHeight
  ====================*/
const tstring&  CComboBox::GetBaseListItemHeight() const
{
    return m_pListBox->GetBaseListItemHeight();
}


/*====================
  CComboBox::GetListItemWidth
  ====================*/
float   CComboBox::GetListItemWidth() const
{
    return m_pListBox->GetListItemWidth();
}


/*====================
  CComboBox::GetListItemHeight
  ====================*/
float   CComboBox::GetListItemHeight() const
{
    return m_pListBox->GetListItemHeight();
}


/*====================
  CComboBox::GetListItemValue
  ====================*/
tstring CComboBox::GetListItemValue(uint uItem)
{
    return m_pListBox->GetListItemValue(uItem);
}


/*====================
  CComboBox::SetListItemText
  ====================*/
void    CComboBox::SetListItemText(uint uItem, tstring sValue)
{
    return m_pListBox->SetListItemText(uItem, sValue);
}


/*====================
  CComboBox::Render
  ====================*/
void    CComboBox::Render(const CVec2f &vOrigin, int iFlag, float fFade)
{
    if (!HasFlags(WFLAG_VISIBLE))
        return;

    IListWidget::Render(vOrigin, iFlag, fFade);

    // Pick best state matching out desired state
    CWidgetState *pState(m_apWidgetStates[m_eWidgetState]);

    if (m_eWidgetState == COMBOBOX_UP)
    {
        if (!pState) pState = m_apWidgetStates[COMBOBOX_OVER];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_DOWN];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_DISABLED];
    }
    else if (m_eWidgetState == COMBOBOX_DOWN)
    {
        if (!pState) pState = m_apWidgetStates[COMBOBOX_OVER];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_UP];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_DISABLED];
    }
    else if (m_eWidgetState == COMBOBOX_OVER)
    {
        if (!pState) pState = m_apWidgetStates[COMBOBOX_UP];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_DOWN];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_DISABLED];
    }
    else if (m_eWidgetState == COMBOBOX_DISABLED)
    {
        if (!pState) pState = m_apWidgetStates[COMBOBOX_UP];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_OVER];
        if (!pState) pState = m_apWidgetStates[COMBOBOX_DOWN];
    }

    if (pState)
        pState->Render(vOrigin + m_recArea.lt(), iFlag, fFade * m_fFadeCurrent);

    if (m_pActiveListItem)
    {
        IWidget *pRealParent(m_pActiveListItem->GetParent());
        m_pActiveListItem->SetParent(this);
        m_pActiveListItem->SetDrawColors(false);
        m_pActiveListItem->Render(vOrigin + m_recArea.lt(), WIDGET_RENDER_ALL, fFade * m_fFadeCurrent, false, false, NULL, NULL);
        m_pActiveListItem->SetDrawColors(true);
        m_pActiveListItem->SetParent(pRealParent);
    }
}


/*====================
  CComboBox::Frame
  ====================*/
void    CComboBox::Frame(uint uiFrameLength, bool bProcessFrame)
{
    //UnsetFlags(WFLAG_RENDER_TOP);

    if (!HasFlags(WFLAG_ENABLED))
        return;

    if (!m_sCvar.empty() && !m_refCvar.IsValid())
    {
        m_refCvar.Assign(m_sCvar);
        
        if (m_refCvar.IsValid() && !m_refCvar.IsIgnored())
            SetSelectedItem(m_refCvar.GetString(), true);
    }

    if (!m_refCvar.IsIgnored() && m_refCvar.GetString() != GetValue())
        SetSelectedItem(m_refCvar.GetString(), true);

    //if (m_pListBox->HasFlags(WFLAG_VISIBLE))
    //  SetFlags(WFLAG_RENDER_TOP);

    DO_EVENT(WEVENT_FRAME)

    // Recursively call children frame functions
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        (*it)->Frame(uiFrameLength, bProcessFrame);
}


/*====================
  CComboBox::SetActiveListItem
  ====================*/
void    CComboBox::SetActiveListItem(CListItem *pListItem, bool bEvent)
{
    if (bEvent && pListItem != NULL)
        pListItem->DoEvent(WEVENT_SELECT);

    m_bSingleClick = false;
    m_pListBox->Hide();
    m_pInterface->SetActiveWidget(NULL);
    m_pInterface->SetExclusiveWidget(NULL);
    UIManager.RefreshCursor();
    m_eWidgetState = COMBOBOX_UP;
    DoEvent(WEVENT_CLOSE);

    if (pListItem != NULL && pListItem->GetCommand())
        return;

    if (m_pActiveListItem != pListItem)
    {
        m_pActiveListItem = pListItem;
        UpdateCvar();
    }

    if (bEvent)
        DO_EVENT(WEVENT_SELECT)
}


/*====================
  CComboBox::SetSelectedItem
  ====================*/
void    CComboBox::SetSelectedItem(int iSelectedItem, bool bEvent)
{
    if (m_pActiveListItem != m_pListBox->GetItem(iSelectedItem))
    {
        m_pListBox->SetSelectedItem(iSelectedItem, bEvent);
        m_pListBox->SetHoverListItem(iSelectedItem);

        SetActiveListItem(m_pListBox->GetSelectedListItem(), bEvent);
    }
}


/*====================
  CComboBox::SetSelectedItem
  ====================*/
void    CComboBox::SetSelectedItem(const tstring &sValue, bool bEvent)
{
    if (m_pActiveListItem != m_pListBox->GetItemByValue(sValue))
    {
        m_pListBox->SetSelectedItem(sValue, bEvent);
        m_pListBox->SetHoverListItem(m_pListBox->GetItemIndex(m_pListBox->GetSelectedListItem()));

        SetActiveListItem(m_pListBox->GetSelectedListItem(), bEvent);
    }
}


/*====================
  CComboBox::UpdateCvar
  ====================*/
void    CComboBox::UpdateCvar()
{
    if (!m_refCvar.IsIgnored() && m_refCvar.GetString() != GetValue())
        m_refCvar.Set(GetValue());
}


/*====================
  CComboBox::AllocateWidgetState
  ====================*/
CWidgetState*   CComboBox::AllocateWidgetState(const CWidgetStyle &style)
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
  CComboBox::AddWidgetState
  ====================*/
bool    CComboBox::AddWidgetState(CWidgetState *pState)
{
    const tstring &sStateName(pState->GetStateName());
    if (TStringCompare(sStateName, _T("listbg")) == 0 || 
        TStringCompare(sStateName, _T("itemhighlight")) == 0 ||
        TStringCompare(sStateName, _T("itembg")) == 0)
    {
        return m_pListBox->AddWidgetState(pState);
    }

    for (uint uiState(0); uiState < NUM_COMBOBOXSTATES; ++uiState)
    {
        if (pState->GetStateName() == g_asComboBoxStateNames[uiState])
        {
            m_apWidgetStates[uiState] = pState;
            return true;
        }
    }

    // Delete this widget state if we don't end up using it
    SAFE_DELETE(pState);
    return false;
}


/*====================
  CComboBox::WidgetLost
  ====================*/
void    CComboBox::WidgetLost(IWidget *pWidget)
{
    if (pWidget == NULL)
        return;

    if (m_pActiveListItem == pWidget)
        m_pActiveListItem = NULL;

    if (m_pListBox == pWidget)
        m_pListBox = NULL;

    IWidget::WidgetLost(pWidget);
}


/*====================
  CComboBox::RecalculateChildSize
  ====================*/
void    CComboBox::RecalculateChildSize()
{
    for (uint uiState(0); uiState < NUM_COMBOBOXSTATES; ++uiState)
    {
        if (m_apWidgetStates[uiState] != NULL)
            m_apWidgetStates[uiState]->RecalculateSize();
    }

    IListWidget::RecalculateChildSize();

    if (m_iMaxListHeight == -1)
    {
        m_pListBox->SetBaseHeight(XtoA(m_pListBox->GetNumListitems() * GetListItemHeight()));
        m_pListBox->RecalculateSize();
    }
    else
    {
        m_pListBox->SetBaseHeight(XtoA(MIN(int(m_pListBox->GetNumListitems()), m_iMaxListHeight) * GetListItemHeight()));
        m_pListBox->RecalculateSize();
    }
}


/*====================
  CComboBox::GetItem
  ====================*/
CListItem*  CComboBox::GetItem(uint uiItem)
{
    return m_pListBox->GetItem(uiItem);
}


/*====================
  CComboBox::GetItemByValue
  ====================*/
CListItem*  CComboBox::GetItemByValue(const tstring &sValue)
{
    return m_pListBox->GetItemByValue(sValue);
}


/*====================
  CComboBox::GetItemIndex
  ====================*/
uint    CComboBox::GetItemIndex(CListItem *pItem)
{
    return m_pListBox->GetItemIndex(pItem);
}


/*--------------------
  ComboBoxCmd
  --------------------*/
UI_VOID_CMD(ComboBoxCmd, 1)
{
    if (pThis == NULL ||
        pThis->GetType() != WIDGET_COMBOBOX)
        return;

    CComboBox *pComboBox(static_cast<CComboBox *>(pThis));

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
        pComboBox->SetSelectedItem(-1, false);
        pComboBox->ClearList();
    }
    else if (CompareNoCase(sCommand, _T("sort")) == 0)
    {
        pComboBox->SortList();
    }
    else if (CompareNoCase(sCommand, _T("select")) == 0)
    {
        if (vArgList.size() > 1)
            pComboBox->SetSelectedItem(AtoI(sInput), true);
    }
    else if (CompareNoCase(sCommand, _T("selectlast")) == 0)
    {
        pComboBox->SetSelectedItem(pComboBox->GetNumListitems() - 1, true);
    }
    else if (CompareNoCase(sCommand, _T("selectnone")) == 0)
    {
        pComboBox->SetSelectedItem(-1, true);
    }
    //This command will search the list for a specified
    //value and change it's color to vTokenArgs[1]
    else if (CompareNoCase(sCommand, _T("setcolor")) == 0)
    {
        if (vTokens.size() >= 2)
        {
            for (uint uiLoop(0); uiLoop < pComboBox->GetNumListitems(); uiLoop++)
            {
                tstring sValue(vTokens[0]);
                if (pComboBox->GetListItemValue(uiLoop) == sValue)
                    pComboBox->SetListItemText(uiLoop, vTokens[1] + StripColorCodes(sValue));
            }
        }
    }
#if 0
    else if (CompareNoCase(sCommand, _T("add")) == 0)
    {
        CXMLNode node;
        node.SetProperty(_T("content"), sInput);
        node.SetProperty(_T("textalign"), pComboBox->GetItemAlign());
        node.SetProperty(_T("font"), pComboBox->GetItemFont());
        node.SetProperty(_T("color"), pComboBox->GetTextColor());

        node.SetProperty(_T("name"), StripColorCodes(sInput) + _T("_") + pComboBox->GetName() + _T("_") + XtoA(pComboBox->GetNumListitems()));

        node.SetProperty(_T("value"), StripColorCodes(sInput));

        node.SetProperty(_T("width"), pComboBox->GetListItemWidth());
        node.SetProperty(_T("height"), pComboBox->GetListItemHeight());

        CWidgetStyle style(pThis->GetInterface(), node);

        // Create new listitem
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(pThis->GetInterface(), pComboBox, style));

        style.SetProperty(_T("name"), style.GetProperty(_T("name")) + _T("_label"));
        style.SetProperty(_T("width"), _T("100%"));
        style.SetProperty(_T("height"), _T("100%"));

        // Fill new listitem
        CLabel *pNewLabel(K2_NEW(ctx_Widgets,  CLabel)(pThis->GetInterface(), pNewListItem, style));
        pNewLabel->SetText(sInput);
        pNewListItem->AddChild(pNewLabel);
        
        pComboBox->AddListItem(pNewListItem);
    }
    else if (CompareNoCase(sCommand, _T("remove")) == 0)
    {
        pComboBox->RemoveListItemByName(sInput);
    }
#endif
}