// (C)2008 S2 Games
// c_cvarlabel.h
//
//=============================================================================
#ifndef __C_CVARLABEL_H__
#define __C_CVARLABEL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_label.h"
//=============================================================================

//=============================================================================
// CCvarLabel
//=============================================================================
class CCvarLabel : public CLabel
{
protected:
    CCvarReference  m_refCvar;
    bool            m_bNumeric;
    int             m_iPrecision;
    tstring         m_sPrefix;
    tstring         m_sSuffix;
    tstring         m_sCvar;

public:
    ~CCvarLabel();
    K2_API CCvarLabel(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);

    void    Frame(uint uiFrameLength, bool bProcessFrame);
};
//=============================================================================

#endif // __C_CVARLABEL_H__
