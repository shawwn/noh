// (C)2005 S2 Games
// c_cvarlabel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cvarlabel.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
//=============================================================================

//=============================================================================
//=============================================================================
EXTERN_CVAR_BOOL(ui_translateLabels);
//=============================================================================

/*====================
  CCvarLabel::~CCvarLabel
  ====================*/
CCvarLabel::~CCvarLabel()
{
}


/*====================
  CCvarLabel::CCvarLabel
  ====================*/
CCvarLabel::CCvarLabel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style) :
CLabel(pInterface, pParent, style),
m_bNumeric(style.GetPropertyBool(_T("numeric"))),
m_iPrecision(style.GetPropertyInt(_T("precision"))),
m_sPrefix(style.GetProperty(_T("prefix"))),
m_sSuffix(style.GetProperty(_T("suffix"))),
m_sCvar(style.GetProperty(_T("cvar")))
{
    if (ui_translateLabels)
    {
        m_sPrefix = UIManager.Translate(m_sPrefix);
        m_sSuffix = UIManager.Translate(m_sSuffix);
    }
}


/*====================
  CCvarLabel::Frame
  ====================*/
void    CCvarLabel::Frame(uint uiFrameLength, bool bProcessFrame)
{
    if (!HasFlags(WFLAG_VISIBLE) || !HasFlags(WFLAG_ENABLED))
        return;

    if (!m_sCvar.empty() && !m_refCvar.IsValid())
        m_refCvar.Assign(m_sCvar);

    DO_EVENT(WEVENT_FRAME)

    if (!m_refCvar.IsIgnored())
    {
        if (m_bNumeric)
            m_sText = m_sPrefix + XtoA(m_refCvar.GetFloat(), 0, 0, m_iPrecision) + m_sSuffix;
        else
            m_sText = m_sPrefix + m_refCvar.GetString() + m_sSuffix;
    }

    // Recursively call children frame functions
    for (WidgetPointerVector_rit it(m_vChildren.rbegin()), itEnd(m_vChildren.rend()); it != itEnd; ++it)
        (*it)->Frame(uiFrameLength, bProcessFrame);
}
