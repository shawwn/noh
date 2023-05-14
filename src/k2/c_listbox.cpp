// (C)2005 S2 Games
// c_listbox.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_listbox.h"

#include "c_listitem.h"
#include "c_listbox_scrollbar.h"
#include "c_interface.h"
#include "c_uiscript.h"
#include "c_uimanager.h"
#include "c_widgetstyle.h"
#include "c_uicmd.h"
#include "c_xmlnode.h"
#include "c_label.h"
#include "c_widgettemplate.h"
#include "c_widgetstate.h"
#include "c_combobox.h"

#include "../k2/c_input.h"
#include "../k2/xtoa.h"
#include "../k2/c_system.h"
#include "../k2/c_draw2d.h"
//=============================================================================

/*====================
  CompareByItemText
  ====================*/
static bool CompareByItemText(const CListItem *elem1, const CListItem *elem2)
{
    int iCompareResult;

    iCompareResult = CompareNoCase(elem1->GetText(), elem2->GetText());

    return (iCompareResult == -1 ? true : false);
}


/*====================
  CompareByItemValue
  ====================*/
static bool CompareByItemValue(const CListItem *elem1, const CListItem *elem2)
{
    int iCompareResult;

    iCompareResult = CompareNoCase(elem1->GetValue(), elem2->GetValue());

    return (iCompareResult == -1 ? true : false);
}


/*====================
  CompareByItemNumericValue
  ====================*/
static bool CompareByItemNumericValue(const CListItem *elem1, const CListItem *elem2)
{
    return AtoF(elem1->GetValue()) < AtoF(elem2->GetValue());
}


/*====================
  CompareByItemSortIndex
  ====================*/
static bool CompareByItemSortIndex(const CListItem *elem1, const CListItem *elem2)
{
    int iCompareResult;

    iCompareResult = CompareNoCase(elem1->GetSortIndex(), elem2->GetSortIndex());

    return (iCompareResult == -1 ? true : false);
}


/*====================
  CListBox::~CListBox
  ====================*/
CListBox::~CListBox()
{
    // Delete all of it it's items
    ListItemVector vItems(m_vItems);
    for (ListItemVector_it it(vItems.begin()); it != vItems.end(); ++it)
        SAFE_DELETE(*it);
    m_vItems.clear();

    ListItemVector vHiddenItems(m_vHiddenItems);
    for (ListItemVector_it it(vHiddenItems.begin()); it != vHiddenItems.end(); ++it)
        SAFE_DELETE(*it);
    m_vHiddenItems.clear();

    SAFE_DELETE(m_pHScrollbar);
    SAFE_DELETE(m_pVScrollbar);

    SAFE_DELETE(m_pBackground);
    SAFE_DELETE(m_pItemBackground);
    SAFE_DELETE(m_pItemHighlight);
}


/*====================
  CListBox::CListBox
  ====================*/
CListBox::CListBox(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style) :
IListWidget(pInterface, pParent, WIDGET_LISTBOX, style),
m_fHPadding(0.0f),
m_fVPadding(0.0f),
m_fScrollbarSize(GetSizeFromString(style.GetProperty(_T("scrollbarsize"), _T("16")), pParent->GetWidth(), pParent->GetHeight())),
m_iListDrawStart(0),
m_iSelectedListItem(-1),
m_iHoverListItem(-1),
m_sListItemImage(style.GetProperty(_T("backgroundimage"))),
m_v4ItemImageColor(GetColorFromString(style.GetProperty(_T("backgroundimagecolor"), _T("white")))),
m_v4BackgroundFill(GetColorFromString(style.GetProperty(_T("backgroundcolor"), _T("invisible")))),
m_v4BackgroundBorder(GetColorFromString(style.GetProperty(_T("backgroundbordercolor"), _T("invisible")))),
m_v4HighlightFill(GetColorFromString(style.GetProperty(_T("highlightcolor"), _T("#4CB2FF33")))),
m_v4HighlightBorder(GetColorFromString(style.GetProperty(_T("highlightbordercolor"), _T("white")))),
m_v4SelectedFill(GetColorFromString(style.GetProperty(_T("selectedcolor"), style.GetProperty(_T("highlightcolor"), _T("#4CB2FF33"))))),
m_v4SelectedBorder(GetColorFromString(style.GetProperty(_T("selectedbordercolor"), style.GetProperty(_T("highlightbordercolor"), _T("white"))))),
m_bColorTransitions(style.GetPropertyBool(_T("colortransition"), false)),
m_uiColorTransitionTime(style.GetPropertyInt(_T("colortransitiontime"), INVALID_TIME)),
m_bAutoSize(true),
m_bHoverSelect(style.GetPropertyBool(_T("hoverselect"), true)),
m_bHoverHighlight(style.GetPropertyBool(_T("hoverhighlight"), false)),
m_bSelect(style.GetPropertyBool(_T("select"), true)),
m_bHighlightOver(true),
m_bExteriorScrollbars(style.GetPropertyBool(_T("exteriorscrollbars"), false)),
m_bScrollbarPlaceholder(style.GetPropertyBool(_T("scrollbarplaceholder"), false)),
m_uiDoubleClickTime(style.GetPropertyInt(_T("doubleclicktime"), 400)),
m_eLastClickType(BUTTON_INVALID),
m_uiLastClickTime(0),
m_iLastClickItem(-1),
m_pHScrollbar(nullptr),
m_pVScrollbar(nullptr),
m_refCvar(style.GetProperty(_T("cvar"))),
m_pHoverWidget(nullptr),
m_pBackground(nullptr),
m_pItemBackground(nullptr),
m_pItemHighlight(nullptr),
m_bClearSelection(style.GetPropertyBool(_T("clearselection"), false)),
m_bUseScrollbars(style.GetPropertyBool(_T("usescrollbars"), true)),
m_fListItemWidthFudge(GetSizeFromString(style.GetProperty(_T("itemwidthfudge")), pParent->GetWidth(), pParent->GetHeight())),
m_fListItemHeightFudge(GetSizeFromString(style.GetProperty(_T("itemheightfudge")), pParent->GetHeight(), pParent->GetWidth())),
m_iLastListItem(-1),
m_bMouseOver(false),
m_bThisTimeOutOfArea(true)
{
    if (m_bSelect)
    {
        if (style.GetPropertyBool(_T("interactive"), true))
            SetFlags(WFLAG_INTERACTIVE);
        
        SetFlagsRecursive(WFLAG_PROCESS_CURSOR);
    }

    const tstring &sHighlight(style.GetProperty(_T("highlight")));
    if (sHighlight == _T("over") || !style.HasProperty(_T("highlight")))
        m_bHighlightOver = true;
    else if (sHighlight == _T("under"))
        m_bHighlightOver = false;
    else
        Console.Warn << SingleQuoteStr(sHighlight) << " - Invalid highlight mode (over|under)" << newl;

    // Listitem offset
    m_sListItemOffsetX = style.GetProperty(_T("itemoffsetx"), _T("0"));
    m_sListItemOffsetY = style.GetProperty(_T("itemoffsety"), _T("0"));
    m_fListItemOffsetX = GetPositionFromString(m_sListItemOffsetX, GetWidth(), GetHeight());
    m_fListItemOffsetY = GetPositionFromString(m_sListItemOffsetY, GetHeight(), GetWidth());

    // Listitem size
    m_sListItemWidth = style.GetProperty(_T("itemwidth"), _T("100%"));
    m_sListItemHeight = style.GetProperty(_T("itemheight"), _T("100%"));
    m_fListItemWidth = GetSizeFromString(m_sListItemWidth, GetWidth(), GetHeight());
    m_fListItemHeight = GetSizeFromString(m_sListItemHeight, GetHeight(), GetWidth());

    if (m_bUseScrollbars)
    {
        float fScrollbarOffset(GetPositionFromString(style.GetProperty(_T("scrollbaroffset"), _T("0")), GetWidth(), GetHeight()));

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
        styleCopy.RemoveProperty(_T("form"));
        styleCopy.RemoveProperty(_T("data"));
        styleCopy.RemoveProperty(_T("watch"));
        styleCopy.RemoveProperty(_T("ontrigger"));

        for (int i(0); i < 10; ++i)
        {
            styleCopy.RemoveProperty(_T("watch") + XtoA(i));
            styleCopy.RemoveProperty(_T("ontrigger") + XtoA(i));
        }

        styleCopy.RemoveProperty(_T("align"));
        styleCopy.SetProperty(_T("valign"), _T("bottom"));

        styleCopy.SetProperty(_T("color"), _T("#ffffff"));
        styleCopy.SetProperty(_T("bgcolor"), _T("#ffffff"));
        styleCopy.SetProperty(_T("handlecolor"), _T("#ffffff"));
        styleCopy.SetProperty(_T("slotcolor"), _T("#ffffff"));

        styleCopy.SetProperty(_T("texture"), style.GetProperty(_T("scrolltexture"), _T("/ui/elements/standardscroll.tga")));

        //
        // Horizontal
        //

        styleCopy.SetProperty(_T("x"), 0.0f);
        styleCopy.SetProperty(_T("y"), (m_bExteriorScrollbars ? XtoA(m_fScrollbarSize + fScrollbarOffset) : XtoA(fScrollbarOffset)));
        styleCopy.SetProperty(_T("width"), _T("100%"));
        styleCopy.SetProperty(_T("height"), m_fScrollbarSize);

        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), this->GetName() + _T("_hscroll"));

        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
            styleCopy.SetProperty(_T("visible"), m_bScrollbarPlaceholder);
        else
            styleCopy.SetProperty(_T("visible"), false);

        m_pHScrollbar = K2_NEW(ctx_Widgets,  CListBoxScrollbar)(m_pInterface, this, LISTBOX_SCROLLBAR_HORIZONTAL, styleCopy);
        m_pHScrollbar->SetMaxValue(0.000001f);
        AddChild(m_pHScrollbar);

        //
        // Vertical
        //

        styleCopy.SetProperty(_T("x"), (m_bExteriorScrollbars ? XtoA(m_fScrollbarSize + fScrollbarOffset) : XtoA(fScrollbarOffset)));
        styleCopy.SetProperty(_T("y"), 0.0f);
        styleCopy.SetProperty(_T("width"), m_fScrollbarSize);
        styleCopy.SetProperty(_T("height"), _T("100%"));
        styleCopy.SetProperty(_T("vertical"), true);

        styleCopy.RemoveProperty(_T("valign"));
        styleCopy.SetProperty(_T("align"), _T("right"));

        if (!GetName().empty())
            styleCopy.SetProperty(_T("name"), GetName() + _T("_vscroll"));

        if (m_eWrapMode == LISTBOX_WRAP_ROW)
            styleCopy.SetProperty(_T("visible"), m_bScrollbarPlaceholder);
        else
            styleCopy.SetProperty(_T("visible"), false);

        m_pVScrollbar = K2_NEW(ctx_Widgets,  CListBoxScrollbar)(m_pInterface, this, LISTBOX_SCROLLBAR_VERTICAL, styleCopy);
        m_pVScrollbar->SetMaxValue(0.000001f);
        AddChild(m_pVScrollbar);
    }

    if (IsAbsoluteVisible())
        DO_EVENT(WEVENT_SHOW)
}


/*====================
  CListBox::ButtonUp
  ====================*/
bool    CListBox::ButtonUp(EButton button)
{
    return true;
}


/*====================
  CListBox::ButtonDown
  ====================*/
bool    CListBox::ButtonDown(EButton button)
{
    const int iNumRows(INT_FLOOR(m_recArea.GetHeight() / m_fListItemHeight));

    switch (button)
    {
    case BUTTON_BACKSPACE:
        break;

    case BUTTON_DEL:
        break;

    case BUTTON_ESC:
        SetSelectedListItem(-1);
        m_pInterface->SetActiveWidget(nullptr);
        break;

    case BUTTON_HOME:
        if (m_iSelectedListItem != 0)
        {
            SetSelectedListItem(0);
            if (m_bSelect)
                DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)
        }

        m_iListDrawStart = 0;
        m_pVScrollbar->SetValue(float(m_iListDrawStart));
        break;

    case BUTTON_END:
        if (m_iSelectedListItem != static_cast<int>(m_vItems.size()) - 1)
        {
            SetSelectedListItem(INT_SIZE(m_vItems.size()) - 1);

            if (m_bSelect)
                DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)
        }

        m_iListDrawStart = m_iSelectedListItem - iNumRows + 1;
        m_pVScrollbar->SetValue(float(m_iListDrawStart));
        break;

    case BUTTON_LEFT:
        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            int iOldItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (m_iSelectedListItem >= iNumRows)
            {
                SetSelectedListItem(m_iSelectedListItem - iNumRows);

                if (!m_vItems[m_iSelectedListItem]->GetSelect())
                    continue;

                bSuccess = true;

                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)

                break;
            }

            if (!bSuccess)
                SetSelectedListItem(iOldItem);
            else if ((m_iSelectedListItem - m_iListDrawStart) < 0)
                m_iListDrawStart -= iNumRows;
        }
        else if (m_eWrapMode == LISTBOX_WRAP_ROW)
        {
            int iNewItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (iNewItem != 0)
            {
                --iNewItem;

                if (!m_vItems[iNewItem]->GetSelect())
                    continue;

                bSuccess = true;

                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(iNewItem), true)

                break;
            }

            if (bSuccess)
            {
                SetSelectedListItem(iNewItem);
                
                if ((m_iSelectedListItem - m_iListDrawStart) < 0)
                {
                    --m_iListDrawStart;
                    m_pVScrollbar->SetValue(float(m_iListDrawStart));
                }
            }
        }
        break;

    case BUTTON_RIGHT:
        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            int iOldItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (m_iSelectedListItem + iNumRows < static_cast<int>(m_vItems.size()))
            {
                SetSelectedListItem(m_iSelectedListItem + iNumRows);

                if (!m_vItems[m_iSelectedListItem]->GetSelect())
                    continue;

                bSuccess = true;
                
                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)

                break;
            }

            if (!bSuccess)
                SetSelectedListItem(iOldItem);
            else if (((m_iSelectedListItem - m_iListDrawStart) / (iNumRows)) * m_fListItemWidth >= m_recArea.GetWidth())
                m_iListDrawStart += iNumRows;
        }
        else if (m_eWrapMode == LISTBOX_WRAP_ROW)
        {
            int iNewItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (iNewItem != static_cast<int>(m_vItems.size()) - 1)
            {
                ++iNewItem;

                if (!m_vItems[iNewItem]->GetSelect())
                    continue;

                bSuccess = true;
                
                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(iNewItem), true)

                break;
            }

            if (bSuccess)
            {
                SetSelectedListItem(iNewItem);

                if (((m_iSelectedListItem - m_iListDrawStart) / (iNumRows)) * m_fListItemWidth >= m_recArea.GetWidth())
                    m_iListDrawStart += iNumRows;
            }
        }
        break;

    case BUTTON_PGUP:
        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            if (m_iSelectedListItem >= iNumRows)
            {
                SetSelectedListItem(m_iSelectedListItem - iNumRows);

                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)
            }

            if ((m_iSelectedListItem - m_iListDrawStart) < 0)
                m_iListDrawStart -= iNumRows;
        }
        else
        {
            if (m_iSelectedListItem - m_iListDrawStart != 0)
            {
                SetSelectedListItem(m_iListDrawStart);
                
                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)
            }
            else
            {
                if (m_iSelectedListItem != 0)
                {
                    SetSelectedListItem(MAX(0, m_iSelectedListItem - iNumRows));
                    
                    if (m_bSelect)
                        DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)
                }

                if (m_iListDrawStart != 0)
                {
                    m_iListDrawStart = m_iSelectedListItem;
                    m_pVScrollbar->SetValue(float(m_iListDrawStart));
                }
            }
        }
        break;

    case BUTTON_PGDN:
        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            if (m_iSelectedListItem + iNumRows < static_cast<int>(m_vItems.size()))
            {
                SetSelectedListItem(m_iSelectedListItem + iNumRows);
                
                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)
            }

            if (((m_iSelectedListItem - m_iListDrawStart) / iNumRows) * m_fListItemWidth >= m_recArea.GetWidth())
                m_iListDrawStart += iNumRows;
        }
        else
        {
            if (m_iSelectedListItem - m_iListDrawStart != iNumRows - 1)
            {
                SetSelectedListItem(m_iListDrawStart + iNumRows - 1);
                
                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)
            }
            else
            {
                m_iListDrawStart = MIN(static_cast<int>(m_vItems.size()) - iNumRows, m_iListDrawStart + iNumRows);

                if (m_iSelectedListItem != m_iListDrawStart + iNumRows - 1)
                {
                    SetSelectedListItem(m_iListDrawStart + iNumRows - 1);

                    if (m_bSelect)
                        DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(m_iSelectedListItem), true)

                    m_pVScrollbar->SetValue(float(m_iListDrawStart));
                }
            }
        }
        break;

    case BUTTON_UP:
        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            int iNewItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (iNewItem != 0)
            {
                if (iNewItem == -1)
                    iNewItem = m_iListDrawStart;
                else
                    --iNewItem;

                if (!m_vItems[iNewItem]->GetSelect())
                    continue;

                bSuccess = true;

                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(iNewItem), true)

                break;
            }

            if (bSuccess)
            {
                SetSelectedListItem(iNewItem);

                if ((m_iSelectedListItem - m_iListDrawStart) < 0)
                    m_iListDrawStart -= iNumRows;
            }
        }
        else
        {
            int iNewItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (iNewItem != 0)
            {
                if (iNewItem == -1)
                    iNewItem = m_iListDrawStart;
                else
                    --iNewItem;

                if (!m_vItems[iNewItem]->GetSelect())
                    continue;

                bSuccess = true;

                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(iNewItem), true)

                break;
            }

            if (bSuccess)
            {
                SetSelectedListItem(iNewItem);

                if ((m_iSelectedListItem - m_iListDrawStart) < 0)
                {
                    m_iListDrawStart--;
                    m_pVScrollbar->SetValue(float(m_iListDrawStart));
                }
            }
        }
        break;

    case BUTTON_DOWN:
        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            int iNewItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (iNewItem != static_cast<int>(m_vItems.size()) - 1)
            {
                if (iNewItem == -1)
                    iNewItem = m_iListDrawStart;
                else
                    ++iNewItem;

                if (!m_vItems[iNewItem]->GetSelect())
                    continue;

                bSuccess = true;

                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(iNewItem), true)

                break;
            }

            if (bSuccess)
            {
                SetSelectedListItem(iNewItem);

                if (((m_iSelectedListItem - m_iListDrawStart) / (iNumRows)) * m_fListItemWidth >= m_recArea.GetWidth())
                    m_iListDrawStart += iNumRows;
            }
        }
        else
        {
            int iNewItem(m_iSelectedListItem);
            bool bSuccess(false);

            while (iNewItem != static_cast<int>(m_vItems.size()) - 1)
            {
                if (iNewItem == -1)
                    iNewItem = m_iListDrawStart;
                else
                    ++iNewItem;

                if (!m_vItems[iNewItem]->GetSelect())
                    continue;

                bSuccess = true;

                if (m_bSelect)
                    DO_EVENT_PARAM_RETURN(WEVENT_SELECT, XtoA(iNewItem), true)

                break;
            }

            if (bSuccess)
            {
                SetSelectedListItem(iNewItem);
                
                if ((m_iSelectedListItem - m_iListDrawStart) * m_fListItemHeight >= m_recArea.GetHeight())
                {
                    m_iListDrawStart++;
                    m_pVScrollbar->SetValue(float(m_iListDrawStart));
                }
            }
        }
        break;

    case BUTTON_ENTER:
        break;

    default:
        break;
    }

    return true;
}


/*====================
  CListBox::Char
  ====================*/
bool    CListBox::Char(TCHAR c)
{
    return false;
}


/*====================
  CListBox::MouseDown
  ====================*/
void    CListBox::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
    if (button == BUTTON_WHEELUP && m_pVScrollbar != nullptr && m_pVScrollbar->HasFlags(WFLAG_VISIBLE))
        m_pVScrollbar->MinButtonCommand();
    else if (button == BUTTON_WHEELDOWN && m_pVScrollbar != nullptr && m_pVScrollbar->HasFlags(WFLAG_VISIBLE))
        m_pVScrollbar->MaxButtonCommand();
    if (button == BUTTON_WHEELUP || button == BUTTON_WHEELDOWN || !Contains(v2CursorPos))
        return;

    if (v2CursorPos.x - m_recArea.left < m_fListItemOffsetX || v2CursorPos.y - m_recArea.top < m_fListItemOffsetY)
        return;

    int iNewListItem(-1);
    if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
    {
        int iX = INT_FLOOR((v2CursorPos.x - m_recArea.left - m_fListItemOffsetX) / m_fListItemWidth);
        int iY = INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight);

        iNewListItem = iX * INT_FLOOR(m_recArea.GetHeight() / m_fListItemHeight) + iY + m_iListDrawStart;
    }
    else if (m_eWrapMode == LISTBOX_WRAP_ROW)
    {
        int iX = INT_FLOOR((v2CursorPos.x - m_recArea.left - m_fListItemOffsetX) / m_fListItemWidth);
        int iY = INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight);

        iNewListItem = iY * INT_FLOOR(m_recArea.GetWidth() / m_fListItemWidth) + iX + m_iListDrawStart;
    }
    else
        iNewListItem = INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight) + m_iListDrawStart;

    if (iNewListItem < 0 || iNewListItem >= (int)GetNumListitems())
        iNewListItem = -1;

    // Pass click down to list item (for fancy stuff inside list items)
    if (iNewListItem != -1)
    {
        CVec2f v2ItemCursorPos(v2CursorPos - m_recArea.lt() - CVec2f(m_fListItemOffsetX, m_fListItemOffsetY));
        v2ItemCursorPos.x = fmod(v2ItemCursorPos.x, m_fListItemWidth);
        v2ItemCursorPos.y = fmod(v2ItemCursorPos.y, m_fListItemHeight);

        m_vItems[iNewListItem]->ProcessInputMouseButton(v2ItemCursorPos, button, 1.0f);
    }

    if (!m_bSelect || iNewListItem == -1)
    {
#if 0
        if (m_iSelectedListItem == -1)
            SetFocus(false);
#endif

        return;
    }

    tsvector sParams;
    sParams.push_back(XtoA(v2CursorPos.x));
    sParams.push_back(XtoA(v2CursorPos.y));

    if ((m_iSelectedListItem != iNewListItem && iNewListItem != -1) || m_bHoverSelect)
    {
        if (m_vItems[iNewListItem]->GetSelect())
        {
            SetSelectedListItem(iNewListItem);
            DO_EVENT_PARAM(WEVENT_SELECT, XtoA(m_iSelectedListItem))

            if (button == BUTTON_MOUSEL &&
                m_eLastClickType == BUTTON_MOUSEL &&
                Host.GetTime() - m_uiLastClickTime < m_uiDoubleClickTime &&
                m_iLastClickItem == iNewListItem)
                DO_EVENT_PARAM(WEVENT_DOUBLECLICK, sParams)
            else if (button == BUTTON_MOUSEL)
                DO_EVENT_PARAM(WEVENT_CLICK, sParams)
            else if (button == BUTTON_MOUSER)
                DO_EVENT_PARAM(WEVENT_RIGHTCLICK, sParams)
        }
        else
            SetSelectedListItem(-1);
    }
    else if (m_iSelectedListItem == iNewListItem)
    {
        if (button == BUTTON_MOUSEL &&
            m_eLastClickType == BUTTON_MOUSEL &&
            Host.GetTime() - m_uiLastClickTime < m_uiDoubleClickTime &&
            m_iLastClickItem == iNewListItem)
            DO_EVENT_PARAM(WEVENT_DOUBLECLICK, sParams)
        else if (button == BUTTON_MOUSER)
            DO_EVENT_PARAM(WEVENT_RIGHTCLICK, sParams)
        else
            DO_EVENT_PARAM(WEVENT_CLICK, sParams)
    }

    m_eLastClickType = button;
    m_uiLastClickTime = Host.GetTime();
    m_iLastClickItem = iNewListItem;
}


/*====================
  CListBox::MouseUp
  ====================*/
void    CListBox::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
    int iNewListItem(-1);
    if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
    {
        int iX = INT_FLOOR((v2CursorPos.x - m_recArea.left - m_fListItemOffsetX) / m_fListItemWidth);
        int iY = INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight);

        iNewListItem = iX * INT_FLOOR(m_recArea.GetHeight() / m_fListItemHeight) + iY + m_iListDrawStart;
    }
    else if (m_eWrapMode == LISTBOX_WRAP_ROW)
    {
        int iX(INT_FLOOR((v2CursorPos.x - m_recArea.left - m_fListItemOffsetX) / m_fListItemWidth));
        int iY(INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight));

        iNewListItem = iY * INT_FLOOR(m_recArea.GetWidth() / m_fListItemWidth) + iX + m_iListDrawStart;
    }
    else
        iNewListItem = INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight) + m_iListDrawStart;

    if (iNewListItem < 0 || iNewListItem >= (int)GetNumListitems())
        iNewListItem = -1;

    // Pass click down to list item (for fancy stuff inside list items)
    if (iNewListItem != -1)
    {
        CVec2f v2ItemCursorPos(v2CursorPos - m_recArea.lt() - CVec2f(m_fListItemOffsetX, m_fListItemOffsetY));
        v2ItemCursorPos.x = fmod(v2ItemCursorPos.x, m_fListItemWidth);
        v2ItemCursorPos.y = fmod(v2ItemCursorPos.y, m_fListItemHeight);

        m_vItems[iNewListItem]->ProcessInputMouseButton(v2ItemCursorPos, button, 0.0f);
    }
}


/*====================
  CListBox::Render
  ====================*/
void    CListBox::Render(const CVec2f &v2Origin, int iFlag, float fFade)
{
    UpdateScrollbars();

    if (!HasFlags(WFLAG_VISIBLE))
        return;

    IListWidget::Render(v2Origin, iFlag, fFade);

    if (m_pBackground)
        m_pBackground->Render(v2Origin + m_recArea.lt(), iFlag, fFade * m_fFadeCurrent);

    CVec2f v2LocalOrigin(v2Origin + m_recArea.lt() + CVec2f(m_fListItemOffsetX, m_fListItemOffsetY));
    CVec2f v2OffsetX(m_fListItemWidth + m_fVPadding, 0);
    CVec2f v2OffsetY(0, m_fListItemHeight + m_fHPadding);
    int iNumRows = INT_FLOOR((m_recArea.GetHeight() + m_fListItemHeightFudge) / m_fListItemHeight);
    int iNumColumns = INT_FLOOR((m_recArea.GetWidth() + m_fListItemWidthFudge) / m_fListItemWidth);

    if (iNumRows == 0 || iNumColumns == 0)
        return;

    int i = 0;
    vector<CListItem *>::iterator it(m_vItems.begin());

    for (; it != m_vItems.end() && i != m_iListDrawStart; ++it, ++i) {}

    i = 0;
    for (; it != m_vItems.end(); ++it, ++i)
    {
        int iIndex(int(it - m_vItems.begin()));

        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            if (INT_CEIL((i + 1) / float(iNumRows)) * m_fListItemWidth >= (m_recArea.GetWidth() + m_fListItemWidthFudge))
                break;
        }
        else if (m_eWrapMode == LISTBOX_WRAP_ROW)
        {
            if (INT_CEIL((i + 1) / float(iNumColumns)) * m_fListItemHeight > (m_recArea.GetHeight() + m_fListItemHeightFudge))
                break;
        }
        else
        {
            if ((i + 1) * m_fListItemHeight > (m_recArea.GetHeight() + m_fListItemHeightFudge))
                break;
        }

        CRectf rect((*it)->GetRect());

        if (m_eWrapMode == LISTBOX_WRAP_ROW)
            rect.MoveTo(v2LocalOrigin + v2OffsetY * static_cast<float>(i / iNumColumns) + v2OffsetX * static_cast<float>(i % iNumColumns));
        else
            rect.MoveTo(v2LocalOrigin + v2OffsetY * static_cast<float>(i % iNumRows) + v2OffsetX * static_cast<float>(i / iNumRows));

        (*it)->Render(rect.lt(), iFlag, fFade * m_fFadeCurrent, m_iHoverListItem == iIndex, m_iSelectedListItem == iIndex, m_pItemBackground, m_pItemHighlight);
    }

    // Render children
    for (WidgetPointerVector_cit it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
        (*it)->Render(v2Origin + m_recArea.lt(), iFlag, fFade * m_fFadeCurrent);
}


/*====================
  CListBox::ProcessInputCursor
  ====================*/
bool    CListBox::ProcessInputCursor(const CVec2f &v2CursorPos)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return false;

    int iNewListItem(-1);
    bool bLastTimeOutOfArea(m_bThisTimeOutOfArea);

    if (m_recArea.AltContains(v2CursorPos))
    {
        m_bThisTimeOutOfArea = false;
        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            int iX(INT_FLOOR((v2CursorPos.x - m_recArea.left - m_fListItemOffsetX) / m_fListItemWidth));
            int iY(INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight));

            iNewListItem = iX * INT_FLOOR(m_recArea.GetHeight() / m_fListItemHeight) + iY + m_iListDrawStart;
        }
        else if (m_eWrapMode == LISTBOX_WRAP_ROW)
        {
            int iX(INT_FLOOR((v2CursorPos.x - m_recArea.left - m_fListItemOffsetX) / m_fListItemWidth));
            int iY(INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight));

            iNewListItem = iY * INT_FLOOR(m_recArea.GetWidth() / m_fListItemWidth) + iX + m_iListDrawStart;
        }
        else
            iNewListItem = INT_FLOOR((v2CursorPos.y - m_recArea.top - m_fListItemOffsetY) / m_fListItemHeight) + m_iListDrawStart;

        if (iNewListItem < 0 || iNewListItem >= (int)GetNumListitems())
            iNewListItem = -1;
    }
    else
        m_bThisTimeOutOfArea = true;


    if (v2CursorPos.x - m_recArea.left < m_fListItemOffsetX || v2CursorPos.y - m_recArea.top < m_fListItemOffsetY)
        iNewListItem = -1;

    if((iNewListItem != m_iLastListItem || m_bThisTimeOutOfArea == true) && m_bMouseOver == true)
    {
            m_bMouseOver = false;
            IWidget::DoEvent(WEVENT_MOUSEOUT);
    }

    // Pass mouse down to list item (for fancy stuff inside list items)
    if (iNewListItem != -1)
    {
        CVec2f v2ItemCursorPos(v2CursorPos - m_recArea.lt() - CVec2f(m_fListItemOffsetX, m_fListItemOffsetY));
        v2ItemCursorPos.x = fmod(v2ItemCursorPos.x, m_fListItemWidth);
        v2ItemCursorPos.y = fmod(v2ItemCursorPos.y, m_fListItemHeight);

        // Check for rollover/rolloff and send the call the appropriate functions
        if (m_pInterface->GetHoverWidget() == this)
        {
            IWidget *pWidget(m_vItems[iNewListItem]->GetWidget(v2ItemCursorPos, true));
            SetHoverWidget(pWidget);
        }
        else
        {
            SetHoverWidget(nullptr);
        }

        SetHoverListItem(iNewListItem);
        m_vItems[iNewListItem]->ProcessInputCursor(v2ItemCursorPos);
    }
    else
    {
        SetHoverListItem(-1);
        SetHoverWidget(nullptr);
    }

    // Let children handle the input directly
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
    {
        if ((*it)->HasFlags(WFLAG_PROCESS_CURSOR) && (*it)->ProcessInputCursor(v2CursorPos - m_recArea.lt()))
            return true;
    }

    // Hover selection
    if (m_bHoverSelect && (m_bClearSelection || iNewListItem != -1))
    {
        if (iNewListItem == -1 || m_vItems[iNewListItem]->GetSelect())
            SetSelectedListItem(iNewListItem);
        else if (m_bClearSelection)
            SetSelectedListItem(-1);
    }

    if(((iNewListItem != m_iLastListItem && m_bThisTimeOutOfArea == false) || (bLastTimeOutOfArea == true && m_bThisTimeOutOfArea == false)) && m_bMouseOver == false)
    {
        m_iLastListItem = iNewListItem;
        IWidget::DoEvent(WEVENT_MOUSEOVER);
        m_bMouseOver = true;
    }

    return (iNewListItem != -1);
}


/*====================
  CListBox::GetValue
  ====================*/
tstring CListBox::GetValue() const
{
    if (m_iSelectedListItem < 0 || m_iSelectedListItem >= int(m_vItems.size()))
        return TSNULL;

    return m_vItems[m_iSelectedListItem]->GetValue();
}


/*====================
  CListBox::HorizontalScrollbarChange
  ====================*/
void    CListBox::HorizontalScrollbarChange(float fNewValue)
{
    if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        m_iListDrawStart = INT_FLOOR(INT_ROUND(fNewValue) * ((m_recArea.GetHeight() + m_fListItemHeightFudge) / m_fListItemHeight));
}


/*====================
  CListBox::VerticalScrollbarChange
  ====================*/
void    CListBox::VerticalScrollbarChange(float fNewValue)
{
    if (m_eWrapMode == LISTBOX_WRAP_ROW)
        m_iListDrawStart = INT_FLOOR(INT_ROUND(fNewValue) * ((m_recArea.GetWidth() + m_fListItemWidthFudge) / m_fListItemWidth));
    else if (m_eWrapMode == LISTBOX_WRAP_NONE)
        m_iListDrawStart = INT_ROUND(fNewValue);
}


/*====================
  CListBox::UpdateScrollbars
  ====================*/
void    CListBox::UpdateScrollbars()
{
    if (!m_bUseScrollbars)
        return;

    float fNumRows((m_recArea.GetHeight() + m_fListItemHeightFudge) / m_fListItemHeight);
    float fNumColumns((m_recArea.GetWidth() + m_fListItemWidthFudge) / m_fListItemWidth);

    if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
    {
        fNumRows = INT_FLOOR(fNumRows);

        if (INT_SIZE(m_vItems.size()) > fNumRows * fNumColumns)
        {
            if (!m_pHScrollbar->HasFlags(WFLAG_VISIBLE))
                m_pHScrollbar->Show();
            if (!m_pHScrollbar->IsEnabled())
                m_pHScrollbar->Enable();
        }
        else
        {
            if (m_pHScrollbar->HasFlags(WFLAG_VISIBLE) && !m_bScrollbarPlaceholder)
                m_pHScrollbar->Hide();
            if (m_pHScrollbar->IsEnabled())
                m_pHScrollbar->Disable();

            if (m_bScrollbarPlaceholder)
            {
                m_pHScrollbar->SetMaxValue(0.000001f);
                m_pHScrollbar->Show();
            }

        }

        if (fNumRows - fNumColumns > 0)
        {
            if (INT_SIZE(m_vItems.size()) / fNumRows - fNumColumns > 0)
            {
                float fNumTotalColumns(INT_SIZE(m_vItems.size()) / fNumRows);

                m_pHScrollbar->SetMaxValue(INT_CEIL(fNumTotalColumns - fNumColumns));
                //m_pHScrollbar->SetHandleSize(float(iNumColumns) / iNumTotalColumns * 100.0f);
            }
        }
        else
        {
            m_pHScrollbar->SetMaxValue(0.000001f);
        }
    }
    else if (m_eWrapMode == LISTBOX_WRAP_ROW)
    {
        fNumColumns = INT_FLOOR(fNumColumns);

        if (INT_SIZE(m_vItems.size()) > fNumRows * fNumColumns)
        {
            if (!m_pVScrollbar->HasFlags(WFLAG_VISIBLE))
                m_pVScrollbar->Show();
            if (!m_pVScrollbar->IsEnabled())
                m_pVScrollbar->Enable();
        }
        else
        {
            if (m_pVScrollbar->HasFlags(WFLAG_VISIBLE) && !m_bScrollbarPlaceholder)
                m_pVScrollbar->Hide();
            if (m_pVScrollbar->IsEnabled())
                m_pVScrollbar->Disable();

            if (m_bScrollbarPlaceholder)
            {
                m_pVScrollbar->SetMaxValue(0.000001f);
                m_pVScrollbar->Show();
            }

        }

        if (fNumColumns > 0)
        {
            if ((INT_SIZE(m_vItems.size()) / fNumColumns) > fNumRows)
            {
                float fNumTotalRows(INT_SIZE(m_vItems.size()) / fNumColumns);

                m_pVScrollbar->SetMaxValue(INT_CEIL(fNumTotalRows - fNumRows));
                //m_pVScrollbar->SetHandleSize(float(uiNumRows) / iNumTotalRows * 100.0f);
            }
        }
        else
            m_pVScrollbar->SetMaxValue(0.000001f);
    }
    else
    {
        fNumRows = INT_FLOOR(fNumRows);

        if (INT_SIZE(m_vItems.size()) > fNumRows)
        {
            if (!m_pVScrollbar->HasFlags(WFLAG_VISIBLE))
                m_pVScrollbar->Show();
            if (!m_pVScrollbar->IsEnabled())
                m_pVScrollbar->Enable();
        }
        else
        {
            if (m_pVScrollbar->HasFlags(WFLAG_VISIBLE) && !m_bScrollbarPlaceholder)
                m_pVScrollbar->Hide();
            if (m_pVScrollbar->IsEnabled())
                m_pVScrollbar->Disable();

            if (m_bScrollbarPlaceholder)
            {
                m_pVScrollbar->SetMaxValue(0.000001f);
                m_pVScrollbar->Show();
            }
        }

        if (INT_SIZE(m_vItems.size()) - fNumRows > 0.0f)
        {
            m_pVScrollbar->SetMaxValue(INT_SIZE(m_vItems.size()) - fNumRows);
            m_pVScrollbar->SetValue(MIN(m_pVScrollbar->GetValueFloat(), m_pVScrollbar->GetMaxValue()));
            //m_pVScrollbar->SetHandleSize(static_cast<float>(uiNumRows) / static_cast<int>(m_vItems.size()) * 100.0f);
        }
    }
}


/*====================
  CListBox::AddListItem
  ====================*/
void    CListBox::AddListItem(CListItem *pListItem, const bool bReverseSort)
{
    if (!bReverseSort)
        m_vItems.push_back(pListItem);
    else
        m_vItems.insert(m_vItems.begin(), pListItem);   

    // Update the list item back/fore colors to this listboxes params
    pListItem->SetTransitionTime(m_uiColorTransitionTime);

    pListItem->SetLastColor(m_v4BackgroundFill, m_v4BackgroundBorder);
    pListItem->SetNextColor(m_v4BackgroundFill, m_v4BackgroundBorder);
    
    pListItem->SetDrawColorUnder(!m_bHighlightOver);

    pListItem->SetUseBackgroundImage(!m_sListItemImage.empty());

    if (!m_sListItemImage.empty())
    {
        pListItem->SetTexture(m_sListItemImage);
        pListItem->SetColor(m_v4ItemImageColor);
    }

    UpdateScrollbars();
}


/*====================
  CListBox::RemoveListItem
  ====================*/
void    CListBox::RemoveListItem(CListItem *pListItem)
{
    if (pListItem == m_pHoverWidget)
        SetHoverWidget(nullptr);

    // Hide item immediately
    for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); )
    {
        if (*it != pListItem)
        {
            ++it;
            continue;
        }

        CListItem *vSelectedItem(GetSelectedListItem());
        CListItem *vHoverItem(GetHoverListItem());

        if (vSelectedItem == *it)
            vSelectedItem = nullptr;
        if (vHoverItem == *it)
            vHoverItem = nullptr;

        SetHoverListItem(-1);
        SetSelectedListItem(-1);

        m_vHiddenItems.push_back(*it);

        pListItem = m_vHiddenItems.back();

        it = m_vItems.erase(it);
        UpdateScrollbars();

        if (vSelectedItem != nullptr)
            SetSelectedItem(vSelectedItem, true);
        if (vHoverItem != nullptr)
            SetHoverItem(vHoverItem, true);

        break;
    }

    pListItem->RequestPurge();
    pListItem->Kill();
}


/*====================
  CListBox::ShowListItem
  ====================*/
void    CListBox::ShowListItem(const tstring &sValue)
{
    for (vector<CListItem*>::iterator it(m_vHiddenItems.begin()); it != m_vHiddenItems.end(); )
    {
        if ((*it)->GetValue() != sValue)
        {
            ++it;
            continue;
        }

        m_vItems.push_back(*it);
        it = m_vHiddenItems.erase(it);
        UpdateScrollbars();
        break;
    }
}


/*====================
  CListBox::HideListItem
  ====================*/
void    CListBox::HideListItem(const tstring &sValue)
{
    for (vector<CListItem*>::iterator it(m_vItems.begin()); it != m_vItems.end(); )
    {
        if ((*it)->GetValue() != sValue)
        {
            ++it;
            continue;
        }

        CListItem *vSelectedItem(GetSelectedListItem());
        CListItem *vHoverItem(GetHoverListItem());

        SetHoverListItem(-1);
        SetSelectedListItem(-1);

        if (vSelectedItem == *it)
            vSelectedItem = nullptr;
        if (vHoverItem == *it)
            vHoverItem = nullptr;

        m_vHiddenItems.push_back(*it);
        it = m_vItems.erase(it);
        UpdateScrollbars();

        if (vSelectedItem != nullptr)
            SetSelectedItem(vSelectedItem, true);
        if (vHoverItem != nullptr)
            SetHoverItem(vHoverItem, true);

        break;
    }
}


/*====================
  CListBox::RemoveListItemByName
  ====================*/
void    CListBox::RemoveListItemByName(const tstring &sName)
{
    for (vector<CListItem*>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it)
    {
        if ((*it)->GetName() != sName)
            continue;

        if (*it == m_pHoverWidget)
            SetHoverWidget(nullptr);

        (*it)->RequestPurge();
        (*it)->Kill();
        break;
    }
}


/*====================
  CListBox::CreateNewListItemFromTemplate
  ====================*/
void    CListBox::CreateNewListItemFromTemplate(const tstring &sName, const tstring &sValue, const CXMLNode::PropertyMap &mapParams)
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
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, this, style));

        CWidgetStyle styleInstance(m_pInterface, mapParams);
        pTemplate->Instantiate(pNewListItem, styleInstance);
        AddListItem(pNewListItem);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CListBox::CreateNewListItemFromTemplate() - "), NO_THROW);
    }
}


/*====================
  CListBox::CreateNewListItemFromTemplateWithSort
  ====================*/
void    CListBox::CreateNewListItemFromTemplateWithSort(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams)
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
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, this, style));

        CWidgetStyle styleInstance(m_pInterface, mapParams);
        pTemplate->Instantiate(pNewListItem, styleInstance);
        AddListItem(pNewListItem);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CListBox::CreateNewListItemFromTemplate() - "), NO_THROW);
    }
}


/*====================
  CListBox::CreateNewListItemFromTemplateWithSortReversed
  ====================*/
void    CListBox::CreateNewListItemFromTemplateWithSortReversed(const tstring &sName, const tstring &sValue, const tstring &sSort, const CXMLNode::PropertyMap &mapParams)
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
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, this, style));

        CWidgetStyle styleInstance(m_pInterface, mapParams);
        pTemplate->Instantiate(pNewListItem, styleInstance);
        AddListItem(pNewListItem, true);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CListBox::CreateNewListItemFromTemplate() - "), NO_THROW);
    }
}


/*====================
  CListBox::ResizeListTemplate
  ====================*/
void    CListBox::ResizeListTemplate(const tstring &sName, uint uiSize, const CXMLNode::PropertyMap &mapParams)
{
    try
    {
        if (GetNumListitems() < uiSize)
        {
            // Lookup the template
            CWidgetTemplate *pTemplate(m_pInterface->GetTemplate(sName));
            if (pTemplate == nullptr)
                EX_ERROR(_T("Could not retrieve template named: ") + sName);

            // Add items
            while (GetNumListitems() < uiSize)
            {
                // Create new listitem
                CXMLNode::PropertyMap mapProperties(mapParams);
                mapProperties[_T("value")] = XtoA(GetNumListitems());
                CWidgetStyle style(m_pInterface, mapProperties);
                style.SetProperty(_T("width"), GetBaseListItemWidth());
                style.SetProperty(_T("height"), GetBaseListItemHeight());
                
                // H4X
                if (style.HasProperty(_T("watch")))
                    style.SetProperty(_T("watch"), style.GetProperty(_T("watch")) + XtoA(GetNumListitems()));

                CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(m_pInterface, this, style));

                CWidgetStyle styleInstance(m_pInterface, mapParams);
                styleInstance.SetProperty(_T("index"), int(GetNumListitems()));
                pTemplate->Instantiate(pNewListItem, styleInstance);

                AddListItem(pNewListItem);
            }
        }
        else if (GetNumListitems() > uiSize)
        {
            for (int iDeleteNum(GetNumListitems() - uiSize); iDeleteNum > 0; --iDeleteNum)
                RemoveListItem(GetItem(uiSize + iDeleteNum - 1));
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CListBox::ResizeListTemplate() - "), NO_THROW);
    }
}


/*====================
  CListBox::Show
  ====================*/
void    CListBox::Show(uint uiDuration)
{
    IWidget::Show(uiDuration);

    // show scrollbars
    if (m_bUseScrollbars)
    {
        if (m_eWrapMode == LISTBOX_WRAP_ROW)
        {
            if (m_vItems.size() > (m_recArea.GetHeight() / m_fListItemHeight) * (m_recArea.GetWidth() / m_fListItemWidth))
                m_pVScrollbar->Show(uiDuration);
        }
        else if (m_eWrapMode == LISTBOX_WRAP_NONE)
        {
            if (m_vItems.size() > m_recArea.GetHeight() / m_fListItemHeight)
                m_pVScrollbar->Show(uiDuration);
        }


        if (m_eWrapMode == LISTBOX_WRAP_COLUMN)
        {
            if (m_vItems.size() > (m_recArea.GetHeight() / m_fListItemHeight) * (m_recArea.GetWidth() / m_fListItemWidth))
                m_pHScrollbar->Show(uiDuration);
        }
    }
}


/*====================
  CListBox::SortList
  ====================*/
void    CListBox::SortList()
{
    CListItem *vSelectedItem(GetSelectedListItem());
    CListItem *vHoverItem(GetHoverListItem());

    SetHoverListItem(-1);
    SetSelectedListItem(-1);

    // Sort, comparing by item value
    sort(m_vItems.begin(), m_vItems.end(), CompareByItemText);

    if (vSelectedItem != nullptr)
        SetSelectedItem(vSelectedItem, true);

    if (vHoverItem != nullptr)
        SetHoverItem(vHoverItem, true);
}


/*====================
  CListBox::SortListValue
  ====================*/
void    CListBox::SortListValue()
{
    CListItem *vSelectedItem(GetSelectedListItem());
    CListItem *vHoverItem(GetHoverListItem());

    SetHoverListItem(-1);
    SetSelectedListItem(-1);

    // Sort, comparing by item value
    sort(m_vItems.begin(), m_vItems.end(), CompareByItemValue);

    if (vSelectedItem != nullptr)
        SetSelectedItem(vSelectedItem, true);

    if (vHoverItem != nullptr)
        SetHoverItem(vHoverItem, true);
}


/*====================
  CListBox::SortListNumericValue
  ====================*/
void    CListBox::SortListNumericValue()
{
    CListItem *vSelectedItem(GetSelectedListItem());
    CListItem *vHoverItem(GetHoverListItem());

    SetHoverListItem(-1);
    SetSelectedListItem(-1);

    // Sort, comparing by item value
    sort(m_vItems.begin(), m_vItems.end(), CompareByItemNumericValue);

    if (vSelectedItem != nullptr)
        SetSelectedItem(vSelectedItem, true);

    if (vHoverItem != nullptr)
        SetHoverItem(vHoverItem, true);
}


// template parameters need external linkage!
struct FCompareSortIndex : public std::binary_function<const CListItem *, const CListItem *, bool>
{
    int     m_iIndex;

    FCompareSortIndex(int iIndex) :
    m_iIndex(iIndex)
    {
    }

    bool operator()(const CListItem *pLeft, const CListItem *pRight) const
    {
        if (m_iIndex < 0)
            return (CompareNoCase(pLeft->GetSortIndex(-m_iIndex - 1), pRight->GetSortIndex(-m_iIndex - 1)) == 1 ? true : false);
        else
            return (CompareNoCase(pLeft->GetSortIndex(m_iIndex), pRight->GetSortIndex(m_iIndex)) == -1 ? true : false);
    }
};

struct FCompareSortIndexNumeric : public std::binary_function<const CListItem *, const CListItem *, bool>
{
    int     m_iIndex;

    FCompareSortIndexNumeric(int iIndex) :
    m_iIndex(iIndex)
    {
    }

    bool operator()(const CListItem *pLeft, const CListItem *pRight) const
    {
        if (m_iIndex < 0)
            return AtoF(pLeft->GetSortIndex(-m_iIndex - 1)) > AtoF(pRight->GetSortIndex(-m_iIndex - 1));
        else
            return AtoF(pLeft->GetSortIndex(m_iIndex)) < AtoF(pRight->GetSortIndex(m_iIndex));
    }
};


/*====================
  CListBox::SortListSortIndex
  ====================*/
void    CListBox::SortListSortIndex(int iIndex)
{
    CListItem *vSelectedItem(GetSelectedListItem());
    CListItem *vHoverItem(GetHoverListItem());

    SetHoverListItem(-1);
    SetSelectedListItem(-1);

    // Sort, comparing by item value
    sort(m_vItems.begin(), m_vItems.end(), FCompareSortIndex(iIndex));

    if (vSelectedItem != nullptr)
        SetSelectedItem(vSelectedItem, true);

    if (vHoverItem != nullptr)
        SetHoverItem(vHoverItem, true);
}


/*====================
  CListBox::SortListSortIndexNumeric
  ====================*/
void    CListBox::SortListSortIndexNumeric(int iIndex)
{
    CListItem *vSelectedItem(GetSelectedListItem());
    CListItem *vHoverItem(GetHoverListItem());

    SetHoverListItem(-1);
    SetSelectedListItem(-1);

    // Sort, comparing by item value
    sort(m_vItems.begin(), m_vItems.end(), FCompareSortIndexNumeric(iIndex));

    if (vSelectedItem != nullptr)
        SetSelectedItem(vSelectedItem, true);

    if (vHoverItem != nullptr)
        SetHoverItem(vHoverItem, true);
}


/*====================
  CListBox::GetSelectedListItem
  ====================*/
CListItem*      CListBox::GetSelectedListItem()
{
    if (m_iSelectedListItem == -1 || m_iSelectedListItem >= int(m_vItems.size()))
        return nullptr;
    else
        return m_vItems[m_iSelectedListItem];
}


/*====================
  CListBox::GetHoverListItem
  ====================*/
CListItem*      CListBox::GetHoverListItem()
{
    if (m_iHoverListItem == -1 || m_iHoverListItem >= int(m_vItems.size()))
        return nullptr;
    else
        return m_vItems[m_iHoverListItem];
}


/*====================
  CListBox::GetListItemValue
  ====================*/
tstring         CListBox::GetListItemValue(uint uItem)
{
    if (m_vItems.size() > uItem)
        return m_vItems[uItem]->GetValue();

    return TSNULL;
}


/*====================
  CListBox::SetListItemText
  ====================*/
void    CListBox::SetListItemText(uint uItem, tstring sValue)
{
    if (m_vItems.size() > uItem)
        m_vItems[uItem]->SetText(sValue);
}


/*====================
  CListBox::SetSelectedItem
  ====================*/
void    CListBox::SetSelectedItem(int iSelectedItem, bool bEvent)
{
    SetSelectedListItem(iSelectedItem);
}


/*====================
  CListBox::SetSelectedItem
  ====================*/
void    CListBox::SetSelectedItem(const tstring &sValue, bool bEvent)
{
    SetSelectedListItem(-1);

    for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it)
    {
        if ((*it)->GetValue() == sValue)
        {
            SetSelectedItem(int(it - m_vItems.begin()), bEvent);
            return;
        }
    }

    SetSelectedItem(-1, bEvent);
}


/*====================
  CListBox::SetSelectedItem
  ====================*/
void    CListBox::SetSelectedItem(CListItem *pListItem, bool bEvent)
{
    for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it)
    {
        if (*it == pListItem)
        {
            SetSelectedItem(int(it - m_vItems.begin()), bEvent);
            return;
        }
    }

    SetSelectedItem(-1, bEvent);
}


/*====================
  CListBox::SetHoverItem
  ====================*/
void    CListBox::SetHoverItem(int iHoverItem, bool bEvent)
{
    SetHoverListItem(iHoverItem);
}


/*====================
  CListBox::SetHoverItem
  ====================*/
void    CListBox::SetHoverItem(CListItem *pListItem, bool bEvent)
{
    for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it)
    {
        if (*it == pListItem)
        {
            SetHoverItem(int(it - m_vItems.begin()), bEvent);
            return;
        }
    }

    SetHoverItem(-1, bEvent);
}


/*====================
  CListBox::GetItem
  ====================*/
CListItem*  CListBox::GetItem(uint uiItem)
{
    if (uiItem < m_vItems.size())
        return m_vItems[uiItem];
    else
        return nullptr;
}


/*====================
  CListBox::GetItemByValue
  ====================*/
CListItem*  CListBox::GetItemByValue(const tstring &sValue)
{
    for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it)
    {
        if ((*it)->GetValue() == sValue)
            return *it;
    }

    return nullptr;
}


/*====================
  CListBox::GetItemIndex
  ====================*/
uint    CListBox::GetItemIndex(CListItem *pItem)
{
    int iIndex(0);
    for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it, ++iIndex)
    {
        if ((*it) == pItem)
            return iIndex;
    }

    return -1;
}


/*====================
  CListBox::ClearList
  ====================*/
void    CListBox::ClearList()
{
    PROFILE("CListBox::ClearList");

    SetHoverWidget(nullptr);

    // Delete all of it it's items
    vector<CListItem *> vItems(m_vItems);
    for (vector<CListItem*>::iterator it(vItems.begin()); it != vItems.end(); ++it)
        SAFE_DELETE(*it);
    m_vItems.clear();
    vector<CListItem *> vHiddenItems(m_vHiddenItems);
    for (vector<CListItem*>::iterator it(vHiddenItems.begin()); it != vHiddenItems.end(); ++it)
        SAFE_DELETE(*it);
    m_vHiddenItems.clear();

    SetSelectedItem(-1, true);

    m_iListDrawStart = 0;
    UpdateScrollbars();
}


/*====================
  CListBox::UpdateCvar
  ====================*/
void    CListBox::UpdateCvar()
{
    if (!m_refCvar.IsIgnored() && m_refCvar.GetString() != GetValue())
        m_refCvar.Set(GetValue());
}


/*====================
  CListBox::DoEvent
  ====================*/
void    CListBox::DoEvent(EWidgetEvent eEvent, const tstring &sParam)
{
    IWidget::DoEvent(eEvent, sParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;
    
    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (WIDGET_EVENT_RECURSIVE[eEvent])
    {
        for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it)
            (*it)->DoEvent(eEvent, sParam);
    }
}


/*====================
  CListBox::DoEvent
  ====================*/
void    CListBox::DoEvent(EWidgetEvent eEvent, const tsvector &vParam)
{
    IWidget::DoEvent(eEvent, vParam);

    if (HasFlags(WFLAG_RELEASED | WFLAG_DEAD))
        return;
    
    // Don't execute show events if we're still hidden
    // Don't execute hide events if we were already hidden
    if ((eEvent == WEVENT_SHOW || eEvent == WEVENT_HIDE) && !IsAbsoluteVisible())
        return;

    if (WIDGET_EVENT_RECURSIVE[eEvent])
    {
        for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); ++it)
            (*it)->DoEvent(eEvent, vParam);
    }
}


/*====================
  CListBox::SetHoverWidget
  ====================*/
void    CListBox::SetHoverWidget(IWidget *pWidget)
{
    if (pWidget != m_pHoverWidget)
    {
        if (m_pHoverWidget && m_pHoverWidget->IsEnabled())
            m_pHoverWidget->Rolloff();

        m_pHoverWidget = pWidget;

        if (m_pHoverWidget && m_pHoverWidget->IsEnabled())
            m_pHoverWidget->Rollover();
    }
}


/*====================
  CListBox::WidgetLost
  ====================*/
void    CListBox::WidgetLost(IWidget *pWidget)
{
    if (pWidget == nullptr)
        return;

    if (m_pHoverWidget == pWidget)
        m_pHoverWidget = nullptr;

    vector<CListItem*>::iterator it(m_vItems.begin());

    CListItem *pHoverItem(nullptr);
    CListItem *pSelectedItem(nullptr);

    if (m_iHoverListItem != -1 && m_iHoverListItem < (int)m_vItems.size())
        pHoverItem = m_vItems[m_iHoverListItem];

    if (m_iSelectedListItem != -1 && m_iSelectedListItem < (int)m_vItems.size())
        pSelectedItem = m_vItems[m_iSelectedListItem];

    m_iHoverListItem = -1;
    m_iSelectedListItem = -1;

    while (it != m_vItems.end())
    {
        if (*it == pWidget)
        {
            it = m_vItems.erase(it);
            continue;
        }

        it++;
    }

    SetHoverListItem(GetItemIndex(pHoverItem));
    SetSelectedListItem(GetItemIndex(pSelectedItem));

    IWidget::WidgetLost(pWidget);
}


/*====================
  CListBox::AllocateWidgetState
  ====================*/
CWidgetState*   CListBox::AllocateWidgetState(const CWidgetStyle &style)
{
    const tstring &sName(style.GetProperty(_T("name")));
    if (TStringCompare(sName, _T("itembg")) == 0 || TStringCompare(sName, _T("itemhighlight")) == 0)
        return K2_NEW(ctx_Widgets,  CListWidgetState)(m_pInterface, this, style);

    return K2_NEW(ctx_Widgets,  CWidgetState)(m_pInterface, this, style);
}


/*====================
  CListBox::AddWidgetState
  ====================*/
bool    CListBox::AddWidgetState(CWidgetState *pState)
{
    if (pState->GetStateName() == _T("listbg"))
    {
        m_pBackground = pState;
        if (m_pBackground != nullptr)
            m_pBackground->RecalculateSize();
        return true;
    }
    else if (pState->GetStateName() == _T("itembg"))
    {
        m_pItemBackground = pState->GetAsListWidgetState();
        if (m_pItemBackground != nullptr)
            m_pItemBackground->RecalculateSize();
        return true;
    }
    else if (pState->GetStateName() == _T("itemhighlight"))
    {
        m_pItemHighlight = pState->GetAsListWidgetState();
        if (m_pItemHighlight != nullptr)
            m_pItemHighlight->RecalculateSize();
        return true;
    }

    // Delete this widget state if we don't end up using it
    SAFE_DELETE(pState);
    return false;
}


/*====================
  CListBox::RecalculateSize
  ====================*/
void    CListBox::RecalculateSize()
{
    IListWidget::RecalculateSize();

    UpdateScrollbars();
}


/*====================
  CListBox::RecalculateChildSize
  ====================*/
void    CListBox::RecalculateChildSize()
{
    m_fListItemOffsetX = GetPositionFromString(m_sListItemOffsetX, GetWidth(), GetHeight());
    m_fListItemOffsetY = GetPositionFromString(m_sListItemOffsetY, GetHeight(), GetWidth());

    m_fListItemWidth = GetSizeFromString(m_sListItemWidth, GetWidth(), GetHeight());
    m_fListItemHeight = GetSizeFromString(m_sListItemHeight, GetHeight(), GetWidth());

    if (m_pBackground != nullptr)
        m_pBackground->RecalculateSize();

    if (m_pItemBackground != nullptr)
        m_pItemBackground->RecalculateSize();

    if (m_pItemHighlight != nullptr)
        m_pItemHighlight->RecalculateSize();

    for (ListItemVector_it it(m_vItems.begin()), itEnd(m_vItems.end()); it != itEnd; ++it)
        (*it)->RecalculateSize();

    IListWidget::RecalculateChildSize();
}


/*====================
  CListBox::Frame
  ====================*/
void    CListBox::Frame(uint uiFrameLength, bool bProcessFrame)
{
    IWidget::Frame(uiFrameLength, bProcessFrame);

    if (IsDead())
        return;

    bProcessFrame = bProcessFrame && HasFlags(WFLAG_VISIBLE) && HasFlags(WFLAG_ENABLED);

    if (bProcessFrame)
    {
        // Recursively call item frame functions
        for (ListItemVector_it it(m_vItems.begin()), itEnd(m_vItems.end()); it != itEnd; ++it)
            (*it)->Frame(uiFrameLength, bProcessFrame);

        for (ListItemVector_it it(m_vHiddenItems.begin()), itEnd(m_vHiddenItems.end()); it != itEnd; ++it)
            (*it)->Frame(uiFrameLength, bProcessFrame);
    }
}


/*====================
  CListBox::Purge
  ====================*/
void    CListBox::Purge()
{
    if (!NeedsPurge())
        return;

    CListItem *pHoverItem(nullptr);
    CListItem *pSelectedItem(nullptr);

    if (m_iHoverListItem != -1 && m_iHoverListItem < (int)m_vItems.size())
        pHoverItem = m_vItems[m_iHoverListItem];

    if (m_iSelectedListItem != -1 && m_iSelectedListItem < (int)m_vItems.size())
        pSelectedItem = m_vItems[m_iSelectedListItem];

    m_iHoverListItem = -1;
    m_iSelectedListItem = -1;

    WidgetPointerVector vChildren(m_vChildren);
    for (WidgetPointerVector_rit it(vChildren.rbegin()), itEnd(vChildren.rend()); it != itEnd; ++it)
    {
        if ((*it)->IsDead())
        {
            SAFE_DELETE(*it);
            continue;
        }

        (*it)->Purge();
    }

    ListItemVector vItems(m_vItems);
    for (ListItemVector_rit it(vItems.rbegin()), itEnd(vItems.rend()); it != itEnd; ++it)
    {
        if ((*it)->IsDead())
        {
            m_vItems.erase(find(m_vItems.begin(), m_vItems.end(), *it));
            K2_DELETE(*it);
            continue;
        }

        (*it)->Purge();
    }

    vItems = m_vHiddenItems;
    for (ListItemVector_rit it(vItems.rbegin()), itEnd(vItems.rend()); it != itEnd; ++it)
    {
        if ((*it)->IsDead())
        {
            m_vHiddenItems.erase(find(m_vHiddenItems.begin(), m_vHiddenItems.end(), *it));
            K2_DELETE(*it);
            continue;
        }

        (*it)->Purge();
    }

    UnsetFlags(WFLAG_NEEDSPURGE);

    SetHoverListItem(GetItemIndex(pHoverItem));
    SetSelectedListItem(GetItemIndex(pSelectedItem));

    UpdateScrollbars();
}


/*====================
  CListBox::UpdateConditions
  ====================*/
void    CListBox::UpdateConditions()
{
    bool bChanged(false);

    for (vector<CListItem *>::iterator it(m_vHiddenItems.begin()); it != m_vHiddenItems.end(); )
    {
        if ((*it)->GetCondition().empty() || !AtoB(UIScript.Evaluate(*it, (*it)->GetCondition())))
        {
            ++it;
            continue;
        }

        m_vItems.push_back(*it);
        it = m_vHiddenItems.erase(it);
        bChanged = true;
    }
    
    for (vector<CListItem *>::iterator it(m_vItems.begin()); it != m_vItems.end(); )
    {
        if ((*it)->GetCondition().empty() || AtoB(UIScript.Evaluate(*it, (*it)->GetCondition())))
        {
            ++it;
            continue;
        }

        m_vHiddenItems.push_back(*it);
        it = m_vItems.erase(it);
        bChanged = true;
    }

    if (bChanged)
    {
        UpdateScrollbars();
        SortListNumericValue();
    }
}


/*====================
  CListBox::SetSelectedListItem
  ====================*/
void    CListBox::SetSelectedListItem(int iItem)
{
    if (m_iSelectedListItem == iItem)
        return;

    if (m_iSelectedListItem >= 0 && m_iSelectedListItem < int(m_vItems.size()))
    {
        CVec4f v4Color;
        CVec4f v4BorderColor;

        if (m_iHoverListItem == m_iSelectedListItem)
        {
            v4Color = m_v4HighlightFill;
            v4BorderColor = m_v4HighlightBorder;
        }
        else
        {
            v4Color = m_v4BackgroundFill;
            v4BorderColor = m_v4BackgroundBorder;
        }

        if (!m_bColorTransitions)
            m_vItems[m_iSelectedListItem]->SetLastColor(v4Color, v4BorderColor);

        m_vItems[m_iSelectedListItem]->SetNextColor(v4Color, v4BorderColor);
    }

    m_iSelectedListItem = iItem;

    if (m_bHoverSelect)
        SetHoverListItem(iItem);

    if (iItem < 0 || iItem >= int(m_vItems.size()))
        return;

    if (!m_bColorTransitions)
        m_vItems[m_iSelectedListItem]->SetLastColor(m_v4SelectedFill, m_v4SelectedBorder);

    m_vItems[m_iSelectedListItem]->SetNextColor(m_v4SelectedFill, m_v4SelectedBorder);
}


/*====================
  CListBox::SetHoverListItem
  ====================*/
void    CListBox::SetHoverListItem(int iItem)
{
    if (m_iHoverListItem == iItem)
        return;

    if (m_iHoverListItem >= 0 && m_iHoverListItem < int(m_vItems.size()))
    {
        CVec4f v4Color;
        CVec4f v4BorderColor;

        if (m_iHoverListItem == m_iSelectedListItem)
        {
            v4Color = m_v4SelectedFill;
            v4BorderColor = m_v4SelectedBorder;
        }
        else
        {
            v4Color = m_v4BackgroundFill;
            v4BorderColor = m_v4BackgroundBorder;
        }

        if (!m_bColorTransitions)
            m_vItems[m_iHoverListItem]->SetLastColor(v4Color, v4BorderColor);

        m_vItems[m_iHoverListItem]->SetNextColor(v4Color, v4BorderColor);
    }

    m_iHoverListItem = iItem;

    if (iItem < 0 || iItem >= int(m_vItems.size()))
        return;

    if (m_iHoverListItem == m_iSelectedListItem)
        return;

    if (m_bHoverHighlight)
    {
        if (!m_bColorTransitions)
            m_vItems[m_iHoverListItem]->SetLastColor(m_v4HighlightFill, m_v4HighlightBorder);

        m_vItems[m_iHoverListItem]->SetNextColor(m_v4HighlightFill, m_v4HighlightBorder);
    }

    if (m_bHoverSelect)
        SetSelectedListItem(iItem);
}


/*--------------------
  ListBoxCmd
  --------------------*/
UI_VOID_CMD(ListBoxCmd, 1)
{
    if (pThis == nullptr ||
        pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    tstring sInput(vArgList[0]->Evaluate());

    if (sInput.empty())
        return;

    tsvector vTokens(TokenizeString(sInput, _T(' ')));

    if (vTokens.empty())
        return;

    tstring sCommand(vTokens[0]);

    vTokens.erase(vTokens.begin());
    sInput = ConcatinateArgs(vTokens);

    if (CompareNoCase(sCommand, _T("sort")) == 0)
    {
        pListBox->SortList();
    }
    else if (CompareNoCase(sCommand, _T("sortvalue")) == 0)
    {
        pListBox->SortList();
    }
    else if (CompareNoCase(sCommand, _T("select")) == 0)
    {
        if (vArgList.size() > 1)
            pListBox->SetSelectedItem(AtoI(sInput), true);
    }
    else if (CompareNoCase(sCommand, _T("selectlast")) == 0)
    {
        pListBox->SetSelectedItem(pListBox->GetNumListitems() - 1, true);
    }
    else if (CompareNoCase(sCommand, _T("selectnone")) == 0)
    {
        pListBox->SetSelectedItem(-1, true);
    }
    //This command will search the list for a specified
    //value and change it's color to vTokenArgs[1]
    else if (CompareNoCase(sCommand, _T("setcolor")) == 0)
    {
        if (vTokens.size() >= 2)
        {
            for (uint uiLoop(0); uiLoop < pListBox->GetNumListitems(); uiLoop++)
            {
                tstring sValue(vTokens[0]);
                if (pListBox->GetListItemValue(uiLoop) == sValue)
                    pListBox->SetListItemText(uiLoop, vTokens[1] + StripColorCodes(sValue));
            }
        }
    }
    else if (CompareNoCase(sCommand, _T("remove")) == 0)
    {
        pListBox->RemoveListItemByName(sInput);
    }
}


/*--------------------
  SortListbox
  --------------------*/
UI_VOID_CMD(SortListbox, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    pListBox->SortList();
}


/*--------------------
  SortListboxValue
  --------------------*/
UI_VOID_CMD(SortListboxValue, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    pListBox->SortListValue();
}


/*--------------------
  SortListboxNumericValue
  --------------------*/
UI_VOID_CMD(SortListboxNumericValue, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    pListBox->SortListNumericValue();
}



/*--------------------
  SortListboxSortIndex
  --------------------*/
UI_VOID_CMD(SortListboxSortIndex, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    pListBox->SortListSortIndex(vArgList.size() > 0 ? vArgList[0]->EvaluateInteger() : 0);
}


/*--------------------
  SortListboxSortIndexNumeric
  --------------------*/
UI_VOID_CMD(SortListboxSortIndexNumeric, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    pListBox->SortListSortIndexNumeric(vArgList.size() > 0 ? vArgList[0]->EvaluateInteger() : 0);
}


/*--------------------
  EraseListItemByValue
  --------------------*/
UI_VOID_CMD(EraseListItemByValue, 1)
{
    if (pThis == nullptr ||
        pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    CListItem *pItem(pListBox->GetItemByValue(vArgList[0]->Evaluate()));

    if (pItem == nullptr)
        return;

    pListBox->RemoveListItem(pItem);
}


/*--------------------
  SetVerticalListScroll
  --------------------*/
UI_VOID_CMD(SetVerticalListScroll, 1)
{
    if (pThis == nullptr ||
        pThis->GetType() != WIDGET_LISTBOX)
        return;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    pListBox->VerticalScrollbarChange(AtoF(vArgList[0]->Evaluate()));
}


/*--------------------
  GetSelectedItemText
  --------------------*/
UI_CMD(GetSelectedItemText, 0)
{
    if (pThis == nullptr)
        return TSNULL;

    if (pThis->GetType() == WIDGET_COMBOBOX)
    {
        CComboBox *pComboBox(static_cast<CComboBox*>(pThis));

        if (pComboBox->GetSelectedListItem() == nullptr)
            return TSNULL;

        return pComboBox->GetSelectedListItem()->GetText();
    }
    else if (pThis->GetType() == WIDGET_LISTBOX)
    {
        CListBox *pListBox(static_cast<CListBox*>(pThis));

        if (pListBox->GetSelectedListItem() == nullptr)
            return TSNULL;

        return pListBox->GetSelectedListItem()->GetText();
    }

    return TSNULL;
}


/*--------------------
  GetSelectedItemName
  --------------------*/
UI_CMD(GetSelectedItemName, 0)
{
    if (pThis == nullptr)
        return TSNULL;

    if (pThis->GetType() == WIDGET_COMBOBOX)
    {
        CComboBox *pComboBox(static_cast<CComboBox*>(pThis));

        if (pComboBox->GetSelectedListItem() == nullptr)
            return TSNULL;

        return pComboBox->GetSelectedListItem()->GetValue();
    }
    else if (pThis->GetType() == WIDGET_LISTBOX)
    {
        CListBox *pListBox(static_cast<CListBox*>(pThis));

        if (pListBox->GetSelectedListItem() == nullptr)
            return TSNULL;

        return pListBox->GetSelectedListItem()->GetValue();
    }

    return TSNULL;
}


/*--------------------
  GetSelectedItemIndex
  --------------------*/
UI_CMD(GetSelectedItemIndex, 0)
{
    if (pThis == nullptr)
        return TSNULL;

    if (pThis->GetType() == WIDGET_COMBOBOX)
    {
        CComboBox *pComboBox(static_cast<CComboBox*>(pThis));

        if (pComboBox->GetSelectedListItem() == nullptr)
            return TSNULL;

        return XtoA(pComboBox->GetItemIndex(pComboBox->GetSelectedListItem()));
    }
    else if (pThis->GetType() == WIDGET_LISTBOX)
    {
        CListBox *pListBox(static_cast<CListBox*>(pThis));

        if (pListBox->GetSelectedListItem() == nullptr)
            return TSNULL;

        return XtoA(pListBox->GetItemIndex(pListBox->GetSelectedListItem()));
    }

    return TSNULL;
}


/*--------------------
  GetNumItems
  --------------------*/
UI_CMD(GetNumItems, 0)
{
    if (pThis == nullptr || pThis->GetType() != WIDGET_LISTBOX)
        return TSNULL;

    CListBox *pListBox(static_cast<CListBox*>(pThis));

    return XtoA(pListBox->GetNumListitems());
}


/*--------------------
  AddTemplateListItem
  --------------------*/
UI_VOID_CMD(AddTemplateListItem, 2)
{
    if (pThis == nullptr ||
        !pThis->HasFlags(WFLAG_LIST))
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 2); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    static_cast<IListWidget*>(pThis)->CreateNewListItemFromTemplate(vArgList[0]->Evaluate(), vArgList[1]->Evaluate(), mapParams);
}


/*--------------------
  AddTemplateListItemWithSort
  --------------------*/
UI_VOID_CMD(AddTemplateListItemWithSort, 3)
{
    if (pThis == nullptr ||
        !pThis->HasFlags(WFLAG_LIST))
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 3); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    static_cast<IListWidget*>(pThis)->CreateNewListItemFromTemplateWithSort(vArgList[0]->Evaluate(), vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), mapParams);
}


/*--------------------
  AddTemplateListItemWithSortReversed
  --------------------*/
UI_VOID_CMD(AddTemplateListItemWithSortReversed, 3)
{
    if (pThis == nullptr ||
        !pThis->HasFlags(WFLAG_LIST))
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 3); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    static_cast<IListWidget*>(pThis)->CreateNewListItemFromTemplateWithSortReversed(vArgList[0]->Evaluate(), vArgList[1]->Evaluate(), vArgList[2]->Evaluate(), mapParams);
}


/*--------------------
  ResizeListTemplate
  --------------------*/
UI_VOID_CMD(ResizeListTemplate, 2)
{
    if (pThis == nullptr ||
        !pThis->HasFlags(WFLAG_LIST))
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 2); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    static_cast<IListWidget*>(pThis)->ResizeListTemplate(vArgList[0]->Evaluate(), AtoUI(vArgList[1]->Evaluate()), mapParams);
}

