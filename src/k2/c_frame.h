// (C)2005 S2 Games
// c_frame.h
//
//=============================================================================
#ifndef __C_FRAME_H__
#define __C_FRAME_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CPanel;
class CFrameBorder;

enum EFramePiece
{
    FRAME_PIECE_START = 0,
    FRAME_PIECE_BOTTOM_LEFT = FRAME_PIECE_START,
    FRAME_PIECE_BOTTOM,
    FRAME_PIECE_BOTTOM_RIGHT,
    FRAME_PIECE_LEFT,
    FRAME_PIECE_CENTER,
    FRAME_PIECE_RIGHT,
    FRAME_PIECE_TOP_LEFT,
    FRAME_PIECE_TOP,
    FRAME_PIECE_TOP_RIGHT,
    FRAME_PIECE_END
};
//=============================================================================

//=============================================================================
// CFrame
//=============================================================================
class CFrame : public IWidget
{
protected:
    ResHandle       m_hBorderTextures[9];

    float           m_fBorderSize;
    tstring         m_sBorderSize;
    CVec4f          m_v4BorderColor;

public:
    ~CFrame()   {}
    CFrame(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style);

    void    SetBorderSize(float fSize)              { m_fBorderSize = fSize; }
    void    SetColor(const CVec4f &v4Color);
    void    SetBorderColor(const CVec4f &v4Color);

    void    RecalculateSize();

    void    RenderWidget(const CVec2f &vOrigin, float fFade);
};
//=============================================================================

#endif //__C_FRAME_H__
