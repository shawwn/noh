// (C)2005 S2 Games
// d3d9_terrain.cpp
//
// Direct3D terrain functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9_main.h"
#include "d3d9_terrain.h"
#include "d3d9_state.h"
#include "d3d9_util.h"
#include "d3d9_shader.h"
#include "d3d9_material.h"
#include "d3d9_scene.h"
#include "d3d9_texture.h"
#include "c_shadervar.h"
#include "c_shaderregistry.h"
#include "c_renderlist.h"

#include "../k2/c_boundingbox.h"
#include "../k2/c_camera.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_material.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_world.h"
#include "../k2/c_worldtree.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_mesh.h"
#include "../k2/c_skin.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CVAR_INTF	(vid_terrainWorldTreeLevel,	0,		CONEL_DEV);
CVAR_BOOLF	(vid_terrainWorldTree,		false,	CONEL_DEV);
CVAR_BOOLF	(vid_terrainHeightSplit,	false,	CONEL_DEV);
CVAR_BOOLF	(vid_terrainHorizon,		false,	CONEL_DEV);
CVAR_BOOLF	(vid_terrainSinglePass,		true,	CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_terrain,				true,	CONEL_DEV);

EXTERN_CVAR_BOOL(vid_drawTerrainBounds);

ResHandle	g_hTerrainMaterial(INVALID_RESOURCE);
ResHandle	g_hTerrainSingleMaterial(INVALID_RESOURCE);
ResHandle	g_hTerrainDiffuseReference(INVALID_RESOURCE);
ResHandle	g_hTerrainNormalmapReference(INVALID_RESOURCE);
ResHandle	g_hTerrainDiffuse2Reference(INVALID_RESOURCE);
ResHandle	g_hTerrainNormalmap2Reference(INVALID_RESOURCE);
ResHandle	g_hTerrainCheckerDiffuse(INVALID_RESOURCE);
ResHandle	g_hTerrainCheckerNormalmap(INVALID_RESOURCE);
bool		g_bAlpha;
int			g_iTerrainAlphaMap;
const int	g_iNumCliffTexcoords(1);
const int	g_iNumCliffTangents(1);
dword		g_dwCliffFVF;
int			g_iCliffVertexDecl;
int			g_iCliffVertexStride;
STerrain	terrain;

extern CCvar<float>		ter_ambient_r;
extern CCvar<float>		ter_ambient_g;
extern CCvar<float>		ter_ambient_b;

void	D3D_FreeChunkElems(STerrainChunk *chunk);
STerrainArray*			D3D_AllocTerrainArray();
STerrainSingleArray*	D3D_AllocTerrainSingleArray();
//=============================================================================

/*====================
  D3D_TerrainTextureCompare
  ====================*/
int		D3D_TerrainTextureCompare(ResHandle ah0[], ResHandle ah1[])
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
  D3D_RebuildTerrainIndexBuffers
  ====================*/
void	D3D_RebuildTerrainIndexBuffers(int iStartX, int iStartY, STerrainChunk *chunk)
{
	PROFILE("D3D_RebuildTerrainIndexBuffers");

	D3D_FreeChunkElems(chunk);

	const CWorld		*pWorld(terrain.pWorld);

	// start iNumArrays at 0
	// set hCurrentTexture to NULL
	// set ahTextureList to empty
	//
	// for (iTileX, iTileY) to (iTileX + chunksize, iTileY + chunksize)
	//
	//	look at the shader at (iTileX, iTileY)
	//	if this shader is different from hCurrentTexture
	//		if this shader exists in ahTextureList
	//			set the current chunk->array to the one referenced by ahTextureList for this shader
	//		else if we can't find the shader in ahTextureList
	//			allocate a new chunk->array
	//			add the current shader to ahTextureList
	//			set the current chunk->array to the one we just allocated
	//
	//	write out all vertex data for (iTileX, iTileY), (iTileX, iTileY + 1), (iTileX + 1, iTileY)...etc...
	//		for the 6 vertices that make up this terrain tile...
	//		to the current chunk->array at chunk->array->pos
	//	increment chunk->array->pos
	//
	// continue for
	//
	// notes about allocating a new chunk->array:
	// we always allocate the max possible number of vertices it could contain, so (chunksize * chunksize)

	if (!vid_terrainSinglePass)
	{
#if 0
		STerrainArray		*pCurrentArray(NULL);

		for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
		{
			pCurrentArray = NULL;

			for (int iTileY(iStartY); iTileY < iStartY + terrain.iChunkSize; ++iTileY)
			{
				for (int iTileX(iStartX); iTileX < iStartX + terrain.iChunkSize; ++iTileX)
				{
					// Test for invisible tiles
					if (!pWorld->IsTileVisible(iTileX, iTileY, iLayer))
						continue;

					ResHandle hCurrentTexture(pWorld->GetTileDiffuseTexture(iTileX, iTileY, iLayer));

					if (!pCurrentArray || hCurrentTexture != pCurrentArray->hDiffuse)
					{
						bool bFoundShader(false);

						for (int n(0); n < chunk->iNumArrays[iLayer]; ++n)
						{
							if (chunk->pArrays[iLayer][n]->hDiffuse == hCurrentTexture)
							{
								pCurrentArray = chunk->pArrays[iLayer][n];
								bFoundShader = true;
								break;
							}
						}

						if (!bFoundShader && chunk->iNumArrays[iLayer] < 256)
						{
							// We've encountered a new shader
							pCurrentArray = chunk->pArrays[iLayer][chunk->iNumArrays[iLayer]] = D3D_AllocTerrainArray();
							pCurrentArray->iNumElems = 0;
							pCurrentArray->hMaterial = pWorld->GetTileMaterial(iTileX, iTileY, iLayer);
							pCurrentArray->hDiffuse = hCurrentTexture;
							pCurrentArray->hNormalmap = pWorld->GetTileNormalmapTexture(iTileX, iTileY, iLayer);

							chunk->iNumArrays[iLayer]++;
						}
					}

					int iVertPos((iTileY - iStartY) * (terrain.iChunkSize + 1) + (iTileX - iStartX));
					int iElem(pCurrentArray->iNumElems);

					if (pWorld->GetTileSplit(iTileX, iTileY) == SPLIT_NEG)
					{
						pCurrentArray->pElemList[iElem++] = iVertPos;
						pCurrentArray->pElemList[iElem++] = iVertPos + 1;
						pCurrentArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1);

						pCurrentArray->pElemList[iElem++] = iVertPos + 1;
						pCurrentArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1) + 1;
						pCurrentArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1);
					}
					else
					{
						pCurrentArray->pElemList[iElem++] = iVertPos;
						pCurrentArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1) + 1;
						pCurrentArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1);


						pCurrentArray->pElemList[iElem++] = iVertPos;
						pCurrentArray->pElemList[iElem++] = iVertPos + 1;
						pCurrentArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1) + 1;
					}

					pCurrentArray->iNumTris += 2;

					pCurrentArray->iNumElems = iElem;

					if (iElem > MAX_ELEMS_PER_CHUNK)
						K2System.Error(_T("D3D_RebuildTerrainIndexBuffers: iElem > MAX_ELEMS_PER_CHUNK"));
				}
			}

			uint uiStartIndex(0);

			WORD *pIndices;
			if (FAILED(chunk->pIB[iLayer]->Lock(0, MAX_ELEMS_PER_CHUNK * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
				continue;

			for (int n = 0; n < chunk->iNumArrays[iLayer]; ++n)
			{
				pCurrentArray = chunk->pArrays[iLayer][n];

				pCurrentArray->uiStartIndex = uiStartIndex;

				for (uint i = 0; i < pCurrentArray->iNumElems; ++i)
					pIndices[uiStartIndex + i] = pCurrentArray->pElemList[i];

				uiStartIndex += pCurrentArray->iNumElems;
			}
			
			chunk->pIB[iLayer]->Unlock();

			if (vid_geometryPreload && !g_bD3D9Ex)
				chunk->pIB[iLayer]->PreLoad();

		}

		// ShadowIB
		WORD *pIndices;
		if (SUCCEEDED(chunk->pShadowIB->Lock(0, MAX_ELEMS_PER_CHUNK * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
		{
			WORD i = 0;
			for (int iTileY(iStartY); iTileY < iStartY + terrain.iChunkSize; ++iTileY)
			{
				for (int iTileX(iStartX); iTileX < iStartX + terrain.iChunkSize; ++iTileX)
				{
					int iVertPos = (iTileY - iStartY) * (terrain.iChunkSize + 1) + (iTileX - iStartX);

					if (pWorld->GetTileSplit(iTileX, iTileY) == SPLIT_NEG)
					{
						pIndices[i++] = iVertPos;
						pIndices[i++] = iVertPos + 1;
						pIndices[i++] = iVertPos + (terrain.iChunkSize + 1);

						pIndices[i++] = iVertPos + 1;
						pIndices[i++] = iVertPos + (terrain.iChunkSize + 1) + 1;
						pIndices[i++] = iVertPos + (terrain.iChunkSize + 1);
					}
					else
					{
						pIndices[i++] = iVertPos;
						pIndices[i++] = iVertPos + (terrain.iChunkSize + 1) + 1;
						pIndices[i++] = iVertPos + (terrain.iChunkSize + 1);

						pIndices[i++] = iVertPos;
						pIndices[i++] = iVertPos + 1;
						pIndices[i++] = iVertPos + (terrain.iChunkSize + 1) + 1;
					}
				}
			}

			chunk->pShadowIB->Unlock();

			if (vid_geometryPreload && !g_bD3D9Ex)
				chunk->pShadowIB->PreLoad();
		}
#endif
	}
	else
	{
		//
		// Setup Index buffers for the single pass terrain shader
		//

		STerrainSingleArray		*pCurrentSingleArray(NULL);

		for (int iTileY(iStartY); iTileY < iStartY + terrain.iChunkSize; iTileY += 4)
		{
			for (int iTileX(iStartX); iTileX < iStartX + terrain.iChunkSize; iTileX += 4)
			{
				// Optimize the vertex cache by rendering in 4x4 tiles.
				// We do this vertically in order to be able to reuse the last vertices on the next tile.
				// Otherwise, there would only be reuse within tiles, rather than also between tiles.
				for (int iTileX2(iTileX); iTileX2 < iTileX + 4; iTileX2++)
				{
					for (int iTileY2(iTileY); iTileY2 < iTileY + 4; iTileY2++)
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

						if (!pCurrentSingleArray || D3D_TerrainTextureCompare(ahCurrentDiffuse, pCurrentSingleArray->ahDiffuse))
						{
							bool bFoundShader(false);

							for (int n(0); n < chunk->iNumSingleArrays; ++n)
							{
								if (D3D_TerrainTextureCompare(ahCurrentDiffuse, chunk->pSingleArrays[n]->ahDiffuse) == 0)
								{
									pCurrentSingleArray = chunk->pSingleArrays[n];
									bFoundShader = true;
									break;
								}
							}

							if (!bFoundShader && chunk->iNumSingleArrays < 256)
							{
								// We've encountered a new shader
								pCurrentSingleArray = chunk->pSingleArrays[chunk->iNumSingleArrays] = D3D_AllocTerrainSingleArray();
								
								pCurrentSingleArray->iNumElems = 0;
								pCurrentSingleArray->hMaterial = pWorld->GetTileMaterial(iTileX2, iTileY2, 0);

								for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
								{
									pCurrentSingleArray->ahDiffuse[iLayer] = ahCurrentDiffuse[iLayer];
									pCurrentSingleArray->ahNormalmap[iLayer] = pWorld->GetTileNormalmapTexture(iTileX2, iTileY2, iLayer);
								}

								chunk->iNumSingleArrays++;
							}
						}

						for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
						{
							if (pCurrentSingleArray->ahDiffuse[iLayer] == INVALID_RESOURCE &&
								ahCurrentDiffuse[iLayer] != INVALID_RESOURCE)
							{
								pCurrentSingleArray->ahDiffuse[iLayer] = ahCurrentDiffuse[iLayer];
								pCurrentSingleArray->ahNormalmap[iLayer] = pWorld->GetTileNormalmapTexture(iTileX2, iTileY2, iLayer);
							}
						}

						int iVertPos((iTileY2 - iStartY) * (terrain.iChunkSize + 1) + (iTileX2 - iStartX));
						int iElem(pCurrentSingleArray->iNumElems);

						if (pWorld->GetTileSplit(iTileX2, iTileY2) == SPLIT_NEG)
						{
							pCurrentSingleArray->pElemList[iElem++] = iVertPos;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + 1;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1);

							pCurrentSingleArray->pElemList[iElem++] = iVertPos + 1;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1) + 1;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1);
						}
						else
						{
							pCurrentSingleArray->pElemList[iElem++] = iVertPos;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1) + 1;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1);


							pCurrentSingleArray->pElemList[iElem++] = iVertPos;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + 1;
							pCurrentSingleArray->pElemList[iElem++] = iVertPos + (terrain.iChunkSize + 1) + 1;
						}

						pCurrentSingleArray->iNumTris += 2;

						pCurrentSingleArray->iNumElems = iElem;

						if (iElem > MAX_ELEMS_PER_CHUNK)
							K2System.Error(_T("D3D_RebuildTerrainIndexBuffers: iElem > MAX_ELEMS_PER_CHUNK"));
					}
				}
			}
		}

		// Write data into index buffer
		uint uiStartIndex(0);
		uint uiNumFaces(0);

		WORD *pIndices;
		if (SUCCEEDED(chunk->pIBSingle->Lock(0, MAX_ELEMS_PER_CHUNK * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
		{
			for (int n(0); n < chunk->iNumSingleArrays; ++n)
			{
				STerrainSingleArray *pSingleArray(chunk->pSingleArrays[n]);

				pSingleArray->uiStartIndex = uiStartIndex;

				for (uint i(0); i < pSingleArray->iNumElems; ++i)
					pIndices[uiStartIndex + i] = pSingleArray->pElemList[i];

				uiStartIndex += pSingleArray->iNumElems;
				uiNumFaces += pSingleArray->iNumTris;
			}

			chunk->pIBSingle->Unlock();

			if (vid_geometryPreload && !g_bD3D9Ex)
				chunk->pIBSingle->PreLoad();
		}

		chunk->uiNumFaces = uiNumFaces;

		// Fix invalid textures
		for (int n(0); n < chunk->iNumSingleArrays; ++n)
		{
			STerrainSingleArray *pSingleArray(chunk->pSingleArrays[n]);

			for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
			{
				if (pSingleArray->ahDiffuse[iLayer] == INVALID_RESOURCE)
				{
					pSingleArray->ahDiffuse[iLayer] = g_ResourceManager.GetWhiteTexture();
					pSingleArray->ahNormalmap[iLayer] = g_ResourceManager.GetFlatTexture();
				}
			}
		}
	}
}


/*====================
  D3D_RebuildTerrainChunk

  rebuild a square block of the terrain at the given layer
  assumes space has already been allocated for holding the
  vertex data this isn't meant to be particularly fast, but
  should be fast enough to keep an interactive framerate in
  the editor when modifying terrain
  ====================*/
void	D3D_RebuildTerrainChunk(int iChunkX, int iChunkY)
{
	PROFILE("D3D_RebuildTerrainChunk");

	try
	{
		STerrainChunk	*chunk(&terrain.chunks[iChunkY][iChunkX]);
		int				iFlags(chunk->iValidityFlags);
		const CWorld	*pWorld(terrain.pWorld);

		if (!iFlags)
			return;

		int iStartX(iChunkX * terrain.iChunkSize);		// Grid top-left tile coord
		int iStartY(iChunkY * terrain.iChunkSize);		// Grid top-left tile coord

		// Rebuild chunk vertices
		if (iFlags & (TERRAIN_REBUILD_VERTICES|TERRAIN_REBUILD_COLORS|TERRAIN_REBUILD_TEXCOORDS|TERRAIN_REBUILD_NORMALS))
		{
			STerrainVertex *pVertices;

			if (FAILED(chunk->pVB->Lock(0, VERTS_PER_CHUNK * sizeof(STerrainVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
				return;

			int iVertPos(0);
			float fMinZ(FAR_AWAY);
			float fMaxZ(-FAR_AWAY);

#if 0
			float fInc(1.0f / pWorld->GetTextureScale());
			float fTexelInc(1.0f / terrain.iChunkSize);
			float fTileSize(pWorld->GetScale());

			for (int iTileY = iStartY; iTileY <= iStartY + terrain.iChunkSize; ++iTileY)
			{
				float fCycleY(iTileY * fInc);
				float fTexelY((iTileY - iStartY) * fTexelInc);

				for (int iTileX = iStartX; iTileX <= iStartX + terrain.iChunkSize; ++iTileX)
				{
					float fCycleX(iTileX * fInc);
					float fTexelX((iTileX - iStartX) * fTexelInc);

					// Vertex position
					float fWorldX((iTileX - iStartX) * fTileSize);
					float fWorldY((iTileY - iStartY) * fTileSize);
					float fWorldZ(pWorld->GetGridPoint(iTileX, iTileY));

					pVertices[iVertPos].v = CVec3f(fWorldX, fWorldY, fWorldZ);
					// Normal
					pVertices[iVertPos].n = pWorld->GetGridNormal(iTileX, iTileY);

					// Tangent
					pVertices[iVertPos].tangent = pWorld->GetGridTangent(iTileX, iTileY);

					// Texcoords
					pVertices[iVertPos].t0 = CVec2f(fCycleX, fCycleY);
					pVertices[iVertPos].t1 = CVec2f(fTexelX, fTexelY);

					// Color
					CVec4b v4Color(pWorld->GetGridColor(iTileX, iTileY));
					pVertices[iVertPos].color = D3DCOLOR_ARGB(v4Color[A], v4Color[R], v4Color[G], v4Color[B]);

					if (fWorldZ < fMinZ)
						fMinZ = fWorldZ;
					if (fWorldZ > fMaxZ)
						fMaxZ = fWorldZ;

					++iVertPos;
				}
			}
#else
			for (int iTileY = iStartY; iTileY <= iStartY + terrain.iChunkSize; ++iTileY)
			{
				int iTileDeltaY(iTileY - iStartY);

				for (int iTileX = iStartX; iTileX <= iStartX + terrain.iChunkSize; ++iTileX)
				{
					int iTileDeltaX(iTileX - iStartX);

					float fWorldZ(pWorld->GetGridPoint(iTileX, iTileY));

					// Height
					pVertices[iVertPos].height = fWorldZ;
					
					// Color
					CVec4b v4Color(pWorld->GetGridColor(iTileX, iTileY));
					pVertices[iVertPos].color = D3DCOLOR_ARGB(v4Color[A], v4Color[R], v4Color[G], v4Color[B]);
					
					// Normal
					const CVec3f &v3Normal(pWorld->GetGridNormal(iTileX, iTileY));
					CVec3b n
					(
						BYTE_ROUND((v3Normal.x + 1.0f) * 0.5f * 255.0f),
						BYTE_ROUND((v3Normal.y + 1.0f) * 0.5f * 255.0f),
						BYTE_ROUND(v3Normal.z * 255.0f)
					);

					pVertices[iVertPos].data0 = CVec4b(n.x, n.y, n.z, iTileDeltaX);

					// Tangent
					const CVec3f &v3Tangent(pWorld->GetGridTangent(iTileX, iTileY));
					CVec3b t
					(
						BYTE_ROUND((v3Tangent.x + 1.0f) * 0.5f * 255.0f),
						BYTE_ROUND((v3Tangent.y + 1.0f) * 0.5f * 255.0f),
						BYTE_ROUND((v3Tangent.z + 1.0f) * 0.5f * 255.0f)
					);

					pVertices[iVertPos].data1 = CVec4b(t.x, t.y, t.z, iTileDeltaY);

					if (fWorldZ < fMinZ)
						fMinZ = fWorldZ;
					if (fWorldZ > fMaxZ)
						fMaxZ = fWorldZ;

					++iVertPos;
				}
			}
#endif

			chunk->bbBounds = CBBoxf
			(
				CVec3f(pWorld->ScaleGridCoord(iStartX), pWorld->ScaleGridCoord(iStartY), fMinZ),
				CVec3f(pWorld->ScaleGridCoord(iStartX + terrain.iChunkSize), pWorld->ScaleGridCoord(iStartY + terrain.iChunkSize), fMaxZ)
			);

			chunk->pVB->Unlock();

			if (vid_geometryPreload && !g_bD3D9Ex)
				chunk->pVB->PreLoad();
		}

		if (iFlags & (TERRAIN_REBUILD_SHADERS))
			D3D_RebuildTerrainIndexBuffers(iStartX, iStartY, chunk);

		if (iFlags & (TERRAIN_REBUILD_VERTICES|TERRAIN_REBUILD_NORMALS))
		{
			SAFE_RELEASE(chunk->pVBTileNormals);
			SAFE_RELEASE(chunk->pVBVertexNormals);
			SAFE_RELEASE(chunk->pVBGrid);
			SAFE_RELEASE(chunk->pIBGrid);
		}

		if (iFlags & (TERRAIN_REBUILD_ALPHAMAP) && vid_terrainAlphamap)
		{
			CBitmap bmpAlphaMap(M_CeilPow2(terrain.iChunkSize * pWorld->GetTexelDensity()), M_CeilPow2(terrain.iChunkSize * pWorld->GetTexelDensity()), BITMAP_ALPHA);
			{
				CRecti recArea
				(
					iChunkX * (terrain.iChunkSize * pWorld->GetTexelDensity()),
					iChunkY * (terrain.iChunkSize * pWorld->GetTexelDensity()),
					(iChunkX + 1) * (terrain.iChunkSize * pWorld->GetTexelDensity()),
					(iChunkY + 1) * (terrain.iChunkSize * pWorld->GetTexelDensity())
				);

				byte *pData(K2_NEW_ARRAY(ctx_D3D9, byte, recArea.GetArea()));

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

			ETextureFormat eFmt(TEXFMT_A8R8G8B8);

			if (g_DeviceCaps.bA8)
				eFmt = TEXFMT_A8;
			else if (g_DeviceCaps.bA8L8)
				eFmt = TEXFMT_A8L8;

			if (bEmpty)
			{
				if (chunk->iAlphaMap != -1)
				{
					D3D_Unregister2DTexture(_T("$terrain_chunk_alphamap_") + XtoA(iChunkX, FMT_PADZERO, 4) + _T("_") + XtoA(iChunkY, FMT_PADZERO, 4));
					chunk->iAlphaMap = -1;
				}
			}
			else
			{
				if (chunk->iAlphaMap == -1)
				{
					tstring sTextureName(_T("$terrain_chunk_alphamap_") + XtoA(iChunkX, FMT_PADZERO, 4) + _T("_") + XtoA(iChunkY, FMT_PADZERO, 4));
					chunk->iAlphaMap = D3D_Register2DTexture(bmpAlphaMap, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt, sTextureName);
				}
				else
					D3D_Update2DTexture(chunk->iAlphaMap, bmpAlphaMap, TEX_FULL_QUALITY | TEX_NO_COMPRESS | TEX_NO_MIPMAPS, eFmt);
			}
		}

		chunk->iValidityFlags = 0;
	}
	catch (CException &ex)
	{
		ex.Process(_T("D3D_RebuildTerrain() - "), NO_THROW);
	}
}


/*====================
  D3D_AllocateTerrainChunk
  ====================*/
void	D3D_AllocateTerrainChunk(int iX, int iY)
{
	PROFILE("D3D_AllocateTerrainChunk");

	STerrainChunk	*chunk(&terrain.chunks[iY][iX]);

	MemManager.Set(chunk, 0, sizeof(STerrainChunk));

	// Create vertex buffer
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(VERTS_PER_CHUNK * sizeof(STerrainVertex),
				g_d3dManagedUsage | D3DUSAGE_WRITEONLY, 0, g_d3dManagedPool, &chunk->pVB, NULL)))
		K2System.Error(_T("D3D_AllocateTerrainChunk(): CreateVertexBuffer failed"));

#if 0
	if (!vid_terrainSinglePass)
	{
		// Create color phase index buffer
		for (int iLayer(0); iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
		{
			if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_ELEMS_PER_CHUNK * sizeof(WORD),
				g_d3dManagedUsage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, g_d3dManagedPool, &chunk->pIB[iLayer], NULL)))
				K2System.Error(_T("D3D_AllocateTerrainChunk(): CreateIndexBuffer failed"));
		}

		// Create shadow phase index buffer
		if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_ELEMS_PER_CHUNK * sizeof(WORD),
			g_d3dManagedUsage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, g_d3dManagedPool, &chunk->pShadowIB, NULL)))
			K2System.Error(_T("D3D_AllocateTerrainChunk(): CreateIndexBuffer failed"));
	}
	else
#endif
	{
		// Create single pass color phase index buffer
		if (FAILED(g_pd3dDevice->CreateIndexBuffer(MAX_ELEMS_PER_CHUNK * sizeof(WORD),
			g_d3dManagedUsage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, g_d3dManagedPool, &chunk->pIBSingle, NULL)))
			K2System.Error(_T("D3D_AllocateTerrainChunk(): CreateIndexBuffer failed"));
	}

	chunk->iAlphaMap = -1;

	chunk->iValidityFlags =	TERRAIN_REBUILD_VERTICES |
							TERRAIN_REBUILD_TEXCOORDS |
							TERRAIN_REBUILD_COLORS |
							TERRAIN_REBUILD_NORMALS |
							TERRAIN_REBUILD_SHADERS |
							TERRAIN_REBUILD_ALPHAMAP;

	D3D_RebuildTerrainChunk(iX, iY);
}


/*====================
  D3D_RebuildTerrain
  ====================*/
void	D3D_RebuildTerrain(int iChunkSize, const CWorld *pWorld)
{
	PROFILE("D3D_RebuildTerrain");

	try
	{
		D3D_InitTerrain();

		if (iChunkSize > pWorld->GetTileWidth())
			iChunkSize = pWorld->GetTileWidth();
		if (iChunkSize > pWorld->GetTileHeight())
			iChunkSize = pWorld->GetTileHeight();

		if (iChunkSize == 0)
			EX_ERROR(_T("iChunkSize is 0, division by 0 will result."));

		D3D_DestroyTerrain();

		if (pWorld->GetTileWidth() % iChunkSize != 0)
			EX_ERROR(_T("World width ") + XtoA(pWorld->GetTileWidth()) + _T(" is not a multiple of iChunkSize ") + XtoA(iChunkSize));
		if (pWorld->GetTileHeight() % iChunkSize != 0)
			EX_ERROR(_T("World height ") + XtoA(pWorld->GetTileHeight()) + _T(" is not a multiple of iChunkSize ") + XtoA(iChunkSize));

		terrain.pWorld = pWorld;
		terrain.iNumChunksX = pWorld->GetGridWidth() / iChunkSize;
		terrain.iNumChunksY = pWorld->GetGridHeight() / iChunkSize;
		terrain.iChunkSize = iChunkSize;

		for (int iY = 0; iY < terrain.iNumChunksY; ++iY)
		{
			for (int iX = 0; iX < terrain.iNumChunksX; ++iX)
				D3D_AllocateTerrainChunk(iX, iY);
		}
	}
	catch (CException &ex)
	{
		ex.Process(_T("D3D_RebuildTerrain() - "), NO_THROW);
	}
}


/*====================
  D3D_AllocTerrainArray
  ====================*/
STerrainArray*	D3D_AllocTerrainArray()
{
	STerrainArray *pNewArray(K2_NEW(ctx_D3D9,   STerrainArray));
	MemManager.Set(pNewArray, 0, sizeof(STerrainArray));
	pNewArray->iNumElems = 0;
	pNewArray->pElemList = K2_NEW_ARRAY(ctx_D3D9, uint, MAX_ELEMS_PER_CHUNK);
	MemManager.Set(pNewArray->pElemList, 0, sizeof(MAX_ELEMS_PER_CHUNK * sizeof(int)));

	return pNewArray;
}


/*====================
  D3D_AllocTerrainSingleArray
  ====================*/
STerrainSingleArray*	D3D_AllocTerrainSingleArray()
{
	STerrainSingleArray *pNewArray(K2_NEW(ctx_D3D9,   STerrainSingleArray));
	MemManager.Set(pNewArray, 0, sizeof(STerrainSingleArray));
	pNewArray->iNumElems = 0;
	pNewArray->pElemList = K2_NEW_ARRAY(ctx_D3D9, uint, MAX_ELEMS_PER_CHUNK);
	MemManager.Set(pNewArray->pElemList, 0, sizeof(MAX_ELEMS_PER_CHUNK * sizeof(int)));

	return pNewArray;
}


/*====================
  D3D_AllocTerrainCliffArray
  ====================*/
STerrainCliffArray*	D3D_AllocTerrainCliffArray()
{
	STerrainCliffArray *pNewArray(K2_NEW(ctx_D3D9,   STerrainCliffArray));
	MemManager.Set(pNewArray, 0, sizeof(STerrainCliffArray));

	return pNewArray;
}


/*====================
  D3D_FreeChunkElems
  ====================*/
void	D3D_FreeChunkElems(STerrainChunk *chunk)
{
#if 0
	for (int iLayer = 0; iLayer < NUM_TERRAIN_LAYERS; ++iLayer)
	{
		for (int n = 0; n < chunk->iNumArrays[iLayer]; ++n)
		{
			K2_DELETE_ARRAY(chunk->pArrays[iLayer][n]->pElemList);
			K2_DELETE(chunk->pArrays[iLayer][n]);
		}

		chunk->iNumArrays[iLayer] = 0;
	}
#endif

	for (int n(0); n < chunk->iNumSingleArrays; ++n)
	{
		K2_DELETE_ARRAY(chunk->pSingleArrays[n]->pElemList);
		K2_DELETE(chunk->pSingleArrays[n]);
	}

	chunk->iNumSingleArrays = 0;

	for (int n(0); n < chunk->iNumCliffArrays; ++n)
		K2_DELETE(chunk->pCliffArrays[n]);

	chunk->iNumCliffArrays = 0;
}


/*====================
  D3D_DestroyTerrainChunk
  ====================*/
void	D3D_DestroyTerrainChunk(int iX, int iY)
{
	STerrainChunk *chunk = &terrain.chunks[iY][iX];

	D3D_FreeChunkElems(chunk);

	SAFE_RELEASE(chunk->pVB);
	//SAFE_RELEASE(chunk->pShadowIB);
	SAFE_RELEASE(chunk->pVBVertexNormals);
	SAFE_RELEASE(chunk->pVBTileNormals);
	SAFE_RELEASE(chunk->pVBGrid);
	SAFE_RELEASE(chunk->pIBGrid);

	SAFE_RELEASE(chunk->pVBCliff);
	SAFE_RELEASE(chunk->pIBCliff);

	SAFE_RELEASE(chunk->pIBSingle);

	D3D_Unregister2DTexture(_T("$terrain_chunk_alphamap_") + XtoA(iX, FMT_PADZERO, 4) + _T("_") + XtoA(iY, FMT_PADZERO, 4));

	MemManager.Set(chunk, 0, sizeof(STerrainChunk));
}


/*====================
  D3D_DestroyTerrain
  ====================*/
void	D3D_DestroyTerrain()
{
	for (int iY = 0; iY < terrain.iNumChunksY; ++iY)
	{
		for (int iX = 0; iX < terrain.iNumChunksX; ++iX)
			D3D_DestroyTerrainChunk(iX, iY);
	}

	terrain.pWorld = NULL;
	terrain.iChunkSize = 0;
	terrain.iNumChunksX = 0;
	terrain.iNumChunksY = 0;
}


/*====================
  D3D_InvalidateTerrainVertex
  ====================*/
void	D3D_InvalidateTerrainVertex(int iTileX, int iTileY, int iFlags)
{
	int iChunkX, iChunkY;

	if (!terrain.iChunkSize || iTileX < 0 || iTileY < 0 || iTileX >= terrain.pWorld->GetGridWidth() || iTileY >= terrain.pWorld->GetGridHeight())
		return;

	iChunkX = (iTileX) / terrain.iChunkSize;
	iChunkY = (iTileY) / terrain.iChunkSize;

	// check if we're right on a chunk border,
	// in which case we'll have to invalidate more than one chunk
	if (iTileX % terrain.iChunkSize == 0 && iChunkX > 0 && iChunkY < terrain.iNumChunksY)
		terrain.chunks[iChunkY][iChunkX - 1].iValidityFlags |= iFlags;

	if (iTileY % terrain.iChunkSize == 0 && iChunkY > 0 && iChunkX < terrain.iNumChunksX)
		terrain.chunks[iChunkY - 1][iChunkX].iValidityFlags |= iFlags;

	if (iTileY % terrain.iChunkSize == 0 && iTileX % terrain.iChunkSize == 0 && iChunkY > 0 && iChunkX > 0)
		terrain.chunks[iChunkY - 1][iChunkX - 1].iValidityFlags |= iFlags;

	if (iChunkX < terrain.iNumChunksX && iChunkY < terrain.iNumChunksY)
		terrain.chunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  D3D_InvalidateTerrainTile
  ====================*/
void	D3D_InvalidateTerrainTile(int iTileX, int iTileY, int iFlags)
{
	int iChunkX, iChunkY;

	if (!terrain.iChunkSize || iTileX < 0 || iTileY < 0 || iTileX >= terrain.pWorld->GetTileWidth() || iTileY >= terrain.pWorld->GetTileHeight())
		return;

	iChunkX = (iTileX) / terrain.iChunkSize;
	iChunkY = (iTileY) / terrain.iChunkSize;

	if (iChunkX < 0 || iChunkX >= terrain.iNumChunksX ||
		iChunkY < 0 || iChunkY >= terrain.iNumChunksX)
		return;

	terrain.chunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  D3D_InvalidateTerrainTexel
  ====================*/
void	D3D_InvalidateTerrainTexel(int iTexelX, int iTexelY, int iFlags)
{
	int iChunkX, iChunkY;

	if (!terrain.iChunkSize || iTexelX < 0 || iTexelY < 0 || iTexelX >= terrain.pWorld->GetTexelWidth() || iTexelY >= terrain.pWorld->GetTexelHeight())
		return;

	iChunkX = (iTexelX) / (terrain.iChunkSize * terrain.pWorld->GetTexelDensity());
	iChunkY = (iTexelY) / (terrain.iChunkSize * terrain.pWorld->GetTexelDensity());

	if (iChunkX < 0 || iChunkX >= terrain.iNumChunksX ||
		iChunkY < 0 || iChunkY >= terrain.iNumChunksX)
		return;

	terrain.chunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  D3D_InvalidateTerrainLayer
  ====================*/
void	D3D_InvalidateTerrainLayer(int iFlags)
{
	for (int iY(0); iY < terrain.iNumChunksY; ++iY)
		for (int iX(0); iX < terrain.iNumChunksX; ++iX)
			terrain.chunks[iY][iX].iValidityFlags |= iFlags;
}


/*====================
  D3D_FlagVisibleTerrainChunks
  ====================*/
void	D3D_FlagVisibleTerrainChunks()
{
	PROFILE("D3D_FlagVisibleTerrainChunks");

	for (int iY(0); iY < terrain.iNumChunksY; ++iY)
	{
		for (int iX(0); iX < terrain.iNumChunksX; ++iX)
		{
			STerrainChunk &cChunk(terrain.chunks[iY][iX]);

			if (cChunk.iValidityFlags)
				D3D_RebuildTerrainChunk(iX, iY);

			cChunk.bVisible = SceneManager.AABBIsVisible(cChunk.bbBounds);
		}
	}
}


/*====================
  D3D_FlagVisibleTerrainChunksShadow
  ====================*/
void	D3D_FlagVisibleTerrainChunksShadow(const CConvexPolyhedron &cScene)
{
	PROFILE("D3D_FlagVisibleTerrainChunksShadow");

	CVec3f v3Start(0.0f, 0.0f, 0.0f);
	CVec3f v3End(SceneManager.GetSunPos() * -100000.0f);

	bool bShadowFalloff(vid_shadowFalloff && !g_pCam->HasFlags(CAM_SHADOW_NO_FALLOFF));

	for (int iY(0); iY < terrain.iNumChunksY; ++iY)
	{
		for (int iX(0); iX < terrain.iNumChunksX; ++iX)
		{
			STerrainChunk &cChunk(terrain.chunks[iY][iX]);

			float fFraction(1.0f);
			cChunk.bVisibleShadow = (cChunk.bVisible && !bShadowFalloff) || I_MovingBoundsSurfaceIntersect(v3Start, v3End, cChunk.bbBounds, cScene, fFraction);
		}
	}
}


/*====================
  D3D_TerrainBounds
  ====================*/
void	D3D_TerrainBounds(CBBoxf &bbTerrain)
{
	for (int iY(0); iY < terrain.iNumChunksY; ++iY)
	{
		for (int iX(0); iX < terrain.iNumChunksX; ++iX)
		{
			STerrainChunk *chunk = &terrain.chunks[iY][iX];

			if (chunk->bVisible)
				bbTerrain.AddBox(chunk->bbBounds);
		}
	}
}


/*====================
  D3D_RenderWorldTree
  ====================*/
void	D3D_RenderWorldTree()
{
	CWorldTreeNode	*pWorldTree(terrain.pWorld->GetWorldTree().GetWorldTree(vid_terrainWorldTreeLevel));

	int iSize(M_Power(2, vid_terrainWorldTreeLevel));

	for (int i(0); i < iSize; ++i)
		D3D_AddBox(pWorldTree[i].GetBounds(), CVec4f(0.0f, 1.0f, 0.0f, 1.0f), g_mWorld);
}


/*====================
  D3D_AddTerrainChunks
  ====================*/
void	D3D_AddTerrainChunks()
{
	PROFILE("D3D_AddTerrainChunks");

	if (!vid_terrain || !terrain.pWorld)
		return;

	g_RenderList.Add(K2_NEW(ctx_D3D9,   CTerrainRenderer)());

	if (vid_terrainWorldTree)
		D3D_RenderWorldTree();
}


/*====================
  D3D_InitTerrain
  ====================*/
void	D3D_InitTerrain()
{
	g_hTerrainCheckerDiffuse = g_ResourceManager.Register(_T("$white"), RES_TEXTURE);
	g_hTerrainCheckerNormalmap = g_ResourceManager.Register(_T("$flat_matte"), RES_TEXTURE);

	g_hTerrainDiffuseReference = g_ResourceManager.Register(_T("!terrain_d"), RES_REFERENCE);
	g_hTerrainNormalmapReference = g_ResourceManager.Register(_T("!terrain_n"), RES_REFERENCE);
	g_hTerrainDiffuse2Reference = g_ResourceManager.Register(_T("!terrain_d2"), RES_REFERENCE);
	g_hTerrainNormalmap2Reference = g_ResourceManager.Register(_T("!terrain_n2"), RES_REFERENCE);
		
	g_ResourceManager.UpdateReference(g_hTerrainDiffuseReference, INVALID_RESOURCE);
	g_ResourceManager.UpdateReference(g_hTerrainNormalmapReference, INVALID_RESOURCE);

	g_hTerrainMaterial = g_ResourceManager.Register(_T("/world/terrain/materials/default.material"), RES_MATERIAL);
	g_hTerrainSingleMaterial = g_ResourceManager.Register(_T("/world/terrain/materials/default_single.material"), RES_MATERIAL);

	//
	// Cliff vertexes
	//

	g_dwCliffFVF = D3DFVF_XYZ;
	g_iCliffVertexStride = 12;

	g_dwCliffFVF |= D3DFVF_NORMAL4B;
	g_iCliffVertexStride += 4;

	for (int k = 0; k < g_iNumCliffTangents; ++k)
		g_dwCliffFVF |= D3DFVF_TEXCOORDSIZE1((g_iNumCliffTexcoords + k));

	g_dwCliffFVF |= (g_iNumCliffTexcoords + g_iNumCliffTangents) << D3DFVF_TEXCOUNT_SHIFT;
	g_iCliffVertexStride += g_iNumCliffTexcoords * sizeof(CVec2f);
	g_iCliffVertexStride += g_iNumCliffTangents * sizeof(CVec4b);

	g_iCliffVertexDecl = D3D_RegisterVertexDeclaration(g_dwCliffFVF);
}


/*====================
  D3D_RebuildCliffs
  ====================*/
void	D3D_RebuildCliffs()
{
	PROFILE("D3D_RebuildCliffs");

	float fWorldScale(terrain.pWorld->GetScale());

	// Size buffers and sort
	WorldEntList &vWorldEnts(terrain.pWorld->GetEntityList());
	for (WorldEntList_it it(vWorldEnts.begin()), itEnd(vWorldEnts.end()); it != itEnd; ++it)
	{
		if (*it == INVALID_POOL_HANDLE)
			continue;

		CWorldEntity *pWorldEnt(terrain.pWorld->GetEntityByHandle(*it));
		if (pWorldEnt == NULL || TStringCompare(pWorldEnt->GetType(), _T("Prop_Cliff")))
			continue;

		int iTileX(INT_FLOOR(pWorldEnt->GetPosition().x / fWorldScale));
		int iTileY(INT_FLOOR(pWorldEnt->GetPosition().y / fWorldScale));

		if (iTileX < 0 || iTileX >= terrain.pWorld->GetTileWidth() ||
			iTileY < 0 || iTileY >= terrain.pWorld->GetTileHeight())
			continue;

		int iChunkX(iTileX / terrain.iChunkSize);
		int iChunkY(iTileY / terrain.iChunkSize);

		if (iChunkX < 0 || iChunkX >= terrain.iNumChunksX ||
			iChunkY < 0 || iChunkY >= terrain.iNumChunksY)
			continue;

		ResHandle hModel(g_ResourceManager.Register(pWorldEnt->GetModelPath(), RES_MODEL));

		pWorldEnt->SetModelHandle(hModel);
		g_ResourceManager.PrecacheSkin(hModel, uint(-1));
		
		CModel* pModelResource(g_ResourceManager.GetModel(hModel));
		if (pModelResource == NULL)
			continue;

		IModel *pModel(pModelResource->GetModelFile());
		if (pModel == NULL || pModel->GetType() != MODEL_K2)
			continue;

		CK2Model *pK2Model(static_cast<CK2Model *>(pModel));

		ResHandle hDiffuse(terrain.pWorld->GetTileDiffuseTexture(iTileX, iTileY, 0));
		ResHandle hNormalmap(terrain.pWorld->GetTileNormalmapTexture(iTileX, iTileY, 0));

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

			int iMidChunkX(iMidTileX / terrain.iChunkSize);
			int iMidChunkY(iMidTileY / terrain.iChunkSize);

			if (iMidChunkX < 0 || iMidChunkX >= terrain.iNumChunksX ||
				iMidChunkY < 0 || iMidChunkY >= terrain.iNumChunksY)
				continue;

			STerrainChunk &cChunk(terrain.chunks[iMidChunkY][iMidChunkX]);

			ResHandle hMaterial(pModel->GetSkin(pWorldEnt->GetSkin())->GetMaterial(uiMesh));

			STerrainCliffArray *pArray(NULL);

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

			if (pArray == NULL && cChunk.iNumCliffArrays < 256)
			{
				// We've encountered a new shader
				pArray = cChunk.pCliffArrays[cChunk.iNumCliffArrays] = D3D_AllocTerrainCliffArray();
				
				pArray->hMaterial = hMaterial;
				pArray->hDiffuse = hDiffuse;
				pArray->hNormalmap = hNormalmap;

				cChunk.iNumCliffArrays++;
			}

			if (pArray == NULL)
				continue;

			pArray->vCliffs.push_back(pair<CWorldEntity *, uint>(pWorldEnt, uiMesh));

			pArray->uiNumVerts += pMesh->num_verts;
			pArray->uiNumFaces += pMesh->numFaces;

			cChunk.iNumCliffVerts += pMesh->num_verts;
			cChunk.iNumCliffFaces += pMesh->numFaces;
		}
	}

	byte *pVertices(NULL);
	WORD *pIndices(NULL);

	int v_offset(0);
	int n_offset(v_offset + sizeof(CVec3f));
	int t_offset(n_offset + sizeof(CVec4b));
	int tan_offset(t_offset + (g_iNumCliffTexcoords * sizeof(vec2_t)));

	for (int iChunkY(0); iChunkY < terrain.iNumChunksY; ++iChunkY)
	{
		for (int iChunkX(0); iChunkX < terrain.iNumChunksX; ++iChunkX)
		{
			STerrainChunk &cChunk(terrain.chunks[iChunkY][iChunkX]);

			SAFE_RELEASE(cChunk.pVBCliff);
			SAFE_RELEASE(cChunk.pIBCliff);

			if (cChunk.iNumCliffFaces == 0)
				continue;

			// Create vertex buffer
			if (FAILED(g_pd3dDevice->CreateVertexBuffer(cChunk.iNumCliffVerts * g_iCliffVertexStride,
				D3DUSAGE_WRITEONLY, 0, g_d3dManagedPool, &cChunk.pVBCliff, NULL)))
				K2System.Error(_T("D3D_RebuildCliffs(): CreateVertexBuffer failed"));

			if (FAILED(cChunk.pVBCliff->Lock(0, cChunk.iNumCliffVerts * g_iCliffVertexStride, (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
				continue;

			// Create index buffer
			if (FAILED(g_pd3dDevice->CreateIndexBuffer(cChunk.iNumCliffFaces * 3 * sizeof(WORD),
				g_d3dManagedUsage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, g_d3dManagedPool, &cChunk.pIBCliff, NULL)))
				K2System.Error(_T("D3D_RebuildCliffs(): CreateIndexBuffer failed"));

			if (FAILED(cChunk.pIBCliff->Lock(0, cChunk.iNumCliffFaces * 3 * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
			{
				cChunk.pVBCliff->Unlock();
				SAFE_RELEASE(cChunk.pVBCliff);
				continue;
			}

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

					if (iTileX < 0 || iTileX >= terrain.pWorld->GetTileWidth() ||
						iTileY < 0 || iTileY >= terrain.pWorld->GetTileHeight())
						continue;

					CModel* pModelResource(g_ResourceManager.GetModel(pWorldEnt->GetModelHandle()));
					if (pModelResource == NULL)
						continue;

					IModel *pModel(pModelResource->GetModelFile());
					if (pModel == NULL || pModel->GetType() != MODEL_K2)
						continue;

					CK2Model *pK2Model(static_cast<CK2Model *>(pModel));

					D3DXMATRIXA16 mWorldTranslation;
					D3DXMATRIXA16 mWorldScaling;
					D3DXMATRIXA16 mWorldRotation;

					CVec3f v3Pos(pWorldEnt->GetPosition());
					D3DXMatrixTranslation(&mWorldTranslation, v3Pos.x, v3Pos.y, v3Pos.z);

					CAxis aAxis(CAxis(pWorldEnt->GetAngles()));

					D3D_AxisToMatrix(aAxis, &mWorldRotation);

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
							CVec3f	*p_v = (CVec3f *)(&pVertices[(iStartVert + v) * g_iCliffVertexStride + v_offset]);

							*p_v = D3D_TransformPoint(pMesh->verts[v], mWorld);
						}

						{
							CVec4b	*p_n = (CVec4b *)(&pVertices[(iStartVert + v) * g_iCliffVertexStride + n_offset]);

							CVec3f v3Normal(D3D_TransformNormal(pMesh->normals[v], mWorldRotation));

							*p_n = CVec4b(
							BYTE_ROUND((v3Normal[0] + 1.0f) * 0.5f * 255.0f),
							BYTE_ROUND((v3Normal[1] + 1.0f) * 0.5f * 255.0f),
							BYTE_ROUND((v3Normal[2] + 1.0f) * 0.5f * 255.0f),
							0);
						}

						for (int n = 0; n < g_iNumCliffTexcoords; ++n)
						{
							vec2_t	*p_t = (vec2_t *)(&pVertices[(iStartVert + v) * g_iCliffVertexStride + t_offset + (n * sizeof(vec2_t))]);

							if (pMesh->tverts[n] != NULL)
								M_CopyVec2(pMesh->tverts[n][v], *p_t);
							else
								M_SetVec2(*p_t, 0.0f, 0.0f);
						}

						for (int m = 0; m < g_iNumCliffTangents; ++m)
						{
							CVec4b	*p_t = (CVec4b *)(&pVertices[(iStartVert + v) * g_iCliffVertexStride + tan_offset + (m * sizeof(CVec4b))]);

							if (pMesh->tangents[m] != NULL)
							{
								CVec3f v3Tangent(D3D_TransformNormal(pMesh->tangents[m][v], mWorldRotation));

								if (pMesh->signs[m] != NULL)
								{
									*p_t = CVec4b(
									BYTE_ROUND((v3Tangent[0] + 1.0f) * 0.5 * 255.0f),
									BYTE_ROUND((v3Tangent[1] + 1.0f) * 0.5 * 255.0f),
									BYTE_ROUND((v3Tangent[2] + 1.0f) * 0.5 * 255.0f),
									pMesh->signs[m][v]);
								}
								else
								{
									*p_t = CVec4b(
									BYTE_ROUND((v3Tangent[0] + 1.0f) * 0.5 * 255.0f),
									BYTE_ROUND((v3Tangent[1] + 1.0f) * 0.5 * 255.0f),
									BYTE_ROUND((v3Tangent[2] + 1.0f) * 0.5 * 255.0f),
									255);
								}
							}
							else
								*p_t = CVec4b(0, 0, 0, 0);
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

			cChunk.pVBCliff->Unlock();
			cChunk.pIBCliff->Unlock();

			if (vid_geometryPreload && !g_bD3D9Ex)
			{
				cChunk.pVBCliff->PreLoad();
				cChunk.pIBCliff->PreLoad();
			}
		}
	}
}


/*====================
  RebuildTerrain
  ====================*/
CMD(RebuildTerrain)
{
	D3D_RebuildTerrain(terrain_chunkSize, terrain.pWorld);
	return true;
}


/*====================
  PrintTerrainSize
  ====================*/
CMD(PrintTerrainSize)
{
	uint uiVertexBuffer(0);
	uint uiIndexBuffer(0);
	uint uiAlphaMap(0);
	uint uiBatches(0);
	uint uiCliffVertexBuffer(0);
	uint uiCliffIndexBuffer(0);
	uint uiCliffBatches(0);

	uint uiTexelSize(4);
	if (g_DeviceCaps.bA8)
		uiTexelSize = 1;
	else if (g_DeviceCaps.bA8L8)
		uiTexelSize = 2;

	if (terrain.pWorld)
	{
		for (int x(0); x < terrain.iNumChunksX; ++x)
		{
			for (int y(0); y < terrain.iNumChunksY; ++y)
			{
				STerrainChunk &oChunk = terrain.chunks[y][x];

				uiVertexBuffer += VERTS_PER_CHUNK * sizeof(STerrainVertex);
				uiIndexBuffer += TILES_PER_CHUNK * 6 * sizeof(WORD);

				if (oChunk.iAlphaMap != -1)
					uiAlphaMap += SQR(terrain.iChunkSize * terrain.pWorld->GetTexelDensity()) * uiTexelSize;

				uiBatches += oChunk.iNumSingleArrays;

				uiCliffVertexBuffer += oChunk.iNumCliffVerts * g_iCliffVertexStride;
				uiCliffIndexBuffer += oChunk.iNumCliffFaces * 3 * sizeof(WORD);
				uiCliffBatches += oChunk.iNumCliffArrays;
			}
		}
	}

	Console << _T("Terrain Vertex Buffers: ") << GetByteString(uiVertexBuffer) << newl;
	Console << _T("Terrain Index Buffers: ") << GetByteString(uiIndexBuffer) << newl;
	Console << _T("Terrain Alpha Map: ") << GetByteString(uiAlphaMap) << newl;
	Console << _T("Terrain Batches: ") << uiBatches << newl;

	Console << _T("Cliff Vertex Buffers: ") << GetByteString(uiCliffVertexBuffer) << newl;
	Console << _T("Cliff Index Buffers: ") << GetByteString(uiCliffIndexBuffer) << newl;
	Console << _T("Cliff Batches: ") << uiCliffBatches << newl;

	return true;
}
