// (C)2007 S2 Games
// c_gadgetvenus.h
//
//=============================================================================
#ifndef __C_GADGETVENUS_H__
#define __C_GADGETVENUS_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gadgetentity.h"
//=============================================================================

//=============================================================================
// CGadgetVenus
//=============================================================================
class CGadgetVenus : public IGadgetEntity
{
private:
    START_ENTITY_CONFIG(IGadgetEntity)
        DECLARE_ENTITY_CVAR(float, Range)
        DECLARE_ENTITY_CVAR(uint, AttackTime)
        DECLARE_ENTITY_CVAR(CVec3f, AttackOffset)
        DECLARE_ENTITY_CVAR(tstring, ProjectileName)
        DECLARE_ENTITY_CVAR(float, SpreadX)
        DECLARE_ENTITY_CVAR(float, SpreadY)
        DECLARE_ENTITY_CVAR(float, MinDamage)
        DECLARE_ENTITY_CVAR(float, MaxDamage)
        DECLARE_ENTITY_CVAR(float, DamageRadius)
        DECLARE_ENTITY_CVAR(float, PierceUnit)
        DECLARE_ENTITY_CVAR(float, PierceHellbourne)
        DECLARE_ENTITY_CVAR(float, PierceSiege)
        DECLARE_ENTITY_CVAR(float, PierceBuilding)
    END_ENTITY_CONFIG
    
    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Gadget, Venus)

    uint    m_uiTarget;
    uint    m_uiNextAttackTime;

public:
    ~CGadgetVenus() {}
    CGadgetVenus() :
    IGadgetEntity(GetEntityConfig()),
    m_pEntityConfig(GetEntityConfig()),
    m_uiTarget(INVALID_INDEX),
    m_uiNextAttackTime(0)
    {}

    bool    ValidateTarget(uint uiTargetIndex);

    bool    ServerFrame();
};
//=============================================================================

#endif //__C_GADGETVENUS_H__
