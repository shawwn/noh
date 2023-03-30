// (C)2005 S2 Games
// c_xmlproc_snaptarget.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_snaptarget.h"
#include "c_xmlproc_interface.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_panel.h"
#include "c_xmlproc_webpanel.h"
#include "c_xmlproc_listitem.h"
#include "c_xmlproc_frame.h"
#include "c_snaptarget.h"
#include "c_widgettemplate.h"
#include "c_interface.h"
//=============================================================================

// <snaptarget>
BEGIN_XML_REGISTRATION(snaptarget)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR(template)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(listitem)
    REGISTER_XML_PROCESSOR(frame)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR_WIDGET(snaptarget, CSnapTarget)
END_XML_PROCESSOR_WIDGET
