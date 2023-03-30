// (C)2007 S2 Games
// c_entitynpccontroller.h
//
//=============================================================================
#ifndef __C_ENTITYNPCCONTROLLER_H__
#define __C_ENTITYNPCCONTROLLER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class INpcEntity;
//=============================================================================

//=============================================================================
// CEntityNpcController
//=============================================================================
class CEntityNpcController : public IVisualEntity
{
private:
    struct SNpcSpawnState
    {
        ushort      unType;
        CVec3f      v3Pos;
        CVec3f      v3Angles;
        ResHandle   hDefinition;
    };

protected:
    START_ENTITY_CONFIG(IVisualEntity)
    END_ENTITY_CONFIG

    CEntityConfig*  m_pEntityConfig;

    DECLARE_ENT_ALLOCATOR2(Entity, NpcController);

    // Settings
    uint                        m_uiSpawnPeriod;
    float                       m_fSpawnRadius;

    uint                        m_uiNextRespawnCheckTime;
    vector<SNpcSpawnState>      m_vNpcSpawnStates;
    uivector                    m_vChildren;

    void                        DeleteChildren();
    void                        SpawnChildren();

public:
    ~CEntityNpcController() {}
    CEntityNpcController();

    void    ApplyWorldEntity(const CWorldEntity &ent);

    void    Spawn();
    bool    ServerFrame();

    virtual CVec4f  GetMapIconColor(CPlayer *pLocalPlayer, bool bLargeMap)      { return WHITE; }
    virtual bool    IsVisibleOnMinimap(CPlayer *pLocalPlayer, bool bLargeMap)   { return !pLocalPlayer->CanSee(this); }

    void    AddNpc(INpcEntity *pNpc);
};
//=============================================================================

#endif //__C_ENTITYNPCCONTROLLER_H__
