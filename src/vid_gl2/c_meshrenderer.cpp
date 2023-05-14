// (C)2008 S2 Games
// c_meshrenderer.cpp
//
// A single item for the Render List
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_meshrenderer.h"

#include "c_gfxutils.h"
#include "c_gfxmaterials.h"
#include "c_gfxshaders.h"
#include "c_gfxterrain.h"
#include "c_shaderregistry.h"
#include "../public/blendedlink_t.h"
#include "c_bonelist.h"
#include "c_renderlist.h"
#include "c_scenebuffer.h"

#include "../k2/c_skeleton.h"
#include "../k2/c_mesh.h"
#include "../k2/c_k2model.h"
#include "../k2/c_material.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_scenestats.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL   (vid_drawMeshes,                true);
CVAR_BOOLF  (vid_meshGPUDeform,             true,   CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_meshAlwaysStatic,          false,  CONEL_DEV);
CVAR_BOOLF  (vid_meshSkipCPUDeform,         false,  CONEL_DEV);
CVAR_BOOLF  (vid_meshForceNonBlendedDeform, false,  CVAR_SAVECONFIG);
CVAR_BOOLF  (vid_drawMeshWireframe,         false,  CONEL_DEV);
CVAR_BOOLF  (vid_drawMeshPoints,            false,  CONEL_DEV);
CVAR_BOOLF  (vid_drawMeshBounds,            false,  CONEL_DEV);
CVAR_BOOLF  (vid_drawMeshNames,             false,  CONEL_DEV);
CVAR_BOOLF  (vid_drawBones,                 false,  CONEL_DEV);
CVAR_BOOLF  (vid_drawBoneNames,             false,  CONEL_DEV);
CVAR_BOOLF  (vid_drawMeshNormals,           false,  CONEL_DEV);
//=============================================================================

CPool<CMeshRenderer> CMeshRenderer::s_Pool(1, uint(-1));

/*====================
  CMeshRenderer::operator new
  ====================*/
void*   CMeshRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
    return s_Pool.Allocate();
}


/*====================
  CMeshRenderer::CMeshRenderer
  ====================*/
CMeshRenderer::CMeshRenderer(ResHandle hMaterial, const CSceneEntity &cEntity, CMesh *pMesh, const D3DXMATRIXA16 &mWorldEntity, const D3DXMATRIXA16 &mWorldEntityRotation) :
m_hMaterial(hMaterial),
m_cEntity(cEntity),
m_pMesh(pMesh),
m_pMapping(nullptr),
m_iCurrentSkelbone(-1),
m_mWorldEntity(mWorldEntity),
m_mWorldEntityRotation(mWorldEntityRotation)
{
}


/*====================
  CMeshRenderer::~CMeshRenderer
  ====================*/
CMeshRenderer::~CMeshRenderer()
{
}


/*====================
  GL_MeshShouldDeform

  is the mesh going to dynamically deform?
  ====================*/
bool    GL_MeshShouldDeform(const CSceneEntity &cEntity, CMesh *mesh)
{
    if (cEntity.skeleton && cEntity.skeleton->IsValid() && mesh->bonelink == -1)
        return true;

    return false;
}


/*====================
  CMeshRenderer::SetupObjectMatrix
  ====================*/
bool    CMeshRenderer::SetupObjectMatrix()
{
    m_iCurrentSkelbone = -1;

    if (m_cEntity.skeleton && m_cEntity.skeleton->IsValid() && m_pMesh->bonelink != -1 && m_pMapping[m_pMesh->bonelink] != -1 && !vid_meshAlwaysStatic)
    {
        //
        // Entire mesh linked to one bone
        //

        m_iCurrentSkelbone = m_pMapping[m_pMesh->bonelink];

        assert(m_iCurrentSkelbone < int(m_cEntity.skeleton->GetNumBones()));

        // Mesh is invisible on this frame
        if (m_iCurrentSkelbone != -1 && m_cEntity.skeleton->GetBoneState(m_iCurrentSkelbone)->visibility == 0)
            return false;

        matrix43_t *tm = &m_cEntity.skeleton->GetBoneState(m_iCurrentSkelbone)->tm_world;

        D3DXMATRIXA16 mBone;
        GfxUtils->TransformToMatrix(tm, &mBone);

        m_mWorld = mBone * m_mWorldEntity;

        // This assumes no scaling in bone transformations
        m_mWorldRotate = m_mWorld;

        // Remove translation from m_mWorldRotate
        m_mWorldRotate[3] = 0.0f;
        m_mWorldRotate[7] = 0.0f;
        m_mWorldRotate[11] = 0.0f;
        m_mWorldRotate[12] = 0.0f;
        m_mWorldRotate[13] = 0.0f;
        m_mWorldRotate[14] = 0.0f;
        m_mWorldRotate[15] = 1.0f;

        m_iNumActiveBones = 0;
    }
    else
    {
        //
        // Not liked to one bone
        //

        m_mWorld = m_mWorldEntity;
        m_mWorldRotate = m_mWorldEntityRotation;

        if (vid_meshGPUDeform && (m_pMesh->flags & MESH_GPU_DEFORM) && GL_MeshShouldDeform(m_cEntity, m_pMesh) && !vid_meshAlwaysStatic)
        {
            m_iNumActiveBones = min(GfxModels->BoneRemap[m_pMesh->sbuffer].GetNumBones(), MAX_GPU_BONES);

            for (int i = 0; i < GfxModels->BoneRemap[m_pMesh->sbuffer].GetNumBones() && i < m_iNumActiveBones; ++i)
            {
                const matrix43_t &tm_world = m_cEntity.skeleton->GetBoneState(m_pMapping[GfxModels->BoneRemap[m_pMesh->sbuffer].GetBoneIndex(i)])->tm_world;

                int iData;

                iData = i * 3 + 0;
                m_vBoneData[iData].x = tm_world.axis[0][0];
                m_vBoneData[iData].y = tm_world.axis[1][0];
                m_vBoneData[iData].z = tm_world.axis[2][0];
                m_vBoneData[iData].w = tm_world.pos[0];

                iData = i * 3 + 1;
                m_vBoneData[iData].x = tm_world.axis[0][1];
                m_vBoneData[iData].y = tm_world.axis[1][1];
                m_vBoneData[iData].z = tm_world.axis[2][1];
                m_vBoneData[iData].w = tm_world.pos[1];

                iData = i * 3 + 2;
                m_vBoneData[iData].x = tm_world.axis[0][2];
                m_vBoneData[iData].y = tm_world.axis[1][2];
                m_vBoneData[iData].z = tm_world.axis[2][2];
                m_vBoneData[iData].w = tm_world.pos[2];
            }
        }
        else
            m_iNumActiveBones = 0;
    }

    m_mWorldViewProj = m_mWorld * g_mViewProj;
    m_bObjectColor = false;
    return true;
}


CVAR_BOOL(vid_skipMeshRenderer, false);

/*====================
  CMeshRenderer::Setup
  ====================*/
void    CMeshRenderer::Setup(EMaterialPhase ePhase)
{
    if (vid_skipMeshRenderer)
        return;

    PROFILE("CMeshRenderer::Setup");

    m_pMaterial = g_ResourceManager.GetMaterial(m_hMaterial);

    if (!m_pMaterial)
        return;

    m_pCam = g_pCam;
    m_pCurrentEntity = &m_cEntity;

    if ((gfx_points || vid_drawMeshPoints || m_cEntity.flags & SCENEENT_POINTS ||
        gfx_wireframe || vid_drawMeshWireframe || m_cEntity.flags & SCENEENT_WIREFRAME) && ePhase == PHASE_DEPTH)
        return;

    if (m_cEntity.custom_mapping)
        m_pMapping = m_cEntity.custom_mapping;
    else
        m_pMapping = m_pMesh->GetModel()->GetBoneMapping();

    if (!SetupObjectMatrix())
        return;

#if 0
    if (ePhase == PHASE_COLOR && (vid_drawMeshBounds || m_cEntity.flags & SCENEENT_MESH_BOUNDS))
        GL_AddBox(CBBoxf(CVec3_cast(m_pMesh->bmin), CVec3_cast(m_pMesh->bmax)), CVec4f(0.0f, 1.0f, 0.0f, 1.0f), m_mWorld);
#endif

    if (ePhase == PHASE_COLOR && g_SceneBuffer.GetActive() && m_pMaterial->HasPhase(PHASE_REFRACT))
        ePhase = PHASE_REFRACT;

    if (m_cEntity.flags & SCENEENT_SOLID_COLOR && m_cEntity.color[A] <= 0.0f)
        return;

    if (m_cEntity.flags & SCENEENT_SOLID_COLOR && m_cEntity.color[A] < 1.0f && m_pMaterial->HasPhase(PHASE_FADE))
    {
        if (ePhase == PHASE_COLOR)
            ePhase = PHASE_FADE;
        else if ((ePhase == PHASE_DEPTH || ePhase == PHASE_SHADOW) && m_cEntity.color[A] <= 0.0f)
        {
            m_bDepthFirst = false;
            return;
        }
    }

    if (!m_pMaterial->HasPhase(ePhase))
        return; // Leave if we don't have this phase

    m_pMaterial->SetPass(0);

    CMaterialPhase &cPhase(m_pMaterial->GetPhase(ePhase));

    // Pick the best point lights to light this model
    m_iNumActivePointLights = 0;

    if (ePhase == PHASE_SHADOW || ePhase == PHASE_DEPTH)
    {
        m_bLighting = true;
        m_vAmbient = CVec3f(1.0f, 1.0f, 1.0f);
        m_vSunColor = CVec3f(0.0f, 0.0f, 0.0f);
    }
    else if (!(m_cEntity.flags & SCENEENT_NO_LIGHTING) && (ePhase == PHASE_COLOR || ePhase == PHASE_FADE || ePhase == PHASE_REFRACT))
    {
        m_bLighting = gfx_lighting && cPhase.GetLighting();
        m_bShadows = g_bCamShadows && cPhase.GetShadows();
        m_bFog = g_bCamFog && cPhase.GetFog();

        if (m_pMaterial->GetLightingScheme() == LIGHTING_TERRAIN)
        {
            m_vAmbient = SceneManager.GetTerrainAmbientColor();
            m_vSunColor = SceneManager.GetTerrainSunColor();
        }
        else
        {
            m_vAmbient = SceneManager.GetEntityAmbientColor();
            m_vSunColor = SceneManager.GetEntitySunColor();
        }

        if (m_cEntity.flags & SCENEENT_USE_BOUNDS)
        {
            SceneLightList &LightList(SceneManager.GetLightList());
            SceneLightList::iterator itEnd(LightList.end());
            for (SceneLightList::iterator it(LightList.begin()); it != itEnd && m_iNumActivePointLights != g_iMaxDynamicLights; ++it)
            {
                SSceneLightEntry &cEntry(**it);
                const CSceneLight &scLight(cEntry.cLight);

                if (cEntry.bCull)
                    continue;

                if (I_SphereBoundsIntersect(CSphere(scLight.GetPosition(), scLight.GetFalloffEnd()), m_cEntity.bounds))
                {
                    m_vPointLightPosition[m_iNumActivePointLights] = scLight.GetPosition();
                    m_vPointLightColor[m_iNumActivePointLights] = scLight.GetColor();
                    m_fPointLightFalloffStart[m_iNumActivePointLights] = scLight.GetFalloffStart();
                    m_fPointLightFalloffEnd[m_iNumActivePointLights] = scLight.GetFalloffEnd();
                    ++m_iNumActivePointLights;
                }
            }
        }
        else
        {
            CBBoxf  bbBoundsWorld(m_pMesh->bmin, m_pMesh->bmax);
            bbBoundsWorld.Transform(m_cEntity.GetPosition(), m_cEntity.axis, m_cEntity.scale);

            SceneLightList &LightList(SceneManager.GetLightList());
            SceneLightList::iterator itEnd(LightList.end());
            for (SceneLightList::iterator it(LightList.begin()); it != itEnd && m_iNumActivePointLights != g_iMaxDynamicLights; ++it)
            {
                SSceneLightEntry &cEntry(**it);
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
    }
    else
    {
        m_bLighting = false;
        m_vAmbient = CVec3f(1.0f, 1.0f, 1.0f);
        m_vSunColor = CVec3f(0.0f, 0.0f, 0.0f);
    }

    m_bTexkill = cPhase.GetAlphaTest() && vid_shaderTexkill && (ePhase == PHASE_COLOR || !vid_shaderTexkillColorOnly);
    m_bRender = true;

    g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
    g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
    g_ShaderRegistry.SetLighting(m_bLighting);
    g_ShaderRegistry.SetShadows(m_bShadows);
    g_ShaderRegistry.SetFog(m_bFog);
    g_ShaderRegistry.SetFogofWar(g_bFogofWar);
    g_ShaderRegistry.SetTexkill(m_bTexkill);

    int iVertexShader(g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader()));
    int iPixelShader(g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader()));

    // Set sorting variables
    m_bTranslucent = cPhase.GetTranslucent();
    m_iLayer = cPhase.GetLayer();
    m_iShaderProgramInstance = g_ShaderRegistry.GetShaderProgramInstance(iVertexShader, iPixelShader);
    m_iEffectLayer = m_cEntity.effectlayer;
    m_uiVertexBuffer = m_pMesh->sbuffer;
    m_fDepth = m_cEntity.effectdepth != 0.0f ? m_cEntity.effectdepth : DotProduct(g_pCam->GetViewAxis(FORWARD), m_cEntity.GetPosition());
    m_bRefractive = m_pMaterial->GetPhase(ePhase).GetRefractive();
}


/*====================
  CMeshRenderer::SetRenderStates
  ====================*/
bool    CMeshRenderer::SetRenderStates(EMaterialPhase ePhase)
{
    PROFILE("CMeshRenderer::SetRenderStates");

    const CMaterial &material(*m_pMaterial);

    if (m_cEntity.flags & SCENEENT_TERRAIN_TEXTURES && (ePhase == PHASE_COLOR || ePhase == PHASE_FADE || ePhase == PHASE_REFRACT) && GfxTerrain->pWorld != nullptr)
    {
        float fWorldScale(GfxTerrain->pWorld->GetScale());

        int iTileX(INT_FLOOR(m_cEntity.GetPosition().x / fWorldScale));
        int iTileY(INT_FLOOR(m_cEntity.GetPosition().y / fWorldScale));

        g_ResourceManager.UpdateReference(GfxTerrain->hTerrainDiffuseReference, GfxTerrain->pWorld->GetTileDiffuseTexture(iTileX, iTileY, 0));
        g_ResourceManager.UpdateReference(GfxTerrain->hTerrainNormalmapReference, GfxTerrain->pWorld->GetTileNormalmapTexture(iTileX, iTileY, 0));

        GfxMaterials->SelectMaterial(material, ePhase, m_pCam->GetTime(), /*gfx_depthFirst && m_bDepthFirst*/false);
    }
    else
    {
        const SMaterialState &cMaterialState(GfxMaterials->GetMaterialState());

        if (cMaterialState.pMaterial == m_pMaterial &&
            cMaterialState.ePhase == ePhase &&
            cMaterialState.iPass == m_pMaterial->GetPass() &&
            g_iCurrentShaderProgram == m_iShaderProgramInstance)
        {
            GfxMaterials->UpdateShaderParams(material, m_pCam->GetTime());
        }
        else
        {
            GfxMaterials->SelectMaterial(material, ePhase, m_pCam->GetTime(), /*gfx_depthFirst && m_bDepthFirst*/false);
        }
    }

    // Handle special object rendering flags
    if (m_cEntity.flags & SCENEENT_ALWAYS_BLEND)
        glEnable(GL_BLEND);

    if (m_cEntity.flags & SCENEENT_NO_ZWRITE)
        glDepthMask(GL_FALSE);

    if (m_cEntity.flags & SCENEENT_NO_ZTEST)
        glDisable(GL_DEPTH_TEST);

    glAlphaFunc(GL_GREATER, vid_alphaTestRef / 255.0f);

    return true;
}


/*====================
  CMeshRenderer::DrawStaticMesh
  ====================*/
bool    CMeshRenderer::DrawStaticMesh()
{
    PROFILE("CMeshRenderer::DrawStaticMesh");

    if (!vid_drawMeshes)
        return true;

    glEnableClientState(GL_VERTEX_ARRAY);
    if (m_pMesh->renderflags & MESH_NORMALS)
        glEnableClientState(GL_NORMAL_ARRAY);
    if (m_pMesh->num_color_channels > 0)
        glEnableClientState(GL_COLOR_ARRAY);
    for (int i(0); i < m_pMesh->num_uv_channels; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    if (m_cEntity.flags & SCENEENT_SOLID_COLOR)
        glColor4f(m_cEntity.color[0], m_cEntity.color[1], m_cEntity.color[2], m_cEntity.color[3]);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(m_mWorld);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, GfxModels->VBMeshes[m_pMesh->sbuffer]);

    int iStride(m_pMesh->vertexStride);
    int v_offset(0);
    int n_offset(v_offset + sizeof(CVec3f));
    int c1_offset(n_offset + ((m_pMesh->renderflags & MESH_NORMALS) ? sizeof(CVec3f) : 0));
    int c2_offset(c1_offset + ((m_pMesh->num_color_channels > 0) ? sizeof(dword) : 0));
    int t_offset(c2_offset + ((m_pMesh->num_color_channels > 1) ? sizeof(dword) : 0));

    glVertexPointer(3, GL_FLOAT, iStride, BUFFER_OFFSET(v_offset));
    if (m_pMesh->renderflags & MESH_NORMALS)
        glNormalPointer(GL_FLOAT, iStride, BUFFER_OFFSET(n_offset));
    if (m_pMesh->num_color_channels > 0)
        glColorPointer(4, GL_UNSIGNED_BYTE, iStride, BUFFER_OFFSET(c1_offset));
    for (int i(0); i < m_pMesh->num_uv_channels; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glTexCoordPointer(2, GL_FLOAT, iStride, BUFFER_OFFSET(t_offset + i * sizeof(CVec2f)));
    }

    // Bind custom attributes
    GfxMaterials->BindAttributes(m_pMesh->mapAttributes, iStride);
    
    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, GfxModels->IBMeshes[m_pMesh->ibuffer]);
    glDrawElements(GL_TRIANGLES, m_pMesh->numFaces * 3, GL_UNSIGNED_SHORT, nullptr);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    if (m_pMesh->renderflags & MESH_NORMALS)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (m_pMesh->num_color_channels > 0)
        glDisableClientState(GL_COLOR_ARRAY);
    for (int i(0); i < m_pMesh->num_uv_channels; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
        
    if (m_cEntity.flags & SCENEENT_SOLID_COLOR)
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    GfxMaterials->UnbindAttributes();

    glClientActiveTextureARB(GL_TEXTURE0);

    return true;
}


/*====================
  CMeshRenderer::DrawSkinnedMesh
  ====================*/
bool    CMeshRenderer::DrawSkinnedMesh()
{
    PROFILE("CMeshRenderer::DrawSkinnedMesh");
    
    if (!vid_drawMeshes)
        return true;

    if (!m_cEntity.skeleton)
    {
        Console.Warn << _T("CMeshRenderer::DrawSkinnedMesh() - m_pMesh ") << m_pMesh->GetName() << _T(" in model ")
                    << m_pMesh->GetModel()->GetName() << _T(" has no skeleton") << newl;
        return false;
    }

    if (m_pMesh->mode != MESH_SKINNED_BLENDED && m_pMesh->mode != MESH_SKINNED_NONBLENDED)
    {
        Console.Warn << _T("CMeshRenderer::DrawSkinnedMesh() - m_pMesh ") << m_pMesh->GetName() << _T(" in model ") << m_pMesh->GetModel()->GetName()
                    << _T(" has an invalid mode (mode==") << m_pMesh->mode << _T(")") << newl;
        return false;
    }

    if (m_pMesh->dbuffer == -1)
    {
        Console.Warn << _T("CMeshRenderer::DrawSkinnedMesh() - m_pMesh ") << m_pMesh->GetName() << _T(" in model ") << m_pMesh->GetModel()->GetName()
                    << _T(" has no dynamic buffer") << newl;
        return false;
    }

    int iVertexStride(12);

    if (m_pMesh->normals)
        iVertexStride += 12;

    int iNumTangents(0);

    for (int j(0); j < MAX_UV_CHANNELS; ++j)
    {
        if (m_pMesh->tangents[j])
            iNumTangents = j + 1;
        else
            break;
    }

    iNumTangents = MIN(iNumTangents, 1);

    iVertexStride += iNumTangents * sizeof(CVec3f);

    int v_offset = 0;
    int n_offset = v_offset + sizeof(CVec3f);
    int tan_offset = n_offset + (m_pMesh->normals ? sizeof(CVec3f) : 0);
    
    if (GfxModels->DBMeshes[m_pMesh->dbuffer] == 0)
        glGenBuffersARB(1, &GfxModels->DBMeshes[m_pMesh->dbuffer]);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, GfxModels->DBMeshes[m_pMesh->dbuffer]);
    glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_pMesh->num_verts * iVertexStride, nullptr, GL_STREAM_DRAW_ARB);
    byte* pVertices = (byte*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY);
    if (vid_meshSkipCPUDeform)
    {
        for (int v = 0; v < m_pMesh->num_verts; ++v)
        {
            {
                vec3_t  *p_v = (vec3_t *)(&pVertices[v * iVertexStride + v_offset]);
                M_CopyVec3(m_pMesh->verts[v], *p_v);
            }

            if (m_pMesh->normals)
            {
                vec3_t  *p_n = (vec3_t *)(&pVertices[v * iVertexStride + n_offset]);
                M_CopyVec3(m_pMesh->normals[v], *p_n);
            }

            for (int m = 0; m < iNumTangents; ++m)
            {
                vec3_t  *p_t = (vec3_t *)(&pVertices[v * iVertexStride + tan_offset + (m * sizeof(vec3_t))]);

                if (m_pMesh->tangents[m] != nullptr)
                    M_CopyVec3(m_pMesh->tangents[m][v], *p_t);
                else
                    M_SetVec3(*p_t, 0.0f, 0.0f, 0.0f);
            }
        }
    }
    else if (m_pMesh->mode == MESH_SKINNED_BLENDED && !vid_meshForceNonBlendedDeform)
    {
        // Blended deformation
        for (int v = 0; v < m_pMesh->num_verts; ++v)
        {
            int index;
            matrix43_t tm_blended;

            MemManager.Set(&tm_blended, 0, sizeof(tm_blended));

            SBlendedLink *link = &m_pMesh->blendedLinks[v];

            for (int w = 0; w < link->num_weights; ++w)
            {
                index = link->indexes[w];

                if (m_pMapping[index] == -1)
                {
                    M_Identity(&tm_blended);
                    break;
                }

                matrix43_t *tm_world = &m_cEntity.skeleton->GetBoneState(m_pMapping[index])->tm_world;

                M_BlendMatrix(&tm_blended, tm_world, link->weights[w], &tm_blended);
            }

            {
                vec3_t  *p_v = (vec3_t *)(&pVertices[v * iVertexStride + v_offset]);
                M_TransformPoint(m_pMesh->verts[v], tm_blended.pos, (const vec3_t *)tm_blended.axis, *p_v);
            }

            if (m_pMesh->normals)
            {
                vec3_t  *p_n = (vec3_t *)(&pVertices[v * iVertexStride + n_offset]);
                M_RotatePoint(m_pMesh->normals[v], (const vec3_t *)tm_blended.axis, *p_n);
            }

            for (int m = 0; m < iNumTangents; ++m)
            {
                vec3_t  *p_t = (vec3_t *)(&pVertices[v * iVertexStride + tan_offset + (m * sizeof(vec3_t))]);

                if (m_pMesh->tangents[m] != nullptr)
                    M_RotatePoint(m_pMesh->tangents[m][v], (const vec3_t *)tm_blended.axis, *p_t);
                else
                    M_SetVec3(*p_t, 0.0f, 0.0f, 0.0f);
            }
        }
    }
    else
    {
        // Non - blended deformation
        for (int v = 0; v < m_pMesh->num_verts; ++v)
        {
            int boneidx;
            matrix43_t *tm_world;

            boneidx = m_pMapping[m_pMesh->singleLinks[v]];
            if (boneidx == -1)
            {
                {
                    vec3_t  *p_v = (vec3_t *)(&pVertices[v * iVertexStride + v_offset]);
                    M_CopyVec3(m_pMesh->verts[v], *p_v);
                }

                if (m_pMesh->normals)
                {
                    vec3_t  *p_n = (vec3_t *)(&pVertices[v * iVertexStride + n_offset]);
                    M_CopyVec3(m_pMesh->normals[v], *p_n);
                }

                for (int m = 0; m < iNumTangents; ++m)
                {
                    vec3_t  *p_t = (vec3_t *)(&pVertices[v * iVertexStride + tan_offset + (m * sizeof(vec3_t))]);

                    if (m_pMesh->tangents[m] != nullptr)
                        M_CopyVec3(m_pMesh->tangents[m][v], *p_t);
                    else
                        M_SetVec3(*p_t, 0.0f, 0.0f, 0.0f);
                }
                continue;
            }

            tm_world = &m_cEntity.skeleton->GetBoneState(boneidx)->tm_world;

            {
                vec3_t  *p_v = (vec3_t *)(&pVertices[v * iVertexStride + v_offset]);
                M_TransformPoint(m_pMesh->verts[v], tm_world->pos, (const vec3_t *)tm_world->axis, *p_v);
            }

            if (m_pMesh->normals)
            {
                vec3_t  *p_n = (vec3_t *)(&pVertices[v * iVertexStride + n_offset]);
                M_RotatePoint(m_pMesh->normals[v], (const vec3_t *)tm_world->axis, *p_n);
            }

            for (int m = 0; m < iNumTangents; ++m)
            {
                vec3_t  *p_t = (vec3_t *)(&pVertices[v * iVertexStride + tan_offset + (m * sizeof(vec3_t))]);

                if (m_pMesh->tangents[m] != nullptr)
                    M_RotatePoint(m_pMesh->tangents[m][v], (const vec3_t *)tm_world->axis, *p_t);
                else
                    M_SetVec3(*p_t, 0.0f, 0.0f, 0.0f);
            }
        }
    }

    glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

    glEnableClientState(GL_VERTEX_ARRAY);
    if (m_pMesh->renderflags & MESH_NORMALS)
        glEnableClientState(GL_NORMAL_ARRAY);
    if (m_pMesh->num_color_channels > 0)
        glEnableClientState(GL_COLOR_ARRAY);
    for (int i(0); i < m_pMesh->num_uv_channels; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    if (m_cEntity.flags & SCENEENT_SOLID_COLOR)
        glColor4f(m_cEntity.color[0], m_cEntity.color[1], m_cEntity.color[2], m_cEntity.color[3]);
    else
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMultMatrixf(m_mWorld);

    glBindBufferARB(GL_ARRAY_BUFFER_ARB, GfxModels->VBMeshes[m_pMesh->sbuffer]);
    
    int iStride(m_pMesh->vertexStride);
    v_offset = 0;
    n_offset = v_offset + sizeof(CVec3f);
    int c1_offset(n_offset + ((m_pMesh->renderflags & MESH_NORMALS) ? sizeof(CVec3f) : 0));
    int c2_offset(c1_offset + ((m_pMesh->num_color_channels > 0) ? sizeof(dword) : 0));
    int t_offset(c2_offset + ((m_pMesh->num_color_channels > 1) ? sizeof(dword) : 0));
    
    if (m_pMesh->num_color_channels > 0)
        glColorPointer(4, GL_UNSIGNED_BYTE, iStride, BUFFER_OFFSET(c1_offset));
    for (int i(0); i < m_pMesh->num_uv_channels; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glTexCoordPointer(2, GL_FLOAT, iStride, BUFFER_OFFSET(t_offset + i * sizeof(CVec2f)));
    }
    
    glBindBufferARB(GL_ARRAY_BUFFER_ARB, GfxModels->DBMeshes[m_pMesh->dbuffer]);
    glVertexPointer(3, GL_FLOAT, iVertexStride, BUFFER_OFFSET(0));
    if (m_pMesh->renderflags & MESH_NORMALS)
        glNormalPointer(GL_FLOAT, iVertexStride, BUFFER_OFFSET(12));
    if (iNumTangents > 0)
    {
        SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

        for (uint ui(0); ui < cShaderProgram.vAttributes.size(); ++ui)
        {
            SShaderAttribute &cAttribute(cShaderProgram.vAttributes[ui]);

            if (cAttribute.sName.compare(_T("a_vTangent")) == 0)
            {
                glEnableVertexAttribArrayARB(cAttribute.iLocation);
                glVertexAttribPointerARB(cAttribute.iLocation, 3, GL_FLOAT, false, iVertexStride, BUFFER_OFFSET(tan_offset));
                break;
            }
        }
    }

    glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, GfxModels->IBMeshes[m_pMesh->ibuffer]);
    glDrawElements(GL_TRIANGLES, m_pMesh->numFaces * 3, GL_UNSIGNED_SHORT, nullptr);

    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    if (m_pMesh->renderflags & MESH_NORMALS)
        glDisableClientState(GL_NORMAL_ARRAY);
    if (m_pMesh->num_color_channels > 0)
        glDisableClientState(GL_COLOR_ARRAY);
    for (int i(0); i < m_pMesh->num_uv_channels; ++i)
    {
        glClientActiveTextureARB(GL_TEXTURE0 + i);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
        
    if (m_cEntity.flags & SCENEENT_SOLID_COLOR)
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    if (iNumTangents > 0)
    {
        SShaderProgramSlot &cShaderProgram(g_aShaderProgramSlots[g_iCurrentShaderProgram]);

        for (uint ui(0); ui < cShaderProgram.vAttributes.size(); ++ui)
        {
            SShaderAttribute &cAttribute(cShaderProgram.vAttributes[ui]);

            if (cAttribute.sName.compare(_T("a_vTangent")) == 0)
            {
                glDisableVertexAttribArrayARB(cAttribute.iLocation);
                break;
            }
        }
    }

    glClientActiveTextureARB(GL_TEXTURE0);

    return true;
}


/*====================
  CMeshRenderer::DrawMesh

  the general mesh drawing function, which selects the appropriate function above
  ====================*/
void    CMeshRenderer::DrawMesh(EMaterialPhase ePhase)
{
    PROFILE("CMeshRenderer::DrawMesh");

    if (GL_MeshShouldDeform(m_cEntity, m_pMesh) && !vid_meshAlwaysStatic)
    {
        if (vid_meshGPUDeform && m_pMesh->flags & MESH_GPU_DEFORM)
            DrawStaticMesh();
        else
            DrawSkinnedMesh();
    }
    else
        DrawStaticMesh();
}


/*====================
  CMeshRenderer::Render
  ====================*/
void    CMeshRenderer::Render(EMaterialPhase ePhase)
{
    PROFILE("CMeshRenderer::Render");

    SetShaderVars();
    
    // Draw mesh names
    if (ePhase == PHASE_COLOR && vid_drawMeshNames)
    {
        CVec3f  v3Pos(GfxUtils->TransformPoint((CVec3_cast(m_pMesh->bmin) + CVec3_cast(m_pMesh->bmax)) * 0.5f, m_mWorld));

        Gfx3D->AddPoint(v3Pos, CVec4f(0.0f, 0.0f, 1.0f, 1.0f));

        CVec2f  v2ScreenPos;
        if (m_pCam->WorldToScreen(v3Pos, v2ScreenPos))
        {
            Draw2D.SetColor(CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
            Draw2D.String(floor(v2ScreenPos.x), floor(v2ScreenPos.y), m_pMesh->GetName(), g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
        }
    }

    if (ePhase == PHASE_COLOR && g_SceneBuffer.GetActive() && m_pMaterial->HasPhase(PHASE_REFRACT))
        ePhase = PHASE_REFRACT;
    else if (ePhase == PHASE_COLOR && m_cEntity.flags & SCENEENT_SOLID_COLOR && m_cEntity.color[A] < 1.0f && m_pMaterial->HasPhase(PHASE_FADE))
        ePhase = PHASE_FADE;

    m_pMaterial->SetPass(0);

    SetRenderStates(ePhase);
    DrawMesh(ePhase);

    if (m_pMaterial->GetPhase(ePhase).GetNumMultiPass() > 0)
    {
        int iNumPasses(m_pMaterial->GetPhase(ePhase).GetNumMultiPass());
        for (int i(0); i < iNumPasses; ++i)
        {
            m_pMaterial->SetPass(i + 1);

            SetRenderStates(ePhase);
            DrawMesh(ePhase);
        }
    }
}
