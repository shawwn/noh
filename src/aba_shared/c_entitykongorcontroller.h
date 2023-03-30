// (C)2008 S2 Games
// c_entitykongorcontroller.h
//
//=============================================================================
#ifndef __C_ENTITYKONGORCONTROLLER_H__
#define __C_ENTITYKONGORCONTROLLER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_visualentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
EXTERN_CVAR_FLOAT(g_bossMapIconSize);
EXTERN_CVAR_VEC4(g_bossMapIconColor);
//=============================================================================

//=============================================================================
// CEntityBossController
//=============================================================================
class CEntityBossController : public IVisualEntity
{
private:
    static ResHandle    s_hMinimapIcon;

    typedef vector<tstring>         SpawnerList;
    typedef SpawnerList::iterator   SpawnerList_it;
    typedef vector<SpawnerList>     SpawnerArray;
    typedef SpawnerArray::iterator  SpawnerArray_it;

    typedef vector<uint>                SpawnerUIDList;
    typedef SpawnerUIDList::iterator    SpawnerUIDList_it;
    typedef vector<SpawnerUIDList>      SpawnerUIDArray;
    typedef SpawnerUIDArray::iterator   SpawnerUIDArray_it;

protected:
    DECLARE_ENT_ALLOCATOR2(Entity, BossController);

    float               m_fMapIconSize;
    ResHandle           m_hMapIcon;
    CVec4f              m_v4MapIconColor;

    uint                m_uiFirstSpawnTime;
    uint                m_uiRespawnInterval;
    uint                m_uiMaxLevel;

    uiset               m_setActiveBossUIDs;
    uint                m_uiNextRespawnTime;
    uint                m_uiLevel;

    SpawnerArray        m_vSpawnerArray;
    SpawnerUIDArray     m_vSpawnerUIDArray;

public:
    ~CEntityBossController()    {}
    CEntityBossController();

    static void     ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme);
    static void     ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme) {}

    void            ApplyWorldEntity(const CWorldEntity &ent);

    void            Spawn();
    void            GameStart();
    bool            ServerFrameThink();

    GAME_SHARED_API bool    AttemptSpawn();

    void            AddActiveBossUID(uint uiBossUID)    { if (uiBossUID != INVALID_INDEX) m_setActiveBossUIDs.insert(uiBossUID); }
    bool            GetActive()                         { return !m_setActiveBossUIDs.empty(); }
    uint            GetLevel() const                    { return m_uiLevel; }

    virtual CVec4f          GetMapIconColor(CPlayer *pLocalPlayer) const    { return g_bossMapIconColor; }
    virtual float           GetMapIconSize(CPlayer *pLocalPlayer) const     { return g_bossMapIconSize; }
    virtual ResHandle       GetMapIcon(CPlayer *pLocalPlayer) const         { return s_hMinimapIcon; }
    virtual bool            IsVisibleOnMap(CPlayer *pLocalPlayer) const     { return pLocalPlayer ? !HasVisibilityFlags(VIS_SIGHTED(pLocalPlayer->GetTeam())) && !HasVisibilityFlags(VIS_REVEALED(pLocalPlayer->GetTeam())) : true; }
};
//=============================================================================

#endif //__C_ENTITYKONGORCONTROLLER_H__
