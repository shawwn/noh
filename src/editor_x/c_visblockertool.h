// (C)2008 S2 Games
// c_visblockertool.h
//
//=============================================================================
#ifndef __C_VISBLOCKERTOOL_H__
#define __C_VISBLOCKERTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBrush;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CVisBlockerTool
//=============================================================================
class CVisBlockerTool : public ITool
{
private:
    bool        m_bWorking;
    bool        m_bInverse;
    CVec3f      m_v3EndPos;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hVisBlockerMaterial;
    ResHandle   m_hFont;

    static void VisBlockerModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd);

public:
    CVisBlockerTool();
    ~CVisBlockerTool()              {}

    void    CalcToolProperties();
    void    PaintVisBlocker(float fFrameTime);

    void    PrimaryUp();
    void    PrimaryDown();
    void    SecondaryUp();
    void    SecondaryDown();
    void    TertiaryUp();
    void    TertiaryDown();
    void    QuaternaryUp();
    void    QuaternaryDown();

    void    Cancel();
    void    Delete();

    void    Frame(float fFrameTime);

    void    Draw();
    void    Render();
};
//=============================================================================

#endif // __C_VISBLOCKERTOOL_H__
