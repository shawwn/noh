// (C)2007 S2 Games
// c_entitychest.h
//
//=============================================================================
#ifndef __C_ENTITYCHEST_H__
#define __C_ENTITYCHEST_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// CEntityChest
//=============================================================================
class CEntityChest : public IVisualEntity
{
private:
    static vector<SDataField>   *s_pvFields;

protected:
    START_ENTITY_CONFIG(IVisualEntity)
        DECLARE_ENTITY_CVAR(float, BoundsRadius)
        DECLARE_ENTITY_CVAR(float, BoundsHeight)
        DECLARE_ENTITY_CVAR(uint, ExpireTime)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Entity, Chest);

    uint        m_uiSpawnTime;

public:
    ~CEntityChest();
    CEntityChest();

    virtual bool        IsSelectable() const            { return true; }

    GAME_SHARED_API static const vector<SDataField>&    GetTypeVector();
    
    virtual void                Baseline();
    virtual void                GetSnapshot(CEntitySnapshot &snapshot) const;
    virtual bool                ReadSnapshot(CEntitySnapshot &snapshot);

    virtual void                Spawn();
    virtual bool                ServerFrame();
    virtual void                Touch(IGameEntity *pActivator);

    virtual void                Link();
    virtual void                Unlink();

    GAME_SHARED_API CSkeleton*  AllocateSkeleton();

    GAME_SHARED_API virtual bool    AIShouldTarget()            { return false; }
};
//=============================================================================

#endif //__C_ENTITYSOUL_H__
