// (C)2005 S2 Games
// c_xmlproc_listitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_listitem.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_listbox.h"
#include "c_xmlproc_combobox.h"
#include "c_xmlproc_menu.h"
#include "c_listitem.h"
#include "i_listwidget.h"
#include "c_widgetstyle.h"
#include "c_widgettemplate.h"
#include "c_interface.h"
#include "c_uimanager.h"
//=============================================================================

// <listitem>
BEGIN_XML_REGISTRATION(listitem)
    REGISTER_XML_PROCESSOR(template)
    REGISTER_XML_PROCESSOR(listbox)
    REGISTER_XML_PROCESSOR(combobox)
    REGISTER_XML_PROCESSOR(menu)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR_WIDGET(listitem, CListItem)
    if (pObject->HasFlags(WFLAG_LIST))
        static_cast<IListWidget*>(pObject)->AddListItem(pNewWidget);
END_XML_PROCESSOR_WIDGET
