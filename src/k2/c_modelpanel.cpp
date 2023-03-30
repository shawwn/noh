// (C)2006 S2 Games
// c_modelpanel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_modelpanel.h"
#include "c_interface.h"
#include "c_uitextureregistry.h"
#include "c_uiscript.h"
#include "c_uicmd.h"
#include "c_widgetstyle.h"
#include "c_uimanager.h"
#include "c_buffer.h"
#include "c_scenemanager.h"
#include "c_skeleton.h"
#include "c_effect.h"
#include "c_effectthread.h"
#include "c_particlesystem.h"
#include "c_resourcemanager.h"
#include "c_resourceinfo.h"
//=============================================================================

extern CCvar<CVec3f, float>	scene_terrainSunColor;
extern CCvar<CVec3f, float>	scene_terrainAmbientColor;
extern CCvar<CVec3f, float>	scene_entitySunColor;
extern CCvar<CVec3f, float>	scene_entityAmbientColor;
extern CCvar<float>			scene_sunAltitude;
extern CCvar<float>			scene_sunAzimuth;


/*====================
  CModelPanel::~CModelPanel
  ====================*/
CModelPanel::~CModelPanel()
{
	SAFE_DELETE(m_pSkeleton);
	SAFE_DELETE(m_pEffectThread);

	SAFE_DELETE(m_sRestoreAnim);
	SAFE_DELETE(m_sRestoreModel);
	SAFE_DELETE(m_sRestoreEffect);
}


/*====================
  CModelPanel::CModelPanel
  ====================*/
CModelPanel::CModelPanel(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_MODELPANEL, style, false),
m_pSkeleton(NULL),
m_pEffectThread(NULL),
m_v3ModelPos(style.GetPropertyVec3(_T("modelpos"), CVec3f(0.0f, 0.0f, 0.0f))),
m_v3ModelAngles(style.GetPropertyVec3(_T("modelangles"), CVec3f(0.0f, 0.0f, 0.0f))),
m_fModelScale(style.GetPropertyFloat(_T("modelscale"), 1.0f)),
m_bOrtho(style.GetPropertyBool(_T("ortho"))),
m_bLookAt(style.GetPropertyBool(_T("lookat"), true)),
m_bShadows(style.GetPropertyBool(_T("shadows"), false)),
m_sAnim(style.GetProperty(_T("anim"))),
m_sSkin(style.GetProperty(_T("skin"))),
m_v3CameraPos(style.GetPropertyVec3(_T("camerapos"), CVec3f(0.0f, 0.0f, 0.0f))),
m_fCameraDist(style.GetPropertyFloat(_T("cameradistance"), 100.0f)),
m_v3SunColor(style.GetPropertyVec3(_T("suncolor"), CVec3f(0.9f, 0.9f, 0.9f))),
m_v3AmbientColor(style.GetPropertyVec3(_T("ambientcolor"), CVec3f(0.25f, 0.25f, 0.25f))),
m_fSunAltitude(style.GetPropertyFloat(_T("sunaltitude"), 30.0f)),
m_fSunAzimuth(style.GetPropertyFloat(_T("sunazimuth"), 115.0f)),
m_bFog(style.GetPropertyBool(_T("fog"), false)),
m_v3FogColor(style.GetPropertyVec3(_T("fogcolor"), CVec3f(0.75f, 0.75f, 0.75f))),
m_fFogNear(style.GetPropertyFloat(_T("fognear"), 0.0f)),
m_fFogFar(style.GetPropertyFloat(_T("fogfar"), 1000.0f)),
m_fFogScale(style.GetPropertyFloat(_T("fogscale"), 1.0f)),
m_fFogDensity(style.GetPropertyFloat(_T("fogdensity"), 0.0005f)),
m_bFovY(false),
m_fCameraNear(style.GetPropertyFloat(_T("cameranear"), 0.0f)),
m_fCameraFar(style.GetPropertyFloat(_T("camerafar"), 0.0f)),
m_bDepthClear(style.GetPropertyBool(_T("depthclear"), true)),
m_bDepthCompress(style.GetPropertyBool(_T("depthcompress"), false)),
m_bPostEffects(style.GetPropertyBool(_T("posteffects"), false)),
m_bReflections(style.GetPropertyBool(_T("reflections"), false)),
m_bSceneBuffer(style.GetPropertyBool(_T("scenebuffer"), false)),
m_bLookUpResource(style.GetPropertyBool(_T("lookupresource"), false)),
m_v3ModelStartPos(m_v3ModelPos),
m_v3ModelEndPos(m_v3ModelPos),
m_v3ModelStartAngles(m_v3ModelPos),
m_v3ModelEndAngles(m_v3ModelPos),
m_uiModelMoveStartTime(INVALID_TIME),
m_uiModelMoveEndTime(INVALID_TIME),
m_uiModelRotationStartTime(INVALID_TIME),
m_uiModelRotationEndTime(INVALID_TIME),
m_v4TeamColor(WHITE),
m_sRestoreModel(NULL),
m_sRestoreEffect(NULL),
m_sRestoreAnim(NULL)
{
	m_uiFlags |= WFLAG_NO_DRAW;

	if (style.HasProperty(_T("model")))
		m_hModel = g_ResourceManager.Register(style.GetProperty(_T("model")), RES_MODEL);
	else
		m_hModel = INVALID_RESOURCE;

	m_Camera.DefaultCamera(m_recArea.GetWidth(), m_recArea.GetHeight());

	m_Camera.SetAngles(style.GetPropertyVec3(_T("cameraangles"), CVec3f(0.0f, 0.0f, 180.0f)));
	
	if (m_hModel != INVALID_RESOURCE)
	{
		if (m_bLookAt)
		{
			CBBoxf bbModel(g_ResourceManager.GetModelBounds(m_hModel));
			CVec3f v3CamLookAt(m_v3ModelPos + bbModel.GetMid());
			float fCamDistance(m_fCameraDist);
			m_Camera.SetOrigin(v3CamLookAt + m_Camera.GetViewAxis(FORWARD) * -fCamDistance);
		}
		
		m_Camera.SetOrigin(m_Camera.GetOrigin() + m_v3CameraPos);

		SetAnim(m_sAnim);

		if (!m_sSkin.empty())
			m_hSkin = g_ResourceManager.GetSkin(m_hModel, m_sSkin);
		else
			m_hSkin = 0;

		g_ResourceManager.PrecacheSkin(m_hModel, m_hSkin);
	}
	else
	{
		m_Camera.SetOrigin(m_v3CameraPos);
	}

	m_Camera.SetWidth(m_recArea.GetWidth());
	m_Camera.SetHeight(m_recArea.GetHeight());

	m_Camera.SetZNear(m_fCameraNear);
	m_Camera.SetZFar(m_fCameraFar);

	int iFlags(CAM_NO_WORLD);

	if (m_bOrtho)
		iFlags |= CAM_ORTHO;
	if (!m_bShadows)
		iFlags |= CAM_NO_SHADOWS;
	if (!m_bFog)
		iFlags |= CAM_NO_FOG;
	if (!m_bDepthClear)
		iFlags |= CAM_NO_DEPTH_CLEAR;
	if (m_bDepthCompress)
		iFlags |= CAM_DEPTH_COMPRESS;
	if (!m_bPostEffects)
		iFlags |= CAM_NO_POST;
	if (!m_bReflections)
		iFlags |= CAM_NO_REFLECTIONS;
	if (!m_bSceneBuffer)
		iFlags |= CAM_NO_SCENE_BUFFER;

	m_Camera.SetFlags(iFlags);

	if (m_bOrtho)
	{
		m_Camera.SetOrthoWidth(m_recArea.GetWidth());
		m_Camera.SetOrthoHeight(m_recArea.GetHeight());
	}
	else
	{
		if (style.HasProperty(_T("camerafovy")))
		{
			m_Camera.SetFovYCalc(style.GetPropertyFloat(_T("camerafovy")));
			m_bFovY = true;
		}
		else
		{
			m_Camera.SetFovXCalc(style.GetPropertyFloat(_T("camerafov"), 90.0f));
			m_bFovY = false;
		}
	}

	// Effect
	SetEffect(style.GetProperty(_T("effect")));

	int i(0);
	while (style.HasProperty(_T("model") + XtoA(i)))
	{
		ReadModelProperties(style, i);
		++i;
	}

	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)
}


/*====================
  CModelPanel::ReadModelProperties
  ====================*/
void	CModelPanel::ReadModelProperties(const CWidgetStyle& style, int i)
{
	SModel &cModel(m_mapModels[i]);
	const tstring &sI(XtoA(i));
	const tstring &sModelPath(style.GetProperty(_T("model") + sI));

	cModel.hModel = INVALID_RESOURCE;

	if (m_bLookUpResource)
		cModel.hModel = g_ResourceManager.LookUpPrecached(sModelPath);

	if (cModel.hModel == INVALID_RESOURCE)
	{
		K2_WITH_RESOURCE_SCOPE(GetResourceContext())
			cModel.hModel = g_ResourceManager.Register(sModelPath, RES_MODEL);
	}

	assert(cModel.hModel != INVALID_RESOURCE);
	if (cModel.hModel == INVALID_RESOURCE)
		return;

	const tstring &sAnim(style.GetProperty(_T("anim") + sI));

	if (!sAnim.empty())
		SetAnim(i, sAnim);

	if (!m_sSkin.empty())
		m_hSkin = g_ResourceManager.GetSkin(m_hModel, m_sSkin);
	else
		m_hSkin = 0;

	K2_WITH_RESOURCE_SCOPE(GetResourceContext())
		g_ResourceManager.PrecacheSkin(m_hModel, m_hSkin);

	SetEffect(i, style.GetProperty(_T("effect") + sI));
}


/*====================
  CModelPanel::UpdateEffect
  ====================*/
void	CModelPanel::UpdateEffect(CEffectThread *&pEffectThread, const CVec3f &v3ModelPos, const CVec3f &v3ModelAngles, float fModelScale, ResHandle hModel)
{
	if (!pEffectThread)
		return;

	pEffectThread->SetActive(true);

	// Update entity attachment information
	pEffectThread->SetSourcePos(v3ModelPos);
	pEffectThread->SetSourceAxis(CAxis(v3ModelAngles));
	pEffectThread->SetSourceScale(fModelScale);
	pEffectThread->SetSourceModel(g_ResourceManager.GetModel(hModel));

	if (pEffectThread->Execute(Host.GetTime()))
	{
		// Effect finished, so delete it
		SAFE_DELETE(pEffectThread);
	}
	else
	{
		// Update and render all particles systems associated with this effect thread
		const InstanceMap &mapInstances(pEffectThread->GetInstances());

		for (InstanceMap::const_iterator psit(mapInstances.begin()); psit != mapInstances.end(); ++psit)
		{
			IEffectInstance *pParticleSystem(psit->second);

			pParticleSystem->Update(Host.GetTime(), NULL);

			if (!pParticleSystem->IsDead() && pParticleSystem->IsParticleSystem())
				SceneManager.AddParticleSystem(static_cast<CParticleSystem *>(pParticleSystem), false);
		}

		pEffectThread->Cleanup();
	}
}


/*====================
  CModelPanel::RenderWidget
  ====================*/
void	CModelPanel::RenderWidget(const CVec2f &vOrigin, float fFade)
{
	if (!HasFlags(WFLAG_VISIBLE))
		return;

	IWidget::RenderWidget(vOrigin, fFade);

	m_Camera.SetX(vOrigin.x + m_recArea.left);
	m_Camera.SetY(vOrigin.y + m_recArea.top);

	bool bRender(false);

	SceneManager.Clear();

	// verify that the model hasn't been unregistered.
	if (m_hModel != INVALID_RESOURCE && g_ResourceManager.Get(m_hModel) != NULL)
	{
		bRender = true;

		if (m_uiModelMoveStartTime != INVALID_TIME && m_uiModelMoveEndTime != INVALID_TIME)
		{
			if (Host.GetTime() >= m_uiModelMoveEndTime)
			{
				m_v3ModelPos = m_v3ModelEndPos;
				m_uiModelMoveStartTime = m_uiModelMoveEndTime = INVALID_TIME;
			}
			else
			{
				float fLerp(MsToSec(Host.GetTime() - m_uiModelMoveStartTime) / MsToSec(m_uiModelMoveEndTime - m_uiModelMoveStartTime));
				m_v3ModelPos = LERP(fLerp, m_v3ModelStartPos, m_v3ModelEndPos);
			}
		}

		if (m_uiModelRotationStartTime != INVALID_TIME && m_uiModelRotationEndTime != INVALID_TIME)
		{
			if (Host.GetTime() >= m_uiModelRotationEndTime)
			{
				m_v3ModelAngles = m_v3ModelEndAngles;
				m_uiModelRotationStartTime = m_uiModelRotationEndTime = INVALID_TIME;
			}
			else
			{
				float fLerp(MsToSec(Host.GetTime() - m_uiModelRotationStartTime) / MsToSec(m_uiModelRotationEndTime - m_uiModelRotationStartTime));
				m_v3ModelAngles = LERP(fLerp, m_v3ModelStartAngles, m_v3ModelEndAngles);
			}
		}

		if (m_pSkeleton)
		{
			m_pSkeleton->Pose(Host.GetTime());
			m_pSkeleton->ClearEvents();
		}

		CSceneEntity cEntity;
		cEntity.objtype = OBJTYPE_MODEL;
		cEntity.hRes = m_hModel;
		cEntity.skeleton = m_pSkeleton;
		cEntity.hSkin = m_hSkin;
		cEntity.color[0] = 1.0f;
		cEntity.color[1] = 1.0f;
		cEntity.color[2] = 1.0f;
		cEntity.color[3] = fFade;
		cEntity.SetPosition(m_v3ModelPos);
		cEntity.angle = m_v3ModelAngles;
		cEntity.scale = m_fModelScale;
		cEntity.flags = SCENEENT_SOLID_COLOR;
		cEntity.teamcolor = m_v4TeamColor;

		SceneManager.AddEntity(cEntity, false);

		// Update attached effect
		UpdateEffect(m_pEffectThread, m_v3ModelPos, m_v3ModelAngles, m_fModelScale, m_hModel);
	}

	for (ModelMap::iterator it(m_mapModels.begin()); it != m_mapModels.end(); ++it)
	{
		bRender = true;

		SModel &cModel(it->second);

		// verify that the model hasn't been unregistered.
		if (g_ResourceManager.Get(cModel.hModel) == NULL)
			continue;

		if (cModel.pSkeleton)
		{
			cModel.pSkeleton->Pose(Host.GetTime());
			cModel.pSkeleton->ClearEvents();
		}

		CSceneEntity cEntity;
		cEntity.objtype = OBJTYPE_MODEL;
		cEntity.hRes = cModel.hModel;
		cEntity.skeleton = cModel.pSkeleton;
		cEntity.hSkin = cModel.hSkin;
		cEntity.color[0] = 1.0f;
		cEntity.color[1] = 1.0f;
		cEntity.color[2] = 1.0f;
		cEntity.color[3] = fFade;
		cEntity.SetPosition(cModel.v3ModelPos);
		cEntity.angle = cModel.v3ModelAngles;
		cEntity.scale = cModel.fModelScale;
		cEntity.flags = SCENEENT_SOLID_COLOR;

		SceneManager.AddEntity(cEntity, false);

		// Update attached effect
		UpdateEffect(cModel.pEffectThread, cModel.v3ModelPos, cModel.v3ModelAngles, cModel.fModelScale, cModel.hModel);
	}

	if (!bRender)
		return;

	// Save old lighting information
	CVec3f	v3OldSunColor(scene_entitySunColor);
	CVec3f	v3OldAmbientColor(scene_entityAmbientColor);
	float	fOldSunAltitude(scene_sunAltitude);
	float	fOldSunAzimuth(scene_sunAzimuth);

	CVec3f	v3OldFogColor(ICvar::GetVec3(_T("gfx_fogColor")));
	float	fOldFogNear(ICvar::GetFloat(_T("gfx_fogNear")));
	float	fOldFogFar(ICvar::GetFloat(_T("gfx_fogFar")));
	float	fOldFogScale(ICvar::GetFloat(_T("gfx_fogScale")));
	float	fOldFogDensity(ICvar::GetFloat(_T("gfx_fogDensity")));

	scene_entitySunColor = m_v3SunColor;
	scene_entityAmbientColor = m_v3AmbientColor;
	scene_sunAltitude = m_fSunAltitude;
	scene_sunAzimuth = m_fSunAzimuth;

	ICvar::SetVec3(_T("gfx_fogColor"), m_v3FogColor);
	ICvar::SetFloat(_T("gfx_fogNear"), m_fFogNear);
	ICvar::SetFloat(_T("gfx_fogFar"), m_fFogFar);
	ICvar::SetFloat(_T("gfx_fogScale"), m_fFogScale);
	ICvar::SetFloat(_T("gfx_fogDensity"), m_fFogDensity);

	SceneManager.PrepCamera(m_Camera);
	SceneManager.Render();

	scene_entitySunColor = v3OldSunColor;
	scene_entityAmbientColor = v3OldAmbientColor;
	scene_sunAltitude = fOldSunAltitude;
	scene_sunAzimuth = fOldSunAzimuth;

	ICvar::SetVec3(_T("gfx_fogColor"), v3OldFogColor);
	ICvar::SetFloat(_T("gfx_fogNear"), fOldFogNear);
	ICvar::SetFloat(_T("gfx_fogFar"), fOldFogFar);
	ICvar::SetFloat(_T("gfx_fogScale"), fOldFogScale);
	ICvar::SetFloat(_T("gfx_fogDensity"), fOldFogDensity);
}


/*====================
  CModelPanel::SetModel
  ====================*/
void	CModelPanel::SetModel(const tstring &sModel)
{
	SAFE_DELETE(m_pSkeleton);

	SAFE_DELETE(m_sRestoreModel);

	if (!m_sResourceContext.empty())
		m_sRestoreModel = K2_NEW(ctx_Widgets, tstring)(sModel);


	if (m_bLookUpResource)
	{
		m_hModel = g_ResourceManager.LookUpPath(sModel);
	}
	else
	{
		K2_WITH_RESOURCE_SCOPE(GetResourceContext())
			m_hModel = g_ResourceManager.Register(sModel, RES_MODEL);
	}

	if (m_hModel != INVALID_RESOURCE)
	{
		m_Camera.SetOrigin(V3_ZERO);

		if (m_bLookAt)
		{
			CBBoxf bbModel(g_ResourceManager.GetModelBounds(m_hModel));
			CVec3f v3CamLookAt(m_v3ModelPos + bbModel.GetMid());
			float fCamDistance(m_fCameraDist);
			m_Camera.SetOrigin(v3CamLookAt + m_Camera.GetViewAxis(FORWARD) * -fCamDistance);
		}
		
		m_Camera.SetOrigin(m_Camera.GetOrigin() + m_v3CameraPos);
		
		if (!m_sAnim.empty())
		{
			m_pSkeleton = K2_NEW(ctx_Widgets,  CSkeleton)();
			m_pSkeleton->SetModel(m_hModel);
			SetAnim(m_sAnim);
		}

		if (!m_sSkin.empty())
			m_hSkin = g_ResourceManager.GetSkin(m_hModel, m_sSkin);
		else
			m_hSkin = 0;
	}
	else
	{
		m_Camera.SetOrigin(m_v3CameraPos);
	}

	SAFE_DELETE(m_pEffectThread);
}


/*====================
  CModelPanel::SetAnim
  ====================*/
void	CModelPanel::SetAnim(const tstring &sAnim)
{
	SAFE_DELETE(m_sRestoreAnim);

	if (sAnim.empty())
	{
		SAFE_DELETE(m_pSkeleton);
	}
	else
	{
		if (!m_sResourceContext.empty())
			m_sRestoreAnim = K2_NEW(ctx_Widgets, tstring)(sAnim);

		if (!m_pSkeleton)
		{
			m_pSkeleton = K2_NEW(ctx_Widgets,  CSkeleton)();
			m_pSkeleton->SetModel(m_hModel);
		}

		m_pSkeleton->StartAnim(sAnim, Host.GetTime(), 0);
	}

	if (m_pEffectThread)
		m_pEffectThread->SetSourceSkeleton(m_pSkeleton);
}


/*====================
  CModelPanel::SetEffect
  ====================*/
void	CModelPanel::SetEffect(const tstring &sEffect)
{
	SAFE_DELETE(m_pEffectThread);

	SAFE_DELETE(m_sRestoreEffect);

	if (sEffect.empty())
		return;

	if (!m_sResourceContext.empty())
		m_sRestoreEffect = K2_NEW(ctx_Widgets, tstring)(sEffect);

	ResHandle hEffect(INVALID_RESOURCE);
	if (m_bLookUpResource)
		hEffect = g_ResourceManager.LookUpPath(sEffect);
	else
	{
		K2_WITH_RESOURCE_SCOPE(GetResourceContext())
			hEffect = g_ResourceManager.Register(sEffect, RES_EFFECT);
	}

	if (hEffect != INVALID_RESOURCE)
	{
		CEffect	*pEffect(g_ResourceManager.GetEffect(hEffect));

		if (pEffect)
		{
			m_pEffectThread = pEffect->SpawnThread(Host.GetTime());

			if (m_pEffectThread)
			{
				m_pEffectThread->SetCamera(&m_Camera);
				m_pEffectThread->SetWorld(NULL);
				m_pEffectThread->SetSourceSkeleton(m_pSkeleton);
				m_pEffectThread->SetSourceModel(g_ResourceManager.GetModel(m_hModel));
				m_pEffectThread->SetTargetSkeleton(NULL);
				m_pEffectThread->SetTargetModel(NULL);
			}
		}
	}
}


/*====================
  CModelPanel::GetAnim
  ====================*/
const tstring&	CModelPanel::GetAnim() const
{
	return (m_sRestoreAnim ? *m_sRestoreAnim : TSNULL);
}


/*====================
  CModelPanel::GetModel
  ====================*/
const tstring&	CModelPanel::GetModel() const
{
	return (m_sRestoreModel ? *m_sRestoreModel : TSNULL);
}


/*====================
  CModelPanel::GetEffect
  ====================*/
const tstring&	CModelPanel::GetEffect() const
{
	return (m_sRestoreEffect ? *m_sRestoreEffect : TSNULL);
}


/*====================
  CModelPanel::SetAnim
  ====================*/
void	CModelPanel::SetAnim(int i, const tstring &sAnim)
{
	ModelMap::iterator itFind(m_mapModels.find(i));
	if (itFind == m_mapModels.end())
		return;

	SModel &cModel(itFind->second);

	SAFE_DELETE(cModel.sAnim);

	if (sAnim.empty())
	{
		SAFE_DELETE(cModel.pSkeleton);
	}
	else
	{
		if (!m_sResourceContext.empty())
			cModel.sAnim = K2_NEW(ctx_Widgets, tstring)(sAnim);

		if (!cModel.pSkeleton)
		{
			cModel.pSkeleton = K2_NEW(ctx_Widgets,  CSkeleton)();
			cModel.pSkeleton->SetModel(cModel.hModel);
		}

		cModel.pSkeleton->StartAnim(sAnim, Host.GetTime(), 0);
	}

	if (cModel.pEffectThread)
		cModel.pEffectThread->SetSourceSkeleton(cModel.pSkeleton);
}


/*====================
  CModelPanel::SetEffect
  ====================*/
void	CModelPanel::SetEffect(int i, const tstring &sEffect)
{
	ModelMap::iterator itFind(m_mapModels.find(i));
	if (itFind == m_mapModels.end())
		return;

	SModel &cModel(itFind->second);

	SAFE_DELETE(cModel.pEffectThread);
	SAFE_DELETE(cModel.sEffect);

	if (sEffect.empty())
		return;

	if (!m_sResourceContext.empty())
		cModel.sEffect = K2_NEW(ctx_Widgets, tstring)(sEffect);

	ResHandle hEffect(INVALID_RESOURCE);
	K2_WITH_RESOURCE_SCOPE(GetResourceContext())
		hEffect = g_ResourceManager.Register(sEffect, RES_EFFECT);

	CEffect	*pEffect(g_ResourceManager.GetEffect(hEffect));

	if (pEffect)
	{
		cModel.pEffectThread = pEffect->SpawnThread(Host.GetTime());

		if (cModel.pEffectThread)
		{
			cModel.pEffectThread->SetCamera(&m_Camera);
			cModel.pEffectThread->SetWorld(NULL);
			cModel.pEffectThread->SetSourceSkeleton(cModel.pSkeleton);
			cModel.pEffectThread->SetSourceModel(g_ResourceManager.GetModel(m_hModel));
			cModel.pEffectThread->SetTargetSkeleton(NULL);
			cModel.pEffectThread->SetTargetModel(NULL);
		}
	}
}


/*====================
  CModelPanel::RecalculateSize
  ====================*/
void	CModelPanel::RecalculateSize()
{
	IWidget::RecalculateSize();

	m_Camera.SetWidth(m_recArea.GetWidth());
	m_Camera.SetHeight(m_recArea.GetHeight());

	if (m_bFovY)
		m_Camera.CalcFovX();
	else
		m_Camera.CalcFovY();
}


/*====================
  CModelPanel::RotateModel
  ====================*/
void	CModelPanel::RotateModel(const CVec3f &v3Start, const CVec3f &v3End, uint uiTime)
{
	m_v3ModelAngles = m_v3ModelStartAngles = v3Start;
	m_v3ModelEndAngles = v3End;

	m_uiModelRotationStartTime = Host.GetTime();
	m_uiModelRotationEndTime = Host.GetTime() + uiTime;
}


/*====================
  CModelPanel::MoveModel
  ====================*/
void	CModelPanel::MoveModel(const CVec3f &v3Start, const CVec3f &v3End, uint uiTime)
{
	m_v3ModelPos = m_v3ModelStartPos = v3Start;
	m_v3ModelEndPos = v3End;

	m_uiModelMoveStartTime = Host.GetTime();
	m_uiModelMoveEndTime = Host.GetTime() + uiTime;
}


/*====================
  CModelPanel::SetModelPos
  ====================*/
void	CModelPanel::SetModelPos(const CVec3f &v3ModelPos)
{
	m_v3ModelPos = v3ModelPos;
}


/*====================
  CModelPanel::SetModelAngles
  ====================*/
void	CModelPanel::SetModelAngles(const CVec3f &v3ModelAngles)
{
	m_v3ModelAngles = v3ModelAngles;
}


/*====================
  CModelPanel::SetModelScale
  ====================*/
void	CModelPanel::SetModelScale(float fModelScale)
{
	m_fModelScale = fModelScale;
}


/*--------------------
  SetModel
  --------------------*/
UI_VOID_CMD(SetModel, 1)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	static_cast<CModelPanel *>(pThis)->SetModel(vArgList[0]->Evaluate());
}


/*--------------------
  GetModel
  --------------------*/
UI_CMD(GetModel, 0)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return TSNULL;

	return static_cast<CModelPanel *>(pThis)->GetModel();
}


/*====================
  precacheSetModel
  ====================*/
CMD_PRECACHE(SetModel)
{
	if (vArgList.size() < 1)
		return false;

	K2_WITH_GAME_RESOURCE_SCOPE()
		g_ResourceManager.Register(vArgList[0], RES_MODEL);
	return true;
}


/*--------------------
  SetTeamColor
  --------------------*/
UI_VOID_CMD(SetTeamColor, 1)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	static_cast<CModelPanel *>(pThis)->SetTeamColor(GetColorFromString(vArgList[0]->Evaluate()));
}


/*--------------------
  SetAnim
  --------------------*/
UI_VOID_CMD(SetAnim, 1)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	if (vArgList.size() == 1)
		static_cast<CModelPanel *>(pThis)->SetAnim(vArgList[0]->Evaluate());
	else
		static_cast<CModelPanel *>(pThis)->SetAnim(AtoI(vArgList[0]->Evaluate()), vArgList[1]->Evaluate());
}


/*--------------------
  GetAnim
  --------------------*/
UI_CMD(GetAnim, 0)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return TSNULL;

	return static_cast<CModelPanel *>(pThis)->GetAnim();
}


/*--------------------
  SetEffect
  --------------------*/
UI_VOID_CMD(SetEffect, 1)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	if (vArgList.size() == 1)
		static_cast<CModelPanel *>(pThis)->SetEffect(vArgList[0]->Evaluate());
	else
		static_cast<CModelPanel *>(pThis)->SetEffect(AtoI(vArgList[0]->Evaluate()), vArgList[1]->Evaluate());
}


/*--------------------
  GetEffect
  --------------------*/
UI_CMD(GetEffect, 0)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return TSNULL;

	return static_cast<CModelPanel *>(pThis)->GetEffect();
}


/*--------------------
  MoveModel
  --------------------*/
UI_VOID_CMD(MoveModel, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	CModelPanel *pModelPanel(static_cast<CModelPanel*>(pThis));

	CVec3f v3StartPos(pModelPanel->GetModelPos());
	CVec3f v3EndPos(v3StartPos);
	uint uiTime(0);

	if (vArgList.size() == 1)
	{
		// Just change the position immediately
		v3EndPos = AtoV3(vArgList[0]->Evaluate());
	}
	else if (vArgList.size() == 2)
	{
		// Move from current position to the new position over time
		v3EndPos = AtoV3(vArgList[0]->Evaluate());
		uiTime = vArgList[1]->EvaluateInteger();
	}
	else if (vArgList.size() == 3)
	{
		v3StartPos = AtoV3(vArgList[0]->Evaluate());
		v3EndPos = AtoV3(vArgList[1]->Evaluate());
		uiTime = vArgList[2]->EvaluateInteger();
	}

	pModelPanel->MoveModel(v3StartPos, v3EndPos, uiTime);
}

/*--------------------
  RotateModel
  --------------------*/
UI_VOID_CMD(RotateModel, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	CModelPanel *pModelPanel(static_cast<CModelPanel*>(pThis));

	CVec3f v3StartRot(pModelPanel->GetRotation());
	CVec3f v3EndRot(v3StartRot);
	uint uiTime(0);

	if (vArgList.size() == 1)
	{
		// Just change the position immediately
		v3EndRot = AtoV3(vArgList[0]->Evaluate());
	}
	else if (vArgList.size() == 2)
	{
		// Move from current position to the new position over time
		v3EndRot = AtoV3(vArgList[0]->Evaluate());
		uiTime = vArgList[1]->EvaluateInteger();
	}
	else if (vArgList.size() == 3)
	{
		v3StartRot = AtoV3(vArgList[0]->Evaluate());
		v3EndRot = AtoV3(vArgList[1]->Evaluate());
		uiTime = vArgList[2]->EvaluateInteger();
	}

	pModelPanel->RotateModel(v3StartRot, v3EndRot, uiTime);
}


/*--------------------
  SetModelPos
  --------------------*/
UI_VOID_CMD(SetModelPos, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	CModelPanel *pModelPanel(static_cast<CModelPanel*>(pThis));

	pModelPanel->SetModelPos(AtoV3(vArgList[0]->Evaluate()));
}


/*--------------------
  SetModelAngles
  --------------------*/
UI_VOID_CMD(SetModelAngles, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	CModelPanel *pModelPanel(static_cast<CModelPanel*>(pThis));

	pModelPanel->SetModelAngles(AtoV3(vArgList[0]->Evaluate()));
}


/*--------------------
  SetModelScale
  --------------------*/
UI_VOID_CMD(SetModelScale, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	CModelPanel *pModelPanel(static_cast<CModelPanel*>(pThis));

	pModelPanel->SetModelScale(AtoF(vArgList[0]->Evaluate()));
}