// (C)2008 S2 Games
// i_slaveentity.h
//
//=============================================================================
#ifndef __I_SLAVEENTITY_H__
#define __I_SLAVEENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
#include "i_slavedefinition.h"
#include "c_combatevent.h"
#include "c_entitydefinitionresource.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const byte INVALID_SLOT(-1);
//=============================================================================

//=============================================================================
// ISlaveEntity
//=============================================================================
class ISlaveEntity : public IGameEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef ISlaveDefinition TDefinition;


protected:
    uint            m_uiOwnerIndex;
    byte            m_ySlot;
    uint            m_uiLevel;
    byte            m_yCharges;
    byte            m_yAttackModPriority;

    uivector        m_vTimedCharges;
    float           m_fAccumulator;

    uint            m_uiProxyUID;
    uint            m_uiSpawnerUID;
    uint            m_uiFadeStartTime;

public:
    virtual ~ISlaveEntity() {}
    ISlaveEntity();

    SUB_ENTITY_ACCESSOR(ISlaveEntity, Slave)

    // Network
    GAME_SHARED_API virtual void        Baseline();
    GAME_SHARED_API virtual void        GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    GAME_SHARED_API virtual bool        ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    
    virtual int         GetPrivateClient();

    virtual bool        IsActive() const            { return false; }

    virtual bool        ServerFrameAction();
    virtual bool        ServerFrameCleanup();

    // Attributes
    virtual void        SetLevel(uint uiLevel)      { m_uiLevel = uiLevel; }
    virtual uint        GetLevel() const            { if (GetMaxLevel() == 0 && m_uiLevel == 0) return 1; return m_uiLevel; }
    virtual uint        GetMaxLevel() const         { return 0; }
    virtual void        LevelUp()                   {}
    virtual bool        CanLevelUp() const          { return false; }

    virtual uint        GetInitialCharges() const   { return 0; }
    virtual byte        GetCharges() const          { return m_yCharges; }
    virtual void        SetCharges(uint uiCharges)  { m_yCharges = uiCharges; }
    virtual void        RemoveCharge()              { if (m_yCharges > 0) --m_yCharges; }
    virtual void        AddCharges(uint uiCharges)  { if (GetMaxCharges() == -1) m_yCharges += uiCharges; else m_yCharges = MIN<int>(m_yCharges + uiCharges, GetMaxCharges()); }

    virtual void        AddTimedCharges(int iCharges, uint uiExpireTime);

    void                SetAccumulator(float fValue)    { m_fAccumulator = fValue; }
    float               GetAccumulator() const          { return m_fAccumulator; }
    void                AdjustAccumulator(float fDelta) { m_fAccumulator += fDelta; }

    void                SetAttackModPriority()          { m_yAttackModPriority = 1; }
    void                RemoveAttackModPriority()       { m_yAttackModPriority = 0; }
    bool                HasAttackModPriority() const    { return m_yAttackModPriority != 0; }

    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tsvector&, UnitType)
    
    ENTITY_DEFINITION_ACCESSOR(const tstring&, DisplayName)
    ENTITY_DEFINITION_LOCALIZED_STRING_ACCESSOR(Description)
    ENTITY_DEFINITION_LOCALIZED_STRING_ACCESSOR(Description2)
    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(Icon)
    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(MapIcon)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, MapIconColor)
    ENTITY_DEFINITION_ACCESSOR(bool, Singleton)

    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(PassiveEffect)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Strength)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Agility)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Intelligence)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MaxHealth)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MaxHealthMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, HealthRegen)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, HealthRegenMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, HealthRegenPercent)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MaxMana)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MaxManaMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ManaRegen)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ManaRegenMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ManaRegenPercent)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Armor)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MagicArmor)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Deflection)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, DeflectionChance)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, IncomingDamageMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, DebuffDurationMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, HealMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, AttackSpeed)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, AttackSpeedMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, AttackSpeedSlow)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, CastSpeed)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, CooldownSpeed)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ReducedCooldowns)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, IncreasedCooldowns)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Damage)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, BaseDamageMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, TotalDamageMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MoveSpeed)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MoveSpeedMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, SlowResistance)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MoveSpeedSlow)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, AttackRange)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, AttackRangeMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, LifeSteal)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, CriticalChance)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, CriticalMultiplier)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, EvasionRanged)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, EvasionMelee)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MissChance)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, HealthRegenReduction)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, ManaRegenReduction)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Stunned)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Silenced)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Perplexed)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Disarmed)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Immobilized)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Immobilized2)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Restrained)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Invulnerable)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Revealed)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Frozen)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Isolated)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, FreeCast)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, ClearVision)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Deniable)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, DeniablePercent)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Smackable)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, NoThreat)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, TrueStrike)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Sighted)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, AlwaysTransmitData)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, StealthType)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, StealthProximity)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, RevealType)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, RevealRange)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, FadeTime)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, ForceAnim)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ImmunityType)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ModifierKey)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ModifierKey2)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, EffectType)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Unitwalking)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Treewalking)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Cliffwalking)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Buildingwalking)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Antiwalking)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, ShopAccess)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, RemoteShopAccess)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, SharedShopAccess)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, SharedRemoteShopAccess)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, RestrictItemAccess)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, StashAccess)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(int, MaxCharges)
    PROGRESSIVE_MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, Counter)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Shield)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, MaxAccumulator)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(const tstring&, DefaultActiveModifierKey)

    IUnitEntity*        GetOwner() const                            { return Game.GetUnitEntity(GetOwnerIndex()); }
    
    uint                GetOwnerIndex() const                       { return m_uiOwnerIndex; }
    void                SetOwnerIndex(uint uiIndex)                 { m_uiOwnerIndex = uiIndex; }

    byte                GetSlot() const                             { return m_ySlot; }
    void                SetSlot(byte ySlot)                         { m_ySlot = ySlot; }

    virtual bool        OwnerDamaged(CDamageEvent &damage)          { return true; }
    virtual bool        OwnerAction()                               { return true; }

    virtual void        Spawn();

    virtual ResHandle   GetEffect()                                 { return GetPassiveEffect(); }
    byte                GetEffectSequence()                         { return 0; }

    static void         ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
    static void         ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier);
    
    virtual float       GetBonusDamage() const { return GetDamage(); }

    virtual void        UpdateModifiers(const uivector &vModifiers);

    GAME_SHARED_API const tstring&  GetEffectDescription(EEntityActionScript eAction);
    GAME_SHARED_API uint            GetEffectDescriptionIndex(EEntityActionScript eAction);

    void            SetProxyUID(uint uiUID)             { m_uiProxyUID = uiUID; }
    uint            GetProxyUID() const                 { return m_uiProxyUID; }
    IGameEntity*    GetProxy(uint uiIndex) const        { return Game.GetEntityFromUniqueID(m_uiProxyUID); }

    void            SetSpawnerUID(uint uiUID)           { m_uiSpawnerUID = uiUID; }
    uint            GetSpawnerUID() const               { return m_uiSpawnerUID; }
    IGameEntity*    GetSpawner() const                  { return Game.GetEntityFromUniqueID(m_uiSpawnerUID); }

    void                SetFadeStartTime(uint uiTime)           { m_uiFadeStartTime = uiTime; }
    uint                GetFadeStartTime() const                { return m_uiFadeStartTime; }

    virtual float       GetStealthFade();
};
//=============================================================================

#endif //__I_SLAVEENTITY_H__
