// (C)2008 S2 Games
// c_foliagerenderer.cpp
//
// Foliage chunk renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_foliagerenderer.h"

#include "c_gfxutils.h"
#include "c_gfxmaterials.h"
#include "c_gfxshaders.h"
#include "gl2_foliage.h"
#include "c_shaderregistry.h"

#include "../k2/c_world.h"
#include "../k2/c_material.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
#include "../k2/s_foliagetile.h"
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOLF  (vid_foliageWireframe,          false,                      CONEL_DEV);
CVAR_INT    (vid_foliageAlphaTestRef,       90);
CVAR_BOOLF  (vid_foliageLayer1,             true,                       CONEL_DEV);
CVAR_BOOLF  (vid_foliageLayer2,             true,                       CONEL_DEV);
//=============================================================================

CPool<CFoliageRenderer> CFoliageRenderer::s_Pool(1, uint(-1));

/*====================
  CFoliageRenderer::operator new
  ====================*/
void*   CFoliageRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CFoliageRenderer::CFoliageRenderer
  ====================*/
CFoliageRenderer::CFoliageRenderer(int iChunkX, int iChunkY, EFoliageRenderOrder eOrder, int iFlags) :
m_iChunkX(iChunkX),
m_iChunkY(iChunkY),
m_eOrder(eOrder),
m_iFlags(iFlags)
{
}


/*====================
  CFoliageRenderer::~CFoliageRenderer
  ====================*/
CFoliageRenderer::~CFoliageRenderer()
{
}


CVAR_BOOL(vid_skipFoliageRenderer, false);

/*====================
  CFoliageRenderer::Setup
  ====================*/
void    CFoliageRenderer::Setup(EMaterialPhase ePhase)
{
    if (vid_skipFoliageRenderer)
        return;

    PROFILE("CFoliageRenderer::Setup");

    SFoliageChunk &oChunk = g_Foliage.ppChunks[m_iChunkY][m_iChunkX];

    m_pCam = g_pCam;
    m_bLighting = gfx_lighting;
    m_vAmbient = SceneManager.GetTerrainAmbientColor();
    m_vSunColor = SceneManager.GetTerrainSunColor();

    m_iNumActiveBones = 0;

    // Pick the four best point lights to light this chunk
    m_iNumActivePointLights = 0;

    if (ePhase == PHASE_COLOR)
    {
        SceneLightList &LightList(SceneManager.GetLightList());
        for (SceneLightList::iterator it(LightList.begin()); it != LightList.end() && m_iNumActivePointLights != g_iMaxDynamicLights; ++it)
        {
            SSceneLightEntry &cEntry(**it);
            const CSceneLight &scLight(cEntry.cLight);

            if (cEntry.bCull)
                continue;

            if (I_SphereBoundsIntersect(CSphere(scLight.GetPosition(), scLight.GetFalloffEnd()), oChunk.bbBounds))
            {
                m_vPointLightPosition[m_iNumActivePointLights] = scLight.GetPosition();
                m_vPointLightColor[m_iNumActivePointLights] = scLight.GetColor();
                m_fPointLightFalloffStart[m_iNumActivePointLights] = scLight.GetFalloffStart();
                m_fPointLightFalloffEnd[m_iNumActivePointLights] = scLight.GetFalloffEnd();
                ++m_iNumActivePointLights;
            }
        }
    }

    m_bObjectColor = false;
    m_pCurrentEntity = nullptr;

    m_mWorld = g_mIdentity;
    m_mWorldRotate = g_mIdentity;
    m_mWorldViewProj = g_mViewProj;

    m_bTexkill = (m_iFlags & FOLIAGE_ALPHATEST) && vid_shaderTexkill;

    m_bRender = true;

    CMaterial &cMaterial(GfxUtils->GetMaterial(g_hFoliageMaterial));

    // Set sorting variables
    m_bTranslucent = (m_iFlags & FOLIAGE_ALPHABLEND) != 0;
    m_iLayer = cMaterial.GetPhase(ePhase).GetLayer();
    m_iEffectLayer = 0;
    m_uiVertexBuffer = 0;
    m_uiIndexBuffer = 0;
    m_fDepth = 0.0f;
}


/*====================
  CFoliageRenderer::RenderChunk
  ====================*/
void    CFoliageRenderer::RenderChunk(EMaterialPhase ePhase)
{
    PROFILE("CFoliageRenderer::RenderChunk");

    SFoliageChunk &oChunk = g_Foliage.ppChunks[m_iChunkY][m_iChunkX];

    CMaterial &cMaterial(GfxUtils->GetMaterial(g_hFoliageMaterial));

    CVec2f vDir(m_pCam->GetViewAxis(FORWARD).xy());

    float fAngle(acos(Normalize(vDir).x));

    if (vDir.y < 0.0f)
        fAngle = 2.0f * M_PI - fAngle;

    // Flip angle to get front->back rendering
    if (m_eOrder == FOLIAGE_FRONTBACK)
        fAngle += M_PI;

    float fDir = fAngle / (2.0f * M_PI) * NUM_SORT_DIRECTIONS;

    int iDir = INT_ROUND(fDir) % NUM_SORT_DIRECTIONS;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(m_mWorld);

    glEnableClientState(GL_VERTEX_ARRAY);

    cMaterial.GetPhase(PHASE_COLOR).SetVertexShader(g_hFoliageVertexShaderNormal);

    GfxMaterials->SelectMaterial(cMaterial, ePhase, g_pCam->GetTime(), false);

    if (m_iFlags & FOLIAGE_ALPHATEST)
    {
        glEnable(GL_ALPHA_TEST);
        glAlphaFunc(GL_GREATER, vid_foliageAlphaTestRef / 255.0f);
    }
    else
    {
        glDisable(GL_ALPHA_TEST);
    }

    if (m_iFlags & FOLIAGE_ALPHABLEND)
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);

    if (m_iFlags & FOLIAGE_DEPTHWRITE)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

    for (int iLayer = 0; iLayer < NUM_FOLIAGE_LAYERS; ++iLayer)
    {
        if (iLayer == 0 && !vid_foliageLayer1)
            continue;
        else if (iLayer == 1 && !vid_foliageLayer2)
            continue;

        for (int n = 0; n < oChunk.iNumArrays[iLayer]; ++n)
        {
            if (!oChunk.pArrays[iLayer][n]->iNumFoliageQuads)
                continue;

            GfxMaterials->UpdateShaderTexture(_T("diffuse"), oChunk.pArrays[iLayer][n]->hTexture, 0);

            glBindBufferARB(GL_ARRAY_BUFFER_ARB, oChunk.pArrays[iLayer][n]->uiVB);

            glVertexPointer(3, GL_FLOAT, sizeof(SFoliageVertex), BUFFER_OFFSET(0));
            GfxMaterials->BindAttributes(g_mapFoliageAttributes, sizeof(SFoliageVertex));

            glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, oChunk.pArrays[iLayer][n]->uiIB[iDir]);
            glDrawElements(GL_TRIANGLES, oChunk.pArrays[iLayer][n]->iNumFoliageQuads * 2 * 3, GL_UNSIGNED_SHORT, nullptr);

            GfxMaterials->UnbindAttributes();

            SceneStats.RecordBatch(oChunk.pArrays[iLayer][n]->iNumFoliageQuads * 4, oChunk.pArrays[iLayer][n]->iNumFoliageQuads * 2, ePhase, SSBATCH_FOLIAGE);
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}


/*====================
  CFoliageRenderer::Render
  ====================*/
void    CFoliageRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CFoliageRenderer::Render");

    if (!m_bRender)
        return;

    SetShaderVars();

    RenderChunk(ePhase);
}
