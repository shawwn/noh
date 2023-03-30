// (C)2009 S2 Games
// c_linearaffector.h
//
//=============================================================================
#ifndef __C_LINEARAFFECTOR_H__
#define __C_LINEARAFFECTOR_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
#include "c_combatevent.h"
#include "c_linearaffectordefinition.h"
#include "c_entitydefinitionresource.h"
//=============================================================================

//=============================================================================
// CLinearAffector
//=============================================================================
class CLinearAffector : public IVisualEntity
{
    DECLARE_ENTITY_DESC

public:
    typedef CLinearAffectorDefinition TDefinition;

private:
    static ResHandle    s_hDebugMaterial;

    CVec3f              m_v3TargetPosition;

    uint                m_uiLevel;
    uint                m_uiChainCount;

    uint                m_uiOwnerIndex;
    uint                m_uiAttachTargetUID;
    uint                m_uiFirstTargetIndex;
    uint                m_uiCreationTime;
    uint                m_uiLastImpactTime;
    uint                m_uiIntervalCount;
    map<uint, uint>     m_mapImpacts;
    uint                m_uiTotalImpactCount;

public:
    ~CLinearAffector()  {}
    CLinearAffector();

    SUB_ENTITY_ACCESSOR(CLinearAffector, LinearAffector)
    
    virtual void    Baseline();
    virtual void    GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const;
    virtual bool    ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion);

    virtual void    Copy(const IGameEntity &B);

    void            SetTargetPosition(const CVec3f &v3Target)   { m_v3TargetPosition = v3Target; }
    const CVec3f&   GetTargetPosition() const                   { return m_v3TargetPosition; }

    void            SetOwnerIndex(uint uiIndex)         { m_uiOwnerIndex = uiIndex; }
    uint            GetOwnerIndex() const               { return m_uiOwnerIndex; }
    IUnitEntity*    GetOwner() const                    { return Game.GetUnitEntity(m_uiOwnerIndex); }

    void            SetAttachTargetUID(uint uiUID)      { m_uiAttachTargetUID = uiUID; }
    uint            GetAttachTargetUID() const          { return m_uiAttachTargetUID; }
    IUnitEntity*    GetAttachTarget() const             { IGameEntity *pEntity(Game.GetEntityFromUniqueID(m_uiAttachTargetUID)); return (pEntity ? pEntity->GetAsUnit() : NULL);}

    void            SetFirstTargetIndex(uint uiIndex)   { m_uiFirstTargetIndex = uiIndex; }

    virtual void    Spawn();

    uint            GetPotentialImpactCount(uint uiTargetUID, uint uiTotalRemainingImpacts);

    void    ImpactEntity(IUnitEntity *pTarget);
    void    Impact(vector<IUnitEntity*> &vTargets);
    void    SubSegmentImpacts();

    virtual bool    ServerFrameSetup();
    virtual bool    ServerFrameMovement();
    virtual bool    ServerFrameAction();
    virtual bool    ServerFrameCleanup();

    virtual bool    AddToScene(const CVec4f &v4Color, int iFlags);

    bool    IsActive() const            { return true; }
    
    uint    GetLevel() const            { return m_uiLevel; }
    void    SetLevel(uint uiLevel)      { m_uiLevel = uiLevel; }
    
    uint    GetChainCount() const       { return m_uiChainCount; }
    void    IncrementChainCount()       { ++m_uiChainCount; }

    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, Lifetime)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, Radius)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, MinLength)
    MULTI_LEVEL_ENTITY_DEFINITION_TEMPORAL_ACCESSOR(float, MaxLength)

    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ImpactInterval)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxIntervals)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, ImpactDelay)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxTotalImpacts)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxImpactsPerInterval)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxImpactsPerTarget)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, MaxImpactsPerTargetPerInterval)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, SubSegmentOffset)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(float, SubSegmentLength)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(ETargetSelection, TargetSelection)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, TargetScheme)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(uint, EffectType)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, Persist)
    MULTI_LEVEL_ENTITY_DEFINITION_ACCESSOR(bool, DestroyTrees)

    ENTITY_DEFINITION_RESOURCE_ACCESSOR(Effect)
    ENTITY_DEFINITION_RESOURCE_ACCESSOR(ImpactEffect)
    ENTITY_DEFINITION_RESOURCE_ACCESSOR(BridgeEffect)
    ENTITY_DEFINITION_RESOURCE_ACCESSOR(LinkEffect)

    void            UpdateModifiers(const uivector &vModifiers);
};
//=============================================================================

#endif //__C_LINEARAFFECTOR_H__
