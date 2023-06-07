// (C)2006 S2 Games
// c_playerconjurer.h
//
//=============================================================================
#ifndef __C_PLAYERCONJURER_H__
#define __C_PLAYERCONJURER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_playerentity.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// CPlayerConjurer
//=============================================================================
class CPlayerConjurer : public IPlayerEntity
{
private:
    START_ENTITY_CONFIG(IPlayerEntity)
        DECLARE_ENTITY_CVAR(float, BuildModeAngle)
        DECLARE_ENTITY_CVAR(float, BuildModeHeight)
        DECLARE_ENTITY_CVAR(float, BuildModeOffset)
        DECLARE_ENTITY_CVAR(uint, BuildModeLerpTime)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Player, Conjurer);

    bool    m_bWasBuilding;
    uint    m_uiCameraChangeStartTime;

public:
    ~CPlayerConjurer()  {}
    CPlayerConjurer();

    void    Move(const CClientSnapshot &snapshot)   { MoveWalk(snapshot); }

    void    SetupBuildModeCamera(CCamera &camera, CVec3f v3InputAngles, float fLerp);
    void    SetupCamera(CCamera &camera, const CVec3f &v3InputPosition, const CVec3f &v3InputAngles);
};
//=============================================================================

#endif //__C_PLAYERCONJURER_H__
