// (C)2008 S2 Games
// c_clifftool.h
//
//=============================================================================
#ifndef __C_CLIFFTOOL_H__
#define __C_CLIFFTOOL_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_toolbox.h"
#include "i_tool.h"
#include "../k2/c_cliffdefinitionresource.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CBrush;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum ECliffMode
{
    CLIFF_CREATE = 0,
    CLIFF_PAINT,
    CLIFF_CLEAR,
    CLIFF_VARIATION,
    CLIFF_DEFINITION
};

/*
enum ECliffPiece 
{
    EMPTY = 0,
    F_UP,
    F_RIGHT,
    F_DOWN,
    F_LEFT,
    C_UP,
    C_RIGHT,
    C_DOWN,
    C_LEFT
};
*/
//=============================================================================

//=============================================================================
// CCliffTool
//=============================================================================
class CCliffTool : public ITool
{
private:
    int*        m_pTempCliffMap;
    int         m_iXoffset;
    int         m_iYoffset;
    bool        m_bWorking;
    bool        m_bInverse;
    bool        m_bPrimaryDown;
    bool        m_bSecondaryDown;
    CVec3f      m_v3EndPos;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hCliffMaterial;
    ResHandle   m_hFont;
    int         m_iBrushX;
    int         m_iBrushY;

    static void CliffModify(byte *pRegion, const CRecti &recArea, const CBrush &brush, bool bAdd);

    uint    GetCliffGridIndex(int x, int y);
    bool    GetSavedRegion(const CRecti &recArea, void *pDest);

    void    CliffMapAdd(int iCliffHeight);
    void    CliffMapSet(int iCliffHeight);
    void    CliffMapRandom();

public:
    CCliffTool();
    ~CCliffTool();

    float   CalculateHeightVertex(int iX, int iY);
    void    CalculateCliffBlockers(CRecti scanArea);
    void    CalcToolProperties();
    void    SaveTempCliffMap();
    void    PaintCliff(float fFrameTime);
    void    CalculateTile(int iXC, int iYC);
    void    CliffRaiseOrLower(bool bRaise);
    void    CliffClear();
    uint    CliffCreate(float fX, float fY, float fz, float iRotation, tstring tModel, int iRotationVertex);
    void    RotationAdjustment(uint uiEntity, int iRotationVertex);
    void    CliffDelete(int iX, int iY);
    void    EnforceHeight(int x, int y, CRecti &enforcedRect);
    void    DefinitionMapSet();
    void    VariationMapSet();
    void    CliffVariation();
    void    CliffDefinition();
    void    CliffRandom();

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
    void    Enter();

    void    Frame(float fFrameTime);

    void    Draw();
    void    Render();
};
//=============================================================================

#endif // __C_CLIFFTOOL_H__
