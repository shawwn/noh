// (C)2006 S2 Games
// c_stateconfused.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stateconfused.h"

#include "../k2/c_camera.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Confused);
//=============================================================================

/*====================
  CStateConfused::CEntityConfig::CEntityConfig
  ====================*/
CStateConfused::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(SpeedMult, 1.0f),
INIT_ENTITY_CVAR(FOVShift, 70.0f),
INIT_ENTITY_CVAR(ColorCycleStrength, 1.0f),
INIT_ENTITY_CVAR(ColorThreshold, 2.0f),
INIT_ENTITY_CVAR(CameraDriftVariance, 7.0f),
INIT_ENTITY_CVAR(CameraYawThreshold, 20.0f),
INIT_ENTITY_CVAR(CameraPitchThreshold, 20.0f),
INIT_ENTITY_CVAR(CameraRollThreshold, 20.0f),
INIT_ENTITY_CVAR(CameraDriftStrength, 10.0f)
{
}


/*====================
  CStateConfused::~CStateConfused
  ====================*/
CStateConfused::~CStateConfused()
{
    if (m_pTerrainColor != NULL)
        m_pTerrainColor->Set(XtoA(m_v3TerrainColorBase));
    if (m_pEntityColor != NULL)
        m_pEntityColor->Set(XtoA(m_v3EntityColorBase));
    if (m_pSkyColor != NULL)
        m_pSkyColor->Set(XtoA(m_v4SkyColorBase));
}


/*====================
  CStateConfused::CStateConfused
  ====================*/
CStateConfused::CStateConfused() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_pTerrainColor(ConsoleRegistry.GetCvar(_T("scene_terrainAmbientColor"))),
m_pEntityColor(ConsoleRegistry.GetCvar(_T("scene_entityAmbientColor"))),
m_pSkyColor(ConsoleRegistry.GetCvar(_T("scene_skyColor"))),

m_v3CameraDrift(M_RandomDirection() * 5.0f),
m_v3CameraOffset(V_ZERO),

m_v3TerrainColorBase(m_pTerrainColor != NULL ? m_pTerrainColor->GetVec3() : V_ZERO),
m_v3EntityColorBase(m_pEntityColor != NULL ? m_pEntityColor->GetVec3() : V_ZERO),
m_v4SkyColorBase(m_pSkyColor != NULL ? m_pSkyColor->GetVec4() : WHITE),

m_v3Color(M_RandomDirection()),
m_v3ColorDelta(M_RandomDirection()),

m_uiLastCameraUpdate(Host.GetTime())
{
    m_modSpeed.SetMult(m_pEntityConfig->GetSpeedMult());
}


/*====================
  CStateConfused::ModifyCamera
  ====================*/
void    CStateConfused::ModifyCamera(CCamera &camera)
{
    float fTime(MsToSec(Host.GetTime() - m_uiLastCameraUpdate));
    float fAmount((Game.GetGameTime() - m_uiStartTime) / float(m_uiDuration));

    // Camera angle motion
    float fDriftMult(m_pEntityConfig->GetCameraDriftVariance());
    
    float fYawThreshold(m_pEntityConfig->GetCameraYawThreshold());
    float fMaxYawDrift((m_v3CameraOffset[YAW] > 0.0f) ? 1.0f - m_v3CameraOffset[YAW] / fYawThreshold : 1.0f);
    float fMinYawDrift((m_v3CameraOffset[YAW] < 0.0f) ? -(1.0f - m_v3CameraOffset[YAW] / -fYawThreshold) : -1.0f);
    m_v3CameraDrift[YAW] += M_Randnum(fMinYawDrift, fMaxYawDrift) * fTime * fDriftMult;

    float fRollThreshold(m_pEntityConfig->GetCameraRollThreshold());
    float fMaxRollDrift((m_v3CameraOffset[ROLL] > 0.0f) ? 1.0f - m_v3CameraOffset[ROLL] / fRollThreshold : 1.0f);
    float fMinRollDrift((m_v3CameraOffset[ROLL] < 0.0f) ? -(1.0f - m_v3CameraOffset[ROLL] / -fRollThreshold) : -1.0f);
    m_v3CameraDrift[ROLL] += M_Randnum(fMinRollDrift, fMaxRollDrift) * fTime * fDriftMult;

    float fPitchThreshold(m_pEntityConfig->GetCameraPitchThreshold());
    float fMaxPitchDrift((m_v3CameraOffset[PITCH] > 0.0f) ? 1.0f - m_v3CameraOffset[PITCH] / fPitchThreshold : 1.0f);
    float fMinPitchDrift((m_v3CameraOffset[PITCH] < 0.0f) ? -(1.0f - m_v3CameraOffset[PITCH] / -fPitchThreshold) : -1.0f);
    m_v3CameraDrift[PITCH] += M_Randnum(fMinPitchDrift, fMaxPitchDrift) * fTime * fDriftMult;

    m_v3CameraOffset += m_v3CameraDrift * fTime * m_pEntityConfig->GetCameraDriftStrength();

    CAxis axis(camera.GetViewAxis());
    camera.SetAngles(M_GetAnglesFromForwardVec(axis.Forward()) + m_v3CameraOffset * (1.0f - fAmount));

    // FOV
    camera.SetFovXCalc(camera.GetFovX() + m_pEntityConfig->GetFOVShift() * sqrt(1.0f - fAmount));

    // Colors
    m_v3Color += m_v3ColorDelta * fTime * m_pEntityConfig->GetColorCycleStrength();
    float fColorThreshold(m_pEntityConfig->GetColorThreshold());
    if ((m_v3Color.x >= fColorThreshold && m_v3ColorDelta.x > 0.0f) ||
        (m_v3Color.x <= 0.0f && m_v3ColorDelta.x < 0.0f))
        m_v3ColorDelta.x = -m_v3ColorDelta.x;
    if ((m_v3Color.y >= fColorThreshold && m_v3ColorDelta.y > 0.0f) ||
        (m_v3Color.y <= 0.0f && m_v3ColorDelta.y < 0.0f))
        m_v3ColorDelta.y = -m_v3ColorDelta.y;
    if ((m_v3Color.z >= fColorThreshold && m_v3ColorDelta.z > 0.0f) ||
        (m_v3Color.z <= 0.0f && m_v3ColorDelta.z < 0.0f))
        m_v3ColorDelta.z = -m_v3ColorDelta.z;

    if (m_pTerrainColor != NULL)
        m_pTerrainColor->Set(XtoA(LERP((fAmount), m_v3Color, m_v3TerrainColorBase)));
    if (m_pEntityColor != NULL)
        m_pEntityColor->Set(XtoA(LERP((fAmount), m_v3Color, m_v3EntityColorBase)));
    if (m_pSkyColor != NULL)
        m_pSkyColor->Set(XtoA(CVec4f(LERP((fAmount), m_v3Color, m_v4SkyColorBase.xyz()), m_v4SkyColorBase[A])));

    m_uiLastCameraUpdate = Host.GetTime();
}
