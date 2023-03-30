// (C)2008 S2 Games
// i_heroentity.h
//
//=============================================================================
#ifndef __I_HEROENTITY_H__
#define __I_HEROENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_unitentity.h"
#include "c_herodefinition.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
GAME_SHARED_API EXTERN_CVAR_UINT(hero_respawnTimePerLevel);
GAME_SHARED_API EXTERN_CVAR_FLOAT(g_heroMapIconSize);
GAME_SHARED_API EXTERN_CVAR_STRING(g_heroMapIcon);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_buyBackCost);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_buyBackCostPerLevel);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_expUnsharedBountyPerLevel);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyRadiusBase);
GAME_SHARED_API EXTERN_CVAR_UINT(hero_goldBountyRadiusPerLevel);

GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_armorPerAgi);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_attackSpeedPerAgi);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_hpPerStr);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_hpRegenPerStr);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_mpPerInt);
GAME_SHARED_API EXTERN_CVAR_FLOAT(hero_mpRegenPerInt);

EXTERN_CVAR_UINT(hero_blockRepathTime);
EXTERN_CVAR_UINT(hero_blockRepathTimeExtra);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint MAX_HERO_LEVEL(25);

// HERO_ATTRIBUTE
#define HERO_ATTRIBUTE(name) \
virtual float   Get##name##PerLevel() const \
{ \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<float>(); \
\
    return static_cast<TDefinition *>(m_pDefinition)->Get##name##PerLevel(); \
} \
virtual float   GetBase##name() const \
{ \
    if (m_pDefinition == NULL) \
        return GetDefaultEmptyValue<float>(); \
\
    return floor(static_cast<TDefinition *>(m_pDefinition)->Get##name() + (GetLevel() - 1) * Get##name##PerLevel()); \
} \
ADJUSTED_ATTRIBUTE_ADD(float, name, true)
//=============================================================================

//=============================================================================
// IHeroEntity
//=============================================================================
class IHeroEntity : public IUnitEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CHeroDefinition TDefinition;

private:
    static ResHandle    s_hMinimapIcon0;
    static ResHandle    s_hMinimapIcon1;
    static ResHandle    s_hMinimapTalkingIcon;
    static ResHandle    s_hLevelupEffect;
    static ResHandle    s_hHeroIndicator;

protected:
    float   m_fExperience;

    uint    m_uiRespawnTime;
    uint    m_uiDeathTimeStamp;

    float   m_fGoldLossMultiplier;
    float   m_fRespawnTimeMultiplier;
    int     m_iGoldLossBonus;
    int     m_iRespawnTimeBonus;
    float   m_fRespawnHealthMultiplier;
    float   m_fRespawnManaMultiplier;

    bool    m_bRespawnPositionSet;
    CVec3f  m_v3RespawnPosition;

    uint    m_uiLastAttackAnnounce;

    uint    m_uiAIControllerUID;
    CVec2f  m_v2Waypoint;

    uint    m_uiFinalExpEarnedTime;

    void    AddIndicator();

public:
    virtual ~IHeroEntity();
    IHeroEntity();

    SUB_ENTITY_ACCESSOR(IHeroEntity, Hero)

    virtual void        Baseline();
    virtual void        GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool        ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);
    virtual void        Copy(const IGameEntity &B);

    static void         ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
    static void         ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);

    virtual CVec4f      GetMapIconColor(CPlayer *pLocalPlayer) const;
    virtual CVec4f      GetTeamColor(CPlayer *pLocalPlayer) const;
    virtual float       GetMapIconSize(CPlayer *pLocalPlayer) const     { return g_heroMapIconSize; }

    uint                GetDeathTimeStamp() const                       { return m_uiDeathTimeStamp; }

    virtual bool        ServerFrameSetup();
    virtual bool        ServerFrameThink();
    virtual bool        ServerFrameMovement();
    virtual bool        ServerFrameCleanup();
    virtual void        Spawn();
    virtual void        Die(IUnitEntity *pAttacker = NULL, ushort unKillingObject = INVALID_ENT_TYPE);
    virtual void        KillReward(IUnitEntity *pKiller, CPlayer *pPlayerKiller);
    void                AssistsReward(IUnitEntity *&pKiller, CPlayer *&pPlayerKiller, ivector &vAssistPlayers);
    void                FirstBloodReward(IUnitEntity *pKiller, CPlayer *pPlayerKiller);
    
    void                Terminate()     { m_uiRespawnTime = INVALID_TIME; SetStatus(ENTITY_STATUS_DORMANT); SetUnitFlags(UNIT_FLAG_TERMINATED); Unlink(); }

    virtual void        Respawn();

    // Attributes
    HERO_ATTRIBUTE(Strength)
    HERO_ATTRIBUTE(Agility)
    HERO_ATTRIBUTE(Intelligence)

    static float    AdjustMaxHealth(float fBaseMaxHealth, float fStrength)      { return ROUND(fBaseMaxHealth + fStrength * hero_hpPerStr); }
    static float    AdjustHealthRegen(float fBaseHealthRegen, float fStrength)  { return fBaseHealthRegen + fStrength * hero_hpRegenPerStr; }
    static float    AdjustMaxMana(float fBaseMaxMana, float fIntelligence)      { return ROUND(fBaseMaxMana + fIntelligence * hero_mpPerInt);}
    static float    AdjustManaRegen(float fBaseManaRegen, float fIntelligence)  { return fBaseManaRegen + fIntelligence * hero_mpRegenPerInt;}
    static float    AdjustArmor(float fBaseArmor, float fAgility)               { return fBaseArmor + fAgility * hero_armorPerAgi; }

    virtual float   GetBaseMaxHealth() const        { return AdjustMaxHealth(IUnitEntity::GetBaseMaxHealth(), GetStrength()); }
    virtual float   GetBaseMaxMana() const          { return AdjustMaxMana(IUnitEntity::GetBaseMaxMana(), GetIntelligence());}
    virtual float   GetBaseHealthRegen() const      { return AdjustHealthRegen(IUnitEntity::GetBaseHealthRegen(), GetStrength()); }
    virtual float   GetBaseManaRegen() const        { return AdjustManaRegen(IUnitEntity::GetBaseManaRegen(), GetIntelligence()); }
    virtual float   GetBaseArmor() const            { return AdjustArmor(IUnitEntity::GetBaseArmor(), GetAgility()); }
    virtual float   GetBaseAttackSpeed() const;

    virtual uint    GetMaxLevel() const             { return MAX_HERO_LEVEL; }

    virtual float   GetAttributeDamageAdjustment() const;
    virtual float   GetAttackDamageMin() const      { return IUnitEntity::GetAttackDamageMin() + GetAttributeDamageAdjustment(); }
    virtual float   GetAttackDamageMax() const      { return IUnitEntity::GetAttackDamageMax() + GetAttributeDamageAdjustment(); }

    virtual ushort  GetGoldBounty() const;
    virtual ushort  GetGoldBountyRadiusAmount() const   { return hero_goldBountyRadiusBase + (GetLevel() * hero_goldBountyRadiusPerLevel); }
    virtual float   GetExperienceBounty() const         { int iLevel(GetLevel()); if (iLevel < 5) return 100.0f + (10.0f * (SQR(iLevel) - iLevel)); return (iLevel * 100.0f) - 200.0f; }
    virtual float   GetUnsharedExperienceBounty() const { return GetLevel() * hero_expUnsharedBountyPerLevel; }
    
    GAME_SHARED_API float   GiveExperience(float fExperience, IUnitEntity *pSource = NULL);
    GAME_SHARED_API void    ResetExperience();

    void    SetExperience(float fExperience)            { m_fExperience = fExperience; }
    float   GetExperience() const                       { return m_fExperience; }
    
    uint    GetLevel() const                            { return MIN(MAX_HERO_LEVEL, uint(floor(sqrt((2.0f * ((m_fExperience / 100.0f) + 1.0f)) + 0.25f) - 0.5f))); }
    float   GetExperienceForLevel(uint uiLevel) const   { uiLevel = CLAMP(uiLevel, 1u, MAX_HERO_LEVEL); return (((uiLevel * uiLevel + uiLevel) / 2.0f) - 1.0f) * 100.0f; }
    float   GetExperienceForNextLevel() const           { uint uiLevel = MIN(MAX_HERO_LEVEL, GetLevel() + 1); return (((uiLevel * uiLevel + uiLevel) / 2.0f) - 1.0f) * 100.0f; }
    float   GetExperienceForCurrentLevel() const        { uint uiLevel = MIN(MAX_HERO_LEVEL - 1, GetLevel()); return (((uiLevel * uiLevel + uiLevel) / 2.0f) - 1.0f) * 100.0f; }
    float   GetPercentNextLevel() const                 { return (GetLevel() == MAX_HERO_LEVEL) ? 1.0f : ILERP(m_fExperience, GetExperienceForLevel(GetLevel()), GetExperienceForNextLevel()); }

    uint    GetRespawnTime() const                  { return m_uiRespawnTime; }
    uint    GetRemainingRespawnTime() const         { if (m_uiRespawnTime == INVALID_TIME) return 0; return m_uiRespawnTime - Game.GetGameTime(); }
    uint    GetRespawnDuration() const              { return GetLevel() * hero_respawnTimePerLevel; }
    float   GetRespawnPercent() const               { if (m_uiRespawnTime == INVALID_TIME) return 0.0f; return GetRemainingRespawnTime() / float(GetRespawnDuration()); }
    uint    GetBuyBackCost() const                  { if (Game.GetMatchTime() == 0) return INT_FLOOR(hero_buyBackCost + GetLevel() * GetLevel() * 1.5f); return INT_FLOOR(hero_buyBackCost + GetLevel() * GetLevel() * 1.5f + (((GetRespawnTime() - Game.GetPhaseStartTime()) / 1000.0f) / 60.0f) * 15.0f); }

    GAME_SHARED_API int             GetAvailablePoints() const;

    ENTITY_DEFINITION_ACCESSOR(EAttribute, PrimaryAttribute)
    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(RespawnEffect)

    virtual IUnitEntity*    SpawnIllusion(const CVec3f &v3Position, const CVec3f &v3Angles, uint uiLifetime, 
                                        float fReceiveDamageMultiplier, float fInflictDamageMultiplier, 
                                        ResHandle hSpawnEffect, ResHandle hDeathEffect, 
                                        bool bDeathAnim, bool bInheritActions);

    void                    SetGoldLossMultiplier(float fMultiplier)        { m_fGoldLossMultiplier = fMultiplier; }
    virtual float           GetGoldLossMultiplier() const                   { return m_fGoldLossMultiplier; }

    void                    SetRespawnTimeMultiplier(float fMultiplier)     { m_fRespawnTimeMultiplier = fMultiplier; }
    virtual float           GetRespawnTimeMultiplier() const                { return m_fRespawnTimeMultiplier; }

    void                    SetGoldLossBonus(int iBonus)                    { m_iGoldLossBonus = iBonus; }
    virtual int             GetGoldLossBonus() const                        { return m_iGoldLossBonus; }

    void                    SetRespawnTimeBonus(int iBonus)                 { m_iRespawnTimeBonus = iBonus; }
    virtual int             GetRespawnTimeBonus() const                     { return m_iRespawnTimeBonus; }

    void                    SetRespawnTime(uint uiTime)                     { m_uiRespawnTime = uiTime; }

    void                    SetRespawnHealthMultiplier(float fMultiplier)   { m_fRespawnHealthMultiplier = fMultiplier; }
    virtual float           GetRespawnHealthMultiplier() const              { return m_fRespawnHealthMultiplier; }

    void                    SetRespawnManaMultiplier(float fMultiplier)     { m_fRespawnManaMultiplier = fMultiplier; }
    virtual float           GetRespawnManaMultiplier() const                { return m_fRespawnManaMultiplier; }

    void                    SetRespawnPosition(const CVec3f &v3Pos)         { m_bRespawnPositionSet = true; m_v3RespawnPosition = v3Pos; }

    virtual void            Moved();

    virtual void            Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);

    virtual void            DrawOnMap(class CUITrigger &minimap, CPlayer *pLocalPlayer) const;

    virtual bool            AddToScene(const CVec4f &v4Color, int iFlags);

    virtual void            Damage(CDamageEvent &damage);

    virtual bool            IsAttribute(EAttribute eAttribute) const        { return GetPrimaryAttribute() == eAttribute; }

    virtual uint            GetRepathTime() const                           { return hero_blockRepathTime; }
    virtual uint            GetRepathTimeExtra() const                      { return hero_blockRepathTimeExtra; }

    void                    SetAIController(uint uiAIController)            { m_uiAIControllerUID = uiAIController; }
    uint                    GetAIController() const                         { return m_uiAIControllerUID; }

    uint                    GetFinalExpEarnedTime() const                   { return m_uiFinalExpEarnedTime; }
};
//=============================================================================

#endif //__I_HEROENTITY_H__
