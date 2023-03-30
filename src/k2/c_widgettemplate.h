// (C)2007 S2 Games
// c_widgettemplate.h
//
//=============================================================================
#ifndef __C_WIDGETTEMPLATE_H__
#define __C_WIDGETTEMPLATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_widgetstyle.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CInterface;
class CXMLNode;
class CWidgetStyle;
class IWidget;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
typedef vector<class CWidgetDefinition>     WidgetDefinitionVector;
typedef WidgetDefinitionVector::iterator    WidgetDefinitionVector_it;

class CWidgetDefinition
{
private:
    CWidgetStyle            m_style;
    WidgetDefinitionVector  m_vChildren;

    CWidgetDefinition();

public:
    ~CWidgetDefinition()    {}
    CWidgetDefinition(CInterface *pInterface, const CXMLNode &node) :
    m_style(pInterface, node)
    {
    }

    WidgetDefinitionVector* GetChildVector()    { return &m_vChildren; }
    CWidgetStyle&           GetStyle()          { return m_style; }

    void                    Instantiate(CInterface *pInterface, IWidget *pParent, const CWidgetStyle &style);
};
//=============================================================================

//=============================================================================
// CWidgetTemplate
//=============================================================================
class CWidgetTemplate
{
private:
    tstring                         m_sName;
    CInterface*                     m_pParentInterface;

    WidgetDefinitionVector          m_vChildren;
    stack<WidgetDefinitionVector*>  m_stackDefinitions;

    CWidgetTemplate();

public:
    K2_API ~CWidgetTemplate()   {}
    K2_API CWidgetTemplate(CInterface *pInterface, const CXMLNode &node);

    K2_API tstring  GetName() const { return m_sName; }

    K2_API void AddChild(const tstring &sType, const CXMLNode &node);
    K2_API void EndChild();

    K2_API void Instantiate(IWidget *pParent, const CWidgetStyle &style);
};
//=============================================================================

#endif //__C_WIDGETTEMPLATE_H__
