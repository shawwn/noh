// (C)2008 S2 Games
// c_entityneutralcampspawner.h
//
//=============================================================================
#ifndef __C_ENTITYNEUTRALCAMPSPAWNER_H__
#define __C_ENTITYNEUTRALCAMPSPAWNER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// CEntityNeutralCampSpawner
//=============================================================================
class CEntityNeutralCampSpawner : public IVisualEntity
{
protected:
    DECLARE_ENT_ALLOCATOR2(Entity, NeutralCampSpawner);

    tstring         m_sSpawnName;
    IUnitEntity*    m_pSpawnedUnit;

public:
    ~CEntityNeutralCampSpawner()    {}
    CEntityNeutralCampSpawner()
        : m_pSpawnedUnit(nullptr)
    {}

    SUB_ENTITY_ACCESSOR(CEntityNeutralCampSpawner, NeutralCampSpawner)

    virtual bool            IsServerEntity() const      { return true; }
    virtual IUnitEntity*    GetSpawnedUnit() const      { return m_pSpawnedUnit; }

    void                    ApplyWorldEntity(const CWorldEntity &ent);
    void                    Trigger(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_ENTITYNEUTRALCAMPSPAWNER_H__
