// (C)2008 S2 Games
// i_areaaffector.h
//
//=============================================================================
#ifndef __I_AREAAFFECTOR_H__
#define __I_AREAAFFECTOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
#include "c_combatevent.h"
#include "c_affectordefinition.h"
#include "c_entitydefinitionresource.h"
//=============================================================================

//=============================================================================
// IAffector
//=============================================================================
class IAffector : public IVisualEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CAffectorDefinition TDefinition;

private:
    static ResHandle    s_hDebugMaterial;

protected:
    uint                m_uiLevel;
    uint                m_uiChainCount;

    uint                m_uiOwnerIndex;
    uint                m_uiAttachTargetUID;
    uint                m_uiProxyUID;
    uint                m_uiIgnoreUID;
    uint                m_uiFirstTargetIndex;
    uint                m_uiCreationTime;
    uint                m_uiLastImpactTime;
    uint                m_uiIntervalCount;
    map<uint, uint>     m_mapImpacts;
    uint                m_uiTotalImpactCount;
    uint                m_uiLastMoveTime;
    float               m_fParam;

    bool                m_bExpired;

public:
    virtual ~IAffector()    {}
    IAffector();

    SUB_ENTITY_ACCESSOR(IAffector, Affector)
    
    virtual void    Baseline();
    virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    virtual void    Copy(const IGameEntity &B);

    static void     ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
    static void     ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);

    void            SetOwnerIndex(uint uiIndex)         { m_uiOwnerIndex = uiIndex; }
    uint            GetOwnerIndex() const               { return m_uiOwnerIndex; }
    IUnitEntity*    GetOwner() const                    { return Game.GetUnitEntity(m_uiOwnerIndex); }

    void            SetAttachTargetUID(uint uiUID)      { m_uiAttachTargetUID = uiUID; }
    uint            GetAttachTargetUID() const          { return m_uiAttachTargetUID; }
    IUnitEntity*    GetAttachTarget() const             { IGameEntity *pEntity(Game.GetEntityFromUniqueID(m_uiAttachTargetUID)); return (pEntity ? pEntity->GetAsUnit() : NULL);}

    void            SetParam(float fParam)              { m_fParam = fParam; }
    float           GetParam() const                    { return m_fParam; }

    void            SetProxyUID(uint uiUID)             { m_uiProxyUID = uiUID; }
    uint            GetProxyUID() const                 { return m_uiProxyUID; }
    IGameEntity*    GetProxy(uint uiIndex) const        { return Game.GetEntityFromUniqueID(m_uiProxyUID); }

    void            SetIgnoreUID(uint uiUID)            { m_uiIgnoreUID = uiUID; }
    uint            GetIgnoreUID() const                { return m_uiIgnoreUID; }

    void            SetFirstTargetIndex(uint uiIndex)   { m_uiFirstTargetIndex = uiIndex; }

    virtual void    Spawn();

    uint            GetPotentialImpactCount(uint uiTargetUID, uint uiTotalRemainingImpacts);

    virtual void    ImpactEntity(IUnitEntity *pTarget);
    virtual void    Impact(vector<IUnitEntity*> &vTargets);

    virtual bool    ServerFrameSetup();
    virtual bool    ServerFrameMovement();
    virtual bool    ServerFrameAction();
    virtual bool    ServerFrameCleanup();

    virtual bool    AddToScene(const CVec4f &v4Color, int iFlags);

    void            ExecuteActionScript(EEntityActionScript eScript, IUnitEntity *pTarget, const CVec3f &v3Target);

    bool    IsActive() const            { return true; }
    
    uint    GetLevel() const            { return m_uiLevel; }
    void    SetLevel(uint uiLevel)      { m_uiLevel = uiLevel; }
    
    uint    GetChainCount() const       { return m_uiChainCount; }
    void    IncrementChainCount()       { ++m_uiChainCount; }

    void    Expire()                    { m_bExpired = true; }

    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, Lifetime)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, Radius)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, InnerRadiusOffset)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, Arc)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, Angle)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, Speed)

    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ImpactInterval)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxIntervals)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ImpactDelay)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxTotalImpacts)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxImpactsPerInterval)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxImpactsPerTarget)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxImpactsPerTargetPerInterval)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(ETargetSelection, TargetSelection)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, TargetScheme)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, EffectType)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, IgnoreInvulnerable)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Persist)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DestroyTrees)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, CanTurn)

    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(Effect)
    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(ImpactEffect)
    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(BridgeEffect)
    MULTI_LEVEL_ENTITY_DEFINITION_RESOURCE_ACCESSOR(LinkEffect)

    void            UpdateModifiers(const uivector &vModifiers);
    GAME_SHARED_API virtual void    SnapshotUpdate();
};
//=============================================================================

#endif //__I_AREAAFFECTOR_H__
