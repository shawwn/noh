// (C)2006 S2 Games
// c_xmlproc_dirlistitems.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_xmlprocessor.h"
#include "c_xmlproc_listbox.h"
#include "c_xmlproc_combobox.h"
#include "c_xmlproc_menu.h"
#include "c_interface.h"
#include "c_widgetstyle.h"
#include "i_listwidget.h"
#include "c_listitem.h"
#include "c_label.h"
#include "c_widgettemplate.h"
//=============================================================================

/*====================
  xmlproc_dirlistitems
  ====================*/
bool	xmlproc_dirlistitems(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style)
{
	if (pParent == NULL)
		return true;

	if (!pParent->HasFlags(WFLAG_LIST))
	{
		Console.Err << _T("Parent of <dirlistitems> is not a list widget") << newl;
		return false;
	}

	IListWidget *pList(static_cast<IListWidget*>(pParent));
	
	// Get the properties
	tstring	sName(style.GetProperty(_T("name")));
	tstring	sFont(style.GetProperty(_T("font"), _T("system_medium")));
	tstring	sColor(style.GetProperty(_T("color"), _T("#ffffff")));
	tstring sValue(style.GetProperty(_T("value"), _T("")));
	tstring sPath(style.GetProperty(_T("path"), _T("/")));
	tstring sWildcard(style.GetProperty(_T("wildcard"), _T("*.*")));
	bool	bRecurse(style.GetPropertyBool(_T("recurse"), true));
	bool	bShowPath(style.GetPropertyBool(_T("showpath"), false));

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
		CListItem *pNewListItem(K2_NEW(ctx_Widgets,  CListItem)(pInterface, pList->GetListWidget(), styleItem));

		// Fill new listitem
		if (bShowPath)
			styleLabel.SetProperty(_T("content"), vFileList[i]);
		else
			styleLabel.SetProperty(_T("content"), Filename_StripPath(vFileList[i]));
		CLabel *pNewLabel(K2_NEW(ctx_Widgets,  CLabel)(pInterface, pNewListItem, styleLabel));
		pNewListItem->AddChild(pNewLabel);

		// Add new item to the current listbox
		pList->AddListItem(pNewListItem);
	}

	// Set the default value
	if (!sValue.empty())
		pList->SetSelectedItem(sValue, true);

	return true;
}


// <dirlistitems>
DECLARE_XML_PROCESSOR(dirlistitems)
BEGIN_XML_REGISTRATION(dirlistitems)
	REGISTER_XML_PROCESSOR(listbox)
	REGISTER_XML_PROCESSOR(combobox)
	REGISTER_XML_PROCESSOR(menu)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(dirlistitems, IWidget)
	CInterface *pInterface(pObject->GetInterface());
	if (pInterface == NULL)
	{
		Console.Err << _T("Invalid interface for <dirlistitems>") << m_sElementName << newl;
		return false;
	}

	CWidgetTemplate *pTemplate(pInterface->GetCurrentTemplate());
	if (pTemplate != NULL)
	{
		pTemplate->AddChild(m_sElementName, node);
		pTemplate->EndChild();
		return true;
	}

	return xmlproc_dirlistitems(pInterface, pObject, CWidgetStyle(pInterface, node));

END_XML_PROCESSOR_NO_CHILDREN
