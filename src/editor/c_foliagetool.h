// (C)2005 S2 Games
// c_foliagetool.h
//
//=============================================================================
#ifndef __C_FOLIAGETOOL_H__
#define __C_FOLIAGETOOL_H__

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
struct SFoliageVertexEntry;
struct SFoliageTile;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EFoliageToolMode
{
    FOLIAGE_TEXTURE,
    FOLIAGE_DENSITY,
    FOLIAGE_SIZE,
    FOLIAGE_SCALE,
    FOLIAGE_SMOOTH,
    FOLIAGE_AUTO
};
//=============================================================================

//=============================================================================
// CFoliageTool
//=============================================================================
class CFoliageTool : public ITool
{
private:
    bool        m_bValidPosition;
    float       m_fAmount;
    bool        m_bWorking;
    bool        m_bInverse;
    ResHandle   m_hTexture;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hFont;
    CVec3f      m_v3EndPos;
    int         m_iX, m_iY, m_iXTile, m_iYTile;

    static void AddFoliageDensity(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush, float fDensity, float fStrength);
    static void AddFoliageSize(SFoliageVertexEntry *pRegion, const CRecti &recArea,
        const CBrush &brush, float fWidth, float fHeight, float fRandWidth, float fRandHeight, float fStrength);
    static void AddFoliageScale(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush, float fFoliageScale, float fStrength);

    static void FoliageSmooth(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush, float fStrength);

    static void FoliageAuto(SFoliageVertexEntry *pRegion, const CRecti &recArea, const CBrush &brush,
        float fDensity, float fWidth, float fHeight, float fRandWidth, float fRandHeight, float fScale, float fStrength);

    static void ApplyFoliageTexture(SFoliageTile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hTexture, int iNumCrossQuads, byte yFlags);

    static void ApplyFoliageTextureStroked(SFoliageTile *pRegion, const CRecti &recArea, const CBrush &brush, ResHandle hTexture, int iNumCrossQuads, byte yFlags);

public:
    CFoliageTool();
    virtual ~CFoliageTool()             {}

    void    CalcToolProperties();
    void    PaintFoliageTile(float fFrameTime);
    void    PaintFoliageVertex(float fFrameTime);

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
#endif //__C_FOLIAGETOOL_H__
