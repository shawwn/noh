// (C)2005 S2 Games
// c_frame_border.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_frame_border.h"
#include "c_interface.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CFrameBorder::CFrameBorder
  ====================*/
CFrameBorder::CFrameBorder(CInterface *pInterface, IWidget *pParent, EFrameBorder eType, const CWidgetStyle& style) :
CPanel(pInterface, pParent, style),
m_eType(eType)
{
}


/*====================
  CFrameBorder::RecalculateWidth
  ====================*/
void    CFrameBorder::RecalculateWidth(float fParentWidth, float fParentHeight)
{
    if (m_eType == FRAME_BORDER_BOTTOM || m_eType == FRAME_BORDER_TOP)
        SetWidth(fParentWidth - (GetHeight() * 2));

    RecalculateX(fParentWidth, fParentHeight);
    RecalculateChildWidth();
}


/*====================
  CFrameBorder::RecalculateHeight
  ====================*/
void    CFrameBorder::RecalculateHeight(float fParentHeight, float fParentWidth)
{
    if (m_eType == FRAME_BORDER_RIGHT || m_eType == FRAME_BORDER_LEFT)
        SetHeight(fParentHeight - (GetWidth() * 2));

    RecalculateY(fParentHeight, fParentWidth);
    RecalculateChildHeight();
}


/*====================
  CFrameBorder::RecalculateX
  ====================*/
void    CFrameBorder::RecalculateX(float fParentWidth, float fParentHeight)
{
    if (m_eType == FRAME_BORDER_BOTTOM_RIGHT || m_eType == FRAME_BORDER_RIGHT || m_eType == FRAME_BORDER_TOP_RIGHT)
        SetX(fParentWidth - GetWidth());
    else if (m_eType == FRAME_BORDER_TOP || m_eType == FRAME_BORDER_BOTTOM)
        SetX(GetHeight());
    else
        SetX(0.0f);
}


/*====================
  CFrameBorder::RecalculateY
  ====================*/
void    CFrameBorder::RecalculateY(float fParentHeight, float fParentWidth)
{
    if (m_eType == FRAME_BORDER_BOTTOM_RIGHT || m_eType == FRAME_BORDER_BOTTOM || m_eType == FRAME_BORDER_BOTTOM_LEFT)
        SetY(fParentHeight - GetHeight());
    else if (m_eType == FRAME_BORDER_RIGHT || m_eType == FRAME_BORDER_LEFT)
        SetY(GetWidth());
    else
        SetY(0.0f);
}
