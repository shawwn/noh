// (C)2008 S2 Games
// c_treefrondrenderer.cpp
//
// SpeedTree frond renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_treefrondrenderer.h"

#include "c_gfxutils.h"
#include "c_gfxmaterials.h"
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
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
//=============================================================================

CPool<CTreeFrondRenderer> CTreeFrondRenderer::s_Pool(1, uint(-1));

/*====================
  CTreeFrondRenderer::operator new
  ====================*/
void*   CTreeFrondRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CTreeFrondRenderer::CTreeFrondRenderer
  ====================*/
CTreeFrondRenderer::CTreeFrondRenderer(const CSceneEntity &cEntity, const CTreeModelDef *pTreeDef,
        const D3DXMATRIXA16 &mWorldViewProj,
        const D3DXMATRIXA16 &mWorld,
        const D3DXMATRIXA16 &mWorldRotate) :
m_cEntity(cEntity),
m_pTreeDef(pTreeDef)
{
    m_mWorldViewProj = mWorldViewProj;
    m_mWorld = mWorld;
    m_mWorldRotate = mWorldRotate;

    m_LOD = m_pTreeDef->GetDiscreetFrondLOD();
}


/*====================
  CTreeFrondRenderer::~CTreeFrondRenderer
  ====================*/
CTreeFrondRenderer::~CTreeFrondRenderer()
{
}


CVAR_BOOL(vid_skipTreeFrondRenderer, false);

/*====================
  CTreeFrondRenderer::Setup
  ====================*/
void    CTreeFrondRenderer::Setup(EMaterialPhase ePhase)
{
    if (vid_skipTreeFrondRenderer)
        return;

    PROFILE("CTreeFrondRenderer::Setup");

    if (m_LOD.m_iLOD == -1)
        return;

    if (!m_pTreeDef->HasFrondLOD(m_LOD.m_iLOD))
    {
        //Console.Err << _T("CTreeFrondRenderer::Setup() - Invalid LOD: ") << m_LOD.m_iLOD << newl;
        return;
    }

    if (!m_pTreeDef->HasFrondGeometry())
        return;

    CMaterial &material(GfxUtils->GetMaterial(m_pTreeDef->GetFrondMaterial()));

    if (!material.HasPhase(ePhase))
        return; // Leave if we don't have this phase

    m_pCurrentEntity = &m_cEntity;
    m_pCam = g_pCam;

    m_bLighting = gfx_lighting;
    m_vAmbient = SceneManager.GetEntityAmbientColor();
    m_vSunColor = SceneManager.GetEntitySunColor();
    m_bObjectColor = false;

    m_iNumActiveBones = 0;

    // Pick the four best point lights to light this model
    m_iNumActivePointLights = 0;
    
    if (ePhase == PHASE_COLOR)
    {
        CBBoxf  bbBoundsWorld(m_pTreeDef->GetBounds());
        bbBoundsWorld.Transform(m_cEntity.GetPosition(), m_cEntity.axis, m_cEntity.scale);

        SceneLightList &LightList(SceneManager.GetLightList());
        for (SceneLightList::iterator itLight(LightList.begin()); itLight != LightList.end() && m_iNumActivePointLights != g_iMaxDynamicLights; ++itLight)
        {
            SSceneLightEntry &cEntry(**itLight);
            const CSceneLight &scLight(cEntry.cLight);

            if (cEntry.bCull)
                continue;

            if (I_SphereBoundsIntersect(CSphere(scLight.GetPosition(), scLight.GetFalloffEnd()), bbBoundsWorld))
            {
                m_vPointLightPosition[m_iNumActivePointLights] = scLight.GetPosition();
                m_vPointLightColor[m_iNumActivePointLights] = scLight.GetColor();
                m_fPointLightFalloffStart[m_iNumActivePointLights] = scLight.GetFalloffStart();
                m_fPointLightFalloffEnd[m_iNumActivePointLights] = scLight.GetFalloffEnd();
                ++m_iNumActivePointLights;
            }
        }
    }

    m_bRender = true;

    g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
    g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
    g_ShaderRegistry.SetLighting(m_bLighting);
    g_ShaderRegistry.SetShadows(m_bShadows);
    g_ShaderRegistry.SetFogofWar(g_bFogofWar);
    g_ShaderRegistry.SetFog(m_bFog);

    const STreeGeometryBuffers &fronds(m_pTreeDef->GetFrondGeometry(m_LOD.m_iLOD));

    // Set sorting variables
    m_bTranslucent = material.GetPhase(ePhase).GetTranslucent();
    m_iLayer = material.GetPhase(ePhase).GetLayer();
    m_iEffectLayer = 0;
    //m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(material.GetPhase(ePhase).GetVertexShader());
    //m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(material.GetPhase(ePhase).GetPixelShader());
    m_uiVertexBuffer = fronds.m_VBuffer;
    m_uiIndexBuffer = 0;
    m_fDepth = 0.0f;
    m_bRefractive = material.GetPhase(ePhase).GetRefractive();
}


/*====================
  CTreeFrondRenderer::Render
  ====================*/
void    CTreeFrondRenderer::Render(EMaterialPhase ePhase)
{
    if (!m_bRender)
        return;

    const STreeGeometryBuffers &fronds(m_pTreeDef->GetFrondGeometry(m_LOD.m_iLOD));
    GfxMaterials->SelectMaterial(GfxUtils->GetMaterial(m_pTreeDef->GetFrondMaterial()), ePhase, 0.0f, 0);

    SetShaderVars();

    if (m_cEntity.flags & SCENEENT_SOLID_COLOR)
        glColor4f(m_cEntity.color[0], m_cEntity.color[1], m_cEntity.color[2], m_cEntity.color[3]);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Set initial render states
    glAlphaFunc(GL_GREATER, m_LOD.m_dwAlphaTest / 255.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(m_mWorld);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, fronds.m_VBuffer);
    glNormalPointer(GL_FLOAT, sizeof(SFrondVert), BUFFER_OFFSET(12));
    glTexCoordPointer(2, GL_FLOAT, sizeof(SFrondVert), BUFFER_OFFSET(24));
    glVertexPointer(3, GL_FLOAT, sizeof(SFrondVert), BUFFER_OFFSET(0));

    GfxMaterials->BindAttributes(CTreeModelDef::s_mapFrondAttributes, sizeof(SFrondVert));

    for (size_t z(0); z < fronds.m_vIBuffers.size(); ++z)
    {
        glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, fronds.m_vIBuffers[z]);
        glDrawElements(GL_TRIANGLE_STRIP, fronds.m_viNumIndices[z] - 2, GL_UNSIGNED_SHORT, nullptr);
    }

    glPopMatrix();

    if (m_cEntity.flags & SCENEENT_SOLID_COLOR)
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    GfxMaterials->UnbindAttributes();
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
