// (C)2006 S2 Games
// c_cliententitydirectory.h
//
//=============================================================================
#ifndef __C_CLIENTENTITY_DIRECTORY__
#define __C_CLIENTENTITY_DIRECTORY__

//=============================================================================
// Headers
//=============================================================================
#include "../game_shared/i_entitydirectory.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IGame;
class CClientEntity;
class IGameEntity;
//=============================================================================

//=============================================================================
// CClientEntityDirectory
//=============================================================================
class CClientEntityDirectory : public IEntityDirectory
{
public:
    typedef map<uint, IGameEntity*>     EntMap;
    typedef EntMap::iterator            EntMap_it;

    typedef map<uint, CClientEntity*>   ClientEntMap;
    typedef ClientEntMap::iterator      ClientEntMap_it;

private:
    EntMap          m_mapEntities;
    ClientEntMap    m_mapClientEntities;

public:
    ~CClientEntityDirectory()   { Clear(); }
    CClientEntityDirectory()    {}

    void            Clear();

    IGameEntity*    Allocate(uint uiIndex, ushort unType);
    IGameEntity*    Allocate(uint uiIndex, const tstring &sName);
    IGameEntity*    AllocateLocal(ushort unType);
    IGameEntity*    AllocateLocal(const tstring &sName);
    IGameEntity*    Reallocate(uint uiIndex, ushort unType);

    bool            IsLocalEntity(uint uiIndex)     { return (uiIndex >= 0x10000); }
    
    void            Delete(uint uiIndex);
    void            Delete(CClientEntity *pEntity)  { if (pEntity != NULL) Delete(pEntity->GetIndex()); }
    void            Delete(IGameEntity *pEntity)    { if (pEntity != NULL) Delete(pEntity->GetIndex()); }

    CClientEntity*          GetClientEntity(uint uiIndex);
    IVisualEntity*          GetClientEntityCurrent(uint uiIndex);
    IVisualEntity*          GetClientEntityPrev(uint uiIndex);
    IVisualEntity*          GetClientEntityNext(uint uiIndex);
    IGameEntity*            GetEntityNext(uint uiIndex);
    
    virtual IGameEntity*    GetEntity(uint uiIndex);
    virtual IPlayerEntity*  GetPlayerEntityFromClientID(int iClientNum);
    virtual IGameEntity*    GetFirstEntity();
    virtual IGameEntity*    GetNextEntity(IGameEntity *pEntity);

    void            PrepForSnapshot();
    void            CleanupEntities();
    void            Frame(float fLerp);
    void            PopulateScene();
    void            DrawScreen();

    ClientEntMap&           GetEntMap() { return m_mapClientEntities; }
};
//=============================================================================

#endif //__C_CLIENTENTITY_DIRECTORY__
