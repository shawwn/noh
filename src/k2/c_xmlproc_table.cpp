// (C)2007 S2 Games
// c_xmlproc_table.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_table.h"
#include "c_xmlproc_interface.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_panel.h"
#include "c_xmlproc_webpanel.h"
#include "c_xmlproc_frame.h"
#include "c_xmlproc_floater.h"
#include "c_table.h"
#include "c_widgettemplate.h"
#include "c_interface.h"
//=============================================================================

// <table>
BEGIN_XML_REGISTRATION(table)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR(template)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(frame)
    REGISTER_XML_PROCESSOR(floater)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR_WIDGET(table, CTable)
END_XML_PROCESSOR_WIDGET
