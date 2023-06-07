// (C)2006 S2 Games
// c_serverentitydirectory.h
//
//=============================================================================
#ifndef __C_SERVERENTITYDIRECTORY_H__
#define __C_SERVERENTITYDIRECTORY_H__

//=============================================================================
// Headers
//=============================================================================
#include "../game_shared/i_entitydirectory.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class CSnapshot;
//=============================================================================

//=============================================================================
// CServerEntityDirectory
//=============================================================================
class CServerEntityDirectory : public IEntityDirectory
{
    typedef map<uint, IGameEntity *>    EntUIDMap;
    typedef EntUIDMap::iterator         EntUIDMap_it;

private:
    uint    m_uiNextUniqueID;
    uint    m_uiLastGameIndex;

    EntMap      m_mapEntities;
    EntUIDMap   m_mapUniqueIDs;
    EntMap      m_mapNamedEntities;

    uint    GetNewEntIndex(uint uiMinIndex);

public:
    ~CServerEntityDirectory();
    CServerEntityDirectory();

    void            Clear();
    IGameEntity*    Allocate(ushort unType, uint uiMinIndex = INVALID_INDEX);
    IGameEntity*    Allocate(const tstring &sName, uint uiMinIndex = INVALID_INDEX)     { return Allocate(EntityRegistry.LookupID(sName), uiMinIndex); }
    void            Delete(uint uiIndex);

    IGameEntity*    GetEntity(uint uiIndex);
    IPlayerEntity*  GetPlayerEntityFromClientID(int iClientNum);
    IGameEntity*    GetEntityFromUniqueID(uint uiUniqueID);
    uint            GetGameIndexFromUniqueID(uint uiUniqueID);
    IGameEntity*    GetFirstEntity();
    IGameEntity*    GetNextEntity(IGameEntity *pEntity);

    IVisualEntity*  GetEntityFromName(const tstring &sName);
    IVisualEntity*  GetNextEntityFromName(IVisualEntity *pEntity);

    void            GetSnapshot(CSnapshot &snapshot);
    void            WarmupStart();
    void            GameStart();
    void            Spawn();
    void            Frame();
    void            BackgroundFrame();
    void            Reset();

    uint            GetNumEntities() const      { return uint(m_mapEntities.size()); }
};
//=============================================================================

#endif //__C_SERVERENTITYDIRECTORY_H__
