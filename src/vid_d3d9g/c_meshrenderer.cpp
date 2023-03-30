// (C)2005 S2 Games
// c_meshrenderer.cpp
//
// A single item for the Render List
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "../public/blendedlink_t.h"

#include "d3d9g_main.h"
#include "d3d9g_util.h"
#include "d3d9g_material.h"
#include "d3d9g_model.h"
#include "d3d9g_scene.h"
#include "d3d9g_state.h"
#include "d3d9g_shader.h"
#include "d3d9g_texture.h"
#include "d3d9g_terrain.h"
#include "c_meshrenderer.h"
#include "c_renderlist.h"
#include "c_bonelist.h"
#include "c_shaderregistry.h"
#include "c_scenebuffer.h"
#include "c_postbuffer.h"

#include "../k2/c_skeleton.h"
#include "../k2/c_mesh.h"
#include "../k2/c_k2model.h"
#include "../k2/c_resourcemanager.h"
#include "../k2/c_material.h"
#include "../k2/intersection.h"
#include "../k2/c_sphere.h"
#include "../k2/c_camera.h"
#include "../k2/c_sceneentity.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_scenelight.h"
#include "../k2/c_scenestats.h"
#include "../k2/c_world.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOL	(vid_drawMeshes,				true);
CVAR_BOOLF	(vid_meshGPUDeform,				true, CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_meshAlwaysStatic,			false, CONEL_DEV);
CVAR_BOOLF	(vid_meshSkipCPUDeform,			false, CONEL_DEV);
CVAR_BOOLF	(vid_meshForceNonBlendedDeform,	false, CVAR_SAVECONFIG);
CVAR_BOOLF	(vid_drawMeshWireframe,			false, CONEL_DEV);
CVAR_BOOLF	(vid_drawMeshPoints,			false, CONEL_DEV);
CVAR_BOOLF	(vid_drawMeshBounds,			false, CONEL_DEV);
CVAR_BOOLF	(vid_drawMeshNames,				false, CONEL_DEV);
CVAR_BOOLF	(vid_drawBones,					false, CONEL_DEV);
CVAR_BOOLF	(vid_drawBoneNames,				false, CONEL_DEV);
CVAR_BOOLF	(vid_drawMeshNormals,			false, CONEL_DEV);
CVAR_BOOLF	(vid_meshShareMaterial,			true, CONEL_DEV);
CVAR_FLOAT	(vid_meshNormalLength,			2.0f);

CVAR_INT	(vid_meshFraction,				0);
//=============================================================================

CPool<CMeshRenderer> CMeshRenderer::s_Pool(1, -1);

/*====================
  CMeshRenderer::operator new
  ====================*/
void*	CMeshRenderer::operator new(size_t z)
{
	return s_Pool.Allocate();
}


/*====================
  CMeshRenderer::CMeshRenderer
  ====================*/
CMeshRenderer::CMeshRenderer(ResHandle hMaterial, const CSceneEntity &cEntity, CMesh *pMesh, const D3DXMATRIXA16 &mWorldEntity, const D3DXMATRIXA16 &mWorldEntityRotation, const SSceneEntityEntry &cEntry) :
IRenderer(RT_MESH),
m_hMaterial(hMaterial),
m_cEntity(cEntity),
m_pMesh(pMesh),
m_pMapping(NULL),
m_iCurrentSkelbone(-1),
m_mWorldEntity(mWorldEntity),
m_mWorldEntityRotation(mWorldEntityRotation),
m_cEntry(cEntry),
m_bInvisible(false)
{
	m_pMaterial = g_ResourceManager.GetMaterial(m_hMaterial);

	if (m_cEntity.custom_mapping)
		m_pMapping = m_cEntity.custom_mapping;
	else
		m_pMapping = m_pMesh->GetModel()->GetBoneMapping();

	//
	// Setup object matrices
	//

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
		{
			m_bInvisible = true;
			return;
		}

		matrix43_t *tm = &m_cEntity.skeleton->GetBoneState(m_iCurrentSkelbone)->tm_world;

		D3DXMATRIXA16 mBone;
		D3D_TransformToMatrix(tm, &mBone);

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

		if (MeshShouldDeform(m_cEntity, m_pMesh) && (m_pMesh->flags & MESH_GPU_DEFORM) && vid_meshGPUDeform && !vid_meshAlwaysStatic)
		{
			m_iNumActiveBones = min(g_BoneRemap[m_pMesh->dbuffer].GetNumBones(), MAX_GPU_BONES);

			for (int i = 0; i < g_BoneRemap[m_pMesh->dbuffer].GetNumBones() && i < m_iNumActiveBones; ++i)
			{
				const matrix43_t &tm_world = m_cEntity.skeleton->GetBoneState(m_pMapping[g_BoneRemap[m_pMesh->dbuffer].GetBoneIndex(i)])->tm_world;

				D3D_TransformToMatrix(&tm_world, &m_vBoneData[i]);
			}
		}
		else
			m_iNumActiveBones = 0;
	}

	m_iTexcoords = m_pMesh->num_uv_channels;
	m_bObjectColor = false;
	m_pCurrentEntity = &m_cEntity;

	// Set phase independant sorting variables
	m_iEffectLayer = m_cEntity.effectlayer;
	m_iVertexType = m_pMesh->vertexDecl;
	m_uiVertexBuffer = reinterpret_cast<size_t>(g_pVBStaticMeshes[m_pMesh->sbuffer]);
	m_uiIndexBuffer = reinterpret_cast<size_t>(g_pIBMeshes[m_pMesh->ibuffer]);
}


/*====================
  CMeshRenderer::~CMeshRenderer
  ====================*/
CMeshRenderer::~CMeshRenderer()
{
}


/*====================
  CMeshRenderer::MeshShouldDeform

  is the mesh going to dynamically deform?
  ====================*/
bool	CMeshRenderer::MeshShouldDeform(const CSceneEntity &cEntity, CMesh *pMesh)
{
	if (cEntity.skeleton && cEntity.skeleton->IsValid() && pMesh->bonelink == -1)
		return true;

	return false;
}


/*====================
  CMeshRenderer::Setup
  ====================*/
void	CMeshRenderer::Setup(EMaterialPhase ePhase)
{
	//PROFILE("CMeshRenderer::Setup");

	m_bRender = false; // Set to true if we make it to the end of the function

	if (((ePhase == PHASE_COLOR || ePhase == PHASE_DEPTH) && m_cEntry.bCull) ||
		(ePhase == PHASE_SHADOW && m_cEntry.bCullShadow) ||
		m_bInvisible)
		return;

	if (!m_pMaterial)
		return;

	m_pCam = g_pCam;

	if ((gfx_points || vid_drawMeshPoints || m_cEntity.flags & SCENEENT_POINTS ||
		gfx_wireframe || vid_drawMeshWireframe || m_cEntity.flags & SCENEENT_WIREFRAME) && ePhase == PHASE_DEPTH)
		return;

	m_mWorldViewProj = m_mWorld * g_mViewProj;

	if (ePhase == PHASE_COLOR && (vid_drawMeshBounds || m_cEntity.flags & SCENEENT_MESH_BOUNDS))
		D3D_AddBox(CBBoxf(CVec3_cast(m_pMesh->bmin), CVec3_cast(m_pMesh->bmax)), CVec4f(0.0f, 1.0f, 0.0f, 1.0f), m_mWorld);

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

	if (g_bReflectPass && cPhase.GetVampire())
		return;

	// Pick the best point lights to light this model
	m_iNumActivePointLights = 0;

	if (ePhase == PHASE_SHADOW || ePhase == PHASE_DEPTH)
	{
		m_bLighting = true;
		m_bShadows = true;
		m_bFog = true;
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

			if (!vid_shaderGroundAmbient)
				m_vAmbient *= 0.75f;
			if (!vid_postEffects)
			{
				m_vAmbient *= 1.3f;
				m_vSunColor *= 0.9f;
			}
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
			CBBoxf	bbBoundsWorld(m_pMesh->bmin, m_pMesh->bmax);
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
		m_bShadows = false;
		m_bFog = g_bFog && cPhase.GetFog();
		m_vAmbient = CVec3f(1.0f, 1.0f, 1.0f);
		m_vSunColor = CVec3f(0.0f, 0.0f, 0.0f);
	}

	m_bDepthFirst = gfx_depthFirst && m_pMaterial->HasPhase(PHASE_DEPTH) && ePhase != PHASE_FADE;
	m_bTexkill = cPhase.GetAlphaTest() && vid_shaderTexkill && (ePhase == PHASE_COLOR || !vid_shaderTexkillColorOnly);
	m_bRender = true;
	
	g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
	g_ShaderRegistry.SetLighting(m_bLighting);
	g_ShaderRegistry.SetShadows(m_bShadows);
	g_ShaderRegistry.SetFog(m_bFog);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetTexcoords(m_iTexcoords);
	g_ShaderRegistry.SetTexkill(m_bTexkill);

	// Set sorting variables
	m_bTranslucent = cPhase.GetTranslucent();
	m_iLayer = cPhase.GetLayer();
	m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cPhase.GetVertexShader());
	m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cPhase.GetPixelShader());
	m_fDepth = m_cEntity.effectdepth != 0.0f ? m_cEntity.effectdepth : DotProduct(g_pCam->GetViewAxis(FORWARD), m_cEntity.GetPosition()) + cPhase.GetDepthSortBias();
	m_bRefractive = cPhase.GetRefractive();
}


/*====================
  CMeshRenderer::SetRenderStates
  ====================*/
bool	CMeshRenderer::SetRenderStates(EMaterialPhase ePhase)
{
	//PROFILE("CMeshRenderer::SetRenderStates");

	const CMaterial &material(*m_pMaterial);

	if ((gfx_points || vid_drawMeshPoints || m_cEntity.flags & SCENEENT_POINTS) && (ePhase == PHASE_COLOR || ePhase == PHASE_FADE || ePhase == PHASE_REFRACT))
	{
		//
		// Point rendering
		//
	
		D3D_SetRenderState(D3DRS_POINTSIZE, D3D_DWORD(1.0f));

		switch (gfx_points)
		{
		default:
		case 1: // Unlit
			D3D_SelectMaterial(g_SimpleMaterial3D, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), false);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			break;
		case 2: // Lit
			D3D_SelectMaterial(g_SimpleMaterial3DLit, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), false);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			break;
		case 3: // Unlit + Textures
			m_bLighting = false;
			D3D_SelectMaterial(material, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), gfx_depthFirst && m_bDepthFirst);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			break;
		case 4: // Lit + Textures
			D3D_SelectMaterial(material, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), gfx_depthFirst && m_bDepthFirst);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_POINT);
			break;
		}
	}
	else if ((gfx_wireframe || vid_drawMeshWireframe || m_cEntity.flags & SCENEENT_WIREFRAME) && (ePhase == PHASE_COLOR || ePhase == PHASE_FADE || ePhase == PHASE_REFRACT))
	{
		//
		// Wireframe rendering
		//

		switch (gfx_wireframe)
		{
		default:
		case 1: // Unlit
			D3D_SelectMaterial(g_SimpleMaterial3D, PHASE_COLOR, m_pMesh->vertexDecl, m_pCam->GetTime(), false);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;
		case 2: // Lit
			D3D_SelectMaterial(g_SimpleMaterial3DLit, PHASE_COLOR, m_pMesh->vertexDecl, m_pCam->GetTime(), false);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;
		case 3: // Unlit + Textures
			m_bLighting = false;
			D3D_SelectMaterial(material, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), gfx_depthFirst && m_bDepthFirst);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;
		case 4: // Lit + Textures
			D3D_SelectMaterial(material, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), gfx_depthFirst && m_bDepthFirst);
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			D3D_SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
			D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
			break;
		}
	}
	else
	{
		//
		// Normal rendering
		//

		D3D_SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

		if (m_cEntity.flags & SCENEENT_TERRAIN_TEXTURES && (ePhase == PHASE_COLOR || ePhase == PHASE_FADE || ePhase == PHASE_REFRACT) && terrain.pWorld != NULL)
		{
			float fWorldScale(terrain.pWorld->GetScale());

			int iTileX(INT_FLOOR(m_cEntity.GetPosition().x / fWorldScale));
			int iTileY(INT_FLOOR(m_cEntity.GetPosition().y / fWorldScale));

			g_ResourceManager.UpdateReference(g_hTerrainDiffuseReference, terrain.pWorld->GetTileDiffuseTexture(iTileX, iTileY, 0));
			g_ResourceManager.UpdateReference(g_hTerrainNormalmapReference, terrain.pWorld->GetTileNormalmapTexture(iTileX, iTileY, 0));

			const SMaterialState &cMaterialState(D3D_GetMaterialState());

			if (cMaterialState.pMaterial == m_pMaterial &&
				cMaterialState.ePhase == ePhase &&
				cMaterialState.iVertexType == m_pMesh->vertexDecl &&
				cMaterialState.iPass == m_pMaterial->GetPass() &&
				g_iCurrentVertexShader == m_iVertexShaderInstance &&
				g_iCurrentPixelShader == m_iPixelShaderInstance)
			{
				D3D_SelectPixelShader(material, ePhase, m_pCam->GetTime());
				D3D_UpdateVertexShaderParams(material, m_pCam->GetTime());
			}
			else
			{
				D3D_SelectMaterial(material, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), gfx_depthFirst && m_bDepthFirst);
			}
		}
		else
		{
			const SMaterialState &cMaterialState(D3D_GetMaterialState());

			if (cMaterialState.pMaterial == m_pMaterial &&
				cMaterialState.ePhase == ePhase &&
				cMaterialState.iVertexType == m_pMesh->vertexDecl &&
				cMaterialState.iPass == m_pMaterial->GetPass() &&
				g_iCurrentVertexShader == m_iVertexShaderInstance &&
				g_iCurrentPixelShader == m_iPixelShaderInstance)
			{
				D3D_UpdatePixelShaderParams(material, m_pCam->GetTime());
				D3D_UpdateVertexShaderParams(material, m_pCam->GetTime());
			}
			else
			{
				D3D_SelectMaterial(material, ePhase, m_pMesh->vertexDecl, m_pCam->GetTime(), gfx_depthFirst && m_bDepthFirst);
			}
		}

		// Handle special object rendering flags
		if (m_cEntity.flags & SCENEENT_ALWAYS_BLEND)
			D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

		if (m_cEntity.flags & SCENEENT_NO_ZWRITE)
			D3D_SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		D3D_SetRenderState(D3DRS_ALPHAREF, vid_alphaTestRef);
	}

	if (m_cEntity.flags & SCENEENT_NO_ZTEST)
		D3D_SetRenderState(D3DRS_ZENABLE, FALSE);

	return true;
}


/*====================
  CMeshRenderer::DrawStaticMesh
  ====================*/
bool	CMeshRenderer::DrawStaticMesh()
{
	PROFILE("D3D_DrawStaticMesh");

	if (vid_drawMeshes)
	{
		D3D_SetStreamSource(0, g_pVBStaticMeshes[m_pMesh->sbuffer], 0, m_pMesh->vertexStride);
		D3D_SetIndices(g_pIBMeshes[m_pMesh->ibuffer]);
		D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_pMesh->num_verts, 0, m_pMesh->numFaces);
	}

	return true;
}


/*====================
  CMeshRenderer::DrawSkinnedMesh
  ====================*/
bool	CMeshRenderer::DrawSkinnedMesh()
{
	PROFILE("D3D_DrawSkinnedMesh");

	if (!m_cEntity.skeleton)
	{
		Console.Warn << _T("CMeshRenderer::DrawSkinnedMesh: m_pMesh ") << m_pMesh->GetName() << _T(" in model ")
					<< m_pMesh->GetModel()->GetName() << _T(" has no skeleton") << newl;
		return false;
	}

	if (m_pMesh->mode != MESH_SKINNED_BLENDED && m_pMesh->mode != MESH_SKINNED_NONBLENDED)
	{
		Console.Warn << _T("CMeshRenderer::DrawSkinnedMesh: m_pMesh ") << m_pMesh->GetName() << _T(" in model ") << m_pMesh->GetModel()->GetName()
					<< _T(" has an invalid mode (mode==") << m_pMesh->mode << _T(")") << newl;
		return false;
	}

	if (m_pMesh->dbuffer == -1)
	{
		Console.Warn << _T("CMeshRenderer::DrawSkinnedMesh: m_pMesh ") << m_pMesh->GetName() << _T(" in model ") << m_pMesh->GetModel()->GetName()
					<< _T(" has no dynamic buffer") << newl;
		return false;
	}

	if (!g_pVBDynamicMeshes[m_pMesh->dbuffer]) // Recreate dynamic buffer if it's been lost do to a reset
	{
		if (FAILED(g_pd3dDevice->CreateVertexBuffer(m_pMesh->num_verts * m_pMesh->vertexStride,
					D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBDynamicMeshes[m_pMesh->dbuffer], NULL)))
			K2System.Error(_T("CMeshRenderer::DrawSkinnedMesh(): CreateVertexBuffer failed"));
	}

	int iNumTexcoords = 0;
	for (int j = 0; j < MAX_UV_CHANNELS; ++j)
	{
		if (m_pMesh->tverts[j])
			iNumTexcoords = j + 1;
	}

	int iNumTangents = 0;
	for (int j = 0; j < MAX_UV_CHANNELS; ++j)
	{
		if (m_pMesh->tangents[j])
			iNumTangents = j + 1;
	}

	int v_offset = 0;
	int n_offset = v_offset + sizeof(vec3_t);

	int c1_offset = n_offset + ((m_pMesh->vertexFVF & D3DFVF_NORMAL) ? sizeof(vec3_t) : 0);
	int c2_offset = c1_offset + ((m_pMesh->vertexFVF & D3DFVF_DIFFUSE) ? sizeof(DWORD) : 0);

	int t_offset = c2_offset + ((m_pMesh->vertexFVF & D3DFVF_SPECULAR) ? sizeof(DWORD) : 0);

	int tan_offset = t_offset + (iNumTexcoords * sizeof(vec2_t));

	byte *pVertices;

	if (FAILED(g_pVBDynamicMeshes[m_pMesh->dbuffer]->Lock(0, m_pMesh->num_verts * m_pMesh->vertexStride, (void**)&pVertices, D3DLOCK_DISCARD)))
		return false;

	if (vid_meshSkipCPUDeform)
	{
		for (int v = 0; v < m_pMesh->num_verts; ++v)
		{
			if (m_pMesh->vertexFVF & D3DFVF_XYZ)
			{
				vec3_t	*p_v = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + v_offset]);
				M_CopyVec3(m_pMesh->verts[v], *p_v);
			}

			if (m_pMesh->vertexFVF & D3DFVF_NORMAL)
			{
				vec3_t	*p_n = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + n_offset]);
				M_CopyVec3(m_pMesh->normals[v], *p_n);
			}

			if (m_pMesh->vertexFVF & D3DFVF_DIFFUSE)
			{
				dword	*p_c1 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c1_offset]);
				*p_c1 = D3DCOLOR_ARGB(m_pMesh->colors[0][v][3], m_pMesh->colors[0][v][0], m_pMesh->colors[0][v][1], m_pMesh->colors[0][v][2]);
			}

			if (m_pMesh->vertexFVF & D3DFVF_SPECULAR)
			{
				dword	*p_c2 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c2_offset]);
				*p_c2 = D3DCOLOR_ARGB(m_pMesh->colors[1][v][3], m_pMesh->colors[1][v][0], m_pMesh->colors[1][v][1], m_pMesh->colors[1][v][2]);
			}

			for (int n = 0; n < iNumTexcoords; ++n)
			{
				vec2_t	*p_t = (vec2_t *)(&pVertices[v * m_pMesh->vertexStride + t_offset + (n * sizeof(vec2_t))]);

				if (m_pMesh->tverts[n] != NULL)
					M_CopyVec2(m_pMesh->tverts[n][v], *p_t);
				else
					M_SetVec2(*p_t, 0.0f, 0.0f);
			}

			for (int m = 0; m < iNumTangents; ++m)
			{
				vec3_t	*p_t = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + tan_offset + (m * sizeof(vec3_t))]);

				if (m_pMesh->tangents[m] != NULL)
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
			vec3_t final;
			int index;
			matrix43_t tm_blended;

			MemManager.Set(&tm_blended, 0, sizeof(tm_blended));

			SBlendedLink *link = &m_pMesh->blendedLinks[v];

			M_ClearVec3(final);

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

			if (m_pMesh->vertexFVF & D3DFVF_XYZ)
			{
				vec3_t	*p_v = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + v_offset]);
				M_TransformPoint(m_pMesh->verts[v], tm_blended.pos, (const vec3_t *)tm_blended.axis, *p_v);
			}

			if (m_pMesh->vertexFVF & D3DFVF_NORMAL)
			{
				vec3_t	*p_n = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + n_offset]);
				M_RotatePoint(m_pMesh->normals[v], (const vec3_t *)tm_blended.axis, *p_n);
			}

			if (m_pMesh->vertexFVF & D3DFVF_DIFFUSE)
			{
				dword	*p_c1 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c1_offset]);
				if (m_pMesh->colors[0] != NULL)
					*p_c1 = D3DCOLOR_ARGB(m_pMesh->colors[0][v][3], m_pMesh->colors[0][v][0], m_pMesh->colors[0][v][1], m_pMesh->colors[0][v][2]);
				else
					*p_c1 = D3DCOLOR_ARGB(255, 255, 255, 255);
			}

			if (m_pMesh->vertexFVF & D3DFVF_SPECULAR)
			{
				dword	*p_c2 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c2_offset]);

				if (m_pMesh->colors[1] != NULL)
					*p_c2 = D3DCOLOR_ARGB(m_pMesh->colors[1][v][3], m_pMesh->colors[1][v][0], m_pMesh->colors[1][v][1], m_pMesh->colors[1][v][2]);
				else
					*p_c2 = D3DCOLOR_ARGB(255, 255, 255, 255);
			}

			for (int n = 0; n < iNumTexcoords; ++n)
			{
				vec2_t	*p_t = (vec2_t *)(&pVertices[v * m_pMesh->vertexStride + t_offset + (n * sizeof(vec2_t))]);

				if (m_pMesh->tverts[n] != NULL)
					M_CopyVec2(m_pMesh->tverts[n][v], *p_t);
				else
					M_SetVec2(*p_t, 0.0f, 0.0f);
			}

			for (int m = 0; m < iNumTangents; ++m)
			{
				vec3_t	*p_t = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + tan_offset + (m * sizeof(vec3_t))]);

				if (m_pMesh->tangents[m] != NULL)
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
				if (m_pMesh->vertexFVF & D3DFVF_XYZ)
				{
					vec3_t	*p_v = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + v_offset]);
					M_CopyVec3(m_pMesh->verts[v], *p_v);
				}

				if (m_pMesh->vertexFVF & D3DFVF_NORMAL)
				{
					vec3_t	*p_n = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + n_offset]);
					M_CopyVec3(m_pMesh->normals[v], *p_n);
				}

				if (m_pMesh->vertexFVF & D3DFVF_DIFFUSE)
				{
					dword	*p_c1 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c1_offset]);
					if (m_pMesh->colors[0] != NULL)
						*p_c1 = D3DCOLOR_ARGB(m_pMesh->colors[0][v][3], m_pMesh->colors[0][v][0], m_pMesh->colors[0][v][1], m_pMesh->colors[0][v][2]);
					else
						*p_c1 = D3DCOLOR_ARGB(255, 255, 255, 255);
				}

				if (m_pMesh->vertexFVF & D3DFVF_SPECULAR)
				{
					dword	*p_c2 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c2_offset]);

					if (m_pMesh->colors[1] != NULL)
						*p_c2 = D3DCOLOR_ARGB(m_pMesh->colors[1][v][3], m_pMesh->colors[1][v][0], m_pMesh->colors[1][v][1], m_pMesh->colors[1][v][2]);
					else
						*p_c2 = D3DCOLOR_ARGB(255, 255, 255, 255);
				}

				for (int n = 0; n < iNumTexcoords; ++n)
				{
					vec2_t	*p_t = (vec2_t *)(&pVertices[v * m_pMesh->vertexStride + t_offset + (n * sizeof(vec2_t))]);

					if (m_pMesh->tverts[n] != NULL)
						M_CopyVec2(m_pMesh->tverts[n][v], *p_t);
					else
						M_SetVec2(*p_t, 0.0f, 0.0f);
				}

				for (int m = 0; m < iNumTangents; ++m)
				{
					vec3_t	*p_t = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + tan_offset + (m * sizeof(vec3_t))]);

					if (m_pMesh->tangents[m] != NULL)
						M_CopyVec3(m_pMesh->tangents[m][v], *p_t);
					else
						M_SetVec3(*p_t, 0.0f, 0.0f, 0.0f);
				}
				continue;
			}

			tm_world = &m_cEntity.skeleton->GetBoneState(boneidx)->tm_world;

			if (m_pMesh->vertexFVF & D3DFVF_XYZ)
			{
				vec3_t	*p_v = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + v_offset]);
				M_TransformPoint(m_pMesh->verts[v], tm_world->pos, (const vec3_t *)tm_world->axis, *p_v);
			}

			if (m_pMesh->vertexFVF & D3DFVF_NORMAL)
			{
				vec3_t	*p_n = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + n_offset]);
				M_RotatePoint(m_pMesh->normals[v], (const vec3_t *)tm_world->axis, *p_n);
			}

			if (m_pMesh->vertexFVF & D3DFVF_DIFFUSE)
			{
				dword	*p_c1 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c1_offset]);
				*p_c1 = D3DCOLOR_ARGB(m_pMesh->colors[0][v][3], m_pMesh->colors[0][v][0], m_pMesh->colors[0][v][1], m_pMesh->colors[0][v][2]);
			}

			if (m_pMesh->vertexFVF & D3DFVF_SPECULAR)
			{
				dword	*p_c2 = (dword *)(&pVertices[v * m_pMesh->vertexStride + c2_offset]);
				*p_c2 = D3DCOLOR_ARGB(m_pMesh->colors[1][v][3], m_pMesh->colors[1][v][0], m_pMesh->colors[1][v][1], m_pMesh->colors[1][v][2]);
			}

			for (int n = 0; n < iNumTexcoords; ++n)
			{
				vec2_t	*p_t = (vec2_t *)(&pVertices[v * m_pMesh->vertexStride + t_offset + (n * sizeof(vec2_t))]);

				if (m_pMesh->tverts[n] != NULL)
					M_CopyVec2(m_pMesh->tverts[n][v], *p_t);
				else
					M_SetVec2(*p_t, 0.0f, 0.0f);
			}

			for (int m = 0; m < iNumTangents; ++m)
			{
				vec3_t	*p_t = (vec3_t *)(&pVertices[v * m_pMesh->vertexStride + tan_offset + (m * sizeof(vec3_t))]);

				if (m_pMesh->tangents[m] != NULL)
					M_RotatePoint(m_pMesh->tangents[m][v], (const vec3_t *)tm_world->axis, *p_t);
				else
					M_SetVec3(*p_t, 0.0f, 0.0f, 0.0f);
			}
		}
	}

	g_pVBDynamicMeshes[m_pMesh->dbuffer]->Unlock();

	if (vid_drawMeshes)
	{
		D3D_SetStreamSource(0, g_pVBDynamicMeshes[m_pMesh->dbuffer], 0, m_pMesh->vertexStride);
		D3D_SetIndices(g_pIBMeshes[m_pMesh->ibuffer]);
		D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_pMesh->num_verts, 0, m_pMesh->numFaces);
	}

	return true;
}


/*====================
  CMeshRenderer::DrawSkinnedMeshGPU
  ====================*/
bool	CMeshRenderer::DrawSkinnedMeshGPU()
{
	if (vid_drawMeshes)
	{
		D3D_SetStreamSource(0, g_pVBStaticMeshes[m_pMesh->sbuffer], 0, m_pMesh->vertexStride);
		D3D_SetIndices(g_pIBMeshes[m_pMesh->ibuffer]);
		D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, m_pMesh->num_verts, 0, m_pMesh->numFaces);
	}

	return true;
}


/*====================
  CMeshRenderer::DrawMesh

  the general mesh drawing function, which selects the appropriate function above
  ====================*/
void	CMeshRenderer::DrawMesh(EMaterialPhase ePhase)
{
	PROFILE("CMeshRenderer::DrawMesh");

	if (MeshShouldDeform(m_cEntity, m_pMesh) && !vid_meshAlwaysStatic)
	{
		// a deforming (skinned) m_pMesh
		if (vid_meshGPUDeform && m_pMesh->flags & MESH_GPU_DEFORM)
			DrawSkinnedMeshGPU();
		else
			DrawSkinnedMesh();

		SceneStats.RecordBatch(m_pMesh->num_verts, m_pMesh->numFaces, ePhase, SSBATCH_DYNAMICMESH);
	}
	else
	{
		DrawStaticMesh();

		SceneStats.RecordBatch(m_pMesh->num_verts, m_pMesh->numFaces, ePhase, SSBATCH_STATICMESH);
	}
}


/*====================
  CMeshRenderer::DrawStaticNormals
  ====================*/
bool	CMeshRenderer::DrawStaticNormals()
{
	PROFILE("CMeshRenderer::DrawStaticNormals");

	if (!vid_drawMeshes)
		return true;

	if (!g_pVBStaticMeshNormals[m_pMesh->sbuffer])
	{
		int iNumTangents(m_pMesh->num_uv_channels);

		if (FAILED(g_pd3dDevice->CreateVertexBuffer(m_pMesh->num_verts * (2 + iNumTangents * 2) * sizeof(SLineVertex),
				D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &g_pVBStaticMeshNormals[m_pMesh->sbuffer], NULL)))
			K2System.Error(_T("D3D_RenderTerrainChunkNormals(): CreateVertexBuffer failed"));

		SLineVertex *pVertices;
		if (FAILED(g_pVBStaticMeshNormals[m_pMesh->sbuffer]->Lock(0, m_pMesh->num_verts * (2 + iNumTangents * 2) * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_NOSYSLOCK)))
			return false;

		int iVert(0);
		for (int iX(0); iX < m_pMesh->num_verts; ++iX)
		{
			pVertices[iVert].v = CVec3_cast(m_pMesh->verts[iX]);
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;

			pVertices[iVert].v = pVertices[iVert - 1].v + CVec3_cast(m_pMesh->normals[iX]) * vid_meshNormalLength;
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;

			for (int iT(0); iT < iNumTangents; ++iT)
			{
				pVertices[iVert].v = CVec3_cast(m_pMesh->verts[iX]);
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 255, 0);
				++iVert;

				pVertices[iVert].v = pVertices[iVert - 1].v + CVec3_cast(m_pMesh->tangents[iT][iX]) * vid_meshNormalLength;
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 255, 0);
				++iVert;
			}
		}

		g_pVBStaticMeshNormals[m_pMesh->sbuffer]->Unlock();
	}

	D3D_SetStreamSource(0, g_pVBStaticMeshNormals[m_pMesh->sbuffer], 0, sizeof(SLineVertex));
	D3D_SelectMaterial(g_SimpleMaterial3DColored, PHASE_COLOR, VERTEX_LINE, m_pCam->GetTime(), false);
	D3D_DrawPrimitive(D3DPT_LINELIST, 0, m_pMesh->num_verts * 2);

	return true;
}


/*====================
  CMeshRenderer::DrawSkinnedNormals
  ====================*/
bool	CMeshRenderer::DrawSkinnedNormals()
{
	PROFILE("CMeshRenderer::DrawSkinnedNormals");

	if (!m_cEntity.skeleton)
	{
		Console.Warn << _T("CMeshRenderer::DrawSkinnedNormals: m_pMesh ") << m_pMesh->GetName() << _T(" in model ")
					<< m_pMesh->GetModel()->GetName() << _T(" has no skeleton") << newl;
		return false;
	}

	if (m_pMesh->mode != MESH_SKINNED_BLENDED && m_pMesh->mode != MESH_SKINNED_NONBLENDED)
	{
		Console.Warn << _T("CMeshRenderer::DrawSkinnedNormals: m_pMesh ") << m_pMesh->GetName() << _T(" in model ") << m_pMesh->GetModel()->GetName()
					<< _T(" has an invalid mode (mode==") << m_pMesh->mode << _T(")") << newl;
		return false;
	}

	if (m_pMesh->dbuffer == -1)
	{
		Console.Warn << _T("CMeshRenderer::DrawSkinnedNormals: m_pMesh ") << m_pMesh->GetName() << _T(" in model ") << m_pMesh->GetModel()->GetName()
					<< _T(" has no dynamic buffer") << newl;
		return false;
	}

	int iNumTangents(m_pMesh->num_uv_channels);

	if (!g_pVBDynamicMeshNormals[m_pMesh->dbuffer])
	{
		if (FAILED(g_pd3dDevice->CreateVertexBuffer(m_pMesh->num_verts * (2 + iNumTangents * 2) * sizeof(SLineVertex),
				D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &g_pVBDynamicMeshNormals[m_pMesh->dbuffer], NULL)))
			K2System.Error(_T("CMeshRenderer::DrawSkinnedNormals(): CreateVertexBuffer failed"));
	}

	SLineVertex *pVertices;

	if (FAILED(g_pVBDynamicMeshNormals[m_pMesh->dbuffer]->Lock(0, m_pMesh->num_verts * (2 + iNumTangents * 2) * sizeof(SLineVertex), (void**)&pVertices, D3DLOCK_DISCARD)))
		return false;

	if (vid_meshSkipCPUDeform || (!m_pMesh->blendedLinks && !m_pMesh->singleLinks))
	{
		int iVert(0);
		for (int iX(0); iX < m_pMesh->num_verts; ++iX)
		{
			pVertices[iVert].v = CVec3_cast(m_pMesh->verts[iX]);
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;

			pVertices[iVert].v = pVertices[iVert - 1].v + CVec3_cast(m_pMesh->normals[iX]) * vid_meshNormalLength;
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;

			for (int iT(0); iT < iNumTangents; ++iT)
			{
				pVertices[iVert].v = CVec3_cast(m_pMesh->verts[iX]);
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 255, 0);
				++iVert;

				pVertices[iVert].v = pVertices[iVert - 1].v + CVec3_cast(m_pMesh->tangents[iT][iX]) * vid_meshNormalLength;
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 255, 0);
				++iVert;
			}
		}
	}
	else if (m_pMesh->mode == MESH_SKINNED_BLENDED && !vid_meshForceNonBlendedDeform && m_pMesh->blendedLinks)
	{
		// Blended deformation
		int iVert(0);
		for (int v(0); v < m_pMesh->num_verts; ++v)
		{
			vec3_t final;
			int index;
			matrix43_t tm_blended;

			MemManager.Set(&tm_blended, 0, sizeof(tm_blended));

			SBlendedLink *link = &m_pMesh->blendedLinks[v];

			M_ClearVec3(final);

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

			M_TransformPoint(m_pMesh->verts[v], tm_blended.pos, (const vec3_t *)tm_blended.axis, vec3_cast(pVertices[iVert].v));
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;

			CVec3f v3Normal;
			M_RotatePoint(m_pMesh->normals[v], (const vec3_t *)tm_blended.axis, vec3_cast(v3Normal));

			pVertices[iVert].v = pVertices[iVert - 1].v + v3Normal * vid_meshNormalLength;
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;

			for (int iT(0); iT < iNumTangents; ++iT)
			{
				M_TransformPoint(m_pMesh->verts[v], tm_blended.pos, (const vec3_t *)tm_blended.axis, vec3_cast(pVertices[iVert].v));
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 255, 0);
				++iVert;

				CVec3f v3Tangent;
				M_RotatePoint(m_pMesh->tangents[iT][v], (const vec3_t *)tm_blended.axis, vec3_cast(v3Tangent));

				pVertices[iVert].v = pVertices[iVert - 1].v + v3Tangent * vid_meshNormalLength;
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 255, 0);
				++iVert;
			}
		}
	}
	else if (m_pMesh->singleLinks)
	{
		// Non - blended deformation
		int iVert(0);
		for (int v(0); v < m_pMesh->num_verts; ++v)
		{
			int boneidx;
			matrix43_t *tm_world;

			boneidx = m_pMapping[m_pMesh->singleLinks[v]];
			if (boneidx == -1)
			{
				pVertices[iVert].v = CVec3_cast(m_pMesh->verts[v]);
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
				++iVert;

				pVertices[iVert].v = pVertices[iVert - 1].v + CVec3_cast(m_pMesh->normals[v]) * vid_meshNormalLength;
				pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
				++iVert;
				continue;
			}

			tm_world = &m_cEntity.skeleton->GetBoneState(boneidx)->tm_world;

			M_TransformPoint(m_pMesh->verts[v], tm_world->pos, (const vec3_t *)tm_world->axis, vec3_cast(pVertices[iVert].v));
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;

			CVec3f v3Normal;
			M_RotatePoint(m_pMesh->normals[v], (const vec3_t *)tm_world->axis, vec3_cast(v3Normal));

			pVertices[iVert].v = pVertices[iVert - 1].v + v3Normal * vid_meshNormalLength;
			pVertices[iVert].color = D3DCOLOR_ARGB(255, 0, 0, 255);
			++iVert;
		}
	}

	g_pVBDynamicMeshNormals[m_pMesh->dbuffer]->Unlock();

	if (vid_drawMeshes)
	{
		D3D_SetStreamSource(0, g_pVBDynamicMeshNormals[m_pMesh->dbuffer], 0, sizeof(SLineVertex));
		D3D_SelectMaterial(g_SimpleMaterial3DColored, PHASE_COLOR, VERTEX_LINE, m_pCam->GetTime(), false);
		D3D_DrawPrimitive(D3DPT_LINELIST, 0, m_pMesh->num_verts * 2);
	}

	return true;
}


/*====================
  CMeshRenderer::DrawNormals
  ====================*/
void	CMeshRenderer::DrawNormals()
{
	PROFILE("CMeshRenderer::DrawNormals");

	if (MeshShouldDeform(m_cEntity, m_pMesh) && !vid_meshAlwaysStatic)
		DrawSkinnedNormals();
	else
		DrawStaticNormals();
}


/*====================
  CMeshRenderer::Render
  ====================*/
void	CMeshRenderer::Render(EMaterialPhase ePhase)
{
	PROFILE("CMeshRenderer::Render");

	SetShaderVars();

	// Draw mesh names
	if (ePhase == PHASE_COLOR && vid_drawMeshNames && !g_bReflectPass)
	{
		CVec3f	v3Pos(D3D_TransformPoint((CVec3_cast(m_pMesh->bmin) + CVec3_cast(m_pMesh->bmax)) * 0.5f, m_mWorld));

		D3D_AddPoint(v3Pos, CVec4f(0.0f, 0.0f, 1.0f, 1.0f));

		CVec2f	v2ScreenPos;
		if (m_pCam->WorldToScreen(v3Pos, v2ScreenPos))
		{
			Draw2D.SetColor(CVec4f(1.0f, 1.0f, 1.0f, 1.0f));
			Draw2D.String(floor(v2ScreenPos.x), floor(v2ScreenPos.y), m_pMesh->GetName(), g_ResourceManager.LookUpName(_T("system_small"), RES_FONTMAP));
		}
	}

	if (ePhase == PHASE_COLOR && vid_sceneBuffer && m_pMaterial->HasPhase(PHASE_REFRACT))
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

	if (ePhase == PHASE_COLOR && (vid_drawMeshNormals || gfx_normals))
		DrawNormals();
}
