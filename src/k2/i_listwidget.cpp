// (C)2006 S2 Games
// i_listwidget.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_listwidget.h"
#include "c_uicmd.h"
#include "c_xmlnode.h"
#include "c_widgetstyle.h"
#include "c_listitem.h"
#include "c_label.h"
#include "c_interface.h"
#include "c_vid.h"
//=============================================================================

/*====================
  IListWidget::IListWidget
  ====================*/
IListWidget::IListWidget(CInterface *pInterface, IWidget *pParent, EWidgetType widgetType, const CWidgetStyle &style) :
IWidget(pInterface, pParent, widgetType, style),
m_sItemFont(style.GetProperty(_T("font"), _T("system_medium"))),
m_sItemAlign(style.GetProperty(_T("itemalign"), _T("left"))),
m_sTextColor(style.GetProperty(_T("textcolor"), _T("silver"))),
m_eWrapMode(LISTBOX_WRAP_NONE)
{   
    SetFlags(WFLAG_LIST);

    const tstring &sWrapMode(style.GetProperty(_T("wrap")));
    if (sWrapMode == _T("none") || !style.HasProperty(_T("wrap")))
        m_eWrapMode = LISTBOX_WRAP_NONE;
    else if (sWrapMode == _T("column"))
        m_eWrapMode = LISTBOX_WRAP_COLUMN;
    else if (sWrapMode == _T("row"))
        m_eWrapMode = LISTBOX_WRAP_ROW;
    else
        Console.Warn << SingleQuoteStr(sWrapMode) << " - Invalid wrap mode (none|column|row)" << newl;
}


/*--------------------
  SetSelectedItemByIndex
  --------------------*/
UI_VOID_CMD(SetSelectedItemByIndex, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    static_cast<IListWidget*>(pThis)->SetSelectedItem(AtoI(vArgList[0]->Evaluate()), vArgList.size() > 1 ? AtoB(vArgList[1]->Evaluate()) : true);
}


/*--------------------
  SetSelectedItemByValue
  --------------------*/
UI_VOID_CMD(SetSelectedItemByValue, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    static_cast<IListWidget *>(pThis)->SetSelectedItem(vArgList[0]->Evaluate(), vArgList.size() > 1 ? AtoB(vArgList[1]->Evaluate()) : true);
}


/*--------------------
  AddListItem
  --------------------*/
UI_VOID_CMD(AddListItem, 1)
{
    if (pThis == nullptr || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget*>(pThis));

    const tstring &sContent(vArgList[0]->Evaluate());
    const tstring &sValue(vArgList.size() > 1 ? vArgList[1]->Evaluate() : sContent);

    CWidgetStyle style(pThis->GetInterface());

    style.SetProperty(_T("content"), sContent);
    style.SetProperty(_T("textalign"), pList->GetItemAlign());
    style.SetProperty(_T("font"), pList->GetItemFont());
    style.SetProperty(_T("color"), pList->GetTextColor());

    style.SetProperty(_T("name"), StripColorCodes(sValue) + _T("_") + pList->GetName() + _T("_") + XtoA(pList->GetNumListitems()));

    style.SetProperty(_T("value"), StripColorCodes(sValue));

    style.SetProperty(_T("width"), pList->GetBaseListItemWidth());
    style.SetProperty(_T("height"), pList->GetBaseListItemHeight());

    // Create new listitem
    CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(pThis->GetInterface(), pList, style));

    style.SetProperty(_T("name"), style.GetProperty(_T("name")) + _T("_label"));
    style.SetProperty(_T("width"), _T("100%"));
    style.SetProperty(_T("height"), _T("100%"));

    // Fill new listitem
    CLabel *pNewLabel(K2_NEW(ctx_Widgets,  CLabel)(pThis->GetInterface(), pNewListItem, style));
    pNewLabel->SetText(sContent);
    pNewListItem->AddChild(pNewLabel);
    
    pList->AddListItem(pNewListItem);
}


/*--------------------
  ClearItems
  --------------------*/
UI_VOID_CMD(ClearItems, 0)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pListWidget(static_cast<IListWidget *>(pThis));

    pListWidget->SetSelectedItem(-1, true);
    pListWidget->ClearList();
}


/*--------------------
  HideItemByValue
  --------------------*/
UI_VOID_CMD(HideItemByValue, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    static_cast<IListWidget *>(pThis)->HideListItem(vArgList[0]->Evaluate());
}


/*--------------------
  ShowItemByValue
  --------------------*/
UI_VOID_CMD(ShowItemByValue, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    static_cast<IListWidget *>(pThis)->ShowListItem(vArgList[0]->Evaluate());
}


/*--------------------
  HasListItem
  --------------------*/
UI_CMD(HasListItem, 1)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return _T("0");

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    CListItem *pItem(pList->GetItemByValue(vArgList[0]->Evaluate()));

    if (pItem == nullptr)
        return _T("0");

    return _T("1");
}


/*--------------------
  GetNumListItems
  --------------------*/
UI_CMD(GetNumListItems, 0)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return _T("0");

    IListWidget *pList(static_cast<IListWidget *>(pThis));

    return XtoA(pList->GetNumListitems());
}


/*--------------------
  UpdateDirList
  --------------------*/
UI_VOID_CMD(UpdateDirList, 1)
{
    if (vArgList.size() < 1)
        return;

    if (pThis == nullptr)
        return;

    if (!pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget*>(pThis));

    CXMLNode Node(vArgList[0]->Evaluate());

    CWidgetStyle style(pList->GetInterface(), Node);

    // Get the properties
    tstring sName(style.GetProperty(_T("name")));
    tstring sFont(style.GetProperty(_T("font"), _T("system_medium")));
    tstring sColor(style.GetProperty(_T("color"), _T("#ffffff")));
    tstring sValue(style.GetProperty(_T("value"), _T("")));
    tstring sPath(style.GetProperty(_T("path"), _T("/")));
    tstring sWildcard(style.GetProperty(_T("wildcard"), _T("*.*")));
    bool    bRecurse(style.GetPropertyBool(_T("recurse"), true));
    bool    bShowPath(style.GetPropertyBool(_T("showpath"), false));

    tstring sWidth(pList->GetBaseListItemWidth());
    tstring sHeight(pList->GetBaseListItemHeight());

    CWidgetStyle styleItem(style);
    styleItem.SetProperty(_T("width"), sWidth);
    styleItem.SetProperty(_T("height"), sHeight);

    CWidgetStyle styleLabel(style);
    styleLabel.SetProperty(_T("width"), _T("100%"));
    styleLabel.SetProperty(_T("height"), _T("100%"));
    styleLabel.SetProperty(_T("font"), sFont);
    styleLabel.SetProperty(_T("color"), sColor);

    tsvector vFileList;
    vFileList.clear();
    FileManager.GetFileList(sPath, sWildcard, bRecurse, vFileList);

    for (size_t i(0); i < vFileList.size(); ++i)
    {
        // Create new listitem
        styleItem.SetProperty(_T("value"), vFileList[i]);
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(pList->GetInterface(), pList->GetListWidget(), styleItem));

        // Fill new listitem
        if (bShowPath)
            styleLabel.SetProperty(_T("content"), vFileList[i]);
        else
            styleLabel.SetProperty(_T("content"), Filename_StripPath(vFileList[i]));
        CLabel *pNewLabel(K2_NEW(ctx_Widgets,  CLabel)(pList->GetInterface(), pNewListItem, styleLabel));
        pNewListItem->AddChild(pNewLabel);

        // Add new item to the current listbox
        pList->AddListItem(pNewListItem);
    }

    // Set the default value
    if (!sValue.empty())
        pList->SetSelectedItem(sValue, true);
}


/*--------------------
  AddDirList
  --------------------*/
UI_VOID_CMD(AddDirList, 5)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
        return;

    CXMLNode::PropertyMap mapParams;
    for (ScriptTokenVector_cit cit(vArgList.begin() + 5); cit != vArgList.end(); ++cit)
    {
        CXMLNode::Key sKey((*cit)->Evaluate());
        ++cit;
        if (cit == vArgList.end())
            break;
        mapParams[sKey] = (*cit)->Evaluate();
    }

    tstring sPath(vArgList.size() > 1 ? vArgList[1]->Evaluate() : _T("/"));
    tstring sWildcard(vArgList.size() > 2 ? vArgList[2]->Evaluate() : _T("*.*"));
    bool bRecurse(vArgList.size() > 3 ? AtoB(vArgList[3]->Evaluate()) : true);
    bool bShowPath(vArgList.size() > 4 ? AtoB(vArgList[4]->Evaluate()) : false);

    tsvector vFileList;
    vFileList.clear();
    FileManager.GetFileList(sPath, sWildcard, bRecurse, vFileList);

    tstring sTemplate(vArgList[0]->Evaluate());

    for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
    {
        mapParams[_T("label")] = bShowPath ? *it : Filename_StripPath(*it);

        pList->CreateNewListItemFromTemplate(sTemplate, *it, mapParams);
    }
}


/*--------------------
  AddTextureList
  --------------------*/
UI_VOID_CMD(AddTextureList, 3)
{
    if (!pThis || !pThis->HasFlags(WFLAG_LIST))
        return;

    IListWidget *pList(static_cast<IListWidget *>(pThis));
    if (pList == nullptr)
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

    tsvector vFileList;
    vFileList.clear();
    Vid.GetTextureList(vArgList[1]->Evaluate(), vArgList[2]->Evaluate() + _T(".dds"), vFileList);
    FileManager.GetFileList(vArgList[1]->Evaluate(), _T("*") + vArgList[2]->Evaluate() + _T(".tga"), true, vFileList);
    FileManager.GetFileList(vArgList[1]->Evaluate(), _T("*") + vArgList[2]->Evaluate() + _T(".dds"), true, vFileList);
    
    std::sort(vFileList.begin(), vFileList.end());
    vFileList.erase(std::unique(vFileList.begin(), vFileList.end()), vFileList.end());

    tstring sTemplate(vArgList[0]->Evaluate());

    for (tsvector_it it(vFileList.begin()); it != vFileList.end(); ++it)
    {
        tstring sExt((*it).substr((*it).length() - 3));
        if (sExt == _T("dds"))
            *it = (*it).substr(0, (*it).length() - 3) + _T("tga");
        mapParams[_T("content")] =  Filename_StripPath(*it);
        mapParams[_T("label")] = Filename_StripPath(*it);//.substr(8);
        mapParams[_T("item")] = *it;
        mapParams[_T("value")] = *it;

        pList->CreateNewListItemFromTemplate(sTemplate, *it, mapParams);
    }
}