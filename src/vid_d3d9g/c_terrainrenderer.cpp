// (C)2006 S2 Games
// c_terrainrenderer.cpp
//
// Terrain chunk renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9g_main.h"
#include "d3d9g_util.h"
#include "d3d9g_material.h"
#include "d3d9g_scene.h"
#include "d3d9g_state.h"
#include "d3d9g_shader.h"
#include "d3d9g_terrain.h"
#include "d3d9g_texture.h"
#include "c_terrainrenderer.h"
#include "c_renderlist.h"
#include "c_shaderregistry.h"

#include "../k2/c_world.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_material.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_pool.h"
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
CPool<CTerrainRenderer> CTerrainRenderer::s_Pool(1, -1);
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOLF  (vid_drawTerrainBounds,     false,  CONEL_DEV);
CVAR_BOOLF  (vid_terrainWireframe,      false,  CONEL_DEV);
CVAR_BOOLF  (vid_terrainLayer1,         true,   CONEL_DEV);
CVAR_BOOLF  (vid_terrainLayer2,         true,   CONEL_DEV);
CVAR_INTF   (vid_terrainShowNormals,    0,      CONEL_DEV);
CVAR_INTF   (vid_terrainShowGrid,       0,      CONEL_DEV);
CVAR_BOOLF  (vid_terrainShadows,        true,   CVAR_SAVECONFIG);
CVAR_BOOL   (vid_terrainNoCull,         false);
CVAR_BOOL   (vid_terrainCliffs,         true);

EXTERN_CVAR_FLOAT(vid_depthBiasScale);

CONST_STRING(ALPHA, _T("alpha"));
CONST_STRING(DIFFUSE0, _T("diffuse0"));
CONST_STRING(NORMALMAP0, _T("normalmap0"));
CONST_STRING(DIFFUSE1, _T("diffuse1"));
CONST_STRING(NORMALMAP1, _T("normalmap1"));
//=============================================================================

/*====================
  CTerrainRenderer::operator new
  ====================*/
void*   CTerrainRenderer::operator new(size_t z)
{
    return s_Pool.Allocate();
}


/*====================
  CTerrainRenderer::CTerrainRenderer
  ====================*/
CTerrainRenderer::CTerrainRenderer() :
IRenderer(RT_UNKNOWN)
{
    if (vid_drawTerrainBounds)
    {
        for (int iChunkY(0); iChunkY != terrain.iNumChunksY; ++iChunkY)
        {
            for (int iChunkX(0); iChunkX != terrain.iNumChunksX; ++iChunkX)
            {
                STerrainChunk *chunk = &terrain.chunks[iChunkY][iChunkX];

                if (!chunk->bVisible)
                    continue;
                
                D3D_AddBox(chunk->bbBounds, CVec4f(0.0f, 1.0f, 1.0f, 1.0f), g_mIdentity);
            }
        }
    }
}


/*====================
  CTerrainRenderer::~CTerrainRenderer
  ====================*/
CTerrainRenderer::~CTerrainRenderer()
{
}


/*====================
  CTerrainRenderer::Setup
  ====================*/
void    CTerrainRenderer::Setup(EMaterialPhase ePhase)
{
    PROFILE("CTerrainRenderer::Setup");

    m_bRender = false; // Set to true if we make it to the end of the function

    if (ePhase == PHASE_SHADOW && !vid_terrainShadows)
        return;

    CMaterial &cMaterial(D3D_GetMaterial(vid_terrainSinglePass ? g_hTerrainSingleMaterial : g_hTerrainMaterial));

    if (!cMaterial.HasPhase(ePhase))
        return; // Leave if we don't have this phase

    m_pCam = g_pCam;
    m_bLighting = gfx_lighting;
    m_bShadows = g_bCamShadows;
    m_bFog = g_bCamFog;
    m_vAmbient = SceneManager.GetTerrainAmbientColor();
    m_vSunColor = SceneManager.GetTerrainSunColor();

    m_iNumActiveBones = 0;

    // Pick the four best point lights to light the terrain
    m_iNumActivePointLights = 0;

    if (ePhase == PHASE_COLOR)
    {
        SceneLightList &LightList(SceneManager.GetLightList());
        SceneLightList::iterator itEnd(LightList.end());
        for (SceneLightList::iterator it(LightList.begin()); it != itEnd && m_iNumActivePointLights != g_iMaxDynamicLights; ++it)
        {
            SSceneLightEntry &cEntry(**it);
            const CSceneLight &scLight(cEntry.cLight);

            if (cEntry.bCull)
                continue;

            CSphere cLightSphere(scLight.GetPosition(), scLight.GetFalloffEnd());

            for (int iChunkY(0); iChunkY != terrain.iNumChunksY; ++iChunkY)
            {
                for (int iChunkX(0); iChunkX != terrain.iNumChunksX; ++iChunkX)
                {
                    STerrainChunk &cChunk(terrain.chunks[iChunkY][iChunkX]);

                    if (!cChunk.bVisible)
                        continue;

                    if (I_SphereBoundsIntersect(cLightSphere, cChunk.bbBounds))
                    {
                        m_vPointLightPosition[m_iNumActivePointLights] = scLight.GetPosition();
                        m_vPointLightColor[m_iNumActivePointLights] = scLight.GetColor();
                        m_fPointLightFalloffStart[m_iNumActivePointLights] = scLight.GetFalloffStart();
                        m_fPointLightFalloffEnd[m_iNumActivePointLights] = scLight.GetFalloffEnd();
                        ++m_iNumActivePointLights;

                        iChunkY = terrain.iNumChunksY - 1; // Kill the chunk search
                        break;
                    }
                }
            }
        }
    }

    m_bObjectColor = false;
    m_pCurrentEntity = NULL;

    m_mWorld = g_mIdentity;
    m_mWorldRotate = g_mIdentity;
    m_mWorldViewProj = m_mWorld * g_mViewProj;

    m_bRender = true;

    m_bFirstChunk = true;
    m_iAlphaStageIndex = -1;
    m_iDiffuse0StageIndex = -1;
    m_iNormalmap0StageIndex = -1;
    m_iNormalmap01StageIndex = -1;
    m_iDiffuse1StageIndex = -1;
    m_iNormalmap1StageIndex = -1;
    m_iNormalmap11StageIndex = -1;

    g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
    g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
    g_ShaderRegistry.SetLighting(m_bLighting);
    g_ShaderRegistry.SetShadows(m_bShadows);
    g_ShaderRegistry.SetFogofWar(g_bFogofWar);
    g_ShaderRegistry.SetFog(m_bFog);
    g_ShaderRegistry.SetTexcoords(m_iTexcoords);
    g_ShaderRegistry.SetTexkill(m_bTexkill);

    CMaterialPhase &cPhase(cMaterial.GetPhase(ePhase));

    // Set sorting variables
    m_bTranslucent = false;
    m_iLayer = cPhase.GetLayer();
    m_iEffectLayer = vid_terrainShowGrid || vid_terrainShowNormals ? 0 : 1;
    m_iVertexType = VERTEX_TERRAIN;
    m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader());
    m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader());
    m_uiVertexBuffer = 0;
    m_uiIndexBuffer = 0;
    m_fDepth = 0.0f;
    m_bRefractive = cPhase.GetRefractive();
}


/*====================
  CTerrainRenderer::RenderChunk
  ====================*/
void    CTerrainRenderer::RenderChunk(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
    PROFILE("CTerrainRenderer::RenderChunk");

    STerrainChunk &chunk(terrain.chunks[iChunkY][iChunkX]);

    //
    // Set per chunk shader vars
    //

    D3DXMatrixTranslation
    (
        &g_mWorld,
        iChunkX * terrain.iChunkSize * terrain.pWorld->GetScale(), 
        iChunkY * terrain.iChunkSize * terrain.pWorld->GetScale(),
        0.0f
    );

    g_mWorldViewProj = g_mWorld * g_mViewProj;

    //
    // Render
    //

    if (gfx_points && ePhase != PHASE_SHADOW)
    {
        D3D_SetRenderState(D3DRS_POINTSIZE, D3D_DWORD(1.0f));
        D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
    }
    else if (g_pCam->HasFlags(CAM_WIREFRAME_TERRAIN) || vid_terrainWireframe || gfx_wireframe && ePhase != PHASE_SHADOW)
    {
        D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    }
    else
    {
        D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
    
    CMaterial &cMaterial(D3D_GetMaterial(vid_terrainSinglePass ? g_hTerrainSingleMaterial : g_hTerrainMaterial));

    D3D_SetStreamSource(0, chunk.pVB, 0, sizeof(STerrainVertex));

    if (ePhase == PHASE_SHADOW || ePhase == PHASE_DEPTH)
    {
        if (m_bFirstChunk)
        {
            D3D_SelectMaterial(cMaterial, ePhase, VERTEX_TERRAIN, g_pCam->GetTime(), gfx_depthFirst);
            m_bFirstChunk = false;

            m_iAlphaStageIndex = D3D_GetSamplerStageIndex(ALPHA);
            m_iDiffuse0StageIndex = D3D_GetSamplerStageIndex(DIFFUSE0);
            m_iNormalmap0StageIndex = D3D_GetSamplerStageIndex(NORMALMAP0);
            m_iDiffuse1StageIndex = D3D_GetSamplerStageIndex(DIFFUSE1);
            m_iNormalmap1StageIndex = D3D_GetSamplerStageIndex(NORMALMAP1);
        }
        else
        {
            D3D_UpdateVertexShaderParams(cMaterial, g_pCam->GetTime());
        }

#if 0
        if (vid_terrainSinglePass)
            D3D_SetIndices(chunk.pIBSingle);
        else
            D3D_SetIndices(chunk.pShadowIB);
#else
        D3D_SetIndices(chunk.pIBSingle);
#endif

        D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VERTS_PER_CHUNK, 0, chunk.uiNumFaces);

        SceneStats.RecordBatch(chunk.uiNumFaces + 2, chunk.uiNumFaces, ePhase, SSBATCH_TERRAIN);
    }
    else
    {
        if (vid_terrainSinglePass)
        {
            bool bRXGB(vid_shaderRXGBNormalmap);
            g_iTerrainAlphaMap = chunk.iAlphaMap == -1 ? g_iWhite : chunk.iAlphaMap;
            
            D3D_SetIndices(chunk.pIBSingle);

            if (m_bFirstChunk)
            {
                D3D_SelectMaterial(cMaterial, ePhase, VERTEX_TERRAIN, g_pCam->GetTime(), gfx_depthFirst);
                m_bFirstChunk = false;

                if (bRXGB)
                {
                    m_iAlphaStageIndex = D3D_GetSamplerStageIndex(ALPHA);
                    m_iDiffuse0StageIndex = D3D_GetSamplerStageIndex(DIFFUSE0);
                    m_iNormalmap0StageIndex = D3D_GetSamplerStageIndex(NORMALMAP0, 0);
                    m_iNormalmap01StageIndex = D3D_GetSamplerStageIndex(NORMALMAP0, 1);
                    m_iDiffuse1StageIndex = D3D_GetSamplerStageIndex(DIFFUSE1);
                    m_iNormalmap1StageIndex = D3D_GetSamplerStageIndex(NORMALMAP1, 0);
                    m_iNormalmap11StageIndex = D3D_GetSamplerStageIndex(NORMALMAP1, 1);
                }
                else
                {
                    m_iAlphaStageIndex = D3D_GetSamplerStageIndex(ALPHA);
                    m_iDiffuse0StageIndex = D3D_GetSamplerStageIndex(DIFFUSE0);
                    m_iNormalmap0StageIndex = D3D_GetSamplerStageIndex(NORMALMAP0);
                    m_iDiffuse1StageIndex = D3D_GetSamplerStageIndex(DIFFUSE1);
                    m_iNormalmap1StageIndex = D3D_GetSamplerStageIndex(NORMALMAP1);
                }
            }
            else
            {
                D3D_UpdateVertexShaderParams(cMaterial, g_pCam->GetTime());
                D3D_UpdateShaderTextureIndex(m_iAlphaStageIndex, g_iTerrainAlphaMap);
            }           

            if (vid_terrainLayer1 && vid_terrainLayer2)
            {
                for (int n(0); n < chunk.iNumSingleArrays; ++n)
                {
                    if (bRXGB)
                    {
                        D3D_UpdateShaderTexture(m_iDiffuse0StageIndex, chunk.pSingleArrays[n]->ahDiffuse[0]);
                        D3D_UpdateShaderTexture(m_iNormalmap0StageIndex, chunk.pSingleArrays[n]->ahNormalmap[0], 0);
                        D3D_UpdateShaderTexture(m_iNormalmap01StageIndex, chunk.pSingleArrays[n]->ahNormalmap[0], 1);
                        D3D_UpdateShaderTexture(m_iDiffuse1StageIndex, chunk.pSingleArrays[n]->ahDiffuse[1]);
                        D3D_UpdateShaderTexture(m_iNormalmap1StageIndex, chunk.pSingleArrays[n]->ahNormalmap[1], 0);
                        D3D_UpdateShaderTexture(m_iNormalmap11StageIndex, chunk.pSingleArrays[n]->ahNormalmap[1], 1);
                    }
                    else
                    {
                        D3D_UpdateShaderTexture(m_iDiffuse0StageIndex, chunk.pSingleArrays[n]->ahDiffuse[0]);
                        D3D_UpdateShaderTexture(m_iNormalmap0StageIndex, chunk.pSingleArrays[n]->ahNormalmap[0]);
                        D3D_UpdateShaderTexture(m_iDiffuse1StageIndex, chunk.pSingleArrays[n]->ahDiffuse[1]);
                        D3D_UpdateShaderTexture(m_iNormalmap1StageIndex, chunk.pSingleArrays[n]->ahNormalmap[1]);
                    }
    
                    D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VERTS_PER_CHUNK, chunk.pSingleArrays[n]->uiStartIndex, chunk.pSingleArrays[n]->iNumTris);

                    SceneStats.RecordBatch(chunk.pSingleArrays[n]->iNumTris * 2, chunk.pSingleArrays[n]->iNumTris, ePhase, SSBATCH_TERRAIN);
                }
            }
            else
            {
                for (int n = 0; n < chunk.iNumSingleArrays; ++n)
                {
                    if (vid_terrainLayer1)
                    {
                        D3D_UpdateShaderTexture(m_iDiffuse0StageIndex, chunk.pSingleArrays[n]->ahDiffuse[0]);
                        
                        if (bRXGB)
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap0StageIndex, chunk.pSingleArrays[n]->ahNormalmap[0], 0);
                            D3D_UpdateShaderTexture(m_iNormalmap01StageIndex, chunk.pSingleArrays[n]->ahNormalmap[0], 1);
                        }
                        else
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap0StageIndex, chunk.pSingleArrays[n]->ahNormalmap[0]);
                        }
                    }
                    else
                    {
                        D3D_UpdateShaderTexture(m_iDiffuse0StageIndex, g_hTerrainCheckerDiffuse);
                        
                        if (bRXGB)
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap0StageIndex, g_hTerrainCheckerNormalmap, 0);
                            D3D_UpdateShaderTexture(m_iNormalmap01StageIndex, g_hTerrainCheckerNormalmap, 1);
                        }
                        else
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap0StageIndex, g_hTerrainCheckerNormalmap);
                        }
                    }

                    if (vid_terrainLayer2)
                    {
                        D3D_UpdateShaderTexture(m_iDiffuse1StageIndex, chunk.pSingleArrays[n]->ahDiffuse[1]);
                        
                        if (bRXGB)
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap1StageIndex, chunk.pSingleArrays[n]->ahNormalmap[1], 0);
                            D3D_UpdateShaderTexture(m_iNormalmap11StageIndex, chunk.pSingleArrays[n]->ahNormalmap[1], 1);
                        }
                        else
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap1StageIndex, chunk.pSingleArrays[n]->ahNormalmap[1]);
                        }
                    }
                    else
                    {
                        D3D_UpdateShaderTexture(m_iDiffuse1StageIndex, g_ResourceManager.GetInvisTexture());

                        if (bRXGB)
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap1StageIndex, g_hTerrainCheckerNormalmap, 0);
                            D3D_UpdateShaderTexture(m_iNormalmap11StageIndex, g_hTerrainCheckerNormalmap, 1);
                        }
                        else
                        {
                            D3D_UpdateShaderTexture(m_iNormalmap1StageIndex, g_hTerrainCheckerNormalmap);
                        }
                    }

                    D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VERTS_PER_CHUNK, chunk.pSingleArrays[n]->uiStartIndex, chunk.pSingleArrays[n]->iNumTris);

                    SceneStats.RecordBatch(chunk.pSingleArrays[n]->iNumTris * 2, chunk.pSingleArrays[n]->iNumTris, ePhase, SSBATCH_TERRAIN);
                }
            }
        }
        else
        {
#if 0
            //
            // Layer 1
            //

            if (vid_terrainLayer1)
            {
                g_iTerrainAlphaMap = g_iWhite;
                cMaterial.GetPhase(ePhase).SetSrcBlend(BLEND_ONE);
                cMaterial.GetPhase(ePhase).SetDstBlend(BLEND_ZERO);
                cMaterial.GetPhase(ePhase).SetTranslucent(false);
                cMaterial.GetPhase(ePhase).SetDepthWrite(true);

                D3D_SetIndices(chunk.pIB[0]);

                for (int n = 0; n < chunk.iNumArrays[0]; ++n)
                {
                    g_ResourceManager.UpdateReference(g_hTerrainDiffuseReference, chunk.pArrays[0][n]->hDiffuse);
                    g_ResourceManager.UpdateReference(g_hTerrainNormalmapReference, chunk.pArrays[0][n]->hNormalmap);

                    D3D_SelectMaterial(cMaterial, ePhase, VERTEX_TERRAIN, g_pCam->GetTime(), gfx_depthFirst);
                    
                    D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VERTS_PER_CHUNK, chunk.pArrays[0][n]->uiStartIndex, chunk.pArrays[0][n]->iNumTris);

                    SceneStats.RecordBatch(chunk.pArrays[0][n]->iNumTris * 2, chunk.pArrays[0][n]->iNumTris, ePhase, SSBATCH_TERRAIN);
                }
            }

            //
            // Layer 2
            //

            if (vid_terrainLayer2)
            {
                g_iTerrainAlphaMap = chunk.iAlphaMap == -1 ? g_iWhite : chunk.iAlphaMap;
                cMaterial.GetPhase(ePhase).SetSrcBlend(BLEND_SRC_ALPHA);
                cMaterial.GetPhase(ePhase).SetDstBlend(BLEND_ONE_MINUS_SRC_ALPHA);
                cMaterial.GetPhase(ePhase).SetTranslucent(true);
                cMaterial.GetPhase(ePhase).SetDepthWrite(false);

                D3D_SetIndices(chunk.pIB[1]);

                for (int n = 0; n < chunk.iNumArrays[1]; ++n)
                {
                    g_ResourceManager.UpdateReference(g_hTerrainDiffuseReference, chunk.pArrays[1][n]->hDiffuse);
                    g_ResourceManager.UpdateReference(g_hTerrainNormalmapReference, chunk.pArrays[1][n]->hNormalmap);

                    D3D_SelectMaterial(cMaterial, ePhase, VERTEX_TERRAIN, g_pCam->GetTime(), gfx_depthFirst);

                    D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, VERTS_PER_CHUNK, chunk.pArrays[1][n]->uiStartIndex, chunk.pArrays[1][n]->iNumTris);

                    SceneStats.RecordBatch(chunk.pArrays[1][n]->iNumTris * 2, chunk.pArrays[1][n]->iNumTris, ePhase, SSBATCH_TERRAIN);
                }
            }
#endif
        }

        if (vid_terrainShowNormals)
            RenderChunkNormals(ePhase, iChunkX, iChunkY);

        if (vid_terrainShowGrid)
            RenderChunkGrid(ePhase, iChunkX, iChunkY);
    }
}


/*====================
  CTerrainRenderer::RenderChunkNormals
  ====================*/
void    CTerrainRenderer::RenderChunkNormals(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
    PROFILE("CTerrainRenderer::RenderChunkNormals");

    STerrainChunk   &chunk(terrain.chunks[iChunkY][iChunkX]);
    const CWorld    *pWorld(terrain.pWorld);

    switch (vid_terrainShowNormals)
    {
    case 1: // Vertex normals
        {
            if (!chunk.pVBVertexNormals)
            {
                if (FAILED(g_pd3dDevice->CreateVertexBuffer(VERTS_PER_CHUNK * 2 * sizeof(SLineVertex),
                        D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &chunk.pVBVertexNormals, NULL)))
                    K2System.Error(_T("D3D_RenderTerrainChunkNormals(): CreateVertexBuffer failed"));

                SLineVertex *pVertices;

                if (FAILED(chunk.pVBVertexNormals->Lock(0, VERTS_PER_CHUNK * 2 * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
                    return;

                int iStartX(iChunkX * terrain.iChunkSize);      // Grid top-left tile coord
                int iStartY(iChunkY * terrain.iChunkSize);      // Grid top-left tile coord

                float fXOrigin((iStartX) * pWorld->GetScale());
                float fYOrigin((iStartY) * pWorld->GetScale());

                int iVert = 0;
                for (int iY = iStartY; iY <= iStartY + terrain.iChunkSize; ++iY)
                {
                    for (int iX = iStartX; iX <= iStartX + terrain.iChunkSize; ++iX)
                    {
                        pVertices[iVert].v = CVec3f(pWorld->ScaleGridCoord(iX) - fXOrigin, pWorld->ScaleGridCoord(iY) - fYOrigin, pWorld->GetGridPoint(iX, iY));
                        pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 255, 255);
                        ++iVert;

                        pVertices[iVert].v = pVertices[iVert - 1].v + pWorld->GetGridNormal(iX, iY) * 75.0f;
                        pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 255, 255);
                        ++iVert;
                    }
                }

                chunk.pVBVertexNormals->Unlock();
            }

            D3D_SetStreamSource(0, chunk.pVBVertexNormals, 0, sizeof(SLineVertex));
            D3D_SelectMaterial(g_SimpleMaterial3DColored, ePhase, VERTEX_LINE, g_pCam->GetTime(), false);
            D3D_DrawPrimitive(D3DPT_LINELIST, 0, VERTS_PER_CHUNK);
        } break;
    case 2: // Tile normals
        {
            if (!chunk.pVBTileNormals)
            {
                if (FAILED(g_pd3dDevice->CreateVertexBuffer(TILES_PER_CHUNK * 4 * sizeof(SLineVertex),
                        D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &chunk.pVBTileNormals, NULL)))
                    K2System.Error(_T("D3D_RenderTerrainChunkNormals(): CreateVertexBuffer failed"));

                SLineVertex *pVertices;

                if (FAILED(chunk.pVBTileNormals->Lock(0, TILES_PER_CHUNK * 4 * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
                    return;

                int iStartX(iChunkX * terrain.iChunkSize);      // Grid top-left tile coord
                int iStartY(iChunkY * terrain.iChunkSize);      // Grid top-left tile coord

                float fXOrigin((iStartX) * pWorld->GetScale());
                float fYOrigin((iStartY) * pWorld->GetScale());

                int iVert = 0;
                for (int iY = iStartY; iY < iStartY + terrain.iChunkSize; ++iY)
                {
                    for (int iX = iStartX; iX < iStartX + terrain.iChunkSize; ++iX)
                    {
                        if (pWorld->GetTileSplit(iX, iY) == SPLIT_NEG)
                        {
                            // Left Triangle
                            float fXLeft(pWorld->ScaleGridCoord(iX) + pWorld->GetScale() * 0.25f);
                            float fYLeft(pWorld->ScaleGridCoord(iY) + pWorld->GetScale() * 0.25f);

                            pVertices[iVert].v = CVec3f(fXLeft - fXOrigin, fYLeft - fYOrigin, pWorld->GetTerrainHeight(fXLeft, fYLeft));
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;

                            pVertices[iVert].v = pVertices[iVert - 1].v + pWorld->GetTileNormal(iX, iY, TRIANGLE_LEFT) * 50.0f;
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;

                            // Right Triangle
                            float fXRight(pWorld->ScaleGridCoord(iX) + pWorld->GetScale() * 0.75f);
                            float fYRight(pWorld->ScaleGridCoord(iY) + pWorld->GetScale() * 0.75f);

                            pVertices[iVert].v = CVec3f(fXRight - fXOrigin, fYRight - fYOrigin, pWorld->GetTerrainHeight(fXRight, fYRight));
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;

                            pVertices[iVert].v = pVertices[iVert - 1].v + pWorld->GetTileNormal(iX, iY, TRIANGLE_RIGHT) * 50.0f;
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;
                        }
                        else
                        {
                            // Left Triangle
                            float fXLeft(pWorld->ScaleGridCoord(iX) + pWorld->GetScale() * 0.25f);
                            float fYLeft(pWorld->ScaleGridCoord(iY) + pWorld->GetScale() * 0.75f);

                            pVertices[iVert].v = CVec3f(fXLeft - fXOrigin, fYLeft - fYOrigin, pWorld->GetTerrainHeight(fXLeft, fYLeft));
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;

                            pVertices[iVert].v = pVertices[iVert - 1].v + pWorld->GetTileNormal(iX, iY, TRIANGLE_LEFT) * 50.0f;
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;

                            // Right Triangle
                            float fXRight(pWorld->ScaleGridCoord(iX) + pWorld->GetScale() * 0.75f);
                            float fYRight(pWorld->ScaleGridCoord(iY) + pWorld->GetScale() * 0.25f);

                            pVertices[iVert].v = CVec3f(fXRight - fXOrigin, fYRight - fYOrigin, pWorld->GetTerrainHeight(fXRight, fYRight));
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;

                            pVertices[iVert].v = pVertices[iVert - 1].v + pWorld->GetTileNormal(iX, iY, TRIANGLE_RIGHT) * 50.0f;
                            pVertices[iVert].color = D3DCOLOR_ARGB(255, 255, 0, 255);
                            ++iVert;
                        }
                    }
                }

                chunk.pVBTileNormals->Unlock();
            }

            D3D_SetStreamSource(0, chunk.pVBTileNormals, 0, sizeof(SLineVertex));
            D3D_SelectMaterial(g_SimpleMaterial3DColored, ePhase, VERTEX_LINE, g_pCam->GetTime(), false);
            D3D_DrawPrimitive(D3DPT_LINELIST, 0, TILES_PER_CHUNK * 2);
        } break;
    }

    m_bFirstChunk = true; // So chunk renderer properly resets terrain material
}


/*====================
  CTerrainRenderer::RenderChunkGrid
  ====================*/
void    CTerrainRenderer::RenderChunkGrid(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
    PROFILE("CTerrainRenderer::RenderChunkGrid");

    STerrainChunk   &chunk(terrain.chunks[iChunkY][iChunkX]);
    const CWorld    *pWorld(terrain.pWorld);

    uint uiGridSize(1 << (vid_terrainShowGrid - 1));
    
    uint uiNumEdges(terrain.iChunkSize * ((terrain.iChunkSize / uiGridSize) + 1) * 2);

    if (chunk.uiGridSize != uiGridSize)
    {
        SAFE_RELEASE(chunk.pIBGrid);
        chunk.uiGridSize = uiGridSize;
    }

    if (!chunk.pVBGrid)
    {
        if (FAILED(g_pd3dDevice->CreateVertexBuffer(VERTS_PER_CHUNK * sizeof(SLineVertex),
                D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &chunk.pVBGrid, NULL)))
            K2System.Error(_T("D3D_RenderTerrainChunkNormals(): CreateVertexBuffer failed"));

        SLineVertex *pVertices;

        if (FAILED(chunk.pVBGrid->Lock(0, VERTS_PER_CHUNK * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
            return;

        int iStartX(iChunkX * terrain.iChunkSize);      // Grid top-left tile coord
        int iStartY(iChunkY * terrain.iChunkSize);      // Grid top-left tile coord

        float fXOrigin((iStartX) * pWorld->GetScale());
        float fYOrigin((iStartY) * pWorld->GetScale());

        int iVert = 0;
        for (int iY = iStartY; iY <= iStartY + terrain.iChunkSize; ++iY)
        {
            for (int iX = iStartX; iX <= iStartX + terrain.iChunkSize; ++iX)
            {
                pVertices[iVert].v = CVec3f(pWorld->ScaleGridCoord(iX) - fXOrigin, pWorld->ScaleGridCoord(iY) - fYOrigin, pWorld->GetGridPoint(iX, iY));
                pVertices[iVert].color = D3DCOLOR_ARGB(128, 255, 255, 255);
                ++iVert;
            }
        }

        chunk.pVBGrid->Unlock();
    }

    if (!chunk.pIBGrid)
    {
        if (FAILED(g_pd3dDevice->CreateIndexBuffer(uiNumEdges * 2 * sizeof(WORD),
                D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &chunk.pIBGrid, NULL)))
            K2System.Error(_T("D3D_RenderTerrainChunkNormals(): CreateVertexBuffer failed"));

        WORD *pIndices;

        if (FAILED(chunk.pIBGrid->Lock(0, uiNumEdges * 2 * sizeof(WORD), (void**)&pIndices, D3DLOCK_NOSYSLOCK)))
            return;

        // Horizontal grid
        for (int iY(0); iY < terrain.iChunkSize; iY += uiGridSize)
        {
            for (int iX(0); iX < terrain.iChunkSize; ++iX)
            {
                *pIndices = iY * (terrain.iChunkSize + 1) + iX;
                ++pIndices;

                *pIndices = iY * (terrain.iChunkSize + 1) + iX + 1;
                ++pIndices;
            }
        }

        // Vertical grid
        for (int iX(0); iX < terrain.iChunkSize; iX += uiGridSize)
        {
            for (int iY(0); iY < terrain.iChunkSize; ++iY)
            {
                *pIndices = iY * (terrain.iChunkSize + 1) + iX;
                ++pIndices;

                *pIndices = (iY + 1) * (terrain.iChunkSize + 1) + iX;
                ++pIndices;
            }
        }

        chunk.pIBGrid->Unlock();
    }

    D3D_SetStreamSource(0, chunk.pVBGrid, 0, sizeof(SLineVertex));
    D3D_SetIndices(chunk.pIBGrid);

    D3D_SelectMaterial(g_SimpleMaterial3DColoredBias, ePhase, VERTEX_LINE, g_pCam->GetTime(), false);

    D3D_DrawIndexedPrimitive(D3DPT_LINELIST, 0, 0, VERTS_PER_CHUNK, 0, uiNumEdges);
    
    m_bFirstChunk = true; // So chunk renderer properly resets terrain material
}


/*====================
  CTerrainRenderer::RenderChunkCliffs
  ====================*/
void    CTerrainRenderer::RenderChunkCliffs(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
    PROFILE("CTerrainRenderer::RenderChunkCliffs");

    g_mWorld = g_mIdentity;
    g_mWorldViewProj = g_mViewProj;

    STerrainChunk &chunk(terrain.chunks[iChunkY][iChunkX]);

    if (chunk.iNumCliffArrays == 0)
        return;

    //
    // Render
    //

    if (gfx_points && ePhase != PHASE_SHADOW)
    {
        D3D_SetRenderState(D3DRS_POINTSIZE, D3D_DWORD(1.0f));
        D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
    }
    else if (g_pCam->HasFlags(CAM_WIREFRAME_TERRAIN) || vid_terrainWireframe || gfx_wireframe && ePhase != PHASE_SHADOW)
    {
        D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    }
    else
    {
        D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
    }
    
    D3D_SetStreamSource(0, chunk.pVBCliff, 0, g_iCliffVertexStride);
    D3D_SetIndices(chunk.pIBCliff);

    if (ePhase == PHASE_SHADOW || ePhase == PHASE_DEPTH)
    {
        CMaterial &cMaterial(D3D_GetMaterial(chunk.pCliffArrays[0]->hMaterial)); // Cliffs share shadow phase

        if (m_bFirstChunk)
        {
            D3D_SelectMaterial(cMaterial, ePhase, g_iCliffVertexDecl, g_pCam->GetTime(), gfx_depthFirst);
            m_bFirstChunk = false;
        }

        D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, chunk.iNumCliffVerts, 0, chunk.iNumCliffFaces);

        SceneStats.RecordBatch(chunk.iNumCliffVerts, chunk.iNumCliffFaces, ePhase, SSBATCH_STATICMESH);
    }
    else
    {
        for (int i(0); i < chunk.iNumCliffArrays; ++i)
        {
            CMaterial &cMaterial(D3D_GetMaterial(chunk.pCliffArrays[i]->hMaterial));

            g_ResourceManager.UpdateReference(g_hTerrainDiffuseReference, chunk.pCliffArrays[i]->hDiffuse);
            g_ResourceManager.UpdateReference(g_hTerrainNormalmapReference, chunk.pCliffArrays[i]->hNormalmap);

            D3D_SelectMaterial(cMaterial, ePhase, g_iCliffVertexDecl, g_pCam->GetTime(), gfx_depthFirst);

            D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0,
                chunk.pCliffArrays[i]->uiStartVert,
                chunk.pCliffArrays[i]->uiNumVerts,
                chunk.pCliffArrays[i]->uiStartIndex,
                chunk.pCliffArrays[i]->uiNumFaces
            );

            SceneStats.RecordBatch(chunk.pCliffArrays[i]->uiNumVerts, chunk.pCliffArrays[i]->uiNumFaces, ePhase, SSBATCH_STATICMESH);
        }
    }
}


/*====================
  CTerrainRenderer::Render
  ====================*/
void    CTerrainRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CTerrainRenderer::Render");

    if (!m_bRender)
        return;

    SetShaderVars();

    for (int iChunkY(0); iChunkY != terrain.iNumChunksY; ++iChunkY)
    {
        for (int iChunkX(0); iChunkX != terrain.iNumChunksX; ++iChunkX)
        {
            STerrainChunk *chunk = &terrain.chunks[iChunkY][iChunkX];

            if ((ePhase == PHASE_SHADOW && chunk->bVisibleShadow) || (ePhase != PHASE_SHADOW && chunk->bVisible) || vid_terrainNoCull)
            {
                RenderChunk(ePhase, iChunkX, iChunkY);
            }
        }
    }

    if (vid_terrainCliffs && !(g_pCam->GetFlags() & CAM_NO_CLIFFS))
    {
        m_bFirstChunk = true;

        for (int iChunkY(0); iChunkY != terrain.iNumChunksY; ++iChunkY)
        {
            for (int iChunkX(0); iChunkX != terrain.iNumChunksX; ++iChunkX)
            {
                STerrainChunk *chunk = &terrain.chunks[iChunkY][iChunkX];

                if ((ePhase == PHASE_SHADOW && chunk->bVisibleShadow) || (ePhase != PHASE_SHADOW && chunk->bVisible) || vid_terrainNoCull)
                {
                    RenderChunkCliffs(ePhase, iChunkX, iChunkY);
                }
            }
        }
    }
}
