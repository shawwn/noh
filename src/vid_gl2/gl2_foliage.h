// (C)2008 S2 Games
// gl2_foliage.h
//
// Foliage
// TODO: Wrap in singleton
//=============================================================================
#ifndef __GL2_FOLIAGE_H__
#define __GL2_FOLIAGE_H__

//=============================================================================
// Headers
//=============================================================================
#include "../k2/i_worldcomponent.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define FOLIAGE_REBUILD_VERTICES    BIT(0)
#define FOLIAGE_REBUILD_COLORS      BIT(1)
#define FOLIAGE_REBUILD_NORMALS     BIT(2)
#define FOLIAGE_REBUILD_SHADERS     BIT(3)

class CWorld;

void    GL_InitFoliage();
void    GL_RebuildFoliage(int iChunkSize, const CWorld *pWorld);
void    GL_DestroyFoliage();
void    GL_InvalidateFoliageVertex(int iTileX, int iTileY, int iFlags);
void    GL_InvalidateFoliageTile(int iTileX, int iTileY, int iFlags);
void    GL_InvalidateFoliageLayer(int iFlags);
void    GL_AddFoliageChunks();
void    GL_FlagVisibleFoliageChunks();
void    GL_FoliageBounds(CBBoxf &bbTerrain);

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

    GLuint      uiVB;
    GLuint      uiIB[NUM_SORT_DIRECTIONS];

    CVec2s      *vTiles;
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
};

struct SFoliage
{
    const CWorld        *pWorld;

    int                 iChunkSize;
    float               fChunkWorldSize;
    int                 iNumChunksX;
    int                 iNumChunksY;

    SFoliageChunk**     ppChunks;

    SFoliage() :
    pWorld(nullptr),
    iChunkSize(0),
    fChunkWorldSize(0.0f),
    iNumChunksX(0),
    iNumChunksY(0),
    ppChunks(nullptr)
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
extern AttributeMap g_mapFoliageAttributes;
//=============================================================================

#endif // __GL2_FOLIAGE_H__
