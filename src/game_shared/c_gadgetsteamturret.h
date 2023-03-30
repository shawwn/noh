// (C)2007 S2 Games
// c_gadgetsteamturret.h
//
//=============================================================================
#ifndef __C_GADGETSTEAMTURRET_H__
#define __C_GADGETSTEAMTURRET_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EGadgetTurretCounters
{
    TURRET_COUNTER_SHOTS,
    TURRET_COUNTER_DAMAGE
};
//=============================================================================

//=============================================================================
// CGadgetSteamTurret
//=============================================================================
class CGadgetSteamTurret : public IGadgetEntity
{
private:
    START_ENTITY_CONFIG(IGadgetEntity)
        DECLARE_ENTITY_CVAR(float, Range)
        DECLARE_ENTITY_CVAR(uint, AttackTime)
        DECLARE_ENTITY_CVAR(CVec3f, AttackOffset)
        DECLARE_ENTITY_CVAR(float, SpreadX)
        DECLARE_ENTITY_CVAR(float, SpreadY)
        DECLARE_ENTITY_CVAR(float, MinDamage)
        DECLARE_ENTITY_CVAR(float, MaxDamage)
        DECLARE_ENTITY_CVAR(float, DamageRadius)
        DECLARE_ENTITY_CVAR(float, PierceUnit)
        DECLARE_ENTITY_CVAR(float, PierceHellbourne)
        DECLARE_ENTITY_CVAR(float, PierceSiege)
        DECLARE_ENTITY_CVAR(float, PierceBuilding)
        DECLARE_ENTITY_CVAR(tstring, FiringAnimName)
        DECLARE_ENTITY_CVAR(tstring, TraceEffectPath)
        DECLARE_ENTITY_CVAR(tstring, AttackEffectPath)
        DECLARE_ENTITY_CVAR(tstring, ImpactTerrainEffectPath)
        DECLARE_ENTITY_CVAR(tstring, ImpactEffectPath)
        DECLARE_ENTITY_CVAR(uint, BurstCount)
        DECLARE_ENTITY_CVAR(uint, BurstTime)
    END_ENTITY_CONFIG
    
    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Gadget, SteamTurret)

    uint    m_uiTarget;
    uint    m_uiNextAttackTime;
    uint    m_uiCurrentBurstCount;

    float   m_fDamageCounter;

    void    FireTrace(const CVec3f &v3Start, const CVec3f &v3End);

public:
    ~CGadgetSteamTurret()   {}
    CGadgetSteamTurret() :
    IGadgetEntity(GetEntityConfig()),
    m_pEntityConfig(GetEntityConfig()),
    m_uiTarget(INVALID_INDEX),
    m_uiNextAttackTime(0),
    m_uiCurrentBurstCount(0),
    m_fDamageCounter(0.0f)
    {
        m_vCounterLabels.push_back(_T("Shots fired"));
        m_vCounterLabels.push_back(_T("Damage"));
        m_auiCounter[0] = 0;
        m_auiCounter[1] = 0;
    }

    void    Baseline();

    bool    ValidateTarget(uint uiTargetIndex);

    bool    ServerFrame();
};
//=============================================================================

#endif //__C_GADGETSTEAMTURRET_H__
