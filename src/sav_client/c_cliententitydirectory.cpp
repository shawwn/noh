// (C)2006 S2 Games
// c_cliententitydirectory.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_cliententitydirectory.h"
#include "c_cliententity.h"
#include "c_gameclient.h"

#include "../game_shared/c_teaminfo.h"
#include "../game_shared/c_entityclientinfo.h"

#include "../k2/c_scenemanager.h"
#include "../k2/c_fontmap.h"
#include "../k2/c_draw2d.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_BOOLF  (cg_debugEntities,  false,  CONEL_DEV);
//=============================================================================

/*====================
  CClientEntityDirectory::Clear
  ====================*/
void    CClientEntityDirectory::Clear()
{
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
        SAFE_DELETE(it->second);

    m_mapEntities.clear();

    for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
    {
        it->second->GetNextEntity()->Unlink();
        SAFE_DELETE(it->second);
    }

    m_mapClientEntities.clear();
}


/*====================
  CClientEntityDirectory::Allocate
  ====================*/
IGameEntity*    CClientEntityDirectory::Allocate(uint uiIndex, ushort unType)
{
    try
    {
        if (m_mapEntities.find(uiIndex) != m_mapEntities.end())
            EX_ERROR(_T("Entity #") + XtoA(uiIndex) + _T(" is already allocated"));

        if (m_mapClientEntities.find(uiIndex) != m_mapClientEntities.end())
            EX_ERROR(_T("Entity #") + XtoA(uiIndex) + _T(" is already allocated"));

        IGameEntity *pNewEntity(EntityRegistry.Allocate(unType));
        if (pNewEntity == NULL)
            EX_ERROR(_T("Allocation failed"));

        pNewEntity->SetIndex(uiIndex);

        if (pNewEntity->IsVisual())
        {
            if (cg_debugEntities)
                Console << _T("Allocated new client entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(unType)) << newl;

            CClientEntity *pNewClientEntity(K2_NEW(MemManager.GetHeap(HEAP_CLIENT_GAME),   CClientEntity));
            pNewClientEntity->Initialize(pNewEntity->GetAsVisualEnt());
            m_mapClientEntities[uiIndex] = pNewClientEntity;
            
            if (m_mapClientEntities[uiIndex] == NULL)
                EX_ERROR(_T("Allocation failed"));
        }
        else
        {
            if (cg_debugEntities)
                Console << _T("Allocated new entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(unType)) << newl;

            m_mapEntities[uiIndex] = pNewEntity;

            if (m_mapEntities[uiIndex] == NULL)
                EX_ERROR(_T("Allocation failed"));
        }
        
        return pNewEntity;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityDirectory::Allocate() - "), NO_THROW);
        return NULL;
    }
}

IGameEntity*    CClientEntityDirectory::Allocate(uint uiIndex, const tstring &sTypeName)
{
    return Allocate(uiIndex, EntityRegistry.LookupID(sTypeName));
}


/*====================
  CClientEntityDirectory::AllocateLocal
  ====================*/
IGameEntity*    CClientEntityDirectory::AllocateLocal(ushort unType)
{
    uint uiIndex(0x10000);
    while (m_mapEntities.find(uiIndex) != m_mapEntities.end() ||
        m_mapClientEntities.find(uiIndex) != m_mapClientEntities.end())
        ++uiIndex;
    return Allocate(uiIndex, unType);
}

IGameEntity*    CClientEntityDirectory::AllocateLocal(const tstring &sTypeName)
{
    return AllocateLocal(EntityRegistry.LookupID(sTypeName));
}


/*====================
  CClientEntityDirectory::Reallocate
  ====================*/
IGameEntity*    CClientEntityDirectory::Reallocate(uint uiIndex, ushort unType)
{
    try
    {
        CClientEntity *pClientEntity(GetClientEntity(uiIndex));
        if (pClientEntity)
        {
            IGameEntity *pNewEntity(EntityRegistry.Allocate(unType));
            if (pNewEntity == NULL || !pNewEntity->IsVisual())
                EX_WARN(_T("new entity failed to allocate"));

            pClientEntity->Initialize(pNewEntity->GetAsVisualEnt());

            return pNewEntity;
        }
        else
        {
            EntMap_it itFind(m_mapEntities.find(uiIndex));
            if (itFind == m_mapEntities.end())
                return NULL;

            IGameEntity *pOldEntity(itFind->second);
            if (pOldEntity == NULL)
                EX_WARN(_T("Invalid entity index"));

            IGameEntity *pNewEntity(EntityRegistry.Allocate(unType));
            if (pNewEntity == NULL)
                EX_WARN(_T("new entity failed to allocate"));
            
            pNewEntity->SetIndex(pOldEntity->GetIndex());

            m_mapEntities[uiIndex] = pNewEntity;
            K2_DELETE(pOldEntity);

            return pNewEntity;
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityDirectory::Reallocate() - "), NO_THROW);
        return NULL;
    }
}


/*====================
  CClientEntityDirectory::Delete
  ====================*/
void    CClientEntityDirectory::Delete(uint uiIndex)
{
    EntMap_it itFind(m_mapEntities.find(uiIndex));
    ClientEntMap_it itFind2(m_mapClientEntities.find(uiIndex));
    if (itFind == m_mapEntities.end() && itFind2 == m_mapClientEntities.end())
    {
        Console.Warn << _T("Tried to delete entity #") << uiIndex << _T(", which does not exist") << newl;
        return;
    }

    if (itFind2 != m_mapClientEntities.end())
    {
        itFind2->second->PassEffects();

        if (cg_debugEntities)
            Console << _T("Deleting client entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(itFind2->second->GetType())) << newl;

        itFind2->second->GetNextEntity()->Unlink();

        K2_DELETE(itFind2->second);
        m_mapClientEntities.erase(itFind2);
    }
    else if (itFind != m_mapEntities.end())
    {
        if (cg_debugEntities)
            Console << _T("Deleting entity #") << uiIndex << _T(" ") << ParenStr(EntityRegistry.LookupName(itFind->second->GetType())) << newl;

        // Update attached entity states and inventory
        if (itFind->second->IsState())
        {
            IEntityState *pState(itFind->second->GetAsState());

            CClientEntity *pOwner(GetClientEntity(pState->GetOwner()));
            if (pOwner)
            {
                pOwner->GetNextEntity()->ClearState(pState);
                pOwner->GetPrevEntity()->ClearState(pState);
                pOwner->GetCurrentEntity()->ClearState(pState);
            }
        }
        else if (itFind->second->IsInventoryItem())
        {
            IInventoryItem *pItem(itFind->second->GetAsInventoryItem());

            CClientEntity *pOwner(GetClientEntity(pItem->GetOwner()));
            if (pOwner)
            {
                pOwner->GetNextEntity()->SetInventorySlot(pItem->GetSlot(), NULL);
                pOwner->GetPrevEntity()->SetInventorySlot(pItem->GetSlot(), NULL);
                pOwner->GetCurrentEntity()->SetInventorySlot(pItem->GetSlot(), NULL);
            }
        }

        K2_DELETE(itFind->second);
        m_mapEntities.erase(itFind);
    }
}


/*====================
  CClientEntityDirectory::GetClientEntity
  ====================*/
CClientEntity*  CClientEntityDirectory::GetClientEntity(uint uiIndex)
{
    ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
    if (itFind == m_mapClientEntities.end())
        return NULL;

    return itFind->second;
}


/*====================
  CClientEntityDirectory::GetClientEntityCurrent
  ====================*/
IVisualEntity*  CClientEntityDirectory::GetClientEntityCurrent(uint uiIndex)
{
    ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
    if (itFind == m_mapClientEntities.end())
        return NULL;

    return itFind->second->GetCurrentEntity();
}


/*====================
  CClientEntityDirectory::GetClientEntityPrev
  ====================*/
IVisualEntity*  CClientEntityDirectory::GetClientEntityPrev(uint uiIndex)
{
    ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
    if (itFind == m_mapClientEntities.end())
        return NULL;

    return itFind->second->GetPrevEntity();
}


/*====================
  CClientEntityDirectory::GetClientEntityNext
  ====================*/
IVisualEntity*  CClientEntityDirectory::GetClientEntityNext(uint uiIndex)
{
    ClientEntMap_it itFind(m_mapClientEntities.find(uiIndex));
    if (itFind == m_mapClientEntities.end())
        return NULL;

    return itFind->second->GetNextEntity();
}


/*====================
  CClientEntityDirectory::GetEntity
  ====================*/
IGameEntity*    CClientEntityDirectory::GetEntity(uint uiIndex)
{
    CClientEntity *pEnt(GetClientEntity(uiIndex));
    if (pEnt)
        return pEnt->GetCurrentEntity();

    EntMap_it itFind(m_mapEntities.find(uiIndex));
    if (itFind != m_mapEntities.end())
        return itFind->second;

    return NULL;
}


/*====================
  CClientEntityDirectory::GetEntityNext
  ====================*/
IGameEntity*    CClientEntityDirectory::GetEntityNext(uint uiIndex)
{
    CClientEntity *pEnt(GetClientEntity(uiIndex));
    if (pEnt)
        return pEnt->GetNextEntity();

    EntMap_it itFind(m_mapEntities.find(uiIndex));
    if (itFind != m_mapEntities.end())
        return itFind->second;

    return NULL;
}



/*====================
  CClientEntityDirectory::GetPlayerEntityFromClientID
  ====================*/
IPlayerEntity*  CClientEntityDirectory::GetPlayerEntityFromClientID(int iClientNum)
{
    if (iClientNum == -1)
        return NULL;

    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
    {
        if (it->second->GetType() == Entity_ClientInfo)
        {
            CEntityClientInfo *pClient;
            pClient = static_cast<CEntityClientInfo*>(it->second);

            if (pClient->GetClientNumber() == iClientNum)
                return pClient->GetPlayerEntity();
        }
    }

    return NULL;
}


/*====================
  CClientEntityDirectory::GetFirstEntity
  ====================*/
IGameEntity*    CClientEntityDirectory::GetFirstEntity()
{
    ClientEntMap_it itFind(m_mapClientEntities.begin());
    if (itFind == m_mapClientEntities.end())
        return NULL;

    return itFind->second->GetNextEntity();
}


/*====================
  CClientEntityDirectory::GetNextEntity
  ====================*/
IGameEntity*    CClientEntityDirectory::GetNextEntity(IGameEntity *pEntity)
{
    if (!pEntity)
        return NULL;

    ClientEntMap_it itFind(m_mapClientEntities.find(pEntity->GetIndex()));
    if (itFind == m_mapClientEntities.end())
        return NULL;

    ++itFind;

    if (itFind == m_mapClientEntities.end())
        return NULL;

    return itFind->second->GetNextEntity();
}


/*====================
  CClientEntityDirectory::PrepForSnapshot
  ====================*/
void    CClientEntityDirectory::PrepForSnapshot()
{
    PROFILE("CClientEntityDirectory::PrepForSnapshot");

    for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
    {
        CClientEntity &entity(*(it->second));

        if (entity.IsStatic() || IsLocalEntity(it->second->GetIndex()) || it->second->IsClientEntity())
            continue;

        if (entity.GetNextEntity()->GetFrame() != entity.GetPrevEntity()->GetFrame())
            entity.CopyNextToPrev();

        entity.GetNextEntity()->Invalidate();
    }

    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
    {
        if (it->second->IsStatic() || IsLocalEntity(it->second->GetIndex()))
            continue;

        it->second->Invalidate();
    }
}


/*====================
  CClientEntityDirectory::CleanupEntities
  ====================*/
void    CClientEntityDirectory::CleanupEntities()
{
    PROFILE("CClientEntityDirectory::PrepForSnapshot");

    ClientEntMap_it itClientEnt(m_mapClientEntities.begin());
    while (itClientEnt != m_mapClientEntities.end())
    {
        IGameEntity *pEntity(itClientEnt->second->GetNextEntity());

        ++itClientEnt;

        if (!pEntity->IsValid())
            Delete(pEntity->GetIndex());
    }

    EntMap_it itEnt(m_mapEntities.begin());
    while (itEnt != m_mapEntities.end())
    {
        IGameEntity *pEntity(itEnt->second);

        ++itEnt;

        if (!pEntity->IsValid())
        {
            // Make sure we clear out the client entry
            if (pEntity->GetType() == Entity_ClientInfo)
            {
                int iClientNum(static_cast<CEntityClientInfo *>(pEntity)->GetClientNumber());
                GameClient.RemoveClient(iClientNum);
            }

            Delete(pEntity->GetIndex());
        }
    }
}


/*====================
  CClientEntityDirectory::Frame
  ====================*/
void    CClientEntityDirectory::Frame(float fLerp)
{
    PROFILE("CClientEntityDirectory::Frame");

    try
    {
#if 0
        // Verify visual entities
        for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
        {
            CClientEntity *pEntity(it->second);

            if (!pEntity->GetNextEntity())
                EX_FATAL(_TS("Invalid next entity in #") + XtoA(pEntity->GetIndex()));
            if (!pEntity->GetPrevEntity())
                EX_FATAL(_TS("Invalid previous entity in #") + XtoA(pEntity->GetIndex()));
            if (!pEntity->GetCurrentEntity())
                EX_FATAL(_TS("Invalid current entity in #") + XtoA(pEntity->GetIndex()));

            // Verify states
            for (int i(0); i < MAX_ACTIVE_ENTITY_STATES; ++i)
            {
                IEntityState *pState(pEntity->GetNextEntity()->GetState(i));
                if (pState && pState->GetOwner() != pEntity->GetIndex())
                    EX_FATAL(_TS("State owner mismatch in #") + XtoA(pEntity->GetIndex()));
            }
        }
#endif
        uint uiLocalPlayerIndex(GameClient.GetLocalClient() && GameClient.GetLocalClient()->GetPlayerEntity() ? GameClient.GetLocalClient()->GetPlayerEntity()->GetIndex() : INVALID_INDEX);

        // Interpolate visual entities
        for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
        {
            CClientEntity *pEntity(it->second);

            pEntity->GetCurrentEntity()->RemoveClientRenderFlags(ECRF_SNAPSELECTED);

            // Skip local client
            if (pEntity->GetIndex() == uiLocalPlayerIndex)
                continue;

            GameClient.SetCurrentEntity(pEntity);

            if (!pEntity->IsStatic() && !pEntity->IsClientEntity())
                pEntity->Interpolate(fLerp);
        }

        // Client entity frames
        for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
        {
            CClientEntity *pEntity(it->second);

            // Skip local client
            if (pEntity->GetIndex() == uiLocalPlayerIndex)
                continue;

            GameClient.SetCurrentEntity(pEntity);

            pEntity->Frame();
        }

        // Update team rosters
        for (int i(0); i <= 2; ++i)
        {
            CEntityTeamInfo *pTeam(GameClient.GetTeam(i));
            if (pTeam == NULL)
                continue;

            pTeam->UpdateRoster();
        }
    }
    catch (CException &ex)
    {
        ex.Process(_T("CClientEntityDirectory::Frame() -"), NO_THROW);
    }
}


/*====================
  CClientEntityDirectory::PopulateScene
  ====================*/
void    CClientEntityDirectory::PopulateScene()
{
    PROFILE("CClientEntityDirectory::PopulateScene");

    for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
    {
        CClientEntity *pEntity(it->second);
        if (pEntity == NULL)
        {
            Console.Warn << _T("CClientEntityDirectory::PopulateScene() - NULL pointer from: ") << it->first << newl;
            continue;
        }

        GameClient.SetCurrentEntity(pEntity);

        pEntity->AddToScene();
    }
}


/*====================
  CClientEntityDirectory::DrawScreen
  ====================*/
void    CClientEntityDirectory::DrawScreen()
{
    static ResHandle hFont(g_ResourceManager.LookUpName(_T("system"), RES_FONTMAP));
    CFontMap *pFontMap(g_ResourceManager.GetFontMap(hFont));

    for (ClientEntMap_it it(m_mapClientEntities.begin()); it != m_mapClientEntities.end(); ++it)
    {
        CClientEntity *pEntity(it->second);
        if (pEntity == NULL)
            continue;

        IVisualEntity *pVisual(pEntity->GetCurrentEntity());

        int iNumActiveEffectThreads(0);
        for (int i(0); i < NUM_CLIENT_EFFECT_THREADS; ++i)
            if (pEntity->GetEffectThread(i)) ++iNumActiveEffectThreads;

        if (iNumActiveEffectThreads == 0)
            continue;

        tstring sText(XtoA(iNumActiveEffectThreads));

        CVec2f v2ScreenPos;
        if (GameClient.GetCamera()->WorldToScreen(pVisual->GetPosition(), v2ScreenPos))
        {
            CVec2f v2StringPos(ROUND(v2ScreenPos.x - pFontMap->GetStringWidth(sText) / 2.0f), ROUND(v2ScreenPos.y - pFontMap->GetMaxHeight() / 2.0f));

            Draw2D.SetColor(BLACK);
            Draw2D.String(v2StringPos.x + 2, v2StringPos.y + 2, sText, hFont);

            Draw2D.SetColor(ORANGE);
            Draw2D.String(v2StringPos.x, v2StringPos.y, sText, hFont);
        }
    }
}
