// (C)2005 S2 Games
// c_deformtool.h
//
//=============================================================================
#ifndef __C_DEFORMTOOL_H__
#define __C_DEFORMTOOL_H__

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
enum EDeformMode
{
    DEFORM_ADD,
    DEFORM_FLATTEN,
    DEFORM_SMOOTH,
    DEFORM_CUT,
    DEFORM_SLICE,
    DEFORM_CLEAR,
    DEFORM_REDUCE,
    DEFORM_NOISE,
    DEFORM_HARMONIC,
    DEFORM_CLONE,
    DEFORM_STAMP,

    NUM_DEFORM_MODES
};
//=============================================================================

//=============================================================================
// CDeformTool
// Terrain deformer
//=============================================================================
class CDeformTool : public ITool
{
private:
    bool        m_bWorking;
    bool        m_bInverse;
    bool        m_bValidPosition;
    CVec3f      m_v3EndPos;
    float       m_fRX, m_fRY;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hFont;
    int         m_iOldX, m_iOldY, m_iXOffset, m_iYOffset;

    void    TerrainAdd(float *pRegion, CRecti &recArea, CBrush &brush, float fScale);
    void    TerrainFlatten(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fFlattenHeight);
    void    TerrainSmooth(float *pRegion, CRecti &recArea, CBrush &brush, float fScale);
    void    TerrainCut(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fCutHeight);
    void    TerrainSlice(float *pRegion, CRecti &recArea, CBrush &brush, float fSliceHeight);
    void    TerrainClear(float *pRegion, CRecti &recArea, CBrush &brush);
    void    TerrainReduce(float *pRegion, CRecti &recArea, CBrush &brush, float fScale);
    void    TerrainNoise(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fRX, float fRY, float fOctave);
    void    TerrainHarmonic(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, float fRX, float fRY, int iHarmonics);
    void    TerrainClone(float *pRegion, CRecti &recArea, CBrush &brush, float fScale, int iTileX, int iTileY);
    void    TerrainStamp(float *pRegion, CRecti &recArea);
    bool    CanPantVert(int iX, int iY);

    void    RenderBrush();
    void    RenderStamp();

public:
    CDeformTool();
    ~CDeformTool()              {}

    void    CalcToolProperties();
    void    DeformTerrain(float fFrameTime);

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
#endif // __C_DEFORMTOOL_H__
