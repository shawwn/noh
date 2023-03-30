// (C)2006 S2 Games
// c_foliagerenderer.cpp
//
// Foliage chunk renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "d3d9f_main.h"
#include "d3d9f_util.h"
#include "d3d9f_material.h"
#include "d3d9f_scene.h"
#include "d3d9f_state.h"
#include "d3d9f_shader.h"
#include "d3d9f_terrain.h"
#include "d3d9f_texture.h"
#include "c_foliagerenderer.h"
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
#include "../k2/s_foliagetile.h"
#include "../k2/c_scenestats.h"
//=============================================================================

//=============================================================================
// Cvars
//=============================================================================
CVAR_BOOLF	(vid_foliageWireframe,			false,						CONEL_DEV);
CVAR_INT	(vid_foliageAlphaTestRef,		90);
CVAR_BOOLF	(vid_foliageLayer1,				true,						CONEL_DEV);
CVAR_BOOLF	(vid_foliageLayer2,				true,						CONEL_DEV);
CVAR_BOOL	(vid_foliageNoCull,				false);

CONST_STRING(DIFFUSE, _T("diffuse"));
//=============================================================================

CPool<CFoliageRenderer> CFoliageRenderer::s_Pool(1, -1);

/*====================
  CFoliageRenderer::operator new
  ====================*/
void*	CFoliageRenderer::operator new(size_t z)
{
	return s_Pool.Allocate();
}


/*====================
  CFoliageRenderer::CFoliageRenderer
  ====================*/
CFoliageRenderer::CFoliageRenderer(EFoliageRenderOrder eOrder, int iFlags) :
IRenderer(RT_UNKNOWN),
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


/*====================
  CFoliageRenderer::Setup
  ====================*/
void	CFoliageRenderer::Setup(EMaterialPhase ePhase)
{
	PROFILE("CFoliageRenderer::Setup");

	m_bRender = false; // Set to true if we make it to the end of the function

	CMaterial &cMaterial(D3D_GetMaterial(g_hFoliageMaterial));

	if (!cMaterial.HasPhase(ePhase))
		return; // Leave if we don't have this phase

	m_pCam = g_pCam;
	m_bLighting = gfx_lighting;
	m_bShadows = g_bCamShadows;
	m_bFog = g_bCamFog;
	m_vAmbient = SceneManager.GetTerrainAmbientColor();
	m_vSunColor = SceneManager.GetTerrainSunColor();

	m_iNumActiveBones = 0;

	// Pick the four best point lights to light foliage
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

			for (int iChunkY(0); iChunkY != g_Foliage.iNumChunksY; ++iChunkY)
			{
				for (int iChunkX(0); iChunkX != g_Foliage.iNumChunksX; ++iChunkX)
				{
					SFoliageChunk &cChunk(g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX]);

					if (!cChunk.bVisible)
						continue;

					if (I_SphereBoundsIntersect(cLightSphere, cChunk.bbBounds))
					{
						m_vPointLightPosition[m_iNumActivePointLights] = scLight.GetPosition();
						m_vPointLightColor[m_iNumActivePointLights] = scLight.GetColor();
						m_fPointLightFalloffStart[m_iNumActivePointLights] = scLight.GetFalloffStart();
						m_fPointLightFalloffEnd[m_iNumActivePointLights] = scLight.GetFalloffEnd();
						++m_iNumActivePointLights;

						iChunkY = g_Foliage.iNumChunksY - 1; // Kill the chunk search
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
	m_mWorldViewProj = g_mViewProj;

	m_bTexkill = false;

	m_bRender = true;

	m_bFirstChunk = true;
	m_iDiffuseStageIndex = -1;

	g_ShaderRegistry.SetNumPointLights(m_iNumActivePointLights);
	g_ShaderRegistry.SetNumBones(m_iNumActiveBones);
	g_ShaderRegistry.SetLighting(m_bLighting);
	g_ShaderRegistry.SetShadows(m_bShadows);
	g_ShaderRegistry.SetFogofWar(g_bFogofWar);
	g_ShaderRegistry.SetFog(m_bFog);
	g_ShaderRegistry.SetTexcoords(m_iTexcoords);
	g_ShaderRegistry.SetTexkill(m_bTexkill);
	
	// Set sorting variables
	m_bTranslucent = (m_iFlags & FOLIAGE_ALPHABLEND) != 0;
	m_iLayer = cMaterial.GetPhase(ePhase).GetLayer();
	m_iEffectLayer = 0;
	m_iVertexType = VERTEX_FOLIAGE;
	m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(cMaterial.GetPhase(ePhase).GetVertexShader());
	m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(cMaterial.GetPhase(ePhase).GetPixelShader());
	m_uiVertexBuffer = 0;
	m_uiIndexBuffer = 0;
	m_fDepth = 0.0f;
}


/*====================
  CFoliageRenderer::RenderChunk
  ====================*/
void	CFoliageRenderer::RenderChunk(EMaterialPhase ePhase, int iChunkX, int iChunkY)
{
	PROFILE("CFoliageRenderer::RenderChunk");

	SFoliageChunk &oChunk = g_Foliage.pChunks[iChunkY * g_Foliage.iNumChunksX + iChunkX];

	//
	// Setup per chunk shader vars
	//

	if (m_pCam->HasFlags(CAM_WIREFRAME_TERRAIN) || vid_foliageWireframe)
		D3D_PushRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	else
		D3D_PushRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	CMaterial &cMaterial(D3D_GetMaterial(g_hFoliageMaterial));

	if ((m_iFlags & FOLIAGE_ALPHATEST))
	{
		cMaterial.GetPhase(PHASE_COLOR).SetAlphaTest(true);
		D3D_SetRenderState(D3DRS_ALPHAREF, vid_foliageAlphaTestRef);
	}
	else
	{
		cMaterial.GetPhase(PHASE_COLOR).SetAlphaTest(false);
	}

	CVec2f vDir(m_pCam->GetViewAxis(FORWARD).xy());

	float fAngle(acos(Normalize(vDir).x));

	if (vDir.y < 0.0f)
		fAngle = 2.0f * M_PI - fAngle;

	// Flip angle to get front->back rendering
	if (m_eOrder == FOLIAGE_FRONTBACK)
		fAngle += M_PI;

	float fDir = fAngle / (2.0f * M_PI) * NUM_SORT_DIRECTIONS;

	int iDir = INT_ROUND(fDir) % NUM_SORT_DIRECTIONS;

	cMaterial.GetPhase(PHASE_COLOR).SetVertexShader(g_hFoliageVertexShaderNormal);

	if (m_bFirstChunk)
	{
		D3D_SelectMaterial(cMaterial, ePhase, VERTEX_FOLIAGE, g_pCam->GetTime(), false);
		m_bFirstChunk = false;

		m_iDiffuseStageIndex = D3D_GetSamplerStageIndex(DIFFUSE);
	}

	D3D_SetRenderState(D3DRS_ALPHABLENDENABLE, (m_iFlags & FOLIAGE_ALPHABLEND) ? TRUE : FALSE);
	D3D_SetRenderState(D3DRS_ZWRITEENABLE, (m_iFlags & FOLIAGE_DEPTHWRITE) ? TRUE : FALSE);

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

			D3D_UpdateShaderTexture(m_iDiffuseStageIndex, oChunk.pArrays[iLayer][n]->hTexture);

			D3D_SetStreamSource(0, oChunk.pArrays[iLayer][n]->pVB, 0, sizeof(SFoliageVertex));
			D3D_SetIndices(oChunk.pArrays[iLayer][n]->pIB[iDir]);
			D3D_DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, oChunk.pArrays[iLayer][n]->iNumFoliageQuads * 4, 0, oChunk.pArrays[iLayer][n]->iNumFoliageQuads * 2);

			SceneStats.RecordBatch(oChunk.pArrays[iLayer][n]->iNumFoliageQuads * 4, oChunk.pArrays[iLayer][n]->iNumFoliageQuads * 2, ePhase, SSBATCH_FOLIAGE);
		}
	}

	D3D_PopRenderState(D3DRS_FILLMODE);
}


/*====================
  CFoliageRenderer::Render
  ====================*/
void	CFoliageRenderer::Render(EMaterialPhase ePhase)
{
	PROFILE("CFoliageRenderer::Render");

	if (!m_bRender)
		return;

	SetShaderVars();

	for (int iY(0); iY < g_Foliage.iNumChunksY; ++iY)
	{
		for (int iX(0); iX < g_Foliage.iNumChunksX; ++iX)
		{
			SFoliageChunk &oChunk(g_Foliage.pChunks[iY * g_Foliage.iNumChunksX + iX]);

			if (oChunk.bVisible || vid_foliageNoCull)
				RenderChunk(ePhase, iX, iY);
		}
	}
}
