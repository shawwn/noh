// (C)2008 S2 Games
// c_gfxcpp
//
// Terrain
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_gfxterrain.h"

#include "c_gfxtextures.h"
#include "c_renderlist.h"
#include "c_gfxutils.h"

#include "../k2/c_material.h"
#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_mesh.h"
#include "../k2/c_skin.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CGfxTerrain)
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CGfxTerrain *GfxTerrain(CGfxTerrain::GetInstance());
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOLF  (vid_terrain,               true,       CONEL_DEV);
CVAR_BOOLF  (vid_terrainShadows,        true,       CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_terrainSinglePass,     true,       CVAR_SAVECONFIG);

EXTERN_CVAR_BOOL(vid_drawTerrainBounds);
//=============================================================================

/*====================
  CGfxTerrain::~CGfxTerrain
  ====================*/
CGfxTerrain::~CGfxTerrain()
{
}


/*====================
  CGfxTerrain::CGfxTerrain
  ====================*/
CGfxTerrain::CGfxTerrain() :
hTerrainMaterial(INVALID_RESOURCE),
hTerrainSingleMaterial(INVALID_RESOURCE),
hTerrainDiffuseReference(INVALID_RESOURCE),
hTerrainNormalmapReference(INVALID_RESOURCE),
hTerrainDiffuse2Reference(INVALID_RESOURCE),
hTerrainNormalmap2Reference(INVALID_RESOURCE),
iNumChunksX(0),
iNumChunksY(0),
iChunkSize(0),
pWorld(nullptr),
m_iCliffVertexStride(0)
{
}


/*====================
  CGfxTerrain::Init
  ====================*/
void    CGfxTerrain::Init()
{
    hTerrainCheckerDiffuse = g_ResourceManager.Register(_T("$white"), RES_TEXTURE);
    hTerrainCheckerNormalmap = g_ResourceManager.Register(_T("$flat_matte"), RES_TEXTURE);

    hTerrainDiffuseReference = g_ResourceManager.Register(_T("!terrain_d"), RES_REFERENCE);
    hTerrainNormalmapReference = g_ResourceManager.Register(_T("!terrain_n"), RES_REFERENCE);
    hTerrainDiffuse2Reference = g_ResourceManager.Register(_T("!terrain_d2"), RES_REFERENCE);
    hTerrainNormalmap2Reference = g_ResourceManager.Register(_T("!terrain_n2"), RES_REFERENCE);

    g_ResourceManager.UpdateReference(hTerrainDiffuseReference, g_ResourceManager.GetWhiteTexture());
    g_ResourceManager.UpdateReference(hTerrainNormalmapReference, g_ResourceManager.GetFlatTexture());

    hTerrainSingleMaterial = g_ResourceManager.Register(_T("/world/terrain/materials/default_single.material"), RES_MATERIAL);

    mapAttributes[_T("a_fHeight")] = SVertexAttribute(GL_FLOAT, 1, 0, false);
    mapAttributes[_T("a_vData0")] = SVertexAttribute(GL_UNSIGNED_BYTE, 4, 8, false);
    mapAttributes[_T("a_vData1")] = SVertexAttribute(GL_UNSIGNED_BYTE, 4, 12, false);

    //
    // Cliff vertexes
    //

    m_iCliffVertexStride = 12;
    m_iCliffVertexStride += sizeof(CVec3f);
    m_iCliffVertexStride += sizeof(CVec2f);
    m_iCliffVertexStride += sizeof(CVec3f);

    int v_offset(0);
    int n_offset(v_offset + sizeof(CVec3f));
    int t_offset(n_offset + sizeof(CVec3f));
    int tan_offset(t_offset + sizeof(CVec2f));

    m_mapCliffAttributes[_T("a_vTangent")] = SVertexAttribute(GL_FLOAT, 3, tan_offset, false);
}


/*====================
  CGfxTerrain::Destroy
  ====================*/
void    CGfxTerrain::Destroy()
{
    for (int iY(0); iY < iNumChunksY; ++iY)
        for (int iX(0); iX < iNumChunksX; ++iX)
            DestroyTerrainChunk(iX, iY);

    pWorld = nullptr;
    iChunkSize = 0;
    iNumChunksX = 0;
    iNumChunksY = 0;
}


/*====================
  CGfxTerrain::Rebuild
  ====================*/
void    CGfxTerrain::Rebuild(int iNewChunkSize, const CWorld *pNewWorld)
{
    PROFILE("CGfxTerrain::Rebuild");

    try
    {
        if(!pNewWorld)
            pNewWorld = pWorld;

        if (iNewChunkSize > pNewWorld->GetTileWidth())
            iNewChunkSize = pNewWorld->GetTileWidth();
        if (iNewChunkSize > pNewWorld->GetTileHeight())
            iNewChunkSize = pNewWorld->GetTileHeight();

        if (iNewChunkSize == 0)
            EX_ERROR(_T("iNewChunkSize is 0, division by 0 will result."));

        Destroy();

        if (pNewWorld->GetTileWidth() % iNewChunkSize != 0)
            EX_ERROR(_T("World width ") + XtoA(pNewWorld->GetTileWidth()) + _T(" is not a multiple of iChunkSize ") + XtoA(iNewChunkSize));
        if (pNewWorld->GetTileHeight() % iNewChunkSize != 0)
            EX_ERROR(_T("World height ") + XtoA(pNewWorld->GetTileHeight()) + _T(" is not a multiple of iChunkSize ") + XtoA(iNewChunkSize));

        pWorld = pNewWorld;
        iNumChunksX = pWorld->GetGridWidth() / iNewChunkSize;
        iNumChunksY = pWorld->GetGridHeight() / iNewChunkSize;
        iChunkSize = iNewChunkSize;

        for (int iY = 0; iY < iNumChunksY; ++iY)
        {
            for (int iX = 0; iX < iNumChunksX; ++iX)
            {
                AllocateTerrainChunk(iX, iY);
            }
        }

        RebuildCliffs();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGfxTerrain::Rebuild - "), NO_THROW);
    }
}


/*====================
  CGfxTerrain::InvalidateTerrainVertex
  ====================*/
void    CGfxTerrain::InvalidateTerrainVertex(int iTileX, int iTileY, int iFlags)
{
    int iChunkX, iChunkY;

    if (!iChunkSize || iTileX < 0 || iTileY < 0 || iTileX >= pWorld->GetGridWidth() || iTileY >= pWorld->GetGridHeight())
        return;

    iChunkX = (iTileX) / iChunkSize;
    iChunkY = (iTileY) / iChunkSize;

    // check if we're right on a chunk border,
    // in which case we'll have to invalidate more than one chunk
    if (iTileX % iChunkSize == 0 && iChunkX > 0)
        chunks[iChunkY][iChunkX - 1].iValidityFlags |= iFlags;

    if (iTileY % iChunkSize == 0 && iChunkY > 0)
        chunks[iChunkY - 1][iChunkX].iValidityFlags |= iFlags;

    if (iTileY % iChunkSize == 0 && iTileX % iChunkSize == 0 && iChunkY > 0 && iChunkX > 0)
        chunks[iChunkY - 1][iChunkX - 1].iValidityFlags |= iFlags;

    if (iChunkX < iNumChunksX && iChunkY < iNumChunksY)
        chunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  CGfxTerrain::InvalidateTerrainTile
  ====================*/
void    CGfxTerrain::InvalidateTerrainTile(int iTileX, int iTileY, int iFlags)
{
    int iChunkX, iChunkY;

    if (!iChunkSize || iTileX < 0 || iTileY < 0 || iTileX >= pWorld->GetTileWidth() || iTileY >= pWorld->GetTileHeight())
        return;

    iChunkX = (iTileX) / iChunkSize;
    iChunkY = (iTileY) / iChunkSize;

    if (iChunkX < 0 || iChunkX >= iNumChunksX ||
        iChunkY < 0 || iChunkY >= iNumChunksX)
        return;

    chunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  CGfxTerrain::InvalidateTerrainTexel
  ====================*/
void    CGfxTerrain::InvalidateTerrainTexel(int iTexelX, int iTexelY, int iFlags)
{
    int iChunkX, iChunkY;

    if (!iChunkSize || iTexelX < 0 || iTexelY < 0 || iTexelX >= pWorld->GetTexelWidth() || iTexelY >= pWorld->GetTexelHeight())
        return;

    iChunkX = (iTexelX) / (iChunkSize * pWorld->GetTexelDensity());
    iChunkY = (iTexelY) / (iChunkSize * pWorld->GetTexelDensity());

    if (iChunkX < 0 || iChunkX >= iNumChunksX ||
        iChunkY < 0 || iChunkY >= iNumChunksX)
        return;

    chunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  CGfxTerrain::InvalidateTerrainLayer
  ====================*/
void    CGfxTerrain::InvalidateTerrainLayer(int iFlags)
{
    for (int iY = 0; iY < iNumChunksY; ++iY)
        for (int iX = 0; iX < iNumChunksX; ++iX)
            chunks[iY][iX].iValidityFlags |= iFlags;
}


/*====================
  CGfxTerrain::FlagVisibleTerrainChunks
  ====================*/
void    CGfxTerrain::FlagVisibleTerrainChunks()
{
    PROFILE("CGfxTerrain::FlagVisibleTerrainChunks");

    for (int iY(0); iY < iNumChunksY; ++iY)
    {
        for (int iX(0); iX < iNumChunksX; ++iX)
        {
            STerrainChunk *chunk = &chunks[iY][iX];

            if (chunk->iValidityFlags)
                RebuildTerrainChunk(iX, iY);

            chunk->bVisible = SceneManager.AABBIsVisible(chunk->bbBounds);
        }
    }
}


/*====================
  CGfxTerrain::FlagVisibleTerrainChunksShadow
  ====================*/
void    CGfxTerrain::FlagVisibleTerrainChunksShadow(const CConvexPolyhedron &cScene)
{
    for (int iY(0); iY < iNumChunksY; ++iY)
    {
        for (int iX(0); iX < iNumChunksX; ++iX)
        {
            STerrainChunk *chunk = &chunks[iY][iX];

            float fFraction(1.0f);
            chunk->bVisibleShadow = I_MovingBoundsSurfaceIntersect(CVec3f(0.0f, 0.0f, 0.0f), SceneManager.GetSunPos() * -100000.0, chunk->bbBounds, cScene, fFraction);
        }
    }
}


/*====================
  CGfxTerrain::TerrainBounds
  ====================*/
void    CGfxTerrain::TerrainBounds(CBBoxf &bbTerrain)
{
    for (int iX(0); iX < iNumChunksX; ++iX)
    {
        for (int iY(0); iY < iNumChunksY; ++iY)
        {
            STerrainChunk *chunk = &chunks[iY][iX];

            if (chunk->bVisible)
                bbTerrain.AddBox(chunk->bbBounds);
        }
    }
}


/*====================
  CGfxTerrain::AllocateTerrainChunk
  ====================*/
void    CGfxTerrain::AllocateTerrainChunk(int iX, int iY)
{
    PROFILE("CGfxTerrain::AllocateTerrainChunk");

    STerrainChunk *chunk(&chunks[iY][iX]);
    MemManager.Set(chunk, 0, sizeof(STerrainChunk));

    chunk->iValidityFlags = TERRAIN_REBUILD_VERTICES |
                            TERRAIN_REBUILD_TEXCOORDS |
                            TERRAIN_REBUILD_COLORS |
                            TERRAIN_REBUILD_NORMALS |
                            TERRAIN_REBUILD_SHADERS |
                            TERRAIN_REBUILD_ALPHAMAP;

    chunk->uiAlphaMap = 0;

    glGenBuffersARB(1, &chunk->uiVB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->uiVB);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, (iChunkSize + 1) * (iChunkSize + 1) * 16, nullptr, GL_STATIC_DRAW_ARB);

    glGenBuffersARB(1, &chunk->uiIB);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, chunk->uiIB);
    glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, iChunkSize * iChunkSize * 6 * sizeof(GLushort), nullptr, GL_STATIC_DRAW_ARB);

    glGenBuffersARB(1, &chunk->uiVBCliff);
    glGenBuffersARB(1, &chunk->uiIBCliff);

    RebuildTerrainChunk(iX, iY);
}


/*====================
  CGfxTerrain::DestroyTerrainChunk
  ====================*/
void    CGfxTerrain::DestroyTerrainChunk(int iX, int iY)
{
    STerrainChunk *chunk = &chunks[iY][iX];
    GfxTextures->UnregisterTexture(_T("*terrain_chunk_alphamap_") + XtoA(iX, FMT_PADZERO, 4) + _T("_") + XtoA(iY, FMT_PADZERO, 4));

    for (int i = 0; i < chunk->iNumArrays; i++)
    {
        K2_DELETE_ARRAY(chunk->pArrays[i]->pElemList);
        K2_DELETE(chunk->pArrays[i]);
        chunk->pArrays[i] = nullptr;
    }
    chunk->iNumArrays = 0;
    
    glDeleteBuffersARB(1, &chunk->uiVB);
    glDeleteBuffersARB(1, &chunk->uiIB);

    glDeleteBuffersARB(1, &chunk->uiVBCliff);
    glDeleteBuffersARB(1, &chunk->uiIBCliff);

    MemManager.Set(chunk, 0, sizeof(STerrainChunk));
}


/*====================
  CGfxTerrain::TerrainTextureCompare
  ====================*/
int     CGfxTerrain::TerrainTextureCompare(ResHandle ah0[], ResHandle ah1[])
{
    for (int i(0); i < NUM_TERRAIN_LAYERS; ++i)
    {
        if (ah0[i] == INVALID_RESOURCE || ah1[i] == INVALID_RESOURCE)
            continue;

        if (ah0[i] != ah1[i])
            return ah1[i] - ah0[i];
    }

    return 0;
}


/*====================
  CGfxTerrain::AllocTerrainArray
  ====================*/
STerrainArray*  CGfxTerrain::AllocTerrainArray()
{
    STerrainArray *pNewArray(K2_NEW(ctx_GL2,    STerrainArray));
    MemManager.Set(pNewArray, 0, sizeof(STerrainArray));
    pNewArray->uiNumElems = 0;
    pNewArray->pElemList = K2_NEW_ARRAY(ctx_GL2, uint, MAX_ELEMS_PER_CHUNK);
    MemManager.Set(pNewArray->pElemList, 0, sizeof(MAX_ELEMS_PER_CHUNK * sizeof(int)));

    return pNewArray;
}


/*====================
  CGfxTerrain::AllocTerrainCliffArray
  ====================*/
STerrainCliffArray* CGfxTerrain::AllocTerrainCliffArray()
{
    STerrainCliffArray *pNewArray(K2_NEW(ctx_GL2,    STerrainCliffArray));

    return pNewArray;
}


/*====================
  CGfxTerrain::RebuildTerrainChunk
  ====================*/
void    CGfxTerrain::RebuildTerrainChunk(int iX, int iY)
{
    PROFILE("CGfxTerrain::RebuildTerrainChunk");

    try
    {
        STerrainChunk   *chunk(&chunks[iY][iX]);
        int             iFlags(chunk->iValidityFlags);

        if (!iFlags)
            return;

        int     iStartX(iX * iChunkSize);       // Grid top-left tile coord
        int     iStartY(iY * iChunkSize);       // Grid top-left tile coord

        int iPos;
        float fMinZ(FAR_AWAY);
        float fMaxZ(-FAR_AWAY);
        
        for(int i = 0; i < chunk->iNumArrays; i++)
        {
            K2_DELETE_ARRAY(chunk->pArrays[i]->pElemList);
            K2_DELETE(chunk->pArrays[i]);
            chunk->pArrays[i] = nullptr;
        }

        chunk->iNumArrays = 0;

        iPos = 0;
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, chunk->uiVB);
        GLfloat* dataVB = (GLfloat*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

        for (int iTileY = iStartY; iTileY <= iStartY + iChunkSize; iTileY++)
        {
            for (int iTileX = iStartX; iTileX <= iStartX + iChunkSize; iTileX++)
            {
                float fWorldZ(pWorld->GetGridPoint(iTileX, iTileY));
                if (fWorldZ < fMinZ)
                    fMinZ = fWorldZ;
                if (fWorldZ > fMaxZ)
                    fMaxZ = fWorldZ;

                CVec4b v4Color(pWorld->GetGridColor(iTileX, iTileY));

                byte yNormal[4];
                const CVec3f &v3Normal(pWorld->GetGridNormal(iTileX, iTileY));
                yNormal[0] = BYTE_ROUND((v3Normal.x + 1.0f) * 0.5f * 255.0f);
                yNormal[1] = BYTE_ROUND((v3Normal.y + 1.0f) * 0.5f * 255.0f);
                yNormal[2] = BYTE_ROUND((v3Normal.z + 1.0f) * 0.5f * 255.0f);
                yNormal[3] = iTileX - iStartX;

                byte yTangent[4];
                const CVec3f &v3Tangent(pWorld->GetGridTangent(iTileX, iTileY));
                yTangent[0] = BYTE_ROUND((v3Tangent.x + 1.0f) * 0.5f * 255.0f);
                yTangent[1] = BYTE_ROUND((v3Tangent.y + 1.0f) * 0.5f * 255.0f);
                yTangent[2] = BYTE_ROUND((v3Tangent.z + 1.0f) * 0.5f * 255.0f);
                yTangent[3] = iTileY - iStartY;

                dataVB[iPos++] = fWorldZ;
                dataVB[iPos++] = *((float*)&v4Color);

                dataVB[iPos++] = *((float*)&yNormal);
                dataVB[iPos++] = *((float*)&yTangent);
            }
        }
        glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

        chunk->bbBounds = CBBoxf
        (
            CVec3f(pWorld->ScaleGridCoord(iStartX), pWorld->ScaleGridCoord(iStartY), fMinZ),
            CVec3f(pWorld->ScaleGridCoord(iStartX + iChunkSize), pWorld->ScaleGridCoord(iStartY + iChunkSize), fMaxZ+0.01f)
        );

        STerrainArray   *pCurrentArray(nullptr);

        for (int iTileY = iStartY; iTileY < iStartY + iChunkSize; iTileY += 4)
        {
            for (int iTileX = iStartX; iTileX < iStartX + iChunkSize; iTileX += 4)
            {
                // Optimize the vertex cache by rendering in 4x4 tiles.
                // We do this vertically in order to be able to reuse the last vertices on the next tile.
                // Otherwise, there would only be reuse within tiles, rather than also between tiles.
                for (int iTileX2 = iTileX; iTileX2 < iTileX + 4; iTileX2++)
                {
                    for (int iTileY2 = iTileY; iTileY2 < iTileY + 4; iTileY2++)
                    {
                        if (pWorld->GetCliff(iTileX2, iTileY2))
                            continue;

                        ResHandle ahCurrentDiffuse[NUM_TERRAIN_LAYERS];
                        for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
                        {
                            if (pWorld->IsTileVisible(iTileX2, iTileY2, iLayer))
                                ahCurrentDiffuse[iLayer] = pWorld->GetTileDiffuseTexture(iTileX2, iTileY2, iLayer);
                            else
                                ahCurrentDiffuse[iLayer] = INVALID_RESOURCE;
                        }

                        if (!pCurrentArray || TerrainTextureCompare(ahCurrentDiffuse, pCurrentArray->ahDiffuse))
                        {
                            bool bFoundShader(false);

                            for (int n(0); n < chunk->iNumArrays; ++n)
                            {
                                if (TerrainTextureCompare(ahCurrentDiffuse, chunk->pArrays[n]->ahDiffuse) == 0)
                                {
                                    pCurrentArray = chunk->pArrays[n];
                                    bFoundShader = true;
                                    break;
                                }
                            }

                            if (!bFoundShader && chunk->iNumArrays < 256)
                            {
                                // We've encountered a new shader
                                pCurrentArray = chunk->pArrays[chunk->iNumArrays] = AllocTerrainArray();
                                
                                pCurrentArray->uiNumElems = 0;
                                pCurrentArray->hMaterial = pWorld->GetTileMaterial(iTileX2, iTileY2, 0);

                                for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
                                {
                                    pCurrentArray->ahDiffuse[iLayer] = ahCurrentDiffuse[iLayer];
                                    pCurrentArray->ahNormalmap[iLayer] = pWorld->GetTileNormalmapTexture(iTileX2, iTileY2, iLayer);
                                }

                                chunk->iNumArrays++;
                            }
                        }

                        for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
                        {
                            if (pCurrentArray->ahDiffuse[iLayer] == INVALID_RESOURCE &&
                                ahCurrentDiffuse[iLayer] != INVALID_RESOURCE)
                            {
                                pCurrentArray->ahDiffuse[iLayer] = ahCurrentDiffuse[iLayer];
                                pCurrentArray->ahNormalmap[iLayer] = pWorld->GetTileNormalmapTexture(iTileX2, iTileY2, iLayer);
                            }
                        }

                        uint uiVertPos((iTileY2 - iStartY) * (iChunkSize + 1) + (iTileX2 - iStartX));
                        uint *pElements(&pCurrentArray->pElemList[pCurrentArray->uiNumElems]);

                        if (pWorld->GetTileSplit(iTileX2, iTileY2) == SPLIT_NEG)
                        {
                            *(pElements++) = uiVertPos;
                            *(pElements++) = uiVertPos + 1;
                            *(pElements++) = uiVertPos + (iChunkSize + 1);

                            *(pElements++) = uiVertPos + 1;
                            *(pElements++) = uiVertPos + (iChunkSize + 1) + 1;
                            *(pElements++) = uiVertPos + (iChunkSize + 1);
                        }
                        else
                        {
                            *(pElements++) = uiVertPos;
                            *(pElements++) = uiVertPos + (iChunkSize + 1) + 1;
                            *(pElements++) = uiVertPos + (iChunkSize + 1);

                            *(pElements++) = uiVertPos;
                            *(pElements++) = uiVertPos + 1;
                            *(pElements++) = uiVertPos + (iChunkSize + 1) + 1;
                        }

                        pCurrentArray->uiNumFaces += 2;
                        pCurrentArray->uiNumElems += 6;

                        if (pCurrentArray->uiNumElems > uint(MAX_ELEMS_PER_CHUNK))
                            K2System.Error(_T("CGfxTerrain::RebuildTerrainChunk() - iElem > MAX_ELEMS_PER_CHUNK"));
                    }
                }
            }
        }

        // Write data into index buffer
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, chunk->uiIB);
        GLushort* dataIB((GLushort*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

        uint uiStartIndex(0);
        uint uiNumFaces(0);

        for (int n(0); n < chunk->iNumArrays; ++n)
        {
            STerrainArray *pArray(chunk->pArrays[n]);

            pArray->uiStartIndex = uiStartIndex;

            for (uint i(0); i < pArray->uiNumElems; ++i)
                dataIB[uiStartIndex + i] = pArray->pElemList[i];

            uiNumFaces += pArray->uiNumFaces;
            uiStartIndex += pArray->uiNumElems;
        }

        glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);

        chunk->uiNumFaces = uiNumFaces;

        // Fix invalid textures
        for (int n(0); n < chunk->iNumArrays; ++n)
        {
            STerrainArray *pArray(chunk->pArrays[n]);

            for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
            {
                if (pArray->ahDiffuse[iLayer] == INVALID_RESOURCE)
                {
                    pArray->ahDiffuse[iLayer] = g_ResourceManager.GetWhiteTexture();
                    pArray->ahNormalmap[iLayer] = g_ResourceManager.GetFlatTexture();
                }
            }
        }

#if 0
        if (iFlags & (TERRAIN_REBUILD_ALPHAMAP))
        {
            CBitmap bmpAlphaMap(M_CeilPow2(iChunkSize * pWorld->GetTexelDensity()), M_CeilPow2(iChunkSize * pWorld->GetTexelDensity()), BITMAP_ALPHA);
            {
                CRecti recArea
                (
                    iX * (iChunkSize * pWorld->GetTexelDensity()),
                    iY * (iChunkSize * pWorld->GetTexelDensity()),
                    (iX + 1) * (iChunkSize * pWorld->GetTexelDensity()),
                    (iY + 1) * (iChunkSize * pWorld->GetTexelDensity())
                );

                byte *pData(K2_NEW_ARRAY(ctx_GL2, byte, recArea.GetArea()));

                pWorld->GetRegion(WORLD_TEXEL_ALPHA_MAP, recArea, pData);

                byte *pSrcData(pData);
                byte *pBMPData(bmpAlphaMap.GetBuffer());

                MemManager.Set(pBMPData, 0, bmpAlphaMap.GetWidth() * bmpAlphaMap.GetHeight());

                for (int iYBMP(0); iYBMP < recArea.GetHeight(); ++iYBMP, pBMPData += bmpAlphaMap.GetWidth(), pSrcData += recArea.GetWidth())
                {
                    MemManager.Copy(pBMPData, pSrcData, recArea.GetWidth());
                }

                K2_DELETE_ARRAY(pData);
            }

            const byte *pData(bmpAlphaMap.GetBuffer());
            bool bEmpty(true);

            for (uint ui(bmpAlphaMap.GetSize()); ui != 0; --ui, ++pData)
            {
                if (*pData != 255)
                {
                    bEmpty = false;
                    break;
                }
            }

            ETextureFormat eFmt(TEXFMT_A8);

            if (bEmpty)
            {
                if (chunk->uiAlphaMap != 0)
                {
                    GfxTextures->UnregisterTexture(_T("*terrain_chunk_alphamap_") + XtoA(iX, FMT_PADZERO, 4) + _T("_") + XtoA(iY, FMT_PADZERO, 4));
                    chunk->uiAlphaMap = 0;
                }
            }
            else
            {
                if (chunk->uiAlphaMap == 0)
                {
                    tstring sTextureName(_T("*terrain_chunk_alphamap_") + XtoA(iX, FMT_PADZERO, 4) + _T("_") + XtoA(iY, FMT_PADZERO, 4));
                    chunk->uiAlphaMap = GfxTextures->Register2DTexture(bmpAlphaMap, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt, sTextureName);
                }
                else
                    GfxTextures->Update2DTexture(bmpAlphaMap, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt, chunk->uiAlphaMap);
            }
        }
#endif

        chunk->iValidityFlags = 0;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CGfxTerrain::RebuildTerrain() - "), NO_THROW);
    }
}


/*====================
  CGfxTerrain::RebuildCliffs
  ====================*/
void    CGfxTerrain::RebuildCliffs()
{
    PROFILE("CGfxTerrain::RebuildCliffs");

    float fWorldScale(pWorld->GetScale());

    // Size buffers and sort
    WorldEntList &vWorldEnts(pWorld->GetEntityList());
    for (WorldEntList_it it(vWorldEnts.begin()), itEnd(vWorldEnts.end()); it != itEnd; ++it)
    {
        if (*it == INVALID_POOL_HANDLE)
            continue;

        CWorldEntity *pWorldEnt(pWorld->GetEntityByHandle(*it));
        if (pWorldEnt == nullptr || TStringCompare(pWorldEnt->GetType(), _T("Prop_Cliff")))
            continue;

        int iTileX(INT_FLOOR(pWorldEnt->GetPosition().x / fWorldScale));
        int iTileY(INT_FLOOR(pWorldEnt->GetPosition().y / fWorldScale));

        if (iTileX < 0 || iTileX >= pWorld->GetTileWidth() ||
            iTileY < 0 || iTileY >= pWorld->GetTileHeight())
            continue;

        int iChunkX(iTileX / iChunkSize);
        int iChunkY(iTileY / iChunkSize);

        if (iChunkX < 0 || iChunkX >= iNumChunksX ||
            iChunkY < 0 || iChunkY >= iNumChunksY)
            continue;

        ResHandle hModel(g_ResourceManager.Register(pWorldEnt->GetModelPath(), RES_MODEL));

        pWorldEnt->SetModelHandle(hModel);
        g_ResourceManager.PrecacheSkin(hModel, uint(-1));
        
        CModel* pModelResource(g_ResourceManager.GetModel(hModel));
        if (pModelResource == nullptr)
            continue;

        IModel *pModel(pModelResource->GetModelFile());
        if (pModel == nullptr || pModel->GetType() != MODEL_K2)
            continue;

        CK2Model *pK2Model(static_cast<CK2Model *>(pModel));

        ResHandle hDiffuse(pWorld->GetTileDiffuseTexture(iTileX, iTileY, 0));
        ResHandle hNormalmap(pWorld->GetTileNormalmapTexture(iTileX, iTileY, 0));

        CAxis aAxis(pWorldEnt->GetAngles());

        // Add each mesh
        for (uint uiMesh(0); uiMesh < pK2Model->GetNumMeshes(); ++uiMesh)
        {
            CMesh *pMesh(pK2Model->GetMesh(uiMesh));

            CBBoxf bbBounds(pMesh->bmin, pMesh->bmax);

            bbBounds.Transform(pWorldEnt->GetPosition(), aAxis, pWorldEnt->GetScale());

            CVec3f v3Center(bbBounds.GetMid());

            int iMidTileX(INT_FLOOR(v3Center.x / fWorldScale));
            int iMidTileY(INT_FLOOR(v3Center.y / fWorldScale));

            int iMidChunkX(iMidTileX / iChunkSize);
            int iMidChunkY(iMidTileY / iChunkSize);

            if (iMidChunkX < 0 || iMidChunkX >= iNumChunksX ||
                iMidChunkY < 0 || iMidChunkY >= iNumChunksY)
                continue;

            STerrainChunk &cChunk(chunks[iMidChunkY][iMidChunkX]);

            ResHandle hMaterial(pModel->GetSkin(pWorldEnt->GetSkin())->GetMaterial(uiMesh));

            STerrainCliffArray *pArray(nullptr);

            for (int i(0); i < cChunk.iNumCliffArrays; ++i)
            {
                STerrainCliffArray *pTestArray(cChunk.pCliffArrays[i]);

                if (pTestArray->hMaterial == hMaterial &&
                    pTestArray->hDiffuse == hDiffuse &&
                    pTestArray->hNormalmap == hNormalmap)
                {
                    pArray = pTestArray;
                    break;
                }
            }

            if (pArray == nullptr && cChunk.iNumCliffArrays < 256)
            {
                // We've encountered a new shader
                pArray = cChunk.pCliffArrays[cChunk.iNumCliffArrays] = AllocTerrainCliffArray();
                
                pArray->hMaterial = hMaterial;
                pArray->hDiffuse = hDiffuse;
                pArray->hNormalmap = hNormalmap;

                cChunk.iNumCliffArrays++;
            }

            if (pArray == nullptr)
                continue;

            pArray->vCliffs.push_back(pair<CWorldEntity *, uint>(pWorldEnt, uiMesh));

            pArray->uiNumVerts += pMesh->num_verts;
            pArray->uiNumFaces += pMesh->numFaces;

            cChunk.iNumCliffVerts += pMesh->num_verts;
            cChunk.iNumCliffFaces += pMesh->numFaces;
        }
    }

    int v_offset(0);
    int n_offset(v_offset + sizeof(CVec3f));
    int t_offset(n_offset + sizeof(CVec3f));
    int tan_offset(t_offset + sizeof(CVec2f));

    for (int iChunkY(0); iChunkY < iNumChunksY; ++iChunkY)
    {
        for (int iChunkX(0); iChunkX < iNumChunksX; ++iChunkX)
        {
            STerrainChunk &cChunk(chunks[iChunkY][iChunkX]);

            if (cChunk.iNumCliffFaces == 0)
                continue;

            glBindBufferARB(GL_ARRAY_BUFFER_ARB, cChunk.uiVBCliff);
            glBufferDataARB(GL_ARRAY_BUFFER_ARB, cChunk.iNumCliffVerts * m_iCliffVertexStride, nullptr, GL_STATIC_DRAW_ARB);
            byte *pVertices((byte *)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY));

            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, cChunk.uiIBCliff);
            glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, cChunk.iNumCliffFaces * 3 * sizeof(GLushort), nullptr, GL_STATIC_DRAW_ARB);
            GLushort *pIndices((GLushort *)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

            int iStartVert(0);
            int iStartIndex(0);

            for (int i(0); i < cChunk.iNumCliffArrays; ++i)
            {
                cChunk.pCliffArrays[i]->uiStartVert = iStartVert;
                cChunk.pCliffArrays[i]->uiStartIndex = iStartIndex;

                for (vector<pair<CWorldEntity *, uint> >::iterator it(cChunk.pCliffArrays[i]->vCliffs.begin()); it != cChunk.pCliffArrays[i]->vCliffs.end(); ++it)
                {
                    CWorldEntity *pWorldEnt(it->first);
                    
                    int iTileX(INT_FLOOR(pWorldEnt->GetPosition().x / fWorldScale));
                    int iTileY(INT_FLOOR(pWorldEnt->GetPosition().y / fWorldScale));

                    if (iTileX < 0 || iTileX >= pWorld->GetTileWidth() ||
                        iTileY < 0 || iTileY >= pWorld->GetTileHeight())
                        continue;

                    CModel* pModelResource(g_ResourceManager.GetModel(pWorldEnt->GetModelHandle()));
                    if (pModelResource == nullptr)
                        continue;

                    IModel *pModel(pModelResource->GetModelFile());
                    if (pModel == nullptr || pModel->GetType() != MODEL_K2)
                        continue;

                    CK2Model *pK2Model(static_cast<CK2Model *>(pModel));

                    D3DXMATRIXA16 mWorldTranslation;
                    D3DXMATRIXA16 mWorldScaling;
                    D3DXMATRIXA16 mWorldRotation;

                    CVec3f v3Pos(pWorldEnt->GetPosition());
                    D3DXMatrixTranslation(&mWorldTranslation, v3Pos.x, v3Pos.y, v3Pos.z);

                    CAxis aAxis(CAxis(pWorldEnt->GetAngles()));

                    GfxUtils->AxisToMatrix(aAxis, &mWorldRotation);

                    if (pWorldEnt->GetScale() != 1.0f)
                        D3DXMatrixScaling(&mWorldScaling, pWorldEnt->GetScale(), pWorldEnt->GetScale(), pWorldEnt->GetScale());
                    else
                        D3DXMatrixIdentity(&mWorldScaling);

                    D3DXMATRIXA16 mWorld = mWorldScaling * mWorldRotation * mWorldTranslation;

                    uint uiMesh(it->second);
                    CMesh *pMesh(pK2Model->GetMesh(uiMesh));

                    for (int v(0); v < pMesh->num_verts; ++v)
                    {
                        {
                            CVec3f  *p_v = (CVec3f *)(&pVertices[(iStartVert + v) * m_iCliffVertexStride + v_offset]);

                            *p_v = GfxUtils->TransformPoint(pMesh->verts[v], mWorld);
                        }

                        {
                            CVec3f  *p_n = (CVec3f *)(&pVertices[(iStartVert + v) * m_iCliffVertexStride + n_offset]);

                            *p_n = GfxUtils->TransformNormal(pMesh->normals[v], mWorldRotation);
                        }

                        for (int n = 0; n < 1; ++n)
                        {
                            CVec2f  *p_t = (CVec2f *)(&pVertices[(iStartVert + v) * m_iCliffVertexStride + t_offset + (n * sizeof(CVec2f))]);

                            *p_t = pMesh->tverts[n][v];
                        }

                        for (int m = 0; m < 1; ++m)
                        {
                            CVec3f  *p_t = (CVec3f *)(&pVertices[(iStartVert + v) * m_iCliffVertexStride + tan_offset + (m * sizeof(CVec3f))]);

                            if (pMesh->tangents[m] != nullptr)
                                *p_t = GfxUtils->TransformNormal(pMesh->tangents[m][v], mWorldRotation);
                            else
                                *p_t = CVec3f(0.0f, 0.0f, 0.0f);
                        }
                    }

                    for (int n(0); n < pMesh->numFaces; ++n)
                    {
                        pIndices[iStartIndex + n * 3 + 0] = iStartVert + pMesh->faceList[n][0];
                        pIndices[iStartIndex + n * 3 + 1] = iStartVert + pMesh->faceList[n][1];
                        pIndices[iStartIndex + n * 3 + 2] = iStartVert + pMesh->faceList[n][2];
                    }
                    
                    iStartVert += pMesh->num_verts;
                    iStartIndex += pMesh->numFaces * 3;
                }
            }

            glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
            glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);
        }
    }
}


/*====================
  CGfxTerrain::AddTerrainChunks
  ====================*/
void    CGfxTerrain::AddTerrainChunks(EMaterialPhase ePhase)
{
    PROFILE("CGfxTerrain::AddTerrainChunks");

    if (ePhase == PHASE_SHADOW && !vid_terrainShadows || !vid_terrain)
        return;

    FlagVisibleTerrainChunks();

    g_RenderList.Add(K2_NEW(ctx_GL2,    CTerrainRenderer)());

#if 0
    if (vid_terrainWorldTree && ePhase != PHASE_SHADOW)
        RenderWorldTree();
#endif
}


/*====================
  CGfxTerrain::Shutdown
  ====================*/
void    CGfxTerrain::Shutdown()
{
    Destroy();

    PRINT_GLERROR_BREAK();
}


/*====================
  RebuildTerrain
  ====================*/
extern CCvar<int> terrain_chunkSize;
CMD(RebuildTerrain)
{
    GfxTerrain->Rebuild(terrain_chunkSize, nullptr);
    return true;
}
