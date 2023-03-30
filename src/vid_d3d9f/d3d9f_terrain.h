// (C)2005 S2 Games
// d3d9f_terrain.h
//
// Direct3D Terrain
//=============================================================================
#ifndef __D3D9F_TERRAIN_H__
#define __D3D9F_TERRAIN_H__

#include "../k2/i_worldcomponent.h"

#define TERRAIN_REBUILD_VERTICES    BIT(0)
#define TERRAIN_REBUILD_COLORS      BIT(1)
#define TERRAIN_REBUILD_TEXCOORDS   BIT(2)
#define TERRAIN_REBUILD_SHADERS     BIT(3)
#define TERRAIN_REBUILD_NORMALS     BIT(4)
#define TERRAIN_REBUILD_ALPHAMAP    BIT(5)

enum EMaterialPhase;
class CConvexPolyhedron;
class CWorldEntity;

void    D3D_InitTerrain();
void    D3D_RebuildTerrain(int chunkSize, const class CWorld *pWorld);
void    D3D_DestroyTerrain();
void    D3D_InvalidateTerrainVertex(int iTileX, int iTileY, int iFlags);
void    D3D_InvalidateTerrainTile(int iTileX, int iTileY, int iFlags);
void    D3D_InvalidateTerrainTexel(int iTexelX, int iTexelY, int iFlags);
void    D3D_InvalidateTerrainLayer(int flags);
void    D3D_AddTerrainChunks();
void    D3D_FlagVisibleTerrainChunks();
void    D3D_FlagVisibleTerrainChunksShadow(const CConvexPolyhedron &cScene);
void    D3D_TerrainBounds(CBBoxf &bbTerrain);

void    D3D_RebuildCliffs();

#define VERTS_PER_CHUNK ((terrain.iChunkSize + 1) * (terrain.iChunkSize + 1))
#define TILES_PER_CHUNK ((terrain.iChunkSize) * (terrain.iChunkSize))
#define MAX_ELEMS_PER_CHUNK (terrain.iChunkSize * terrain.iChunkSize * 6)

struct STerrainVertex
{
    CVec3f  v;
    CVec3f  n;
    dword   color;
    CVec2f  t0;
    CVec2f  t1;
    CVec3f  tangent;
};

const int MAX_CHUNKS_X = 32;
const int MAX_CHUNKS_Y = 32;

struct STerrainArray
{
    ResHandle       hMaterial;
    ResHandle       hDiffuse;
    ResHandle       hNormalmap;

    uint    *pElemList;
    uint    iNumElems;

    uint    iNumTris;
    
    uint    uiStartIndex;
};

struct STerrainSingleArray
{
    ResHandle       hMaterial;
    ResHandle       ahDiffuse[NUM_TERRAIN_LAYERS];
    ResHandle       ahNormalmap[NUM_TERRAIN_LAYERS];

    uint    *pElemList;
    uint    iNumElems;

    uint    iNumTris;
    
    uint    uiStartIndex;
};

struct STerrainCliffArray
{
    ResHandle       hMaterial;
    ResHandle       hDiffuse;
    ResHandle       hNormalmap;

    uint    uiNumVerts;
    uint    uiNumFaces;
    
    uint    uiStartVert;
    uint    uiStartIndex;

    vector<pair<CWorldEntity *, uint> > vCliffs;
};

struct STerrainChunk
{
    //int           iNumArrays[NUM_TERRAIN_LAYERS];
    //STerrainArray     *pArrays[NUM_TERRAIN_LAYERS][256];

    int         iNumSingleArrays;
    STerrainSingleArray *pSingleArrays[256];

    int         iNumCliffArrays;
    STerrainCliffArray  *pCliffArrays[256];

    int         iValidityFlags;
    bool        bVisible;
    bool        bVisibleShadow;

    IDirect3DVertexBuffer9  *pVB;
    IDirect3DIndexBuffer9   *pIBSingle;

    //IDirect3DIndexBuffer9 *pShadowIB;
    //IDirect3DIndexBuffer9 *pIB[NUM_TERRAIN_LAYERS];

    IDirect3DVertexBuffer9  *pVBVertexNormals;
    IDirect3DVertexBuffer9  *pVBTileNormals;

    IDirect3DVertexBuffer9  *pVBGrid;
    IDirect3DIndexBuffer9   *pIBGrid;
    uint                    uiGridSize;
    
    int         iAlphaMap; // Texture index

    CBBoxf      bbBounds;

    IDirect3DVertexBuffer9  *pVBCliff;
    IDirect3DIndexBuffer9   *pIBCliff;

    int         iNumCliffVerts;
    int         iNumCliffFaces;

    uint        uiNumFaces;
};

struct STerrain
{
    const CWorld        *pWorld;

    int                 iChunkSize;
    int                 iNumChunksX;
    int                 iNumChunksY;

    STerrainChunk       chunks[MAX_CHUNKS_Y][MAX_CHUNKS_X];
};
extern STerrain terrain;

extern bool         g_bAlpha;
extern int          g_iTerrainAlphaMap;
extern ResHandle    g_hTerrainMaterial;
extern ResHandle    g_hTerrainSingleMaterial;
extern ResHandle    g_hTerrainDiffuseReference;
extern ResHandle    g_hTerrainNormalmapReference;
extern ResHandle    g_hTerrainDiffuse2Reference;
extern ResHandle    g_hTerrainNormalmap2Reference;
extern ResHandle    g_hTerrainCheckerDiffuse;
extern ResHandle    g_hTerrainCheckerNormalmap;
extern int          g_iCliffVertexDecl;
extern int          g_iCliffVertexStride;

EXTERN_CVAR_BOOL(vid_terrainSinglePass);

#endif // __D3D9F_TERRAIN_H__
