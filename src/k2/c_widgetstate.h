// (C)2007 S2 Games
// c_widgetstate.h
//
//=============================================================================
#ifndef __C_WIDGETSTATE_H__
#define __C_WIDGETSTATE_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CListWidgetState;
//=============================================================================

//=============================================================================
// CWidgetState
//=============================================================================
class CWidgetState : public IWidget
{
protected:
    tstring     m_sStateName;

public:
    virtual ~CWidgetState() {}
    CWidgetState(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    const tstring&  GetStateName()                              { return m_sStateName; }

    void    RenderWidget(const CVec2f &vOrigin, float fFade);

    virtual CListWidgetState*   GetAsListWidgetState()  { return NULL; }
};
//=============================================================================

#endif //__C_WIDGETSTATE_H__
