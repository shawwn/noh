// (C)2006 S2 Games
// c_xmlproc_modelpanel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_xmlproc_modelpanel.h"
#include "c_xmlproc_interface.h"
#include "c_xmlproc_template.h"
#include "c_xmlproc_panel.h"
#include "c_xmlproc_webpanel.h"
#include "c_xmlproc_listitem.h"
#include "c_xmlproc_frame.h"
#include "c_xmlproc_button.h"
#include "c_xmlproc_widgetstate.h"
#include "c_xmlproc_floater.h"
#include "c_modelpanel.h"
#include "c_widgettemplate.h"
#include "c_interface.h"
//=============================================================================

// <modelpanel>
BEGIN_XML_REGISTRATION(modelpanel)
    REGISTER_XML_PROCESSOR(interface)
    REGISTER_XML_PROCESSOR(template)
    REGISTER_XML_PROCESSOR(panel)
    REGISTER_XML_PROCESSOR(webpanel)
    REGISTER_XML_PROCESSOR(listitem)
    REGISTER_XML_PROCESSOR(frame)
    REGISTER_XML_PROCESSOR(button)
    REGISTER_XML_PROCESSOR(widgetstate)
    REGISTER_XML_PROCESSOR(floater)
END_XML_REGISTRATION
BEGIN_XML_PROCESSOR_WIDGET(modelpanel, CModelPanel)
END_XML_PROCESSOR_WIDGET
