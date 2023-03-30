// (C)2005 S2 Games
// c_frame_border.h
//
//=============================================================================
#ifndef __C_FRAME_BORDER_H__
#define __C_FRAME_BORDER_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_panel.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EFrameBorder
{
    FRAME_BORDER_START = 0,
    FRAME_BORDER_BOTTOM_LEFT = FRAME_BORDER_START,
    FRAME_BORDER_BOTTOM,
    FRAME_BORDER_BOTTOM_RIGHT,
    FRAME_BORDER_LEFT,
    FRAME_BORDER_RIGHT,
    FRAME_BORDER_TOP_LEFT,
    FRAME_BORDER_TOP,
    FRAME_BORDER_TOP_RIGHT,
    FRAME_BORDER_END = FRAME_BORDER_TOP_RIGHT
};
//=============================================================================

//=============================================================================
// CFrameBorder
//=============================================================================
class CFrameBorder : public CPanel
{
protected:
    EFrameBorder    m_eType;

public:
    ~CFrameBorder() {}
    CFrameBorder(CInterface *pInterface, IWidget *pParent, EFrameBorder eType, const CWidgetStyle& style);

    void    RecalculateSize(float fParentWidth, float fParentHeight);
    void    RecalculateX(float fParentWidth, float fParentHeight);
    void    RecalculateY(float fParentHeight, float fParentWidth);
};
//=============================================================================

#endif // __C_FRAME_BORDER_H__
