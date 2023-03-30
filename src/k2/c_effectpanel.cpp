// (C)2009 S2 Games
// c_effectpanel.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_effectpanel.h"

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
//=============================================================================

extern CCvar<CVec3f, float>	scene_terrainSunColor;
extern CCvar<CVec3f, float>	scene_terrainAmbientColor;
extern CCvar<CVec3f, float>	scene_entitySunColor;
extern CCvar<CVec3f, float>	scene_entityAmbientColor;
extern CCvar<float>			scene_sunAltitude;
extern CCvar<float>			scene_sunAzimuth;


/*====================
  CEffectPanel::~CEffectPanel
  ====================*/
CEffectPanel::~CEffectPanel()
{
}


/*====================
  CEffectPanel::CEffectPanel
  ====================*/
CEffectPanel::CEffectPanel(CInterface* pInterface, IWidget* pParent, const CWidgetStyle& style) :
IWidget(pInterface, pParent, WIDGET_EFFECTPANEL, style, false),
m_v3SunColor(style.GetPropertyVec3(_T("suncolor"), CVec3f(0.9f, 0.9f, 0.9f))),
m_v3AmbientColor(style.GetPropertyVec3(_T("ambientcolor"), CVec3f(0.25f, 0.25f, 0.25f))),
m_fSunAltitude(style.GetPropertyFloat(_T("sunaltitude"), 30.0f)),
m_fSunAzimuth(style.GetPropertyFloat(_T("sunazimuth"), 115.0f)),
m_v3FogColor(style.GetPropertyVec3(_T("fogcolor"), CVec3f(0.75f, 0.75f, 0.75f))),
m_fFogNear(style.GetPropertyFloat(_T("fognear"), 0.0f)),
m_fFogFar(style.GetPropertyFloat(_T("fogfar"), 1000.0f)),
m_fFogScale(style.GetPropertyFloat(_T("fogscale"), 1.0f)),
m_fFogDensity(style.GetPropertyFloat(_T("fogdensity"), 0.0005f)),
m_bFovY(false),
m_fCameraNear(style.GetPropertyFloat(_T("cameranear"), -1000.0f)),
m_fCameraFar(style.GetPropertyFloat(_T("camerafar"), 1000.0f))
{
	m_uiFlags |= WFLAG_NO_DRAW;

	m_Camera.DefaultCamera(m_recArea.GetWidth(), m_recArea.GetHeight());

	m_Camera.SetAngles(style.GetPropertyVec3(_T("cameraangles"), CVec3f(90.0f, 0.0f, 0.0f)));

	m_Camera.SetWidth(m_recArea.GetWidth());
	m_Camera.SetHeight(m_recArea.GetHeight());

	m_Camera.SetZNear(m_fCameraNear);
	m_Camera.SetZFar(m_fCameraFar);

	int iFlags(CAM_NO_WORLD);

	iFlags |= CAM_ORTHO;
	iFlags |= CAM_NO_SHADOWS;
	iFlags |= CAM_NO_FOG;
	iFlags |= CAM_NO_DEPTH_CLEAR;
	iFlags |= CAM_DEPTH_COMPRESS;
	iFlags |= CAM_NO_POST;
	iFlags |= CAM_NO_REFLECTIONS;
	iFlags |= CAM_NO_SCENE_BUFFER;
	iFlags |= CAM_NO_CULL;

	m_Camera.SetFlags(iFlags);

	m_v2SceneSize.x = GetSizeFromString(style.GetProperty(_T("camerawidth"), _T("100%")), GetWidth(), GetHeight());
	m_v2SceneSize.y = GetSizeFromString(style.GetProperty(_T("cameraheight"), _T("100%")), GetHeight(), GetWidth());

	m_Camera.SetOrthoWidth(m_v2SceneSize.x);
	m_Camera.SetOrthoHeight(m_v2SceneSize.y);

	m_Camera.SetOrigin(CVec3f(m_Camera.GetOrthoWidth() / 2.0f, m_Camera.GetOrthoHeight() / 2.0f, 0.0f));

	if (IsAbsoluteVisible())
		DO_EVENT(WEVENT_SHOW)
}


/*====================
  CEffectPanel::UpdateEffect
  ====================*/
void	CEffectPanel::UpdateEffect(CEffectThread *&pEffectThread, const CVec3f &v3EffectPos, const CVec3f &v3EffectAngles, float fEffectScale)
{
	if (!pEffectThread)
		return;

	pEffectThread->SetActive(true);

	// Update entity attachment information
	pEffectThread->SetSourcePos(v3EffectPos);
	pEffectThread->SetSourceAxis(CAxis(v3EffectAngles));
	pEffectThread->SetSourceScale(fEffectScale);

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
  CEffectPanel::RenderWidget
  ====================*/
void	CEffectPanel::RenderWidget(const CVec2f &vOrigin, float fFade)
{
	if (!HasFlags(WFLAG_VISIBLE))
		return;

	IWidget::RenderWidget(vOrigin, fFade);

	m_Camera.SetX(vOrigin.x + m_recArea.left);
	m_Camera.SetY(vOrigin.y + m_recArea.top);

	bool bRender(false);

	SceneManager.Clear();

	for (map<int, SEffect>::iterator it(m_mapEffects.begin()); it != m_mapEffects.end();)
	{
		SEffect &cEffect(it->second);

		// Update attached effect
		UpdateEffect(cEffect.pEffectThread, cEffect.v3EffectPos, cEffect.v3EffectAngles, cEffect.fEffectScale);

		if (cEffect.pEffectThread == NULL)
			STL_ERASE(m_mapEffects, it);
		else
		{
			bRender = true;
			++it;
		}
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
  CEffectPanel::StartEffect
  ====================*/
void	CEffectPanel::StartEffect(int iChannel, const tstring &sEffect, const CVec3f &v3EffectPos, const CVec3f &v3Color)
{
	// Search from an unused effect slot
	if (iChannel == -1)
	{
		for (int i(-2); i > -32; --i)
		{
			map<int, SEffect>::iterator itFind(m_mapEffects.find(i));

			if (itFind == m_mapEffects.end())
			{
				iChannel = i;
				break;
			}
		}

		if (iChannel == -1)
			return;
	}

	map<int, SEffect>::iterator itFind(m_mapEffects.find(iChannel));
	if (itFind != m_mapEffects.end())
	{
		SAFE_DELETE(itFind->second.pEffectThread);
		m_mapEffects.erase(itFind);
	}

	if (sEffect.empty())
		return;

	SEffect &cEffect(m_mapEffects[iChannel]);

	SAFE_DELETE(cEffect.pEffectThread);

	ResHandle hEffect(g_ResourceManager.Register(sEffect, RES_EFFECT));

	CEffect	*pEffect(g_ResourceManager.GetEffect(hEffect));
	if (pEffect != NULL)
	{
		cEffect.pEffectThread = pEffect->SpawnThread(Host.GetTime());
		cEffect.v3EffectPos = v3EffectPos;

		cEffect.v3EffectPos.x *= m_v2SceneSize.x;
		cEffect.v3EffectPos.y *= m_v2SceneSize.y;

		if (cEffect.pEffectThread != NULL)
		{
			cEffect.pEffectThread->SetCamera(&m_Camera);
			cEffect.pEffectThread->SetWorld(NULL);
			cEffect.pEffectThread->SetTargetSkeleton(NULL);
			cEffect.pEffectThread->SetTargetModel(NULL);
			cEffect.pEffectThread->SetColor(v3Color);
		}
	}
}


/*====================
  CEffectPanel::RecalculateSize
  ====================*/
void	CEffectPanel::RecalculateSize()
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
  CEffectPanel::MouseDown
  ====================*/
void	CEffectPanel::MouseDown(EButton button, const CVec2f &v2CursorPos)
{
	if (button == BUTTON_MOUSEL)
	{
		DO_EVENT(WEVENT_MOUSELDOWN)
		DO_EVENT(WEVENT_CLICK)
		
	}
	else if (button == BUTTON_MOUSER)
	{
		DO_EVENT(WEVENT_MOUSERDOWN)
		DO_EVENT(WEVENT_RIGHTCLICK)
	}
}


/*====================
  CEffectPanel::MouseUp
  ====================*/
void	CEffectPanel::MouseUp(EButton button, const CVec2f &v2CursorPos)
{
	if (button == BUTTON_MOUSEL)
	{
		DO_EVENT(WEVENT_MOUSELUP)
	}
	else if (button == BUTTON_MOUSER)
	{
		DO_EVENT(WEVENT_MOUSERUP)
	}
}


#if 0
/*--------------------
  SetModel
  --------------------*/
UI_VOID_CMD(SetModel, 1)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	static_cast<CEffectPanel *>(pThis)->SetModel(vArgList[0]->Evaluate());
}


/*====================
  precacheSetModel
  ====================*/
CMD_PRECACHE(SetModel)
{
	if (vArgList.size() < 1)
		return false;

	g_ResourceManager.Register(vArgList[0], RES_MODEL);
	return true;
}


/*--------------------
  SetAnim
  --------------------*/
UI_VOID_CMD(SetAnim, 1)
{
	if (!pThis || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	if (vArgList.size() == 1)
		static_cast<CEffectPanel *>(pThis)->SetAnim(vArgList[0]->Evaluate());
	else
		static_cast<CEffectPanel *>(pThis)->SetAnim(AtoI(vArgList[0]->Evaluate()), vArgList[1]->Evaluate());
}





/*--------------------
  MoveModel
  --------------------*/
UI_VOID_CMD(MoveModel, 1)
{
	if (pThis == NULL || pThis->GetType() != WIDGET_MODELPANEL)
		return;

	CEffectPanel *pModelPanel(static_cast<CEffectPanel*>(pThis));

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
#endif


/*--------------------
  StartEffect
  --------------------*/
UI_VOID_CMD(StartEffect, 3)
{
	if (!pThis || pThis->GetType() != WIDGET_EFFECTPANEL)
		return;

	CVec3f v3Pos(V3_ZERO);

	v3Pos.x = AtoF(vArgList[1]->Evaluate());
	v3Pos.y = AtoF(vArgList[2]->Evaluate());

	CVec3f v3Color(AtoV3(vArgList[3]->Evaluate()));

	static_cast<CEffectPanel *>(pThis)->StartEffect(-1, vArgList[0]->Evaluate(), v3Pos, v3Color);
}
