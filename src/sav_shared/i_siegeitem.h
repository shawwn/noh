// (C)2006 S2 Games
// i_siegeitem.h
//
//=============================================================================
#ifndef __I_SIEGEITEM_H__
#define __I_SIEGEITEM_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_inventoryitem.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
extern CCvarf   p_gravity;
//=============================================================================

//=============================================================================
// ISiegeItem
//=============================================================================
class ISiegeItem : public IInventoryItem
{
protected:
    // Cvar settings
    START_ENTITY_CONFIG(IInventoryItem)
        DECLARE_ENTITY_CVAR(float, MinDamage)
        DECLARE_ENTITY_CVAR(float, MaxDamage)
        DECLARE_ENTITY_CVAR(float, PierceUnit)
        DECLARE_ENTITY_CVAR(float, PierceHellbourne)
        DECLARE_ENTITY_CVAR(float, PierceSiege)
        DECLARE_ENTITY_CVAR(float, PierceBuilding)
        DECLARE_ENTITY_CVAR(float, DamageRadius)
        DECLARE_ENTITY_CVAR(uint, SpinupTime)
        DECLARE_ENTITY_CVAR(uint, AttackTime)
        DECLARE_ENTITY_CVAR(uint, AttackDelay)
        DECLARE_ENTITY_CVAR(tstring, ProjectileName)
        DECLARE_ENTITY_CVAR(bool, AreaOfEffect)
        DECLARE_ENTITY_CVAR(tstring, TargetMaterialPath)
        DECLARE_ENTITY_CVAR(float, TargetRadius)
        DECLARE_ENTITY_CVAR(float, MaxRange)
        DECLARE_ENTITY_CVAR(float, MinRange)
        DECLARE_ENTITY_CVAR(tstring, AttackAnimName)
        DECLARE_ENTITY_CVAR(CVec3f, AttackOffset)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    CVec3f          m_v3DelayedTarget;

public:
    virtual ~ISiegeItem()   {}
    ISiegeItem(CEntityConfig *pConfig);

    bool            IsSiege() const                 { return true; }

    virtual void    Selected();

    virtual bool    Spinup(int iButtonStatus);
    virtual bool    Fire(int iButtonStatus, const CVec3f &v3CameraAngles);
    virtual bool    Delay(int iButtonStatus, const CVec3f &v3CameraAngles);
    virtual bool    CoolDown();

    virtual bool    ActivatePrimary(int iButtonStatus);
    virtual void    FinishedAction(int iAction);

    bool            IsAreaofEffect() const          { return m_pEntityConfig->GetAreaOfEffect(); }
    float           GetMinDamage() const            { return m_pEntityConfig->GetMinDamage(); }
    float           GetMaxDamage() const            { return m_pEntityConfig->GetMaxDamage(); }
    tstring         GetSpeed() const                { if (m_pEntityConfig->GetAttackTime() < 300) return _T("Fast"); else if (m_pEntityConfig->GetAttackTime() < 600) return _T("Medium"); else return _T("Slow"); }
    tstring         GetTargetMaterialPath() const   { return m_pEntityConfig->GetTargetMaterialPath(); }
    float           GetTargetRadius() const         { return m_pEntityConfig->GetTargetRadius(); }
    float           GetMaxRange() const             { return m_pEntityConfig->GetMaxRange(); }
    float           GetMinRange() const             { return m_pEntityConfig->GetMinRange(); }

    GAME_SHARED_API CVec3f  GetTargetLocation() const;

    GAME_SHARED_API bool    IsValidTarget(IGameEntity *pEntity);

    static void     ClientPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__I_SIEGEITEM_H__
