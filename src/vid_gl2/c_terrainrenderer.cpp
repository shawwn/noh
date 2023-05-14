// (C)2008 S2 Games
// c_terrainrenderer.cpp
//
// Terrain chunk renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_terrainrenderer.h"

#include "c_gfxutils.h"
#include "c_gfxmaterials.h"
#include "c_gfxterrain.h"
#include "c_gfxtextures.h"
#include "c_gfxshaders.h"
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
// Cvars
//=============================================================================
CVAR_BOOLF  (vid_drawTerrainBounds,     false,  CONEL_DEV);
CVAR_BOOLF  (vid_terrainWireframe,      false,  CONEL_DEV);
CVAR_BOOLF  (vid_terrainLayer1,         true,   CONEL_DEV);
CVAR_BOOLF  (vid_terrainLayer2,         true,   CONEL_DEV);
CVAR_INTF   (vid_terrainShowNormals,    0,      CONEL_DEV);
CVAR_BOOL   (vid_terrainNoCull,         false);
CVAR_BOOL   (vid_terrainCliffs,         true);
//=============================================================================

CPool<CTerrainRenderer> CTerrainRenderer::s_Pool(1, uint(-1));

/*====================
  CTerrainRenderer::operator new
  ====================*/
void*   CTerrainRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CTerrainRenderer::CTerrainRenderer
  ====================*/
CTerrainRenderer::CTerrainRenderer()
{
#if 0
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
#endif
}


/*====================
  CTerrainRenderer::~CTerrainRenderer
  ====================*/
CTerrainRenderer::~CTerrainRenderer()
{
}


CVAR_BOOL(vid_skipTerrainRenderer, false);

/*====================
  CTerrainRenderer::Setup
  ====================*/
void    CTerrainRenderer::Setup(EMaterialPhase ePhase)
{
    if (vid_skipTerrainRenderer)
        return;

    PROFILE("CTerrainRenderer::Setup");

    m_pCam = g_pCam;
    m_bLighting = gfx_lighting;
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

            for (int iChunkY(0); iChunkY != GfxTerrain->iNumChunksY; ++iChunkY)
            {
                for (int iChunkX(0); iChunkX != GfxTerrain->iNumChunksX; ++iChunkX)
                {
                    STerrainChunk &cChunk(GfxTerrain->chunks[iChunkY][iChunkX]);

                    if (!cChunk.bVisible)
                        continue;

                    if (I_SphereBoundsIntersect(cLightSphere, cChunk.bbBounds))
                    {
                        m_vPointLightPosition[m_iNumActivePointLights] = scLight.GetPosition();
                        m_vPointLightColor[m_iNumActivePointLights] = scLight.GetColor();
                        m_fPointLightFalloffStart[m_iNumActivePointLights] = scLight.GetFalloffStart();
                        m_fPointLightFalloffEnd[m_iNumActivePointLights] = scLight.GetFalloffEnd();
                        ++m_iNumActivePointLights;

                        iChunkY = GfxTerrain->iNumChunksY - 1; // Kill the chunk search
                        break;
                    }
                }
            }
        }
    }

    m_bObjectColor = false;
    m_pCurrentEntity = nullptr;

    m_mWorld = g_mIdentity;
    m_mWorldRotate = g_mIdentity;
    m_mWorldViewProj = m_mWorld * g_mViewProj;

    m_bRender = true;

    m_bFirstChunk = true;

    g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
    g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
    g_ShaderRegistry.SetLighting(m_bLighting);
    g_ShaderRegistry.SetShadows(m_bShadows);
    g_ShaderRegistry.SetFogofWar(g_bFogofWar);
    g_ShaderRegistry.SetFog(m_bFog);

    CMaterial &cMaterial(GfxUtils->GetMaterial(vid_terrainSinglePass ? GfxTerrain->hTerrainSingleMaterial : GfxTerrain->hTerrainMaterial));

    // Set sorting variables
    m_bTranslucent = false;
    m_iLayer = cMaterial.GetPhase(ePhase).GetLayer();
    m_iEffectLayer = 1;
    //m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cMaterial.GetPhase(ePhase).GetVertexShader());
    //m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cMaterial.GetPhase(ePhase).GetPixelShader());
    m_uiVertexBuffer = 0;
    m_uiIndexBuffer = 0;
    m_fDepth = 0.0f;
    m_bRefractive = cMaterial.GetPhase(ePhase).GetRefractive();
}


/*====================
  CTerrainRenderer::RenderChunk
  ====================*/
void    CTerrainRenderer::RenderChunk(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
    PROFILE("CTerrainRenderer::RenderChunk");

    STerrainChunk &cChunk(GfxTerrain->chunks[iChunkY][iChunkX]);

    //
    // Set per chunk shader vars
    //

    D3DXMatrixTranslation
    (
        &g_mWorld,
        iChunkX * GfxTerrain->iChunkSize * GfxTerrain->pWorld->GetScale(), 
        iChunkY * GfxTerrain->iChunkSize * GfxTerrain->pWorld->GetScale(),
        0.0f
    );

#if 0
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
#endif

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(g_mWorld);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glEnableClientState(GL_COLOR_ARRAY);

    CMaterial &cMaterial(GfxUtils->GetMaterial(GfxTerrain->hTerrainSingleMaterial));
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, cChunk.uiVB);

    glColorPointer(4, GL_UNSIGNED_BYTE, 16, BUFFER_OFFSET(4));

    if (ePhase == PHASE_SHADOW || ePhase == PHASE_DEPTH)
    {
        GfxMaterials->SelectMaterial(cMaterial, ePhase, g_pCam->GetTime(), false);
        GfxMaterials->BindAttributes(GfxTerrain->mapAttributes, 16);

        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, cChunk.uiIB);

        glDrawElements(GL_TRIANGLES, cChunk.uiNumFaces * 3, GL_UNSIGNED_SHORT, nullptr);

        SceneStats.RecordBatch(cChunk.uiNumFaces + 2, cChunk.uiNumFaces, ePhase, SSBATCH_TERRAIN);
    }
    else
    {
        if (vid_terrainSinglePass)
        {
            bool bRXGB(vid_shaderRXGBNormalmap);
            GfxTerrain->uiTerrainAlphaMap = cChunk.uiAlphaMap == 0 ? GfxTextures->GetWhiteTexture() : cChunk.uiAlphaMap;

            GfxMaterials->SelectMaterial(cMaterial, ePhase, g_pCam->GetTime(), false);
            GfxMaterials->BindAttributes(GfxTerrain->mapAttributes, 16);

            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, cChunk.uiIB);

            if (vid_terrainLayer1 && vid_terrainLayer2)
            {
                for (int n = 0; n < cChunk.iNumArrays; ++n)
                {
                    if (bRXGB)
                    {
                        GfxMaterials->UpdateShaderTexture(_T("diffuse0"), cChunk.pArrays[n]->ahDiffuse[0], SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap0"), cChunk.pArrays[n]->ahNormalmap[0], SAM_REPEAT, 0);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap0"), cChunk.pArrays[n]->ahNormalmap[0], SAM_REPEAT, 1);
                        GfxMaterials->UpdateShaderTexture(_T("diffuse1"), cChunk.pArrays[n]->ahDiffuse[1], SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap1"), cChunk.pArrays[n]->ahNormalmap[1], SAM_REPEAT, 0);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap1"), cChunk.pArrays[n]->ahNormalmap[1], SAM_REPEAT, 1);
                    }
                    else
                    {
                        GfxMaterials->UpdateShaderTexture(_T("diffuse0"), cChunk.pArrays[n]->ahDiffuse[0], SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap0"), cChunk.pArrays[n]->ahNormalmap[0], SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("diffuse1"), cChunk.pArrays[n]->ahDiffuse[1], SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap1"), cChunk.pArrays[n]->ahNormalmap[1], SAM_REPEAT);
                    }
                    
                    glDrawElements(GL_TRIANGLES, cChunk.pArrays[n]->uiNumElems, GL_UNSIGNED_SHORT, BUFFER_OFFSET(cChunk.pArrays[n]->uiStartIndex * sizeof(GLushort)));

                    SceneStats.RecordBatch(cChunk.pArrays[n]->uiNumFaces * 2, cChunk.pArrays[n]->uiNumFaces, ePhase, SSBATCH_TERRAIN);
                }
            }
            else
            {
                for (int n = 0; n < cChunk.iNumArrays; ++n)
                {
                    if (vid_terrainLayer1)
                    {
                        GfxMaterials->UpdateShaderTexture(_T("diffuse0"), cChunk.pArrays[n]->ahDiffuse[0], SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap0"), cChunk.pArrays[n]->ahNormalmap[0], SAM_REPEAT);
                    }
                    else
                    {
                        GfxMaterials->UpdateShaderTexture(_T("diffuse0"), GfxTerrain->hTerrainCheckerDiffuse, SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap0"), GfxTerrain->hTerrainCheckerNormalmap, SAM_REPEAT);
                    }

                    if (vid_terrainLayer2)
                    {
                        GfxMaterials->UpdateShaderTexture(_T("diffuse1"), cChunk.pArrays[n]->ahDiffuse[1], SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap1"), cChunk.pArrays[n]->ahNormalmap[1], SAM_REPEAT);
                    }
                    else
                    {
                        GfxMaterials->UpdateShaderTexture(_T("diffuse1"), g_ResourceManager.GetInvisTexture(), SAM_REPEAT);
                        GfxMaterials->UpdateShaderTexture(_T("normalmap1"), GfxTerrain->hTerrainCheckerNormalmap, SAM_REPEAT);
                    }

                    glDrawElements(GL_TRIANGLES, cChunk.pArrays[n]->uiNumElems, GL_UNSIGNED_SHORT, BUFFER_OFFSET(cChunk.pArrays[n]->uiStartIndex * sizeof(GLushort)));

                    SceneStats.RecordBatch(cChunk.pArrays[n]->uiNumFaces * 2, cChunk.pArrays[n]->uiNumFaces, ePhase, SSBATCH_TERRAIN);
                }
            }
        }

        if (vid_terrainShowNormals)
            RenderChunkNormals(ePhase, iChunkX, iChunkY);
    }

    GfxMaterials->UnbindAttributes();

    glPopMatrix();

    glDisableClientState(GL_COLOR_ARRAY);
}


/*====================
  CTerrainRenderer::RenderChunkNormals
  ====================*/
void    CTerrainRenderer::RenderChunkNormals(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
    PROFILE("CTerrainRenderer::RenderChunkNormals");
}


/*====================
  CTerrainRenderer::RenderChunkCliffs
  ====================*/
void    CTerrainRenderer::RenderChunkCliffs(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
    PROFILE("CTerrainRenderer::RenderChunkCliffs");

    STerrainChunk &cChunk(GfxTerrain->chunks[iChunkY][iChunkX]);

    if (cChunk.iNumCliffFaces == 0)
        return;

    //
    // Set per chunk shader vars
    //

    g_mWorld = g_mIdentity;

#if 0
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
#endif

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(g_mWorld);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glClientActiveTextureARB(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, cChunk.uiVBCliff);
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, cChunk.uiIBCliff);

    int iStride(GfxTerrain->GetCliffStride());
    int v_offset(0);
    int n_offset(v_offset + sizeof(CVec3f));
    int t_offset(n_offset + sizeof(CVec3f));
    //int tan_offset(t_offset + sizeof(CVec2f));

    glVertexPointer(3, GL_FLOAT, iStride, BUFFER_OFFSET(v_offset));
    glNormalPointer(GL_FLOAT, iStride, BUFFER_OFFSET(n_offset));

    glClientActiveTextureARB(GL_TEXTURE0);
    glTexCoordPointer(2, GL_FLOAT, iStride, BUFFER_OFFSET(t_offset));

    if (ePhase == PHASE_SHADOW || ePhase == PHASE_DEPTH)
    {
        CMaterial &cMaterial(GfxUtils->GetMaterial(cChunk.pCliffArrays[0]->hMaterial)); // Cliffs share shadow phase

        cMaterial.GetPhase(ePhase).SetSrcBlend(BLEND_ONE);
        cMaterial.GetPhase(ePhase).SetDstBlend(BLEND_ZERO);
        cMaterial.GetPhase(ePhase).SetTranslucent(false);
        cMaterial.GetPhase(ePhase).SetDepthWrite(true);

        GfxMaterials->SelectMaterial(cMaterial, ePhase, g_pCam->GetTime(), false);
        GfxMaterials->BindAttributes(GfxTerrain->GetCliffAttributes(), iStride);

        glDrawElements(GL_TRIANGLES, cChunk.iNumCliffFaces * 3, GL_UNSIGNED_SHORT, nullptr);

        SceneStats.RecordBatch(cChunk.iNumCliffVerts, cChunk.iNumCliffFaces, ePhase, SSBATCH_STATICMESH);
    }
    else
    {
        for (int i(0); i < cChunk.iNumCliffArrays; ++i)
        {
            CMaterial &cMaterial(GfxUtils->GetMaterial(cChunk.pCliffArrays[i]->hMaterial));

            g_ResourceManager.UpdateReference(GfxTerrain->hTerrainDiffuseReference, cChunk.pCliffArrays[i]->hDiffuse);
            g_ResourceManager.UpdateReference(GfxTerrain->hTerrainNormalmapReference, cChunk.pCliffArrays[i]->hNormalmap);

            GfxMaterials->SelectMaterial(cMaterial, ePhase, g_pCam->GetTime(), false);
            GfxMaterials->BindAttributes(GfxTerrain->GetCliffAttributes(), iStride);

            glDrawElements(GL_TRIANGLES, cChunk.pCliffArrays[i]->uiNumFaces * 3, GL_UNSIGNED_SHORT, BUFFER_OFFSET(cChunk.pCliffArrays[i]->uiStartIndex * sizeof(GLushort)));

            SceneStats.RecordBatch(cChunk.pCliffArrays[i]->uiNumVerts, cChunk.pCliffArrays[i]->uiNumFaces, ePhase, SSBATCH_STATICMESH);
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glClientActiveTextureARB(GL_TEXTURE0);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    GfxMaterials->UnbindAttributes();

    glPopMatrix();
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

    for (int iChunkY(0); iChunkY != GfxTerrain->iNumChunksY; ++iChunkY)
    {
        for (int iChunkX(0); iChunkX != GfxTerrain->iNumChunksX; ++iChunkX)
        {
            STerrainChunk *chunk = &GfxTerrain->chunks[iChunkY][iChunkX];

            if ((ePhase == PHASE_SHADOW && chunk->bVisibleShadow) || (ePhase != PHASE_SHADOW && chunk->bVisible) || vid_terrainNoCull)
            {
                RenderChunk(ePhase, iChunkX, iChunkY);
            }
        }
    }

    if (vid_terrainCliffs && !(g_pCam->GetFlags() & CAM_NO_CLIFFS))
    {
        m_bFirstChunk = true;

        for (int iChunkY(0); iChunkY != GfxTerrain->iNumChunksY; ++iChunkY)
        {
            for (int iChunkX(0); iChunkX != GfxTerrain->iNumChunksX; ++iChunkX)
            {
                STerrainChunk *chunk = &GfxTerrain->chunks[iChunkY][iChunkX];

                if ((ePhase == PHASE_SHADOW && chunk->bVisibleShadow) || (ePhase != PHASE_SHADOW && chunk->bVisible) || vid_terrainNoCull)
                {
                    RenderChunkCliffs(ePhase, iChunkX, iChunkY);
                }
            }
        }
    }
}
