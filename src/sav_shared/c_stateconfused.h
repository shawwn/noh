// (C)2007 S2 Games
// c_sateconfused.h
//
//=============================================================================
#ifndef __C_STATECONFUSED_H__
#define __C_STATECONFUSED_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_entitystate.h"
//=============================================================================

//=============================================================================
// CStateConfused
//=============================================================================
class CStateConfused : public IEntityState
{
private:
    START_ENTITY_CONFIG(IEntityState)
        DECLARE_ENTITY_CVAR(float, SpeedMult)
        DECLARE_ENTITY_CVAR(float, FOVShift)
        DECLARE_ENTITY_CVAR(float, ColorCycleStrength)
        DECLARE_ENTITY_CVAR(float, ColorThreshold)
        DECLARE_ENTITY_CVAR(float, CameraDriftVariance)
        DECLARE_ENTITY_CVAR(float, CameraYawThreshold)
        DECLARE_ENTITY_CVAR(float, CameraPitchThreshold)
        DECLARE_ENTITY_CVAR(float, CameraRollThreshold)
        DECLARE_ENTITY_CVAR(float, CameraDriftStrength)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(State, Confused);

    ICvar*  m_pTerrainColor;
    ICvar*  m_pEntityColor;
    ICvar*  m_pSkyColor;

    CVec3f  m_v3CameraDrift;
    CVec3f  m_v3CameraOffset;

    CVec3f  m_v3TerrainColorBase;
    CVec3f  m_v3EntityColorBase;
    CVec4f  m_v4SkyColorBase;
    
    CVec3f  m_v3Color;
    CVec3f  m_v3ColorDelta;
    
    uint    m_uiLastCameraUpdate;

public:
    ~CStateConfused();
    CStateConfused();

    void    ModifyCamera(CCamera &camera);
};
//=============================================================================

#endif //__C_STATECONFUSED_H__
