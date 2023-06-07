// (C)2006 S2 Games
// c_playerengineer.h
//
//=============================================================================
#ifndef __CPLAYERENGINEER_H__
#define __CPLAYERENGINEER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerEngineer
//=============================================================================
class CPlayerEngineer : public IPlayerEntity
{
private:
    START_ENTITY_CONFIG(IPlayerEntity)
        DECLARE_ENTITY_CVAR(float, BuildModeAngle)
        DECLARE_ENTITY_CVAR(float, BuildModeHeight)
        DECLARE_ENTITY_CVAR(float, BuildModeOffset)
        DECLARE_ENTITY_CVAR(uint, BuildModeLerpTime)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Player, Engineer);

    bool    m_bWasBuilding;
    uint    m_uiCameraChangeStartTime;

public:
    ~CPlayerEngineer()  {}
    CPlayerEngineer();

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }

    void    SetupBuildModeCamera(CCamera &camera, CVec3f v3InputAngles, float fLerp);
    void    SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles);
};
//=============================================================================

#endif //__CPLAYERENGINEER_H__
