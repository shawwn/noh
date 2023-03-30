// (C)2008 S2 Games
// c_entitycritterspawner.h
//
//=============================================================================
#ifndef __C_ENTITYCRITTERSPAWNER_H__
#define __C_ENTITYCRITTERSPAWNER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// CEntityCritterSpawner
//=============================================================================
class CEntityCritterSpawner : public IVisualEntity
{
protected:
    DECLARE_ENT_ALLOCATOR2(Entity, CritterSpawner);

public:
    ~CEntityCritterSpawner()    {}
    CEntityCritterSpawner();

    virtual bool        IsServerEntity() const          { return true; }

    void    ApplyWorldEntity(const CWorldEntity &ent);

    void    Spawn();
};
//=============================================================================

#endif //__C_ENTITYCRITTERSPAWNER_H__
