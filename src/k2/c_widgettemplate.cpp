// (C)2007 S2 Games
// c_widgettemplate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_widgettemplate.h"
#include "c_xmlnode.h"

#include "c_button.h"
#include "c_combobox.h"
#include "c_menu.h"
#include "c_cvarlabel.h"
#include "c_frame.h"
#include "c_image.h"
#include "c_webimage.h"
#include "c_webpanel.h"
#include "c_listbox.h"
#include "c_listitem.h"
#include "c_minimap.h"
#include "c_modelpanel.h"
#include "c_effectpanel.h"
#include "c_panel.h"
#include "c_piegraph.h"
#include "c_scrollbar.h"
#include "c_slider.h"
#include "c_swatch.h"
#include "c_textbox.h"
#include "c_textbuffer.h"
#include "c_interface.h"
#include "c_table.h"
#include "c_floater.h"
#include "c_snaptarget.h"
#include "c_widgetstate.h"
#include "c_uicmd.h"
#include "c_buttoncatcher.h"
#include "c_animatedimage.h"
#include "c_avatar.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
void    xmlproc_anchor(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style);
bool    xmlproc_dirlistitems(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style);
//=============================================================================

/*====================
  CWidgetTemplate::CWidgetTemplate
  ====================*/
CWidgetTemplate::CWidgetTemplate(CInterface *pInterface, const CXMLNode &node) :
m_sName(node.GetProperty(_CWS("name"))),
m_pParentInterface(pInterface)
{
    m_stackDefinitions.push(&m_vChildren);
}


/*====================
  CWidgetTemplate::AddChild
  ====================*/
void    CWidgetTemplate::AddChild(const tstring &sType, const CXMLNode &node)
{
    CWidgetDefinition def(m_pParentInterface, node);
    def.GetStyle().SetProperty(_CWS("widget"), sType);

    m_stackDefinitions.top()->push_back(def);
    m_stackDefinitions.push(m_stackDefinitions.top()->back().GetChildVector());
}


/*====================
  CWidgetTemplate::EndChild
  ====================*/
void    CWidgetTemplate::EndChild()
{
    m_stackDefinitions.pop();
}


/*====================
  AllocateWidget
  ====================*/
IWidget*    AllocateWidget(const tstring &sName, CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style)
{
    PROFILE("AllocateWidget");

    // HACK: This function feels very ugly, but doing it any other way just feels over complicated
    if (TStringCompare(sName, _T("panel")) == 0)
        return K2_NEW(ctx_Widgets,  CPanel)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("frame")) == 0)
        return K2_NEW(ctx_Widgets,  CFrame)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("label")) == 0)
        return K2_NEW(ctx_Widgets,  CLabel)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("button")) == 0)
        return K2_NEW(ctx_Widgets,  CButton)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("floater")) == 0)
        return K2_NEW(ctx_Widgets,  CFloater)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("combobox")) == 0)
        return K2_NEW(ctx_Widgets,  CComboBox)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("menu")) == 0)
        return K2_NEW(ctx_Widgets,  CMenu)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("cvarlabel")) == 0)
        return K2_NEW(ctx_Widgets,  CCvarLabel)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("image")) == 0)
        return K2_NEW(ctx_Widgets,  CImage)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("webimage")) == 0)
        return K2_NEW(ctx_Widgets,  CWebImage)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("webpanel")) == 0)
        return K2_NEW(ctx_Widgets,  CWebPanel)(pInterface, pParent, style, Host.GetHTTPManager());
    else if (TStringCompare(sName, _T("listbox")) == 0)
        return K2_NEW(ctx_Widgets,  CListBox)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("listitem")) == 0)
        return K2_NEW(ctx_Widgets,  CListItem)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("map")) == 0)
        return K2_NEW(ctx_Widgets,  CMinimap)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("modelpanel")) == 0)
        return K2_NEW(ctx_Widgets,  CModelPanel)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("effectpanel")) == 0)
        return K2_NEW(ctx_Widgets,  CEffectPanel)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("piegraph")) == 0)
        return K2_NEW(ctx_Widgets,  CPieGraph)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("scrollbar")) == 0)
        return K2_NEW(ctx_Widgets,  CScrollbar)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("slider")) == 0)
        return K2_NEW(ctx_Widgets,  CSlider)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("snaptarget")) == 0)
        return K2_NEW(ctx_Widgets,  CSnapTarget)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("swatch")) == 0)
        return K2_NEW(ctx_Widgets,  CSwatch)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("textbox")) == 0)
        return K2_NEW(ctx_Widgets,  CTextBox)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("textbuffer")) == 0)
        return K2_NEW(ctx_Widgets,  CTextBuffer)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("table")) == 0)
        return K2_NEW(ctx_Widgets,  CTable)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("buttoncatcher")) == 0)
        return K2_NEW(ctx_Widgets,  CButtonCatcher)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("animatedimage")) == 0)
        return K2_NEW(ctx_Widgets,  CAnimatedImage)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("avatar")) == 0)
        return K2_NEW(ctx_Widgets,  CAvatar)(pInterface, pParent, style);
    else if (TStringCompare(sName, _T("anchor")) == 0)
    {
        xmlproc_anchor(pInterface, pParent, style);
        return NULL;
    }
    else if (TStringCompare(sName, _T("widgetstate")) == 0)
    {
        return pParent->AllocateWidgetState(style);
    }
    else if (TStringCompare(sName, _T("dirlistitems")) == 0)
    {
        xmlproc_dirlistitems(pInterface, pParent, style);
        return NULL;
    }

    Console << _T("CWidgetDefinition::Instantiate() - Failed creating ") << sName << newl;
    return NULL;
}


/*====================
  CWidgetDefinition::Instantiate
  ====================*/
void    CWidgetDefinition::Instantiate(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &styleInstance)
{
    PROFILE("CWidgetTemplate::Instantiate");

    CWidgetStyle style(m_style);

    tstring sType(style.GetProperty(_CWS("widget")));
    style.RemoveProperty(_CWS("widget"));

    style.ApplySubstitutions(styleInstance);

    IWidget *pNewWidget(NULL);
    if (TStringCompare(sType, _T("instance")) == 0)
    {
        CWidgetTemplate *pTemplate(pInterface->GetTemplate(style.GetProperty(_CWS("name"))));
        if (pTemplate != NULL)
            pTemplate->Instantiate(pParent, style);
    }
    else
    {
        pNewWidget = AllocateWidget(sType, pInterface, pParent, style);
        if (pNewWidget == NULL)
            return;

        if (pParent->HasFlags(WFLAG_LIST) && TStringCompare(sType, _T("listitem")) == 0)
        {
            static_cast<IListWidget*>(pParent)->AddListItem(static_cast<CListItem*>(pNewWidget));
        }
        else if (TStringCompare(sType, _T("widgetstate")) == 0)
        {
            if (!pParent->AddWidgetState(static_cast<CWidgetState *>(pNewWidget)))
                return;
        }
        else
        {
            pParent->AddChild(pNewWidget);
        }

        pNewWidget->DoEvent(WEVENT_INSTANTIATE);
    }

    for (WidgetDefinitionVector_it it(m_vChildren.begin()); it != m_vChildren.end(); ++it)
        it->Instantiate(pInterface, (pNewWidget == NULL) ? pParent : pNewWidget, styleInstance);
}


/*====================
  CWidgetTemplate::Instantiate
  ====================*/
void    CWidgetTemplate::Instantiate(IWidget *pParent, const CWidgetStyle &style)
{
    PROFILE("CWidgetTemplate::Instantiate");

    for (WidgetDefinitionVector_it it(m_vChildren.begin()), itEnd(m_vChildren.end()); it != itEnd; ++it)
        it->Instantiate(m_pParentInterface, pParent, style);
}


/*--------------------
  Instantiate
  --------------------*/
UI_VOID_CMD(Instantiate, 1)
{
    CInterface *pInterface(pThis->GetInterface());
    if (pInterface == NULL)
        return;

    const tstring &sTemplateName(vArgList[0]->Evaluate());
    CWidgetTemplate *pTemplate(pInterface->GetTemplate(sTemplateName));
    if (pTemplate == NULL)
    {
        Console.Err << _T("Template ") << QuoteStr(sTemplateName) << _T(" not found for interface: ") << pInterface->GetFilename() << newl;
        return;
    }

    CWidgetStyle style(pInterface);
    for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
    {
        const tstring &sName((*it)->Evaluate());
        ++it;
        if (it == vArgList.end())
            break;
        const tstring &sValue((*it)->Evaluate());

        style.SetProperty(sName, sValue);
    }

    pTemplate->Instantiate(pThis, style);
}
