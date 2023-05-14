// (C)2008 S2 Games
// gl2_foliage.cpp
//
// Foliage
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "gl2_foliage.h"

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

void    SFoliage::AllocateChunks(int iNewChunkSize)
{
    ClearChunks();

    iChunkSize = iNewChunkSize;
    iNumChunksX = pWorld->GetTileWidth() / iChunkSize;
    iNumChunksY = pWorld->GetTileHeight() / iChunkSize;
    fChunkWorldSize = iChunkSize * pWorld->GetScale();

    g_Foliage.ppChunks = K2_NEW_ARRAY(ctx_GL2, SFoliageChunk*, iNumChunksY);
    for (int i(0); i < iNumChunksY; ++i)
        g_Foliage.ppChunks[i] = K2_NEW_ARRAY(ctx_GL2, SFoliageChunk, iNumChunksX);
}

void    SFoliage::ClearChunks()
{
    if (ppChunks != nullptr)
    {
        for (int i(0); i < iNumChunksY; ++i)
            K2_DELETE_ARRAY(ppChunks[i]);
        K2_DELETE_ARRAY(ppChunks);
    }
}

struct SFoliageCenter
{
    CVec3f  vCenter;
    int     iIndex;
    int     iSeed;
};

extern CCvar<bool>      gfx_foliage;
extern CCvar<int>       foliage_chunkSize;

void            GL_FreeChunkElems(SFoliageChunk &Chunk);
SFoliageArray*  GL_AllocFoliageArray(int iLayer);
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL   (vid_foliageNoCull,             false);
CVAR_INT    (vid_foliageQuality,            0);
CVAR_BOOL   (vid_foliageSort,               false);

CVAR_FLOATF (vid_foliageAnimateStrength,    4.0f,                       CVAR_WORLDCONFIG);
CVAR_FLOATF (vid_foliageAnimateSpeed,       0.2f,                       CVAR_WORLDCONFIG);
CVAR_FLOATF (vid_foliageMaxSlope,           0.5f,                       CVAR_WORLDCONFIG);
CVAR_FLOATF (vid_foliageFalloffDistance,    500.0f,                     CVAR_SAVECONFIG);
CVAR_FLOATF (vid_foliageDensity,            1.0f,                       CVAR_SAVECONFIG);
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
ResHandle   g_hFoliageMaterial(INVALID_RESOURCE);
ResHandle   g_hFoliageDiffuseReference(INVALID_RESOURCE);
ResHandle   g_hFoliageVertexShaderReference(INVALID_RESOURCE);
ResHandle   g_hFoliageVertexShaderNormal(INVALID_RESOURCE);
ResHandle   g_hFoliageVertexShaderCamera(INVALID_RESOURCE);

AttributeMap    g_mapFoliageAttributes;

static CVec2f   s_vDir;
//=============================================================================


/*====================
  FoliageSortBackFront

  Return whether first element should be draw before the second
  ====================*/

static bool FoliageSortBackFront(const SFoliageCenter &elem1, const SFoliageCenter &elem2)
{
    return DotProduct(s_vDir, elem1.vCenter.xy()) > DotProduct(s_vDir, elem2.vCenter.xy());
}


/*====================
  FoliageChunkSortBackFront

  Return whether first element should be draw before the second
  ====================*/
static bool FoliageChunkSortBackFront(const uint &elem1, const uint &elem2)
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
  GL_RebuildFoliageArray
  ====================*/
void    GL_RebuildFoliageArray(int iStartX, int iStartY, SFoliageArray &oArray)
{
    SFoliageVertex *pVertices;

    // Calculate centers
    vector<SFoliageCenter>  vCenters;
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

        float   fAreaLeft(LengthSq(CrossProduct(v3Points[1] - v3Points[0], v3Points[2] - v3Points[0]) / 2.0f));
        float   fAreaRight(LengthSq(CrossProduct(v3Points[3] - v3Points[1], v3Points[2] - v3Points[1]) / 2.0f));
        float   fMaxArea(sqrt(MAX(fAreaLeft, fAreaRight)) / (fTileSize * fTileSize * 0.5f));
        float   fMaxDensity(MAX(MAX(fDensity[0], fDensity[1]), MAX(fDensity[2], fDensity[3])));
        float   fMaxScale(MAX(MAX(fScale[0], fScale[1]), MAX(fScale[2], fScale[3])));

        if (fMaxDensity <= 0.0f || fMaxScale <= 0.25f)
            continue;

        vector<CVec2f> v2Points;
        M_BlueNoise(CRectf((iTileX % 8) / 8.0f, (iTileY % 8) / 8.0f, ((iTileX % 8) + 1) / 8.0f, ((iTileY % 8) + 1) / 8.0f), INT_CEIL(8192.0f * fMaxDensity * fMaxArea / 128.0f), v2Points);

        for (float d(0); d < float(v2Points.size()); d += 1.0f)
        {
            if (d >= v2Points.size())
                break;

            CVec3f  vCenter;
            CVec2f  v2Point(v2Points[INT_ROUND(d)]);
            vCenter.x = fX + fTileSize * v2Point.x;
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
    glGenBuffersARB(1, &oArray.uiVB);
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, oArray.uiVB);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, oArray.iNumFoliageQuads * 4 * sizeof(SFoliageVertex), nullptr, GL_STATIC_DRAW_ARB);

    pVertices = (SFoliageVertex *)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);

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

        float tempAngle = M_Randnum(0.0f, 2.0f*M_PI);
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

    glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

    // create sorted index buffers
    for (int iDir = 0; iDir < NUM_SORT_DIRECTIONS; ++iDir)
    {
        if (vid_foliageSort)
        {
            float fAngle = 2.0f * M_PI * (float(iDir) / NUM_SORT_DIRECTIONS);

            s_vDir = CVec2f(cos(fAngle), sin(fAngle));

            sort(vCenters.begin(), vCenters.end(), FoliageSortBackFront);
        }

        glGenBuffersARB(1, &oArray.uiIB[iDir]);
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, oArray.uiIB[iDir]);
        glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, oArray.iNumFoliageQuads * 6 * sizeof(GLushort), nullptr, GL_STATIC_DRAW_ARB);

        GLushort *pIndices((GLushort*)glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY));

        GLushort i(0);

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

        glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER);
    }

    PRINT_GLERROR_BREAK();

    // Re-seed random number generator
    srand(K2System.GetRandomSeed32());
}


/*====================
  GL_RebuildFoliageChunk
  ====================*/
void    GL_RebuildFoliageChunk(int iChunkX, int iChunkY)
{
    PROFILE("GL_RebuildFoliageChunk");

    try
    {
        SFoliageChunk       &oChunk(g_Foliage.ppChunks[iChunkY][iChunkX]);
        const CWorld        *pWorld(g_Foliage.pWorld);

        if (!oChunk.iValidityFlags)
            return;

        int iStartX = iChunkX * g_Foliage.iChunkSize;       // Grid left tile coord
        int iStartY = iChunkY * g_Foliage.iChunkSize;       // Grid top tile coord

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

        GL_FreeChunkElems(oChunk);

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

            SFoliageBatch       sCurrentBatch;
            SFoliageArray       *pCurrentArray(nullptr);
            SFoliageBatch       asBatchList[256];
            int                 iTextureListSize(0);
            SFoliageBatch       sLastBatch(INVALID_RESOURCE, false);

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
                            oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]] = GL_AllocFoliageArray(iLayer);
                            oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]]->hTexture = sCurrentBatch.hTexture;
                            oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]]->bCamera = sCurrentBatch.bCamera;

                            asBatchList[iTextureListSize] = sCurrentBatch;

                            pCurrentArray = oChunk.pArrays[iLayer][oChunk.iNumArrays[iLayer]];

                            ++oChunk.iNumArrays[iLayer];
                            ++iTextureListSize;
                        }

                        sLastBatch = sCurrentBatch;
                    }

                    if (pCurrentArray != nullptr)
                    {
                        pCurrentArray->vTiles[pCurrentArray->iNumTiles] = CVec2s(iTileX, iTileY);
                        ++pCurrentArray->iNumTiles;
                    }
                }
            }

            for (int n = 0; n < oChunk.iNumArrays[iLayer]; ++n)
                GL_RebuildFoliageArray(iStartX, iStartY, *oChunk.pArrays[iLayer][n]);
        }

        oChunk.iValidityFlags = 0;
    }
    catch (CException &ex)
    {
        ex.Process(_T("GL_RebuildFoliageChunk() - "), NO_THROW);
    }
}


/*====================
  GL_AllocateFoliageChunk
  ====================*/
void    GL_AllocateFoliageChunk(int iChunkX, int iChunkY)
{
    SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

    MemManager.Set(&oChunk, 0, sizeof(SFoliageChunk));

    oChunk.iValidityFlags = FOLIAGE_REBUILD_VERTICES |
                            FOLIAGE_REBUILD_COLORS |
                            FOLIAGE_REBUILD_NORMALS |
                            FOLIAGE_REBUILD_SHADERS;

    GL_RebuildFoliageChunk(iChunkX, iChunkY);
}


/*====================
  GL_RebuildFoliage
  ====================*/
void    GL_RebuildFoliage(int iChunkSize, const CWorld *pWorld)
{
    PROFILE("GL_RebuildFoliage");

    GL_InitFoliage();

    if (iChunkSize > pWorld->GetTileWidth())
        iChunkSize = pWorld->GetTileWidth();
    if (iChunkSize > pWorld->GetTileHeight())
        iChunkSize = pWorld->GetTileHeight();

    GL_DestroyFoliage();

    if (pWorld->GetTileHeight() % iChunkSize != 0 || pWorld->GetTileWidth() % iChunkSize != 0)
    {
        K2System.Error(_T("GL_RebuildFoliage() - world dimensions were not a multiple of iChunkSize\n"));
        return;
    }

    // Allocate
    g_Foliage.pWorld = pWorld;
    g_Foliage.AllocateChunks(iChunkSize);

    for (int iChunkY = 0; iChunkY < g_Foliage.iNumChunksY; ++iChunkY)
    {
        for (int iChunkX = 0; iChunkX < g_Foliage.iNumChunksX; ++iChunkX)
            GL_AllocateFoliageChunk(iChunkX, iChunkY);
    }
}


/*====================
  GL_AllocFoliageArray
  ====================*/
SFoliageArray*  GL_AllocFoliageArray(int iLayer)
{
    SFoliageArray *pNewArray;

    pNewArray = K2_NEW(ctx_GL2,    SFoliageArray);
    MemManager.Set(pNewArray, 0, sizeof(SFoliageArray));
    pNewArray->vTiles = K2_NEW_ARRAY(ctx_GL2, CVec2s, MAX_TILES);
    pNewArray->iLayer = iLayer;
    MemManager.Set(pNewArray->vTiles, 0, sizeof(MAX_TILES * sizeof(CVec2s)));

    return pNewArray;
}


/*====================
  GL_FreeChunkElems
  ====================*/
void    GL_FreeChunkElems(SFoliageChunk &oChunk)
{
    for (int iLayer = 0; iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
    {
        for (int n = 0; n < oChunk.iNumArrays[iLayer]; ++n)
        {
            glDeleteBuffersARB(1, &oChunk.pArrays[iLayer][n]->uiVB);

            for (int iDir = 0; iDir < NUM_SORT_DIRECTIONS; ++iDir)
            {
                glDeleteBuffersARB(1, &oChunk.pArrays[iLayer][n]->uiIB[iDir]);
            }

            K2_DELETE_ARRAY(oChunk.pArrays[iLayer][n]->vTiles);
            K2_DELETE(oChunk.pArrays[iLayer][n]);
        }

        oChunk.iNumArrays[iLayer] = 0;
    }
}


/*====================
  GL_DestroyFoliageChunk
  ====================*/
void    GL_DestroyFoliageChunk(int iChunkX, int iChunkY)
{
    SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

    GL_FreeChunkElems(oChunk);

    MemManager.Set(&oChunk, 0, sizeof(SFoliageChunk));
}


/*====================
  GL_DestroyFoliage
  ====================*/
void    GL_DestroyFoliage()
{
    for (int iY(0); iY < g_Foliage.iNumChunksY; ++iY)
        for (int iX(0); iX < g_Foliage.iNumChunksX; ++iX)
            GL_DestroyFoliageChunk(iX, iY);
}


/*====================
  GL_InvalidateFoliageVertex
  ====================*/
void    GL_InvalidateFoliageVertex(int iTileX, int iTileY, int iFlags)
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
        g_Foliage.ppChunks[iChunkY][iChunkX - 1].iValidityFlags |= iFlags;

    if (iTileY % g_Foliage.iChunkSize == 0 && iChunkY > 0)
        g_Foliage.ppChunks[iChunkY - 1][iChunkX].iValidityFlags |= iFlags;

    if (iTileY % g_Foliage.iChunkSize == 0 && iTileX % g_Foliage.iChunkSize == 0 && iChunkY > 0 && iChunkX > 0)
        g_Foliage.ppChunks[iChunkY - 1][iChunkX - 1].iValidityFlags |= iFlags;

    if (iChunkX < g_Foliage.iNumChunksX && iChunkY < g_Foliage.iNumChunksY)
        g_Foliage.ppChunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  GL_InvalidateFoliageTile
  ====================*/
void    GL_InvalidateFoliageTile(int iTileX, int iTileY, int iFlags)
{
    if (!g_Foliage.iChunkSize)
        return;

    int iChunkX = (iTileX) / g_Foliage.iChunkSize;
    int iChunkY = (iTileY) / g_Foliage.iChunkSize;

    if (iChunkX < 0 || iChunkX >= g_Foliage.iNumChunksX ||
        iChunkY < 0 || iChunkY >= g_Foliage.iNumChunksX)
        return;

    g_Foliage.ppChunks[iChunkY][iChunkX].iValidityFlags |= iFlags;
}


/*====================
  GL_InvalidateFoliageLayer
  ====================*/
void    GL_InvalidateFoliageLayer(int iFlags)
{
    for (int y = 0; y < g_Foliage.iNumChunksY; ++y)
        for (int x = 0; x < g_Foliage.iNumChunksX; ++x)
            g_Foliage.ppChunks[y][x].iValidityFlags |= iFlags;
}


/*====================
  GL_FlagFoliageChunkForRendering
  ====================*/
void    GL_FlagFoliageChunkForRendering(int iChunkX, int iChunkY)
{
    g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = true;
}


/*====================
  GL_FlagVisibleFoliageChunks
  ====================*/
void    GL_FlagVisibleFoliageChunks()
{
    for (int x(0); x < g_Foliage.iNumChunksX; ++x)
    {
        for (int y(0); y < g_Foliage.iNumChunksY; ++y)
        {
            SFoliageChunk &oChunk = g_Foliage.ppChunks[y][x];

            int iNumFoliageQuads(0);
            for (int iLayer = 0; iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
                for (int n = 0; n < oChunk.iNumArrays[iLayer]; ++n)
                    iNumFoliageQuads += oChunk.pArrays[iLayer][n]->iNumFoliageQuads;

            if (iNumFoliageQuads > 0 &&
                SceneManager.AABBIsVisible(oChunk.bbBounds) &&
                I_SphereBoundsIntersect(CSphere(g_pCam->GetOrigin(), SceneManager.GetFoliageDrawDistance()), oChunk.bbBounds))
                GL_FlagFoliageChunkForRendering(x, y);
        }
    }
}


/*====================
  GL_RebuildFoliageChunks
  ====================*/
void    GL_RebuildFoliageChunks()
{
    for (int x(0); x < g_Foliage.iNumChunksX; ++x)
    {
        for (int y(0); y < g_Foliage.iNumChunksY; ++y)
        {
            SFoliageChunk &oChunk = g_Foliage.ppChunks[y][x];

            if (oChunk.iValidityFlags)
                GL_RebuildFoliageChunk(x, y);
        }
    }
}


/*====================
  GL_FoliageBounds
  ====================*/
void    GL_FoliageBounds(CBBoxf &bbFoliage)
{
    for (int x = 0; x < g_Foliage.iNumChunksX; ++x)
    {
        for (int y = 0; y < g_Foliage.iNumChunksY; ++y)
        {
            SFoliageChunk &oChunk = g_Foliage.ppChunks[y][x];

            if (oChunk.bVisible)
            {
                bbFoliage.AddBox(oChunk.bbBounds);
            }
        }
    }
}


/*====================
  GL_SortFoliageChunks
  ====================*/
void    GL_SortFoliageChunks(vector<uint> &vFoliageOrder)
{
    s_vDir = g_pCam->GetViewAxis(FORWARD).xy();

    sort(vFoliageOrder.begin(), vFoliageOrder.end(), FoliageChunkSortBackFront);
}


/*====================
  GL_AddFoliageChunks
  ====================*/
void    GL_AddFoliageChunks()
{
    PROFILE("GL_AddFoliageChunks");

    if (!g_Foliage.pWorld ||
        !gfx_foliage || SceneManager.GetFoliageDrawDistance() <= 0.0f ||
        g_pCam->HasFlags(CAM_WIREFRAME_TERRAIN) ||
        g_pCam->HasFlags(CAM_NO_TERRAIN))
        return;

    GL_RebuildFoliageChunks();
    GL_FlagVisibleFoliageChunks();

    static vector<uint> vFoliageOrder;
    vFoliageOrder.clear();

    // Load up vFoliageOrder prior to sorting
    for (int iY(0); iY < g_Foliage.iNumChunksY; ++iY)
        for (int iX(0); iX < g_Foliage.iNumChunksX; ++iX)
            if (g_Foliage.ppChunks[iY][iX].bVisible)
                vFoliageOrder.push_back((iX & 0xffff) | (iY << 16));

    GL_SortFoliageChunks(vFoliageOrder);

    switch (vid_foliageQuality)
    {
    case 0: // forward->back alphatest
        GL_FlagVisibleFoliageChunks();
        for (vector<uint>::reverse_iterator it(vFoliageOrder.rbegin()); it != vFoliageOrder.rend(); ++it)
        {
            int iChunkX(*it & 0xffff);
            int iChunkY(*it >> 16);

            SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

            if (oChunk.bVisible || vid_foliageNoCull)
            {
                g_RenderList.Add(K2_NEW(ctx_GL2,    CFoliageRenderer)(iChunkX, iChunkY, FOLIAGE_FRONTBACK, FOLIAGE_ALPHATEST | FOLIAGE_DEPTHWRITE));

                g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = false;
            }
        }
        break;
    case 1: // back->forward alphablend
        GL_FlagVisibleFoliageChunks();
        for (vector<uint>::iterator it(vFoliageOrder.begin()); it != vFoliageOrder.end(); ++it)
        {
            int iChunkX(*it & 0xffff);
            int iChunkY(*it >> 16);

            SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

            if (oChunk.bVisible || vid_foliageNoCull)
            {
                g_RenderList.Add(K2_NEW(ctx_GL2,    CFoliageRenderer)(iChunkX, iChunkY, FOLIAGE_BACKFRONT, FOLIAGE_ALPHABLEND));

                g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = false;
            }
        }
        break;
    case 2: // back->forward alphablend alphatest
        GL_FlagVisibleFoliageChunks();
        for (vector<uint>::iterator it(vFoliageOrder.begin()); it != vFoliageOrder.end(); ++it)
        {
            int iChunkX(*it & 0xffff);
            int iChunkY(*it >> 16);

            SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

            if (oChunk.bVisible || vid_foliageNoCull)
            {
                g_RenderList.Add(K2_NEW(ctx_GL2,    CFoliageRenderer)(iChunkX, iChunkY, FOLIAGE_BACKFRONT, FOLIAGE_ALPHATEST | FOLIAGE_ALPHABLEND | FOLIAGE_DEPTHWRITE));

                g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = false;
            }
        }
        break;
    case 3: // back->forward alphablend -> forward->back alphatest
        GL_FlagVisibleFoliageChunks();
        for (vector<uint>::iterator it(vFoliageOrder.begin()); it != vFoliageOrder.end(); ++it)
        {
            int iChunkX(*it & 0xffff);
            int iChunkY(*it >> 16);

            SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

            if (oChunk.bVisible || vid_foliageNoCull)
            {
                g_RenderList.Add(K2_NEW(ctx_GL2,    CFoliageRenderer)(iChunkX, iChunkY, FOLIAGE_BACKFRONT, FOLIAGE_ALPHABLEND));

                g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = false;
            }
        }

        GL_FlagVisibleFoliageChunks();
        for (vector<uint>::reverse_iterator it(vFoliageOrder.rbegin()); it != vFoliageOrder.rend(); ++it)
        {
            int iChunkX(*it & 0xffff);
            int iChunkY(*it >> 16);

            SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

            if (oChunk.bVisible || vid_foliageNoCull)
            {
                g_RenderList.Add(K2_NEW(ctx_GL2,    CFoliageRenderer)(iChunkX, iChunkY, FOLIAGE_FRONTBACK, FOLIAGE_ALPHATEST | FOLIAGE_DEPTHWRITE));

                g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = false;
            }
        }
        break;
    case 4: // back->forward alphablend -> forward->back alpablend alphatest
        GL_FlagVisibleFoliageChunks();
        for (vector<uint>::iterator it(vFoliageOrder.begin()); it != vFoliageOrder.end(); ++it)
        {
            int iChunkX(*it & 0xffff);
            int iChunkY(*it >> 16);

            SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

            if (oChunk.bVisible || vid_foliageNoCull)
            {
                g_RenderList.Add(K2_NEW(ctx_GL2,    CFoliageRenderer)(iChunkX, iChunkY, FOLIAGE_BACKFRONT, FOLIAGE_ALPHABLEND));

                g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = false;
            }
        }

        GL_FlagVisibleFoliageChunks();
        for (vector<uint>::iterator it(vFoliageOrder.begin()); it != vFoliageOrder.end(); ++it)
        {
            int iChunkX(*it & 0xffff);
            int iChunkY(*it >> 16);

            SFoliageChunk &oChunk = g_Foliage.ppChunks[iChunkY][iChunkX];

            if (oChunk.bVisible || vid_foliageNoCull)
            {
                g_RenderList.Add(K2_NEW(ctx_GL2,    CFoliageRenderer)(iChunkX, iChunkY, FOLIAGE_BACKFRONT, FOLIAGE_ALPHATEST | FOLIAGE_ALPHABLEND | FOLIAGE_DEPTHWRITE));

                g_Foliage.ppChunks[iChunkY][iChunkX].bVisible = false;
            }
        }
        break;
    }
}


/*====================
  GL_InitFoliage
  ====================*/
void    GL_InitFoliage()
{
    g_hFoliageDiffuseReference = g_ResourceManager.Register(_T("!foliage_d"), RES_REFERENCE);
    g_hFoliageVertexShaderReference = g_ResourceManager.Register(_T("!foliage_v"), RES_REFERENCE);
        
    g_ResourceManager.UpdateReference(g_hFoliageDiffuseReference, g_ResourceManager.GetWhiteTexture());

    g_hFoliageMaterial = g_ResourceManager.Register(_T("/world/foliage/materials/default.material"), RES_MATERIAL);

    g_hFoliageVertexShaderNormal = g_ResourceManager.Register(K2_NEW(ctx_GL2,    CVertexShader)(_T("foliage_color"), 0), RES_VERTEX_SHADER);
    //g_hFoliageVertexShaderCamera = g_ResourceManager.Register(new CVertexShader(_T("foliage_color_camera"), 0), RES_VERTEX_SHADER);

    g_mapFoliageAttributes[_T("a_vNormal")] = SVertexAttribute(GL_UNSIGNED_BYTE, 4, 12, false);
    g_mapFoliageAttributes[_T("a_vData")] = SVertexAttribute(GL_UNSIGNED_BYTE, 4, 16, false);
}


/*====================
  RebuildFoliage
  ====================*/
CMD(RebuildFoliage)
{
    GL_RebuildFoliage(foliage_chunkSize, g_Foliage.pWorld);

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
            SFoliageChunk &oChunk = g_Foliage.ppChunks[y][x];

            for (int iLayer(0); iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
            {
                for (int a(0); a < oChunk.iNumArrays[iLayer]; ++a)
                {
                    uiVertexBuffer += oChunk.pArrays[iLayer][a]->iNumFoliageQuads * sizeof(SFoliageVertex) * 4;
                    uiIndexBuffer += oChunk.pArrays[iLayer][a]->iNumFoliageQuads * sizeof(GLushort) * 6;
                }
            }
        }
    }

    Console << _T("Foliage Vertex Buffers: ") << GetByteString(uiVertexBuffer) << newl;
    Console << _T("Foliage Index Buffers: ") << GetByteString(uiIndexBuffer) << newl;

    return true;
}
