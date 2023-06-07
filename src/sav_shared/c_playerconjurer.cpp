// (C)2006 S2 Games
// c_playerconjurer.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_playerconjurer.h"

#include "../k2/c_skeleton.h"
#include "../k2/c_camera.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Player, Conjurer)
//=============================================================================

/*====================
  CPlayerConjurer::CEntityConfig::CEntityConfig
  ====================*/
CPlayerConjurer::CEntityConfig::CEntityConfig(const tstring &sName) :
IPlayerEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(BuildModeAngle, -70.0f),
INIT_ENTITY_CVAR(BuildModeHeight, 600.0f),
INIT_ENTITY_CVAR(BuildModeOffset, 300.0f),
INIT_ENTITY_CVAR(BuildModeLerpTime, 500)
{
}


/*====================
  CPlayerConjurer::CPlayerConjurer
  ====================*/
CPlayerConjurer::CPlayerConjurer() :
IPlayerEntity(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),

m_bWasBuilding(false),
m_uiCameraChangeStartTime(0)
{
}


/*====================
  CPlayerConjurer::SetupBuildModeCamera
  ====================*/
void    CPlayerConjurer::SetupBuildModeCamera(CCamera &camera, CVec3f v3InputAngles, float fLerp)
{
    if (HasNetFlags(ENT_NET_FLAG_BUILD_MODE) || fLerp > 0.0f )
        camera.RemoveFlags(CAM_FIRST_PERSON);

    // Interpolate view angles
    // Since the position is calculated based on the angle, calculating the
    // angle first gives a nice swoop effect to the transition
    CVec3f v3TargetAngles(m_pEntityConfig->GetBuildModeAngle(), 0.0f, v3InputAngles[YAW]);
    camera.SetViewAxis(M_QuatToAxis(M_LerpQuat(fLerp, M_AxisToQuat(camera.GetViewAxis()), M_EulerToQuat(v3TargetAngles))));
    
    // Interpolate position
    CVec3f v3Target(m_v3Position);
    CAxis axis(v3InputAngles);
    CVec3f v3Dir(axis.Forward().xy(), 0.0f);
    v3Dir.Normalize();
    v3Target += v3Dir * m_pEntityConfig->GetBuildModeOffset();
    v3Target.z = Game.GetTerrainHeight(v3Target.x, v3Target.y);

    CVec3f v3TargetPosition(M_PointOnLine(v3Target, camera.GetViewAxis(FORWARD), -m_pEntityConfig->GetBuildModeHeight()));
    camera.SetOrigin(LERP(fLerp, camera.GetOrigin(), v3TargetPosition));
}


/*====================
  CPlayerConjurer::SetupCamera
  ====================*/
void    CPlayerConjurer::SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles)
{
    // Check for a transition
    if (HasNetFlags(ENT_NET_FLAG_BUILD_MODE) != m_bWasBuilding)
        m_uiCameraChangeStartTime = Host.GetTime();
    m_bWasBuilding = HasNetFlags(ENT_NET_FLAG_BUILD_MODE);

    float fLerp((Host.GetTime() - m_uiCameraChangeStartTime) / float(m_pEntityConfig->GetBuildModeLerpTime()));
    fLerp = CLAMP(fLerp, 0.0f, 1.0f);
    if (!HasNetFlags(ENT_NET_FLAG_BUILD_MODE))
        fLerp = 1.0f - fLerp;

    IPlayerEntity::SetupCamera(camera, v3InputPosition, v3InputAngles);
    SetupBuildModeCamera(camera, v3InputAngles, fLerp);
}
