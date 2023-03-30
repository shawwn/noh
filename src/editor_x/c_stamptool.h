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
    DEFORM_RAMP,

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
    CVec3f      m_v3EndPos;
    float       m_fRX, m_fRY;
    ResHandle   m_hLineMaterial;
    ResHandle   m_hFont;

    static void TerrainAdd(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale);
    static void TerrainFlatten(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale, float fFlattenHeight);
    static void TerrainSmooth(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale);
    static void TerrainCut(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale, float fCutHeight);
    static void TerrainSlice(float *pRegion, const CRecti &recArea, const CBrush &brush, float fSliceHeight);
    static void TerrainClear(float *pRegion, const CRecti &recArea, const CBrush &brush);
    static void TerrainReduce(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale);
    static void TerrainNoise(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale, float fRX, float fRY, float fOctave);
    static void TerrainHarmonic(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale, float fRX, float fRY, int iHarmonics);
    static void TerrainClone(float *pRegion, const CRecti &recArea, const CBrush &brush, float fScale, int iTileX, int iTileY);

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
