// (C)2006 S2 Games
// c_minimap.h
//
//=============================================================================
#ifndef __C_MINIMAP_H__
#define __C_MINIMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_widget.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CUICmd;
class ICvar;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EMMType
{
    MINIMAP_ICON = 0,
    MINIMAP_LINE,
    MINIMAP_QUAD,
    MINIMAP_ICON_OUTLINE
};

struct SMinimapIcon
{
    EMMType     eType;

    // Icon
    CRectf      recArea;
    CRectf      recHoverArea;
    CVec4f      v4Color;
    ResHandle   hTexture;

    // Line
    CVec4f      v4Color2;

    uint        uiUnitIndex;

    SMinimapIcon() :
    eType(MINIMAP_ICON),
    recArea(CRectf(0.0f, 0.0f, 0.0f, 0.0f)),
    recHoverArea(CRectf(0.0f, 0.0f, 0.0f, 0.0f)),
    v4Color(CVec4f(0.0f, 0.0f, 0.0f, 0.0f)),
    hTexture(INVALID_RESOURCE),
    uiUnitIndex(INVALID_INDEX)
    {}
};

typedef vector<SMinimapIcon> MinimapIconVector;

struct SMinimapButton
{
    CRectf      recArea;
    CVec4f      v4Color;
    CVec4f      v4RolloverColor;
    CVec2f      v2RolloverSize;
    ResHandle   hTexture;

    uint        uiIndex;
    
    SMinimapButton() :
    recArea(CRectf(0.0f, 0.0f, 0.0f, 0.0f)),
    v4Color(CVec4f(0.0f, 0.0f, 0.0f, 0.0f)),
    hTexture(INVALID_RESOURCE),
    v4RolloverColor(CVec4f(0.0f, 0.0f, 0.0f, 0.0f)),
    v2RolloverSize(CVec2f(0.0f, 0.0f)),
    uiIndex(INVALID_INDEX)
    {}
};

typedef vector<SMinimapButton> MinimapButtonVector;

struct SMinimapLineVertex
{
    CVec3f      v3Color;
    CVec2f      v2Pos;
    uint        uiTime;

    SMinimapLineVertex() :
    v3Color(CVec3f(0.0f, 0.0f, 0.0f)),
    v2Pos(CVec2f(0.0f, 0.0f)),
    uiTime(INVALID_TIME)
    {}
};

typedef deque<SMinimapLineVertex> MinimapDrawDeque;
//=============================================================================

//=============================================================================
// CMinimap
//=============================================================================
class CMinimap : public IWidget
{
protected:
    MinimapIconVector               m_vIcons;
    MinimapIconVector::iterator     m_itIcon;
    MinimapButtonVector             m_vButtons;
    MinimapDrawDeque                m_deqDrawing;

    ResHandle           m_hFogofWarTexture;

    uint    m_uiResetCursor;

    bool    m_bDragging;
    bool    m_bDraggingRight;
    bool    m_bFlop;
    bool    m_bNoPing;

    CRectf  m_recPadding;
    float   m_fWorldOffsetX, m_fWorldOffsetY;
    float   m_fWorldPercentX, m_fWorldPercentY;
    float   m_fOutlineSize;
    float   m_fHoverSize;

    uint    m_uiHoverUnit;

    tstring m_sOutlineSize;

public:
    ~CMinimap();
    CMinimap(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style);

    void            MouseDown(EButton button, const CVec2f &v2CursorPos);
    void            MouseUp(EButton button, const CVec2f &v2CursorPos);
    K2_API CVec2f   GetCursorInWorld(const CVec2f &v2CursorPos);

    bool    ButtonDown(EButton button);

    bool    ProcessInputCursor(const CVec2f &v2CursorPos);
    uint    GetMinimapHoverUnit();

    void    Enable();
    void    Disable();

    void    Rollover();
    void    Rolloff();

    void    RenderWidget(const CVec2f &vOrigin, float fFade);

    void    Frame(uint uiFrameLength, bool bProcessFrame);

    void    Execute(const tstring &sCmd, IBuffer &buffer);

    void    SetDragging(bool bDragging)         { m_bDragging = bDragging; }
    void    SetDraggingRight(bool bDragging)    { m_bDraggingRight = bDragging; }

    float   GetMinimapDrawX(float fFraction) const;
    float   GetMinimapDrawY(float fFraction) const;

    void    RecalculateSize();
};
//=============================================================================

#endif //__C_MINIMAP_H__
