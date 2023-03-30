// (C)2007 S2 Games
// c_entitychest.h
//
//=============================================================================
#ifndef __C_ENTITYCHEST_H__
#define __C_ENTITYCHEST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint  TRACE_CHEST_SPAWN(SURF_MODEL | SURF_PROJECTILE | SURF_FOLIAGE | SURF_NOT_SOLID | SURF_SHIELD | SURF_DEAD | SURF_CORPSE | SURF_TERRAIN | SURF_BLOCKER | SURF_NOBLOCK | SURF_FLYING | SURF_UNITWALKING | SURF_UNIT);
//=============================================================================

//=============================================================================
// CEntityChest
//=============================================================================
class CEntityChest : public IUnitEntity
{
    DECLARE_ENTITY_DESC

private:
    static ResHandle    s_hModel;

protected:
    START_ENTITY_CONFIG(IUnitEntity)
        DECLARE_ENTITY_CVAR(tstring, DisplayName)
        DECLARE_ENTITY_CVAR(tstring, ModelPath)
        DECLARE_ENTITY_CVAR(float, PreGlobalScale)
        DECLARE_ENTITY_CVAR(float, ModelScale)
        DECLARE_ENTITY_CVAR(float, SelectionRadius)
        DECLARE_ENTITY_CVAR(float, BoundsRadius)
        DECLARE_ENTITY_CVAR(float, BoundsHeight)
        DECLARE_ENTITY_CVAR(float, MaxHealth)
        DECLARE_ENTITY_CVAR(uint, CorpseTime)
        DECLARE_ENTITY_CVAR(uint, DeathTime)
        DECLARE_ENTITY_CVAR(uint, CorpseFadeTime)
        DECLARE_ENTITY_CVAR(tstring, DeathAnim)
        DECLARE_ENTITY_CVAR(uint, DeathNumAnims)
        DECLARE_ENTITY_CVAR(tstring, AltDeathAnim)
        DECLARE_ENTITY_CVAR(uint, AltDeathNumAnims)
        DECLARE_ENTITY_CVAR(bool, NoBlockNeutralSpawn)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Entity, Chest);

    mutable tstring m_sDisplayName;

public:
    ~CEntityChest();
    CEntityChest();

    SUB_ENTITY_ACCESSOR(CEntityChest, Chest)
    
    virtual void    Baseline();
    virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    virtual void    Spawn();
    virtual void    Touch(IGameEntity *pActivator, int iIssuedClientNumber = -1);
    virtual void    Die(IUnitEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

    const tstring&  GetDisplayName() const;

    bool            GetCanRotate() const        { return false; }
    bool            GetIsSelectable() const     { return true; }
    bool            GetIsMobile() const         { return false; }
    bool            GetCanAttack() const        { return false; }
    bool            GetPassiveInventory() const { return true; }

    virtual bool    ServerFrameCleanup();

    ENTITY_CVAR_ACCESSOR(float, PreGlobalScale)
    ENTITY_CVAR_ACCESSOR(float, ModelScale)
    ENTITY_CVAR_ACCESSOR(float, SelectionRadius)
    ENTITY_CVAR_ACCESSOR(float, BoundsRadius)
    ENTITY_CVAR_ACCESSOR(float, BoundsHeight)
    uint        GetImmunityType() const;
    ENTITY_CVAR_ACCESSOR(float, MaxHealth)

    ENTITY_CVAR_ACCESSOR(const tstring&, ModelPath)
    ENTITY_CVAR_ACCESSOR(uint, CorpseTime)
    ENTITY_CVAR_ACCESSOR(uint, DeathTime)
    ENTITY_CVAR_ACCESSOR(uint, CorpseFadeTime)

    ENTITY_CVAR_ACCESSOR(const tstring&, DeathAnim)
    ENTITY_CVAR_ACCESSOR(uint, DeathNumAnims)

    ENTITY_CVAR_ACCESSOR(const tstring&, AltDeathAnim)
    ENTITY_CVAR_ACCESSOR(uint, AltDeathNumAnims)

    ENTITY_CVAR_ACCESSOR(bool, NoBlockNeutralSpawn)

    ResHandle       GetModel() const            { return s_hModel; }

    static void     ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
    static void     ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
};
//=============================================================================

#endif //__C_ENTITYSOUL_H__
