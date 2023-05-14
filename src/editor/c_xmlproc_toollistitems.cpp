// (C)2005 S2 Games
// c_xmlproc_toollistitems.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "editor.h"
#include "c_toolbox.h"
#include "i_tool.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_xmlproc_listbox.h"
#include "../k2/c_xmlproc_combobox.h"
#include "../k2/c_xmlproc_menu.h"
#include "../k2/c_listitem.h"
#include "../k2/i_listwidget.h"
#include "../k2/c_label.h"
#include "../k2/c_widgetstyle.h"
//=============================================================================

// <toollistitems>
DECLARE_XML_PROCESSOR(toollistitems)
BEGIN_XML_REGISTRATION(toollistitems)
    REGISTER_XML_PROCESSOR(listbox)
    REGISTER_XML_PROCESSOR(combobox)
    REGISTER_XML_PROCESSOR(menu)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(toollistitems, IWidget)
    CInterface *pInterface(pObject->GetInterface());
    if (pInterface == nullptr)
    {
        Console.Err << _T("Invalid interface for <dirlistitems>") << m_sElementName << newl;
        return false;
    }

    if (!pObject->HasFlags(WFLAG_LIST))
    {
        Console.Err << _T("Parent of <dirlistitems> is not a list widget") << newl;
        return false;
    }

    IListWidget *pList(static_cast<IListWidget*>(pObject));

    bool bHadBgColor(false);

    // Get the properties
    const tstring &sBgColor(node.GetProperty(_T("bgcolor")));

    tstring sWidth(pList->GetBaseListItemWidth());
    tstring sHeight(pList->GetBaseListItemHeight());

    if (!sBgColor.empty())
        bHadBgColor = true;

    CWidgetStyle style(pInterface, node);

    style.SetProperty(_T("width"), sWidth);
    style.SetProperty(_T("height"), sHeight);

    const ToolNameMap &mapTools(ToolBox.GetToolMap());

    for (ToolNameMap::const_iterator it(mapTools.begin()); it != mapTools.end(); ++it)
    {
        // Create new listitem
        CListItem *pNewListItem(K2_NEW(ctx_Editor,   CListItem)(pInterface, nullptr, style));

        // Fill new listitem
        CLabel *pNewLabel(K2_NEW(ctx_Editor,   CLabel)(pInterface, pNewListItem, style));
        pNewListItem->AddChild(pNewLabel);

        pList->AddListItem(pNewListItem);
    }
END_XML_PROCESSOR_NO_CHILDREN