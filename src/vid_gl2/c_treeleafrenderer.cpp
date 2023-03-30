// (C)2008 S2 Games
// c_treeleafrenderer.cpp
//
// SpeedTree leaf renderer
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "vid_common.h"

#include "c_treeleafrenderer.h"

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

CPool<CTreeLeafRenderer> CTreeLeafRenderer::s_Pool(1, uint(-1));

/*====================
  CTreeLeafRenderer::operator new
  ====================*/
void*	CTreeLeafRenderer::operator new(size_t z, const char *szContext, const char *szType, const char *szFile, short nLine)
{
	return s_Pool.Allocate();
}


/*====================
  CTreeLeafRenderer::CTreeLeafRenderer
  ====================*/
CTreeLeafRenderer::CTreeLeafRenderer(const CSceneEntity &cEntity, const CTreeModelDef *pTreeDef,
		const D3DXMATRIXA16 &mWorldViewProj,
		const D3DXMATRIXA16 &mWorld,
		const D3DXMATRIXA16 &mWorldInverseRotate) :
m_cEntity(cEntity),
m_pTreeDef(pTreeDef)
{
	m_mWorldViewProj = mWorldViewProj;
	m_mWorld = mWorld;
	m_mWorldRotate = mWorldInverseRotate;

	{
		PROFILE("GetLeafBillboardTable");

		// Fill in the cluster data that the vertex shader will use
		uint uiSize(0);
		const float *pTable = m_pTreeDef->GetLeafBillboardTable(uiSize);
		if (uiSize > LEAF_CLUSTER_TABLE_SIZE)
		{
			Console.Warn << _T("Too many entries in this tree's billboard table") << newl;
			uiSize = LEAF_CLUSTER_TABLE_SIZE;
		}
		m_uiLeafClusterDataSize = uiSize;
		MemManager.Copy(m_afLeafClusterData, pTable, uiSize * sizeof(float));
	}

	{
		PROFILE("GetLeafLODData");

		// Update leaf cluster data for current camera position
		m_avLeafLODs[0].m_bActive = m_avLeafLODs[1].m_bActive = false;
		m_pTreeDef->GetLeafLODData(m_avLeafLODs);
	}
}


/*====================
  CTreeLeafRenderer::~CTreeLeafRenderer
  ====================*/
CTreeLeafRenderer::~CTreeLeafRenderer()
{
}


/*====================
  CTreeLeafRenderer::Setup
  ====================*/
void	CTreeLeafRenderer::Setup(EMaterialPhase ePhase)
{
	PROFILE("CTreeLeafRenderer::Setup");

	if (!m_avLeafLODs[0].m_bActive && m_avLeafLODs[1].m_bActive)
		return;

	if (!m_pTreeDef->HasLeafGeometry())
		return;

	CMaterial &material(GfxUtils->GetMaterial(m_pTreeDef->GetLeafMaterial()));

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
		CBBoxf	bbBoundsWorld(m_pTreeDef->GetBounds());
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

	// Set sorting variables
	m_bTranslucent = material.GetPhase(ePhase).GetTranslucent();
	m_iLayer = material.GetPhase(ePhase).GetLayer();
	m_iEffectLayer = 0;
	//m_iVertexShaderInstance = g_ShaderRegistry.GetVertexShaderInstance(material.GetPhase(ePhase).GetVertexShader());
	//m_iPixelShaderInstance = g_ShaderRegistry.GetPixelShaderInstance(material.GetPhase(ePhase).GetPixelShader());

	if (m_avLeafLODs[0].m_bActive)
		m_uiVertexBuffer = m_pTreeDef->GetLeafGeometry(m_avLeafLODs[0].m_iLOD).m_VBuffer;
	
	m_uiIndexBuffer = 0;
	m_fDepth = 0.0f;
	m_bRefractive = material.GetPhase(ePhase).GetRefractive();
}


/*====================
  CTreeLeafRenderer::Render
  ====================*/
void	CTreeLeafRenderer::Render(EMaterialPhase ePhase)
{
	if (!m_bRender)
		return;

	if (ePhase == PHASE_COLOR && m_cEntity.flags & SCENEENT_SOLID_COLOR && m_cEntity.color[A] < 1.0f && GfxUtils->GetMaterial(m_pTreeDef->GetLeafMaterial()).HasPhase(PHASE_FADE))
		ePhase = PHASE_FADE;

	SetShaderVars();
	GfxMaterials->SelectMaterial(GfxUtils->GetMaterial(m_pTreeDef->GetLeafMaterial()), ePhase, 0.0f, 0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glMultMatrixf(m_mWorld);

	for (int i(0); i < 2; ++i)
	{
		if (!m_avLeafLODs[i].m_bActive)
			continue;

		const SLeafGeometryBuffers &leaves(m_pTreeDef->GetLeafGeometry(m_avLeafLODs[i].m_iLOD));
		glAlphaFunc(GL_GREATER, m_avLeafLODs[i].m_dwAlphaTest / 255.0f);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, leaves.m_VBuffer);
		glNormalPointer(GL_FLOAT, sizeof(SLeafVert), BUFFER_OFFSET(12));
		glTexCoordPointer(2, GL_FLOAT, sizeof(SLeafVert), BUFFER_OFFSET(24));
		glVertexPointer(3, GL_FLOAT, sizeof(SLeafVert), BUFFER_OFFSET(0));

		GfxMaterials->BindAttributes(CTreeModelDef::s_mapLeafAttributes, sizeof(SLeafVert));

		for (size_t z(0); z < leaves.m_vIBuffers.size(); ++z)
		{
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, leaves.m_vIBuffers[z]);
			glDrawElements(GL_TRIANGLES, leaves.m_viNumIndices[z], GL_UNSIGNED_SHORT, NULL);
		}

		GfxMaterials->UnbindAttributes();
	}
	
	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glAlphaFunc(GL_GREATER, vid_alphaTestRef / 255.0f);
}
