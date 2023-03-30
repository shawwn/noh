// (C)2005 S2 Games
// c_xmlproc_brushlistitems.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "editor_common.h"

#include "../k2/i_xmlprocessor.h"
#include "../k2/c_brush.h"
#include "../k2/c_xmlproc_listbox.h"
#include "../k2/c_xmlproc_combobox.h"
#include "../k2/c_xmlproc_menu.h"
#include "../k2/c_listitem.h"
#include "../k2/i_listwidget.h"
#include "../k2/c_image.h"
#include "../k2/c_label.h"
#include "../k2/c_widgetstyle.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvar<int>   le_loadBrushes;
//=============================================================================

// <brushlistitems>
DECLARE_XML_PROCESSOR(brushlistitems)
BEGIN_XML_REGISTRATION(brushlistitems)
    REGISTER_XML_PROCESSOR(listbox)
    REGISTER_XML_PROCESSOR(combobox)
    REGISTER_XML_PROCESSOR(menu)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR(brushlistitems, IWidget)
    CInterface *pInterface(pObject->GetInterface());
    if (pInterface == NULL)
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

    // Get the properties
    const tstring &sValue(node.GetProperty(_T("value"), XtoA(CBrush::GetCurrentBrushIndex())));
    tstring sWidth(pList->GetBaseListItemWidth());
    tstring sHeight(pList->GetBaseListItemHeight());

    CWidgetStyle style(pInterface, node);
    style.SetProperty(_T("width"), sWidth);
    style.SetProperty(_T("height"), sHeight);
    style.SetProperty(_T("shadow"), true);
    style.SetProperty(_T("font"), _T("system_medium"));

    for (int i(0); i < le_loadBrushes; ++i)
    {
        tstring sFilename(_T("/core/brushes/standard/brush") + XtoA(i + 1) + _T(".tga"));

        // Create new listitem
        CListItem *pNewListItem(K2_NEW(g_heapWidgets,   CListItem)(pInterface, pList->GetListWidget(), style));
        pNewListItem->SetValue(XtoA(i));

        // Fill new listitem
        {
            CImage *pNewImage(K2_NEW(global,   CImage)(pInterface, pNewListItem, style));
            pNewImage->SetColor(WHITE);
            pNewImage->SetTexture(sFilename);

            CBrush *pBrush(CBrush::GetBrush(i));

            if (pBrush)
            {
                CLabel *pNewLabel(K2_NEW(global,   CLabel)(pInterface, pNewImage, style));
                pNewLabel->SetText(XtoA(pBrush->GetBrushSize()));
                pNewLabel->SetColor(WHITE);
                pNewImage->AddChild(pNewLabel);
            }

            pNewListItem->AddChild(pNewImage);
        }

        pList->AddListItem(pNewListItem);
    }

    pList->SetSelectedItem(sValue, true);
END_XML_PROCESSOR_NO_CHILDREN
