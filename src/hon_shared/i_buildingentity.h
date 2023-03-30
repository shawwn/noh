// (C)2008 S2 Games
// i_buildingentity.h
//
//=============================================================================
#ifndef __I_BUILDINGENTITY_H__
#define __I_BUILDINGENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_buildingdefinition.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint NUM_BUILDING_TARGETS(5);
//=============================================================================

//=============================================================================
// IBuildingEntity
//=============================================================================
class IBuildingEntity : public IUnitEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CBuildingDefinition TDefinition;

protected:
    uint            m_uiLastGameTime;
    uint            m_uiLastDamageTime;
    uint            m_uiLastAttackAnnounce;
    bool            m_bLowHealthEffectActive;

    tstring                 m_asTargets[NUM_BUILDING_TARGETS];
    uint                    m_auiTargetUIDs[NUM_BUILDING_TARGETS];
    static ResHandle        s_hMinimapDefendTargetIcon;
    static ResHandle        s_hMinimapAttackTargetIcon;

public:
    virtual ~IBuildingEntity();
    IBuildingEntity();

    SUB_ENTITY_ACCESSOR(IBuildingEntity, Building)

    virtual void            SetTeam(uint uiTeam);

    virtual void            Baseline();
    virtual void            GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool            ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    virtual CSkeleton*      AllocateSkeleton();
    virtual void            UpdateSkeleton(bool bPose);

    virtual void            ApplyWorldEntity(const CWorldEntity &ent);

    virtual void            Spawn();
    virtual void            Die(IUnitEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);

    virtual void            GameStart();

    virtual void            DamageNotification(uint uiIndex, uint uiAttacker, float fDamage)    {}

    virtual bool            ServerFrameSetup();
    virtual bool            ServerFrameThink();
    virtual bool            ServerFrameMovement();

    virtual void            Damage(CDamageEvent &damage);

    virtual bool            AddToScene(const CVec4f &v4Color, int iFlags);

    virtual bool            IsVisibleOnMap(CPlayer *pLocalPlayer) const     { return GetStatus() == ENTITY_STATUS_ACTIVE && GetDrawOnMap(); }

    virtual uint            GetLinkFlags();
    virtual void            Link();
    virtual void            Unlink();

    ENTITY_DEFINITION_RESOURCE_ACCESSOR(DestroyedSound)
    ENTITY_DEFINITION_RESOURCE_ACCESSOR(LowHealthSound)
    ENTITY_DEFINITION_RESOURCE_ACCESSOR(LowHealthEffect)
    ENTITY_DEFINITION_ACCESSOR(bool, IsShop)
    ENTITY_DEFINITION_ACCESSOR(bool, IsBase)
    ENTITY_DEFINITION_ACCESSOR(bool, IsTower)
    ENTITY_DEFINITION_ACCESSOR(bool, IsRax)
    ENTITY_DEFINITION_ACCESSOR(bool, NoAltClickPing)
    ENTITY_DEFINITION_ACCESSOR(const tstring&, DefaultShop)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoHeroArmorReduction)

    void            SetTarget(uint uiIndex, const tstring &sTarget) { m_asTargets[CLAMP(uiIndex, 0u, NUM_BUILDING_TARGETS)] = sTarget; }
    const tstring&  GetTarget(uint uiIndex) const                   { return m_asTargets[CLAMP(uiIndex, 0u, NUM_BUILDING_TARGETS)]; }

    // Operators
    virtual void            Copy(const IGameEntity &B);

    static void             ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
    static void             ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);

    virtual float           GetThreatLevel(IUnitEntity *pOther, bool bCurrentTarget);

    virtual void            DrawOnMap(CUITrigger &minimap, CPlayer *pLocalPlayer) const;
    virtual void            DrawOutlineOnMap(CUITrigger &minimap, CPlayer *pLocalPlayer) const;

    uint                    GetLastDamageTime() const       { return m_uiLastDamageTime; }

    void                    SetLastAttackAnnouncement(uint uiTime)  { m_uiLastAttackAnnounce = uiTime; }

    virtual float           GetBaseArmor() const;
};
//=============================================================================

#endif //__I_BUILDINGENTITY_H__
