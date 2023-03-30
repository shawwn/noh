// (C)2005 S2 Games
// c_xmlproc_skylistitems.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"
#include "c_toolbox.h"
#include "i_tool.h"

#include "../k2/c_xmlnode.h"
#include "../k2/i_xmlprocessor.h"
#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_skin.h"
#include "../k2/i_xmlproc_widget.h"
#include "../k2/c_xmlproc_listbox.h"
#include "../k2/c_xmlproc_combobox.h"
#include "../k2/c_xmlproc_menu.h"
#include "../k2/c_listitem.h"
#include "../k2/i_listwidget.h"
#include "../k2/c_label.h"
#include "../k2/c_widgetstyle.h"
#include "../k2/c_widgettemplate.h"
#include "../k2/c_interface.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

/*====================
  xmlproc_skylistitems
  ====================*/
bool    xmlproc_skylistitems(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style)
{
    if (!pParent->HasFlags(WFLAG_LIST))
    {
        Console.Err << _T("Parent of <dirlistitems> is not a list widget") << newl;
        return false;
    }

    IListWidget *pList(static_cast<IListWidget*>(pParent));

    // Get the properties
    tstring sName(style.GetProperty(_T("name")));
    tstring sFont(style.GetProperty(_T("font"), _T("system_medium")));
    tstring sColor(style.GetProperty(_T("color"), _T("#ffffff")));
    tstring sValue(style.GetProperty(_T("value"), _T("")));
    tstring sPath(style.GetProperty(_T("path"), _T("/world/sky/sky.mdf")));

    tstring sWidth(pList->GetBaseListItemWidth());
    tstring sHeight(pList->GetBaseListItemHeight());

    CWidgetStyle styleItem(style);
    styleItem.SetProperty(_T("width"), sWidth);
    styleItem.SetProperty(_T("height"), sHeight);
    styleItem.SetProperty(_T("font"), sFont);
    styleItem.SetProperty(_T("color"), sColor);

    ResHandle   hSkyModel = g_ResourceManager.Register(sPath, RES_MODEL);
    CK2Model *pModel(static_cast<CK2Model*>(g_ResourceManager.GetModel(hSkyModel)->GetModelFile()));

    for (uint n(0); n < pModel->NumSkins(); ++n)
    {
        // Create new listitem
        styleItem.SetProperty(_T("value"), pModel->GetSkin(n)->GetName());
        CListItem *pNewListItem(K2_NEW(ctx_Widgets,   CListItem)(pInterface, NULL, styleItem));

        // Fill new listitem
        styleItem.SetProperty(_T("content"), pModel->GetSkin(n)->GetName());
        CLabel *pNewLabel(K2_NEW(ctx_Widgets,   CLabel)(pInterface, pNewListItem, styleItem));
        pNewListItem->AddChild(pNewLabel);

        // Add new item to the current listbox
        pList->AddListItem(pNewListItem);
    }

    // Set the default value
    if (pParent->HasFlags(WFLAG_LIST) && !sValue.empty())
        static_cast<IListWidget *>(pParent)->SetSelectedItem(sValue, true);

    return true;
}

// <skylistitems>
DECLARE_XML_PROCESSOR(skylistitems)
BEGIN_XML_REGISTRATION(skylistitems)
    REGISTER_XML_PROCESSOR(listbox)
    REGISTER_XML_PROCESSOR(combobox)
    REGISTER_XML_PROCESSOR(menu)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(skylistitems, IWidget)
    CInterface *pInterface(pObject->GetInterface());
    if (pInterface == NULL)
    {
        Console.Err << _T("Invalid interface for <skylistitems>") << m_sElementName << newl;
        return false;
    }

    CWidgetTemplate *pTemplate(pInterface->GetCurrentTemplate());
    if (pTemplate != NULL)
    {
        pTemplate->AddChild(m_sElementName, node);
        pTemplate->EndChild();
        return true;
    }

    xmlproc_skylistitems(pInterface, pObject, CWidgetStyle(pInterface, node));

END_XML_PROCESSOR_NO_CHILDREN
