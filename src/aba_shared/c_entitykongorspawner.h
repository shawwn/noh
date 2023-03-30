// (C)2008 S2 Games
// c_entitykongorspawner.h
//
//=============================================================================
#ifndef __C_ENTITYKONGORSPAWNER_H__
#define __C_ENTITYKONGORSPAWNER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// CEntityBossSpawner
//=============================================================================
class CEntityBossSpawner : public IVisualEntity
{
protected:
    DECLARE_ENT_ALLOCATOR2(Entity, BossSpawner);

    tstring     m_sSpawnName;

public:
    ~CEntityBossSpawner()   {}
    CEntityBossSpawner()
    {}

    virtual bool        IsServerEntity() const          { return true; }

    void    ApplyWorldEntity(const CWorldEntity &ent);
    void    Trigger(IGameEntity *pActivator);
};
//=============================================================================

#endif //__C_ENTITYKONGORSPAWNER_H__
