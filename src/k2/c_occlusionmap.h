// (C)2008 S2 Games
// c_occlusionmap.h
//
//=============================================================================
#ifndef __C_OCCLUSIONMAP_H__
#define __C_OCCLUSIONMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// COcclusionMap
//=============================================================================
class COcclusionMap : public IWorldComponent
{
private:
    bool            m_bInitialized;

    uint            m_uiTileWidth;
    uint            m_uiTileHeight;
    uint            m_uiSize;
    float           m_fScale;
    bool            m_bRebuildOcclusion;

    float           *m_pTerrain;
    float           *m_pCombined;

public:
    ~COcclusionMap();
    COcclusionMap(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld)   { return Generate(pWorld); }
    bool    Generate(const CWorld *pWorld);
    bool    Save(CArchive &archive)                         { return true; }
    void    Release();

    void    InitTerrainHeight();
    void    Update(const CRecti &rect);

    K2_API bool GetRegion(const CRecti &recArea, byte *pDst, float fHeight);

    void    OccludeRegion(const CVec3f &v3Pos, float fRadius, float fHeight);

    K2_API void AddOccludeRegion(const CVec3f &v3Pos, float fRadius);
    K2_API void RemoveOccludeRegion(const CVec3f &v3Pos, float fRadius);

    uint    GetSize() const                                 { return m_uiSize; }
    int     GetCellIndex(int iX, int iY) const              { return iX + m_uiTileWidth * iY; }
};
//=============================================================================

#endif //__C_OCCLUSIONMAP_H__
