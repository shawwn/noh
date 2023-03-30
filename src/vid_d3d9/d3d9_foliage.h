// (C)2005 S2 Games
// d3d9_foliage.h
//
// Direct3D Foliage
//=============================================================================
#ifndef __D3D9_FOLIAGE_H__
#define __D3D9_FOLIAGE_H__

#include "../k2/i_worldcomponent.h"

#define FOLIAGE_REBUILD_VERTICES    BIT(0)
#define FOLIAGE_REBUILD_COLORS      BIT(1)
#define FOLIAGE_REBUILD_NORMALS     BIT(2)
#define FOLIAGE_REBUILD_SHADERS     BIT(3)

class CWorld;

void    D3D_InitFoliage();
void    D3D_RebuildFoliage(int iChunkSize, const CWorld *pWorld);
void    D3D_DestroyFoliage();
void    D3D_InvalidateFoliageVertex(int iTileX, int iTileY, int iFlags);
void    D3D_InvalidateFoliageTile(int iTileX, int iTileY, int iFlags);
void    D3D_InvalidateFoliageLayer(int iFlags);
void    D3D_AddFoliageChunks();
void    D3D_FlagVisibleFoliageChunks();
void    D3D_FoliageBounds(CBBoxf &bbTerrain);

extern CCvar<float>     vid_foliageFalloffDistance;
extern CCvar<float>     vid_foliageAnimateSpeed;
extern CCvar<float>     vid_foliageAnimateStrength;

const int MAX_QUADS_PER_TILE(32);
#define MAX_TILES (g_Foliage.iChunkSize * g_Foliage.iChunkSize)
#define MAX_QUADS_PER_CHUNK (MAX_TILES * MAX_QUADS_PER_TILE)

enum EFoliageRenderOrder
{
    FOLIAGE_BACKFRONT = 0,
    FOLIAGE_FRONTBACK,
};

const int NUM_SORT_DIRECTIONS(1);

struct SFoliageArray
{
    ResHandle   hTexture;
    bool        bCamera;

    IDirect3DVertexBuffer9  *pVB;
    IDirect3DIndexBuffer9   *pIB[NUM_SORT_DIRECTIONS];

    CVec2s      *vTiles;    // short to save some memory
    int         iNumTiles;
    int         iLayer;

    int         iNumFoliageQuads;
};

struct SFoliageChunk
{
    SFoliageArray   *pArrays[NUM_FOLIAGE_LAYERS][256];
    int             iNumArrays[NUM_FOLIAGE_LAYERS];

    int         iValidityFlags;
    bool        bVisible;
    
    CBBoxf      bbBounds;
    int         iNumFoliageQuads;
};

struct SFoliage
{
    const CWorld        *pWorld;

    int                 iChunkSize;
    float               fChunkWorldSize;
    int                 iNumChunksX;
    int                 iNumChunksY;

    SFoliageChunk*      pChunks;

    SFoliage() :
    pWorld(NULL),
    iChunkSize(0),
    fChunkWorldSize(0.0f),
    iNumChunksX(0),
    iNumChunksY(0),
    pChunks(NULL)
    {}
    
    void    AllocateChunks(int iNewChunkSize);
    void    ClearChunks();
};

extern SFoliage     g_Foliage;
extern ResHandle    g_hFoliageMaterial;
extern ResHandle    g_hFoliageDiffuseReference;
extern ResHandle    g_hFoliageVertexShaderReference;
extern ResHandle    g_hFoliageVertexShaderNormal;
extern ResHandle    g_hFoliageVertexShaderCamera;
#endif // __D3D9_FOLIAGE_H__
