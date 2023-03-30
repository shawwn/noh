// (C)2005 S2 Games
// d3d9_foliage.cpp
//
// Direct3D foliage functions
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9_main.h"
#include "d3d9_foliage.h"
#include "d3d9_state.h"
#include "d3d9_material.h"
#include "d3d9_shader.h"
#include "d3d9_scene.h"
#include "d3d9_util.h"
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
#include "../k2/c_vertexshader.h"
#include "../k2/s_foliagetile.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
struct SFoliage g_Foliage;

void	SFoliage::AllocateChunks(int iNewChunkSize)
{
	ClearChunks();

	iChunkSize = iNewChunkSize;
	iNumChunksX = pWorld->GetTileWidth() / iChunkSize;
	iNumChunksY = pWorld->GetTileHeight() / iChunkSize;
	fChunkWorldSize = iChunkSize * pWorld->GetScale();

	g_Foliage.pChunks = K2_NEW_ARRAY(ctx_D3D9, SFoliageChunk, iNumChunksX*iNumChunksY);
}

void	SFoliage::ClearChunks()
{
	SAFE_DELETE_ARRAY(pChunks);
}

struct SFoliageCenter
{
	CVec3f	vCenter;
	int		iIndex;
	int		iSeed;
};

extern CCvar<bool>		gfx_foliage;
extern CCvar<int>		foliage_chunkSize;



void			D3D_FreeChunkElems(SFoliageChunk &Chunk);
SFoliageArray*	D3D_AllocFoliageArray(int iLayer);
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_INT	(vid_foliageQuality,			0);
CVAR_BOOL	(vid_foliageSort,				false);

CVAR_FLOATF	(vid_foliageAnimateStrength,	4.0f,						CVAR_WORLDCONFIG);
CVAR_FLOATF	(vid_foliageAnimateSpeed,		0.2f,						CVAR_WORLDCONFIG);
CVAR_FLOATF	(vid_foliageMaxSlope,			0.5f,						CVAR_WORLDCONFIG);
CVAR_FLOATF	(vid_foliageFalloffDistance,	500.0f,						CVAR_SAVECONFIG);
CVAR_FLOATF	(vid_foliageDensity,			1.0f,						CVAR_SAVECONFIG);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
ResHandle	g_hFoliageMaterial(INVALID_RESOURCE);
ResHandle	g_hFoliageDiffuseReference(INVALID_RESOURCE);
ResHandle	g_hFoliageVertexShaderReference(INVALID_RESOURCE);
ResHandle	g_hFoliageVertexShaderNormal(INVALID_RESOURCE);
ResHandle	g_hFoliageVertexShaderCamera(INVALID_RESOURCE);

static CVec2f	s_vDir;
//=============================================================================


/*====================
  FoliageSortBackFront

  Return whether first element should be draw before the second
  ====================*/
static bool	FoliageSortBackFront(const SFoliageCenter &elem1, const SFoliageCenter &elem2)
{
	return DotProduct(s_vDir, elem1.vCenter.xy()) > DotProduct(s_vDir, elem2.vCenter.xy());
}


/*====================
  FoliageChunkSortBackFront

  Return whether first element should be draw before the second
  ====================*/
static bool	FoliageChunkSortBackFront(const uint &elem1, const uint &elem2)
{
	CVec2f v2Center1(float(elem1 & 0xffff), float(elem1 >> 16));
	v2Center1 += 0.5f;
	v2Center1 *= g_Foliage.fChunkWorldSize;

	CVec2f v2Center2(float(elem2 & 0xffff), float(elem2 >> 16));
	v2Center2 += 0.5f;
	v2Center2 *= g_Foliage.fChunkWorldSize;

	return DotProduct(s_vDir, v2Center1) > DotProduct(s_vDir, v2Center2);
}


/*====================
  D3D_RebuildFoliageArray
  ====================*/
void	D3D_RebuildFoliageArray(int iStartX, int iStartY, SFoliageArray &oArray)
{
	SFoliageVertex *pVertices;

	// Calculate centers
	vector<SFoliageCenter>	vCenters;
	const CWorld *pWorld(g_Foliage.pWorld);
	const float fTileSize(float(pWorld->GetScale()));
	const int iGridWidth(pWorld->GetGridWidth());
	int iLayer(oArray.iLayer);

	oArray.iNumFoliageQuads = 0;

	int iIndex(0);

	for (int i(0); i < oArray.iNumTiles; ++i)
	{
		int iTileX = oArray.vTiles[i].x;
		int iTileY = oArray.vTiles[i].y;

		float fX = pWorld->ScaleGridCoord(iTileX);
		float fY = pWorld->ScaleGridCoord(iTileY);

		int iSeed = (iTileY * iGridWidth + (iTileX ^ 60969)) + iLayer;
		
		srand(iSeed);

		float fDensity[4] =
		{
			pWorld->GetFoliageDensity(oArray.vTiles[i].x, oArray.vTiles[i].y, iLayer) * vid_foliageDensity,
			pWorld->GetFoliageDensity(oArray.vTiles[i].x + 1, oArray.vTiles[i].y, iLayer) * vid_foliageDensity,
			pWorld->GetFoliageDensity(oArray.vTiles[i].x, oArray.vTiles[i].y + 1, iLayer) * vid_foliageDensity,
			pWorld->GetFoliageDensity(oArray.vTiles[i].x + 1, oArray.vTiles[i].y + 1, iLayer) * vid_foliageDensity
		};

		CVec3f v3Normal[4] =
		{
			pWorld->GetGridNormal(oArray.vTiles[i].x, oArray.vTiles[i].y),
			pWorld->GetGridNormal(oArray.vTiles[i].x + 1, oArray.vTiles[i].y),
			pWorld->GetGridNormal(oArray.vTiles[i].x, oArray.vTiles[i].y + 1),
			pWorld->GetGridNormal(oArray.vTiles[i].x + 1, oArray.vTiles[i].y + 1)
		};

		CVec3f v3Points[4] =
		{
			CVec3f(fX, fY, pWorld->GetGridPoint(oArray.vTiles[i].x, oArray.vTiles[i].y)),
			CVec3f(fX + fTileSize, fY, pWorld->GetGridPoint(oArray.vTiles[i].x + 1, oArray.vTiles[i].y)),
			CVec3f(fX, fY + fTileSize, pWorld->GetGridPoint(oArray.vTiles[i].x, oArray.vTiles[i].y + 1)),
			CVec3f(fX + fTileSize, fY + fTileSize, pWorld->GetGridPoint(oArray.vTiles[i].x + 1, oArray.vTiles[i].y + 1))
		};

		float fScale[4] =
		{
			pWorld->GetFoliageSize(oArray.vTiles[i].x, oArray.vTiles[i].y, iLayer).z,
			pWorld->GetFoliageSize(oArray.vTiles[i].x + 1, oArray.vTiles[i].y, iLayer).z,
			pWorld->GetFoliageSize(oArray.vTiles[i].x, oArray.vTiles[i].y + 1, iLayer).z,
			pWorld->GetFoliageSize(oArray.vTiles[i].x + 1, oArray.vTiles[i].y + 1, iLayer).z
		};

		// UTTAR: No reason to do sqrt() or that division by fTileSize twice - fixed...
		float	fAreaLeft(LengthSq(CrossProduct(v3Points[1] - v3Points[0], v3Points[2] - v3Points[0]) / 2.0f));
		float	fAreaRight(LengthSq(CrossProduct(v3Points[3] - v3Points[1], v3Points[2] - v3Points[1]) / 2.0f));
		float	fMaxArea(sqrt(MAX(fAreaLeft, fAreaRight)) / (fTileSize * fTileSize * 0.5f));
		float	fMaxDensity(MAX(MAX(fDensity[0], fDensity[1]), MAX(fDensity[2], fDensity[3])));
		float	fMaxScale(MAX(MAX(fScale[0], fScale[1]), MAX(fScale[2], fScale[3])));

		if (fMaxDensity <= 0.0f || fMaxScale <= 0.25f)
			continue;

		vector<CVec2f> v2Points;
		M_BlueNoise(CRectf((iTileX % 8) / 8.0f, (iTileY % 8) / 8.0f, ((iTileX % 8) + 1) / 8.0f, ((iTileY % 8) + 1) / 8.0f), INT_CEIL(8192.0f * fMaxDensity * fMaxArea / 128.0f), v2Points);

		for (float d(0); d < float(v2Points.size()); d += 1.0f)
		{
			if (d >= v2Points.size())
				break;

			CVec3f	vCenter;
			CVec2f	v2Point(v2Points[INT_ROUND(d)]);
			vCenter.x =	fX + fTileSize * v2Point.x;
			vCenter.y = fY + fTileSize * v2Point.y;

			float fLerps[2] =
			{
				FRAC(vCenter.x / fTileSize),
				FRAC(vCenter.y / fTileSize)
			};

			CVec3f v3PCFNormal(Normalize(PCF(fLerps, v3Normal)));

			if (1.0f - DotProduct(v3PCFNormal, CVec3f(0.0f, 0.0f, 1.0f)) > vid_foliageMaxSlope)
				continue;

			float fPCFDensity(PCF(fLerps, fDensity));

			if (fLerps[0] + fLerps[1] < 1.0f)
				fPCFDensity *= fAreaLeft;
			else
				fPCFDensity *= fAreaRight;

			if (d <= fPCFDensity - M_Randnum(0.0f, 1.0f))
			{
				vCenter.z = pWorld->GetTerrainHeight(vCenter.x, vCenter.y);

				int iCrossQuads(pWorld->GetFoliageCrossQuads(iTileX, iTileY, iLayer));

				if (oArray.iNumFoliageQuads + iCrossQuads < 8192)
				{
					SFoliageCenter t = {vCenter, iIndex, iSeed};
					vCenters.push_back(t);

					oArray.iNumFoliageQuads += iCrossQuads;
					iIndex += iCrossQuads;
				}
			}
		}
	}

	if (oArray.iNumFoliageQuads == 0)
		return;

	// Create foliage vertex buffer
	if (FAILED(g_pd3dDevice->CreateVertexBuffer(oArray.iNumFoliageQuads * 4 * sizeof(SFoliageVertex),
		g_d3dManagedUsage | D3DUSAGE_WRITEONLY, 0, g_d3dManagedPool, &oArray.pVB, NULL)))
		K2System.Error(_T("D3D_AllocateFoliageChunk(): CreateVertexBuffer failed"));

	if (FAILED(oArray.pVB->Lock(0, oArray.iNumFoliageQuads * 4 * sizeof(SFoliageVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
		return;

	int n = 0;
	int iSeed = -1;

	for (vector<SFoliageCenter>::iterator it(vCenters.begin()); it != vCenters.end(); ++it)
	{
		CVec3f vCenter(it->vCenter);

		if (it->iSeed != iSeed)
		{
			iSeed = it->iSeed;
			srand(it->iSeed); rand();
		}

		float fLerps[2] =
		{
			FRAC(vCenter.x / fTileSize),
			FRAC(vCenter.y / fTileSize)
		};

		int iTileX(pWorld->GetTileFromCoord(vCenter.x));
		int iTileY(pWorld->GetTileFromCoord(vCenter.y));

		int iCrossQuads(pWorld->GetFoliageCrossQuads(iTileX, iTileY, iLayer));
		//byte yFlags(pWorld->GetFoliageFlags(iTileX, iTileY, iLayer));

		CVec3f v3Sizes[4] =
		{
			pWorld->GetFoliageSize(iTileX, iTileY, iLayer),
			pWorld->GetFoliageSize(iTileX + 1, iTileY, iLayer),
			pWorld->GetFoliageSize(iTileX, iTileY + 1, iLayer),
			pWorld->GetFoliageSize(iTileX + 1, iTileY + 1, iLayer)
		};

		CVec3f v3Variances[4] =
		{
			pWorld->GetFoliageVariance(iTileX, iTileY, iLayer),
			pWorld->GetFoliageVariance(iTileX + 1, iTileY, iLayer),
			pWorld->GetFoliageVariance(iTileX, iTileY + 1, iLayer),
			pWorld->GetFoliageVariance(iTileX + 1, iTileY + 1, iLayer)
		};

		float fWidths[4] =
		{
			v3Sizes[0].x,
			v3Sizes[1].x,
			v3Sizes[2].x,
			v3Sizes[3].x
		};
		float fPCFWidth(PCF(fLerps, fWidths));

		float fHeights[4] =
		{
			v3Sizes[0].y,
			v3Sizes[1].y,
			v3Sizes[2].y,
			v3Sizes[3].y
		};
		float fPCFHeight(PCF(fLerps, fHeights));

		float fRandWidths[4] =
		{
			v3Variances[0].x,
			v3Variances[1].x,
			v3Variances[2].x,
			v3Variances[3].x
		};
		float fPCFRandWidth(PCF(fLerps, fRandWidths));

		float fRandHeights[4] =
		{
			v3Variances[0].y,
			v3Variances[1].y,
			v3Variances[2].y,
			v3Variances[3].y
		};
		float fPCFRandHeight(PCF(fLerps, fRandHeights));

		float fScales[4] =
		{
			v3Sizes[0].z,
			v3Sizes[1].z,
			v3Sizes[2].z,
			v3Sizes[3].z
		};
		float fPCFScale(PCF(fLerps, fScales));

		float fWidth = (fPCFWidth + M_Randnum(-fPCFRandWidth, fPCFRandWidth)) * fPCFScale;
		float fHeight = (fPCFHeight + M_Randnum(-fPCFRandHeight, fPCFRandHeight)) * fPCFScale;
		float fPhase = M_Randnum(0.0f, 1.0f);

		vCenter.z += fHeight / 2.0f;

		CVec3f v3Normal[4] =
		{
			pWorld->GetGridNormal(iTileX, iTileY),
			pWorld->GetGridNormal(iTileX + 1, iTileY),
			pWorld->GetGridNormal(iTileX, iTileY + 1),
			pWorld->GetGridNormal(iTileX + 1, iTileY + 1)
		};
		CVec3f v3PCFNormal(Normalize(PCF(fLerps, v3Normal)));

		float tempAngle = M_Randnum(0.0f, 2.0f * M_PI);
		CVec2f v2FoliageRight(cos(tempAngle), sin(tempAngle));

		for (int iQuad(0); iQuad < iCrossQuads; ++iQuad)
		{
			CVec4b v4Data[4];

			byte nx(BYTE_ROUND((v3PCFNormal.x + 1.0f) * 0.5f * 255.0f));
			byte ny(BYTE_ROUND((v3PCFNormal.y + 1.0f) * 0.5f * 255.0f));
			byte nz(BYTE_ROUND((v3PCFNormal.z + 1.0f) * 0.5f * 255.0f));

			CVec4b v4Normal(nx, ny, nz, 0);

			/*if (yFlags & F_PARALLEL)
			{
				pVertices[n].weight[0] = fWidth / 2.0f;
				pVertices[n].weight[1] = -fWidth / 2.0f;
				pVertices[n].weight[2] = 0.0f;

				pVertices[n + 1].weight[0] = fWidth / 2.0f;
				pVertices[n + 1].weight[1] = fWidth / 2.0f;
				pVertices[n + 1].weight[2] = 0.0f;

				pVertices[n + 2].weight[0] = -fWidth / 2.0f;
				pVertices[n + 2].weight[1] = fWidth / 2.0f;
				pVertices[n + 2].weight[2] = 0.0f;

				pVertices[n + 3].weight[0] = -fWidth / 2.0f;
				pVertices[n + 3].weight[1] = -fWidth / 2.0f;
				pVertices[n + 3].weight[2] = 0.0f;

				float fAngle(RAD2DEG(M_PI * (1.0f / iCrossQuads)));
				v2FoliageRight.Rotate(fAngle);

				CVec3f v3FoliageRight(v2FoliageRight.x, v2FoliageRight.y, 0.0f);

				// Orthogonalize vFoliageRight against v3PCFNormal
				CVec3f v3Right(Normalize(v3FoliageRight - v3PCFNormal * DotProduct(v3PCFNormal, v3FoliageRight)));
				CVec3f v3Up(CrossProduct(v3Right, v3PCFNormal));

				pVertices[n].v = vCenter + v3Right * pVertices[n].weight[0] + v3Up * pVertices[n].weight[1] + v3PCFNormal * fHeight;
				pVertices[n + 1].v = vCenter + v3Right * pVertices[n + 1].weight[0] + v3Up * pVertices[n + 1].weight[1] + v3PCFNormal * fHeight;
				pVertices[n + 2].v = vCenter + v3Right * pVertices[n + 2].weight[0] + v3Up * pVertices[n + 2].weight[1] + v3PCFNormal * fHeight;
				pVertices[n + 3].v = vCenter + v3Right * pVertices[n + 3].weight[0] + v3Up * pVertices[n + 3].weight[1] + v3PCFNormal * fHeight;
			}
			else if (yFlags & F_CAMERA)
			{
				pVertices[n].weight[0] = fWidth / 2.0f;
				pVertices[n].weight[1] = -fHeight / 2.0f;
				pVertices[n].weight[2] = 0.0f;

				pVertices[n + 1].weight[0] = fWidth / 2.0f;
				pVertices[n + 1].weight[1] = fHeight / 2.0f;
				pVertices[n + 1].weight[2] = 1.0f;

				pVertices[n + 2].weight[0] = -fWidth / 2.0f;
				pVertices[n + 2].weight[1] = fHeight / 2.0f;
				pVertices[n + 2].weight[2] = -1.0f;

				pVertices[n + 3].weight[0] = -fWidth / 2.0f;
				pVertices[n + 3].weight[1] = -fHeight / 2.0f;
				pVertices[n + 3].weight[2] = 0.0f;

				pVertices[n].v = vCenter;
				pVertices[n + 1].v = vCenter;
				pVertices[n + 2].v = vCenter;
				pVertices[n + 3].v = vCenter;
			}
			else*/
			{
				CVec3f vWeight[4];

				vWeight[0][0] = fWidth / 2.0f;
				vWeight[0][1] = -fHeight / 2.0f;
				vWeight[0][2] = 0.0f;

				vWeight[1][0] = fWidth / 2.0f;
				vWeight[1][1] = fHeight / 2.0f;
				vWeight[1][2] = 1.0f * fPCFScale;

				vWeight[2][0] = -fWidth / 2.0f;
				vWeight[2][1] = fHeight / 2.0f;
				vWeight[2][2] = 1.0f * fPCFScale;

				vWeight[3][0] = -fWidth / 2.0f;
				vWeight[3][1] = -fHeight / 2.0f;
				vWeight[3][2] = 0.0f;

				float fAngle(RAD2DEG(M_PI * (1.0f / iCrossQuads)));
				v2FoliageRight.Rotate(fAngle);

				CVec3f v3FoliageRight(v2FoliageRight.x, v2FoliageRight.y, 0.0f);

				// Orthogonalize vFoliageRight against v3PCFNormal
				CVec3f v3Right(Normalize(v3FoliageRight - v3PCFNormal * DotProduct(v3PCFNormal, v3FoliageRight)));
				CVec3f v3Up(0.0f, 0.0f, 1.0f);

				pVertices[n].v = vCenter + v3Right * vWeight[0][0] + v3Up * vWeight[0][1];
				pVertices[n + 1].v = vCenter + v3Right * vWeight[1][0] + v3Up * vWeight[1][1];
				pVertices[n + 2].v = vCenter + v3Right * vWeight[2][0] + v3Up * vWeight[2][1];
				pVertices[n + 3].v = vCenter + v3Right * vWeight[3][0] + v3Up * vWeight[3][1];
			}

#if 0
			for (int i(0); i < 4; ++i)
			{
				float fColorLerps[2] = 
				{
					FRAC(pVertices[n + i].v.x / fTileSize),
					FRAC(pVertices[n + i].v.y / fTileSize)
				};

				int iColorTileX(CLAMP(INT_FLOOR(pVertices[n + i].v.x / fTileSize), 0, iGridWidth - 2));
				int iColorTileY(CLAMP(INT_FLOOR(pVertices[n + i].v.y / fTileSize), 0, iGridWidth - 2));

				CVec3f v3Colors[4] = 
				{
					pWorld->GetFoliageColor(iColorTileX, iColorTileY, iLayer),
					pWorld->GetFoliageColor(iColorTileX + 1, iColorTileY, iLayer),
					pWorld->GetFoliageColor(iColorTileX, iColorTileY + 1, iLayer),
					pWorld->GetFoliageColor(iColorTileX + 1, iColorTileY + 1, iLayer)
				};
				CVec3f v3Color(PCF(fColorLerps, v3Colors));

				dword dwColor(CVec4f(v3Color, 1.0f).GetAsDWord());

				pVertices[n + i].color = dwColor;
			}
#endif

#if 0
			pVertices[n].phase = fPhase;
			pVertices[n + 1].phase = fPhase;
			pVertices[n + 2].phase = fPhase;
			pVertices[n + 3].phase = fPhase;
#endif

			// Randomly flip foliage texture horizontally
			if (M_Randnum(0, 1))
			{
				v4Data[0].x = 255;
				v4Data[0].y = 0;

				v4Data[1].x = 255;
				v4Data[1].y = 255;

				v4Data[2].x = 0;
				v4Data[2].y = 255;

				v4Data[3].x = 0;
				v4Data[3].y = 0;
			}
			else
			{
				v4Data[0].x = 0;
				v4Data[0].y = 0;

				v4Data[1].x = 0;
				v4Data[1].y = 255;

				v4Data[2].x = 255;
				v4Data[2].y = 255;

				v4Data[3].x = 255;
				v4Data[3].y = 0;
			}

			// Mark top weights
			v4Data[0].z = 0;
			v4Data[1].z = BYTE_ROUND(fPCFScale * 255.0f);
			v4Data[2].z = BYTE_ROUND(fPCFScale * 255.0f);
			v4Data[3].z = 0;

			byte yPhase(BYTE_ROUND(fPhase * 255.0f));

			v4Data[0].w = yPhase;
			v4Data[1].w = yPhase;
			v4Data[2].w = yPhase;
			v4Data[3].w = yPhase;

			pVertices[n].n = v4Normal;
			pVertices[n + 1].n = v4Normal;
			pVertices[n + 2].n = v4Normal;
			pVertices[n + 3].n = v4Normal;

			pVertices[n].data = v4Data[0];
			pVertices[n + 1].data = v4Data[1];
			pVertices[n + 2].data = v4Data[2];
			pVertices[n + 3].data = v4Data[3];

			n += 4;

			if (n >= oArray.iNumFoliageQuads * 4)
				break;
		}
	}

	oArray.pVB->Unlock();

	if (vid_geometryPreload && !g_bD3D9Ex)
		oArray.pVB->PreLoad();

	// create sorted index buffers
	for (int iDir = 0; iDir < NUM_SORT_DIRECTIONS; ++iDir)
	{
		if (vid_foliageSort)
		{
			float fAngle = 2.0f * M_PI * (float(iDir) / NUM_SORT_DIRECTIONS);

			s_vDir = CVec2f(cos(fAngle), sin(fAngle));

			sort(vCenters.begin(), vCenters.end(), FoliageSortBackFront);
		}

		if (FAILED(g_pd3dDevice->CreateIndexBuffer(oArray.iNumFoliageQuads * 6 * sizeof(WORD),
				g_d3dManagedUsage | D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, g_d3dManagedPool, &oArray.pIB[iDir], NULL)))
			K2System.Error(_T("D3D_AllocateFoliageChunk(): CreateIndexBuffer failed"));

		WORD *pIndices;

		if (SUCCEEDED(oArray.pIB[iDir]->Lock(0, oArray.iNumFoliageQuads * 6 * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
		{
			WORD i = 0;

			for (vector<SFoliageCenter>::iterator it(vCenters.begin()); it != vCenters.end(); ++it)
			{
				int iTileX(pWorld->GetTileFromCoord(it->vCenter.x));
				int iTileY(pWorld->GetTileFromCoord(it->vCenter.y));
				
				int iCrossQuads(pWorld->GetFoliageCrossQuads(iTileX, iTileY, iLayer));

				for (int iQuad(0); iQuad < iCrossQuads; ++iQuad)
				{
					int iIndex(it->iIndex + iQuad);

					pIndices[i++] = (iIndex << 2);
					pIndices[i++] = (iIndex << 2) + 1;
					pIndices[i++] = (iIndex << 2) + 2;
					pIndices[i++] = (iIndex << 2);
					pIndices[i++] = (iIndex << 2) + 2;
					pIndices[i++] = (iIndex << 2) + 3;
				}
			}

			oArray.pIB[iDir]->Unlock();

			if (vid_geometryPreload && !g_bD3D9Ex)
				oArray.pIB[iDir]->PreLoad();
		}
	}

	// Re-seed random number generator
	srand(K2System.GetRandomSeed32());
}


/*====================
  D3D_RebuildFoliageChunk
  ====================*/
void	D3D_RebuildFoliageChunk(int iChunkX, int iChunkY)
{
	PROFILE("D3D_RebuildFoliageChunk");

	try
	{
		SFoliageChunk		&oChunk(g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX]);
		const CWorld		*pWorld(g_Foliage.pWorld);

		if (!oChunk.iValidityFlags)
			return;

		int iStartX = iChunkX * g_Foliage.iChunkSize;		// Grid left tile coord
		int iStartY = iChunkY * g_Foliage.iChunkSize;		// Grid top tile coord

		// Recalculate bounding box
		if (oChunk.iValidityFlags & (FOLIAGE_REBUILD_VERTICES))
		{
			float fMinZ(FAR_AWAY);
			float fMaxZ(-FAR_AWAY);

			for (int iTileY = iStartY; iTileY < iStartY + g_Foliage.iChunkSize; ++iTileY)
			{
				for (int iTileX = iStartX; iTileX < iStartX + g_Foliage.iChunkSize; ++iTileX)
				{
					float fHeight = pWorld->GetGridPoint(iTileX, iTileY);

					fMinZ = MIN(fMinZ, fHeight);
					fMaxZ = MAX(fMaxZ, fHeight);
				}
			}

			oChunk.bbBounds.Clear();
			oChunk.bbBounds.AddPoint(CVec3f(pWorld->ScaleGridCoord(iStartX), pWorld->ScaleGridCoord(iStartY), fMinZ));
			oChunk.bbBounds.AddPoint(CVec3f(pWorld->ScaleGridCoord(iStartX + g_Foliage.iChunkSize), pWorld->ScaleGridCoord(iStartY + g_Foliage.iChunkSize), fMaxZ));
		}

		D3D_FreeChunkElems(oChunk);

		for (int iLayer = 0; iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
		{
			struct SFoliageBatch
			{
				SFoliageBatch() {}
				SFoliageBatch(ResHandle h, bool b) : hTexture(h), bCamera(b) {}

				bool operator==(const SFoliageBatch &c) { return hTexture == c.hTexture && bCamera == c.bCamera; }
				bool operator!=(const SFoliageBatch &c) { return !(hTexture == c.hTexture && bCamera == c.bCamera); }

				ResHandle hTexture;
				bool bCamera;
			};

			SFoliageBatch		sCurrentBatch;
			SFoliageArray		*pCurrentArray(NULL);
			SFoliageBatch		asBatchList[256];
			int					iTextureListSize(0);
			SFoliageBatch		sLastBatch(INVALID_RESOURCE, false);

			for (int iTileY = iStartY; iTileY < iStartY + g_Foliage.iChunkSize; ++iTileY)
			{
				for (int iTileX = iStartX; iTileX < iStartX + g_Foliage.iChunkSize; ++iTileX)
				{
					sCurrentBatch = SFoliageBatch
					(
						pWorld->GetFoliageTexture(iTileX, iTileY, iLayer),
						pWorld->GetFoliageFlags(iTileX, iTileY, iLayer) & F_CAMERA
					);

					if (sCurrentBatch != sLastBatch)
					{
						bool bFoundBatch = false;

						for (int n = 0; n < iTextureListSize; ++n)
						{
							if (asBatchList[n] == sCurrentBatch)
							{
								pCurrentArray = oChunk.pArrays[iLayer][n];
								bFoundBatch = true;
							}
						}

						if (!bFoundBatch)
						{
							// We've encountered a new shader
							oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]] = D3D_AllocFoliageArray(iLayer);
							oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]]->hTexture = sCurrentBatch.hTexture;
							oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]]->bCamera = sCurrentBatch.bCamera;

							asBatchList[iTextureListSize] = sCurrentBatch;

							pCurrentArray = oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]];

							++oChunk.iNumArrays[iLayer];
							++iTextureListSize;
						}

						sLastBatch = sCurrentBatch;
					}

					if (pCurrentArray != NULL)
					{
						pCurrentArray->vTiles[pCurrentArray->iNumTiles] = CVec2s(iTileX, iTileY);
						++pCurrentArray->iNumTiles;
					}
				}
			}

			for (int n = 0; n < oChunk.iNumArrays[iLayer]; ++n)
				D3D_RebuildFoliageArray(iStartX, iStartY, *oChunk.pArrays[iLayer][n]);
		}

		oChunk.iNumFoliageQuads = 0;
		for (int iLayer = 0; iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
			for (int n = 0; n < oChunk.iNumArrays[iLayer]; ++n)
				oChunk.iNumFoliageQuads += oChunk.pArrays[iLayer][n]->iNumFoliageQuads;

		oChunk.iValidityFlags = 0;
	}
	catch (CException &ex)
	{
		ex.Process(_T("D3D_RebuildFoliageChunk() - "), NO_THROW);
	}
}


/*====================
  D3D_AllocateFoliageChunk
  ====================*/
void	D3D_AllocateFoliageChunk(int iChunkX, int iChunkY)
{
	SFoliageChunk &oChunk = g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX];

	MemManager.Set(&oChunk, 0, sizeof(SFoliageChunk));

	oChunk.iValidityFlags =	FOLIAGE_REBUILD_VERTICES |
							FOLIAGE_REBUILD_COLORS |
							FOLIAGE_REBUILD_NORMALS |
							FOLIAGE_REBUILD_SHADERS;

	D3D_RebuildFoliageChunk(iChunkX, iChunkY);
}


/*====================
  D3D_RebuildFoliage
  ====================*/
void	D3D_RebuildFoliage(int iChunkSize, const CWorld *pWorld)
{
	PROFILE("D3D_RebuildFoliage");

	D3D_InitFoliage();

	if (iChunkSize > pWorld->GetTileWidth())
		iChunkSize = pWorld->GetTileWidth();
	if (iChunkSize > pWorld->GetTileHeight())
		iChunkSize = pWorld->GetTileHeight();

	D3D_DestroyFoliage();

	if (pWorld->GetTileHeight() % iChunkSize != 0 || pWorld->GetTileWidth() % iChunkSize != 0)
	{
		K2System.Error(_T("D3D_RebuildFoliage: world dimensions were not a multiple of iChunkSize\n"));
		return;
	}

	// Allocate
	g_Foliage.pWorld = pWorld;
	g_Foliage.AllocateChunks(iChunkSize);

	for (int iChunkY = 0; iChunkY < g_Foliage.iNumChunksY; ++iChunkY)
	{
		for (int iChunkX = 0; iChunkX < g_Foliage.iNumChunksX; ++iChunkX)
			D3D_AllocateFoliageChunk(iChunkX, iChunkY);
	}
}


/*====================
  D3D_AllocFoliageArray
  ====================*/
SFoliageArray*	D3D_AllocFoliageArray(int iLayer)
{
	SFoliageArray *pNewArray;

	pNewArray = K2_NEW(ctx_D3D9,   SFoliageArray);
	MemManager.Set(pNewArray, 0, sizeof(SFoliageArray));
	pNewArray->vTiles = K2_NEW_ARRAY(ctx_D3D9, CVec2s, MAX_TILES);
	pNewArray->iLayer = iLayer;
	MemManager.Set(pNewArray->vTiles, 0, sizeof(MAX_TILES * sizeof(CVec2s)));

	return pNewArray;
}


/*====================
  D3D_FreeChunkElems
  ====================*/
void	D3D_FreeChunkElems(SFoliageChunk &oChunk)
{
	for (int iLayer = 0; iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
	{
		for (int n = 0; n < oChunk.iNumArrays[iLayer]; ++n)
		{
			SAFE_RELEASE(oChunk.pArrays[iLayer][n]->pVB);

			for (int iDir = 0; iDir < NUM_SORT_DIRECTIONS; ++iDir)
			{
				SAFE_RELEASE(oChunk.pArrays[iLayer][n]->pIB[iDir]);
			}

			K2_DELETE_ARRAY(oChunk.pArrays[iLayer][n]->vTiles);
			K2_DELETE_ARRAY(oChunk.pArrays[iLayer][n]);
		}

		oChunk.iNumArrays[iLayer] = 0;
	}
}


/*====================
  D3D_DestroyFoliageChunk
  ====================*/
void	D3D_DestroyFoliageChunk(int iChunkX, int iChunkY)
{
	SFoliageChunk &oChunk = g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX];

	D3D_FreeChunkElems(oChunk);

	MemManager.Set(&oChunk, 0, sizeof(SFoliageChunk));
}


/*====================
  D3D_DestroyFoliage
  ====================*/
void	D3D_DestroyFoliage()
{
	for (int y = 0; y < g_Foliage.iNumChunksY; ++y)
		for (int x = 0; x < g_Foliage.iNumChunksX; ++x)
			D3D_DestroyFoliageChunk(x, y);
}


/*====================
  D3D_InvalidateFoliageVertex
  ====================*/
void	D3D_InvalidateFoliageVertex(int iTileX, int iTileY, int iFlags)
{
	if (!g_Foliage.iChunkSize)
		return;

	int iChunkX = (iTileX) / g_Foliage.iChunkSize;
	int iChunkY = (iTileY) / g_Foliage.iChunkSize;

	if (iChunkX < 0 || iChunkY < 0 || iChunkX >= g_Foliage.iChunkSize || iChunkY >= g_Foliage.iChunkSize)
		return;

	// check if we're right on a oChunk border,
	// in which case we'll have to invalidate more than one oChunk
	if (iTileX % g_Foliage.iChunkSize == 0 && iChunkX > 0)
		g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX - 1].iValidityFlags |= iFlags;

	if (iTileY % g_Foliage.iChunkSize == 0 && iChunkY > 0)
		g_Foliage.pChunks[(iChunkY - 1) * g_Foliage.iNumChunksX + iChunkX].iValidityFlags |= iFlags;

	if (iTileY % g_Foliage.iChunkSize == 0 && iTileX % g_Foliage.iChunkSize == 0 && iChunkY > 0 && iChunkX > 0)
		g_Foliage.pChunks[(iChunkY - 1) * g_Foliage.iNumChunksX + iChunkX - 1].iValidityFlags |= iFlags;

	if (iChunkX < g_Foliage.iNumChunksX && iChunkY < g_Foliage.iNumChunksY)
		g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX].iValidityFlags |= iFlags;
}


/*====================
  D3D_InvalidateFoliageTile
  ====================*/
void	D3D_InvalidateFoliageTile(int iTileX, int iTileY, int iFlags)
{
	if (!g_Foliage.iChunkSize)
		return;

	int iChunkX = (iTileX) / g_Foliage.iChunkSize;
	int iChunkY = (iTileY) / g_Foliage.iChunkSize;

	if (iChunkX < 0 || iChunkX >= g_Foliage.iNumChunksX ||
		iChunkY < 0 || iChunkY >= g_Foliage.iNumChunksX)
		return;

	g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX].iValidityFlags |= iFlags;
}


/*====================
  D3D_InvalidateFoliageLayer
  ====================*/
void	D3D_InvalidateFoliageLayer(int iFlags)
{
	for (int iY = 0; iY < g_Foliage.iNumChunksY; ++iY)
		for (int iX = 0; iX < g_Foliage.iNumChunksX; ++iX)
			g_Foliage.pChunks[iY * g_Foliage.iNumChunksX + iX].iValidityFlags |= iFlags;
}


/*====================
  D3D_FlagVisibleFoliageChunks
  ====================*/
void	D3D_FlagVisibleFoliageChunks()
{
	if (SceneManager.GetFoliageDrawDistance() < SceneManager.GetSceneDrawDistance())
	{
		CSphere cSphere(g_pCam->GetOrigin(), SceneManager.GetFoliageDrawDistance());

		for (int iY(0); iY < g_Foliage.iNumChunksY; ++iY)
		{
			for (int iX(0); iX < g_Foliage.iNumChunksX; ++iX)
			{
				SFoliageChunk &oChunk(g_Foliage.pChunks[iY * g_Foliage.iNumChunksX + iX]);

				if (oChunk.iValidityFlags)
					D3D_RebuildFoliageChunk(iX, iY);

				if (oChunk.iNumFoliageQuads > 0 &&
					SceneManager.AABBIsVisible(oChunk.bbBounds) &&
					I_SphereBoundsIntersect(cSphere, oChunk.bbBounds))
					oChunk.bVisible = true;
				else
					oChunk.bVisible = false;
			}
		}
	}
	else
	{
		for (int iY(0); iY < g_Foliage.iNumChunksY; ++iY)
		{
			for (int iX(0); iX < g_Foliage.iNumChunksX; ++iX)
			{
				SFoliageChunk &oChunk(g_Foliage.pChunks[iY * g_Foliage.iNumChunksX + iX]);

				if (oChunk.iValidityFlags)
					D3D_RebuildFoliageChunk(iX, iY);

				if (oChunk.iNumFoliageQuads > 0 &&
					SceneManager.AABBIsVisible(oChunk.bbBounds))
					oChunk.bVisible = true;
				else
					oChunk.bVisible = false;
			}
		}
	}
}


/*====================
  D3D_FoliageBounds
  ====================*/
void	D3D_FoliageBounds(CBBoxf &bbFoliage)
{
	for (int iY = 0; iY < g_Foliage.iNumChunksY; ++iY)
	{
		for (int iX = 0; iX < g_Foliage.iNumChunksX; ++iX)
		{
			SFoliageChunk &oChunk = g_Foliage.pChunks[iY * g_Foliage.iNumChunksX + iX];

			if (oChunk.bVisible)
				bbFoliage.AddBox(oChunk.bbBounds);
		}
	}
}


/*====================
  D3D_SortFoliageChunks
  ====================*/
void	D3D_SortFoliageChunks(vector<uint> &vFoliageOrder)
{
	s_vDir = g_pCam->GetViewAxis(FORWARD).xy();

	sort(vFoliageOrder.begin(), vFoliageOrder.end(), FoliageChunkSortBackFront);
}


/*====================
  D3D_AddFoliageChunks
  ====================*/
void	D3D_AddFoliageChunks()
{
	PROFILE("D3D_AddFoliageChunks");

	if (!g_Foliage.pWorld ||
		!gfx_foliage || SceneManager.GetFoliageDrawDistance() <= 0.0f ||
		g_pCam->HasFlags(CAM_WIREFRAME_TERRAIN) ||
		g_pCam->HasFlags(CAM_NO_TERRAIN))
		return;

	D3D_FlagVisibleFoliageChunks();
	
	g_RenderList.Add(K2_NEW(ctx_D3D9,   CFoliageRenderer)(FOLIAGE_FRONTBACK, FOLIAGE_ALPHATEST | FOLIAGE_DEPTHWRITE));
}


/*====================
  D3D_InitFoliage
  ====================*/
void	D3D_InitFoliage()
{
	g_hFoliageDiffuseReference = g_ResourceManager.Register(_T("!foliage_d"), RES_REFERENCE);
	g_hFoliageVertexShaderReference = g_ResourceManager.Register(_T("!foliage_v"), RES_REFERENCE);
		
	g_ResourceManager.UpdateReference(g_hFoliageDiffuseReference, g_ResourceManager.GetWhiteTexture());

	g_hFoliageMaterial = g_ResourceManager.Register(_T("/world/foliage/materials/default.material"), RES_MATERIAL);

	g_hFoliageVertexShaderNormal = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CVertexShader)(_T("foliage_color"), 0), RES_VERTEX_SHADER);
	g_hFoliageVertexShaderCamera = g_ResourceManager.Register(K2_NEW(ctx_D3D9,   CVertexShader)(_T("foliage_color_camera"), 0), RES_VERTEX_SHADER);
}


/*====================
  RebuildFoliage
  ====================*/
CMD(RebuildFoliage)
{
	D3D_RebuildFoliage(foliage_chunkSize, g_Foliage.pWorld);

	return true;
}


/*====================
  PrintFoliageSize
  ====================*/
CMD(PrintFoliageSize)
{
	uint uiVertexBuffer(0);
	uint uiIndexBuffer(0);

	for (int x(0); x < g_Foliage.iNumChunksX; ++x)
	{
		for (int y(0); y < g_Foliage.iNumChunksY; ++y)
		{
			SFoliageChunk &oChunk = g_Foliage.pChunks[y * g_Foliage.iNumChunksX + x];

			for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
			{
				for (int a(0); a < oChunk.iNumArrays[iLayer]; ++a)
				{
					uiVertexBuffer += oChunk.pArrays[iLayer][a]->iNumFoliageQuads * sizeof(SFoliageVertex) * 4;
					uiIndexBuffer += oChunk.pArrays[iLayer][a]->iNumFoliageQuads * sizeof(WORD) * 6;
				}
			}
		}
	}

	Console << _T("Foliage Vertex Buffers: ") << GetByteString(uiVertexBuffer) << newl;
	Console << _T("Foliage Index Buffers: ") << GetByteString(uiIndexBuffer) << newl;

	return true;
}
