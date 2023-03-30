// (C)2006 S2 Games
// c_tilefoliagemap.h
//
//=============================================================================
#ifndef __C_TILEFOLIAGEMAP_H__
#define __C_TILEFOLIAGEMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SFoliageTile;
//=============================================================================

//=============================================================================
// CTileFoliageMap
//=============================================================================
class CTileFoliageMap : public IWorldComponent
{
private:
    SFoliageTile        *m_pFoliageTiles[NUM_FOLIAGE_LAYERS];

public:
    ~CTileFoliageMap();
    CTileFoliageMap(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    bool    Serialize(IBuffer *pBuffer);
    void    Release();

    void    SetUsage();

    bool    GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
    bool    SetRegion(const CRecti &recArea, void *pSource, int iLayer);

    K2_API uint GetTileMaterialID(int iX, int iY, int iLayer);
    K2_API uint GetTileTextureID(int iX, int iY, int iLayer);
    K2_API byte GetNumCrossQuads(int iX, int iY, int iLayer);
    K2_API byte GetFlags(int iX, int iY, int iLayer);
};
//=============================================================================
#endif //__C_TILEFOLIAGEMAP_H__
