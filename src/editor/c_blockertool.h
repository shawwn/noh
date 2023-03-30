// (C)2007 S2 Games
// c_blockertool.h
//
//=============================================================================
#ifndef __C_BLOCKERTOOL_H__
#define __C_BLOCKERTOOL_H__

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
enum EBlockerMode
{
    BLOCKER_MOVEMENT = 0,
    BLOCKER_VISUAL
};
//=============================================================================

//=============================================================================
// CBlockerTool
// Blocker painter
//=============================================================================
class CBlockerTool : public ITool
{
private:
    bool        m_bValidPosition;
    bool        m_bWorking;
    bool        m_bInverse;
    CVec3f      m_v3EndPos;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hBlockerMaterial;
    ResHandle   m_hVisBlockerMaterial;
    ResHandle   m_hFont;
    int         m_iXTile, m_iYTile;

    static void BlockerModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd);
    static void VisBlockerModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd);

public:
    CBlockerTool();
    ~CBlockerTool()             {}

    void    CalcToolProperties();
    void    BlockerTerrain(float fFrameTime);
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
#endif // __C_BLOCKERTOOL_H__
