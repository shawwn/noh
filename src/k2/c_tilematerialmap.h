// (C)2005 S2 Games
// c_tilematerialmap.h
//
//=============================================================================
#ifndef __C_TILEMATERIALMAP_H__
#define __C_TILEMATERIALMAP_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct STileInternal;
//=============================================================================

//=============================================================================
// CTileMaterialMap
//=============================================================================
class CTileMaterialMap : public IWorldComponent
{
private:
    STileInternal*  m_pTiles[NUM_TERRAIN_LAYERS];

public:
    ~CTileMaterialMap();
    CTileMaterialMap(EWorldComponent eComponent);

    bool    Load(CArchive &archive, const CWorld *pWorld);
    bool    Generate(const CWorld *pWorld);
    bool    Serialize(IBuffer *pBuffer);
    void    Release();

    void    SetUsage();

    bool    GetRegion(const CRecti &recArea, void *pDest, int iLayer) const;
    bool    SetRegion(const CRecti &recArea, void *pSource, int iLayer);

    K2_API uint             GetTileMaterialID(int iX, int iY, int iLayer);
    K2_API uint             GetTileDiffuseTextureID(int iX, int iY, int iLayer);
    K2_API uint             GetTileNormalmapTextureID(int iX, int iY, int iLayer);
};
//=============================================================================
#endif //__C_TILEMATERIALMAP_H__
