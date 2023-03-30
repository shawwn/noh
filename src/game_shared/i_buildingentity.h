// (C)2006 S2 Games
// i_buildingentity.h
//
//=============================================================================
#ifndef __I_BUILDINGENTITY_H__
#define __I_BUILDINGENTITY_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_gameentity.h"
//=============================================================================

//=============================================================================
// IBuildingEntity
//=============================================================================
class IBuildingEntity : public IVisualEntity
{
private:
    static vector<SDataField>   *s_pvFields;

    IBuildingEntity();

protected:
    START_ENTITY_CONFIG(IVisualEntity)
        DECLARE_ENTITY_CVAR(int, UpkeepCost)
        DECLARE_ENTITY_CVAR(float, DecayRate)
        DECLARE_ENTITY_CVAR(tstring, Foundation)
        DECLARE_ENTITY_CVAR(float, BuildHealth)
        DECLARE_ENTITY_CVAR(uint, BuildTime)
        DECLARE_ENTITY_CVAR(float, Armor)
        DECLARE_ENTITY_CVAR(tstring, LowHealthEffectPath)
        DECLARE_ENTITY_CVAR(bool, IsMajorBuilding)
        DECLARE_ENTITY_CVAR(tstring, SelectSoundPath)
        DECLARE_ENTITY_CVAR(tstring, SelectConstructionSoundPath)
        DECLARE_ENTITY_CVAR(tstring, ConstructionCompleteSoundPath)
        DECLARE_ENTITY_CVAR(tstring, ConstructionStartedSoundPath)
        DECLARE_ENTITY_CVAR(tstring, DestroyedSoundPath)
        DECLARE_ENTITY_CVAR(tstring, DestroyedSoundPathHuman)
        DECLARE_ENTITY_CVAR(tstring, DestroyedSoundPathBeast)
        DECLARE_ENTITY_CVAR(tstring, LowHealthSoundPath)
        DECLARE_ENTITY_CVAR(float, StateRadius)
        DECLARE_ENTITY_CVAR(bool, StateTargetEnemy)
        DECLARE_ENTITY_CVAR(bool, StateTargetAlly)
        DECLARE_ENTITY_CVAR(uint, StateDuration)
        DECLARE_ENTITY_CVAR(tstring, StateName)
        DECLARE_ENTITY_CVAR(bool, Stateselfapply)
        DECLARE_ENTITY_CVAR(CVec4f, StatePreviewColor)
        DECLARE_ENTITY_CVAR(float, UniqueStructureRadius)
        DECLARE_ENTITY_CVAR(float, MaximumDistanceFromMain)
        DECLARE_ENTITY_CVAR(float, MaximumDistanceFromMajor)
        DECLARE_ENTITY_CVAR(float, MaximumDistanceFromCommandStructure)
        DECLARE_ENTITY_CVAR(float, MinimumDistanceFromCommandStructure)
        DECLARE_ENTITY_CVAR(bool, IsMainBuilding)
        DECLARE_ENTITY_CVAR(tstring, UniqueRadiusEffectPath)
        DECLARE_ENTITY_CVAR(tstring, StateEffectPreviewPath)
        DECLARE_ENTITY_CVAR(bool, StateRequiresUpkeep)
        DECLARE_ENTITY_CVAR(float, BuildingCheckUpscale)
        DECLARE_ENTITY_CVAR(uint, MaxBuild)
        DECLARE_ENTITY_CVAR(bool, IsCommandStructure)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;
    uint            m_uiOwnerIndex;

    float   m_fUpkeepLevel;
    uint    m_uiFoundation;

    float   m_fStartBuildingHealthPool;
    float   m_fBuildingHealthPool;
    uint    m_uiStartBuildTime;
    uint    m_uiLastGameTime;
    uint    m_uiLastIncomeTime;
    float   m_fStartBuildingHealth;
    uint    m_uiLastDamageTime;
    bool    m_bLowHealthEffectActive;
    uint    m_uiBuildAnimTime;
    float   m_fFoundationScale;

    float   m_fRepairCostAccumulator;
    float   m_fHealLimit;
    int     m_iBountyRemaining;
    float   m_fArmorBreakPenalty;

    uint    m_uiPoseTime;

public:
    virtual ~IBuildingEntity();
    IBuildingEntity(CEntityConfig *pConfig);

    bool                    IsBuilding() const              { return true; }

    bool                    IsSelectable() const            { return true; }

    bool                    IsRepairable() const            { return HasNetFlags(ENT_NET_FLAG_NO_REPAIR); }
    void                    SetRepairable(bool bRepairable) { if (bRepairable) RemoveNetFlags(ENT_NET_FLAG_NO_REPAIR); else SetNetFlags(ENT_NET_FLAG_NO_REPAIR); }

    bool                    HasAltInfo() const              { return true; }
    const tstring&          GetAltInfoName() const          { return GetEntityName(); }

    virtual void            SetTeam(char cTeam);


    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();

    void                    AddRepairCost(float fCost)      { m_fRepairCostAccumulator += fCost; }
    int                     GetAccumulatedRepairCost()      { return INT_FLOOR(m_fRepairCostAccumulator); }
    void                    CostPaid()                      { m_fRepairCostAccumulator -= INT_FLOOR(m_fRepairCostAccumulator); }
    void                    SetHealLimit(float flimit)      { m_fHealLimit = flimit; }
    float                   GetHealLimit() const            { return m_fHealLimit; }

    virtual void            Baseline();
    virtual void            GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool            ReadSnapshot(CEntitySnapshot &snapshot);

    virtual CSkeleton*      AllocateSkeleton();
    virtual void            UpdateSkeleton(bool bPose);

    virtual bool            GetGameTip(IPlayerEntity *pPlayer, tstring &sMessage);

    virtual bool            CanBuild(const IPlayerEntity *pBuilder, tstring &sCanBuild);

    virtual void            Spawn();
    virtual float           Damage(float fDamage, int iFlags, IVisualEntity *pAttacker = NULL, ushort unDamagingObjectID = INVALID_ENT_TYPE, bool bFeedback = true);
    virtual void            Kill(IVisualEntity *pAttacker = NULL, ushort unKillingObjectID = INVALID_ENT_TYPE);
    virtual bool            PlayerEnter(IGameEntity *pActivator);

    void                    SetUpkeepLevel(float fLevel)    { m_fUpkeepLevel = CLAMP(fLevel, 0.0f, 1.0f); }
    float                   GetUpkeepLevel() const          { return m_fUpkeepLevel; }
    uint                    GetActiveUpkeepCost() const     { return INT_ROUND(m_pEntityConfig->GetUpkeepCost() * m_fUpkeepLevel); }
    uint                    GetUpkeepCost() const           { return m_pEntityConfig->GetUpkeepCost(); }
    virtual void            Upkeep();
    virtual void            UpkeepFailed(float fFraction);
    virtual uint            GetIncomeAmount() const         { return 0; }
    virtual uint            HarvestGold()                   { return 0; }
    

    virtual void            DamageNotification(uint uiIndex, uint uiAttacker, float fDamage)    {}
    
    void                    SetFoundationScale(float fScale)    { m_fFoundationScale = fScale; }
    float                   GetFoundationScale() const      { return m_fFoundationScale; }

    float                   GetBuildingScale() const        { return m_pEntityConfig->GetScale(); }

    virtual bool            ServerFrame();
    virtual void            Interpolate(float fLerp, IVisualEntity *pPrevState, IVisualEntity *pNextState);

    virtual bool            AddToScene(const CVec4f &v4Color, int iFlags);

    virtual bool            IsVisibleOnMinimap(IPlayerEntity *pLocalPlayer, bool bLargeMap);
    virtual CVec4f          GetMapIconColor(IPlayerEntity *pLocalPlayer, bool bLargeMap);

    virtual void            Link();
    virtual void            Unlink();

    virtual bool            IsCommandCenter() const         { return false; }
    virtual bool            IsMine() const                  { return false; }

    tstring                 GetFoundationType() const       { return m_pEntityConfig->GetFoundation(); }

    GAME_SHARED_API virtual void    SpawnPreview();
    
    uint                    GetFoundation()                 { return m_uiFoundation; }
    void                    SetFoundation(uint uiIndex)     { m_uiFoundation = uiIndex; }

    virtual float           Heal(float fHealth, IVisualEntity *pSource);

    float                   GetBaseArmor() const            { if (m_pEntityConfig == NULL) return 0.0f; return m_pEntityConfig->GetArmor(); }
    GAME_SHARED_API float   GetArmor() const;
    GAME_SHARED_API float   GetArmorDamageReduction() const;

    GAME_SHARED_API float   GetBuildPercent();
    GAME_SHARED_API void    SetBuildPercent(float fPercent) { m_fBuildingHealthPool = (GetMaxHealth() - m_fStartBuildingHealth) * CLAMP(1.0f - fPercent, 0.0f, 1.0f); }

    virtual CVec3f          GetApproachPosition(const CVec3f &v3Start, const CBBoxf &bbBounds);

    ENTITY_CVAR_ACCESSOR(bool, IsCommandStructure, false);
    ENTITY_CVAR_ACCESSOR(bool, IsMajorBuilding, false);
    ENTITY_CVAR_ACCESSOR(tstring, SelectSoundPath, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, SelectConstructionSoundPath, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, ConstructionCompleteSoundPath, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, ConstructionStartedSoundPath, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, DestroyedSoundPath, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, DestroyedSoundPathHuman, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, DestroyedSoundPathBeast, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, LowHealthSoundPath, _T(""));
    ENTITY_CVAR_ACCESSOR(tstring, StateName, _T(""));
    ENTITY_CVAR_ACCESSOR(bool, StateTargetEnemy, false);
    ENTITY_CVAR_ACCESSOR(bool, StateTargetAlly, false);
    ENTITY_CVAR_ACCESSOR(float, StateRadius, 0.0f);
    ENTITY_CVAR_ACCESSOR(bool, Stateselfapply, false);
    ENTITY_CVAR_ACCESSOR(CVec4f, StatePreviewColor, CVec4f(0.00f, 0.00f, 0.500f, 1.0f));
    ENTITY_CVAR_ACCESSOR(bool, IsMainBuilding, true);
    ENTITY_CVAR_ACCESSOR(float, MaximumDistanceFromMain, 0.0f);
    ENTITY_CVAR_ACCESSOR(float, MaximumDistanceFromMajor, 0.0f);
    ENTITY_CVAR_ACCESSOR(float, MaximumDistanceFromCommandStructure, 0.0f);
    ENTITY_CVAR_ACCESSOR(float, MinimumDistanceFromCommandStructure, 0.0f);
    ENTITY_CVAR_ACCESSOR(float, UniqueStructureRadius, 0.0f);
    ENTITY_CVAR_ACCESSOR(tstring, UniqueRadiusEffectPath, "");
    ENTITY_CVAR_ACCESSOR(tstring, StateEffectPreviewPath, "");
    ENTITY_CVAR_ACCESSOR(uint, MaxBuild, 0);
    ENTITY_CVAR_ACCESSOR(uint, BuildTime, 0);

    // Operators
    virtual void            Copy(const IGameEntity &B);

    static void             ClientPrecache(CEntityConfig *pConfig);
    static void             ServerPrecache(CEntityConfig *pConfig);
};
//=============================================================================

#endif //__I_BUILDINGENTITY_H__
