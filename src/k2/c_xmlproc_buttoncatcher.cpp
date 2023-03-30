// (C)2007 S2 Games
// c_xmlproc_buttoncatcher.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_buttoncatcher.h"
#include "c_xmlproc_interface.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_panel.h"
#include "c_xmlproc_webpanel.h"
#include "c_xmlproc_frame.h"
#include "c_xmlproc_widgetstate.h"
#include "c_buttoncatcher.h"
#include "c_widgettemplate.h"
#include "c_interface.h"
//=============================================================================

// <buttoncatcher>
BEGIN_XML_REGISTRATION(buttoncatcher)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR(template)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(frame)
    REGISTER_XML_PROCESSOR(widgetstate)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR_WIDGET(buttoncatcher, CButtonCatcher)
END_XML_PROCESSOR_WIDGET
