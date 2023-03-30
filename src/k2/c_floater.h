// (C)2005 S2 Games
// c_floater.h
//
//=============================================================================
#ifndef __C_FLOATER_H__
#define __C_FLOATER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// CFloater
//=============================================================================
class CFloater : public IWidget
{
protected:
    bool    m_bKeepOnScren;
        //MikeG New Option
    bool    m_bLockToParent;

public:
    ~CFloater() {}
    K2_API CFloater(CInterface *pInterface, IWidget *pParent, const CWidgetStyle& style);
        
        //MikeG
    void    Render(const CVec2f &vOrigin, int iFlag, float fFade);
    bool    ProcessInputCursor(const CVec2f &v2CursorPos);
    void    RecalculatePosition();
    void    Frame(uint uiFrameLength, bool bProcessFrame);
};
//=============================================================================

#endif //__C_FLOATER_H__
