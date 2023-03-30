// (C)2008 S2 Games
// c_entitycreepspawner.h
//
//=============================================================================
#ifndef __C_ENTITYCREEPSPAWNER_H__
#define __C_ENTITYCREEPSPAWNER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
#include "c_lane.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const uint NUM_CREEP_SPAWNER_TARGETS(4);
//=============================================================================

//=============================================================================
// CEntityCreepSpawner
//=============================================================================
class CEntityCreepSpawner : public IVisualEntity
{
protected:
    DECLARE_ENT_ALLOCATOR2(Entity, CreepSpawner);

    CLane       m_cLane;
    tstring     m_asTargets[NUM_CREEP_SPAWNER_TARGETS];
    uint        m_auiTargetUIDs[NUM_CREEP_SPAWNER_TARGETS];

public:
    ~CEntityCreepSpawner()  {}
    CEntityCreepSpawner();

    virtual bool            IsServerEntity() const          { return true; }

    virtual void            ApplyWorldEntity(const CWorldEntity &ent);

    virtual void            Spawn();

    virtual void            GameStart();

    void                    SetTarget(uint uiIndex, const tstring &sTarget) { m_asTargets[CLAMP(uiIndex, 0u, NUM_CREEP_SPAWNER_TARGETS)] = sTarget; }
    const tstring&          GetTarget(uint uiIndex) const                   { return m_asTargets[CLAMP(uiIndex, 0u, NUM_CREEP_SPAWNER_TARGETS)]; }
    uint                    GetTargetUID(uint uiIndex) const                { return m_auiTargetUIDs[CLAMP(uiIndex, 0u, NUM_CREEP_SPAWNER_TARGETS)]; }

    CLane&  GetLane()       { return m_cLane; }
};
//=============================================================================

#endif //__C_ENTITYCREEPSPAWNER_H__
