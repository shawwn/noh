// (C)2006 S2 Games
// c_serverentitydirectory.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_server_common.h"

#include "c_serverentitydirectory.h"
#include "c_gameserver.h"

#include "../hon_shared/i_bitentity.h"
#include "../hon_shared/i_entitytool.h"

#include "../k2/c_snapshot.h"
#include "../k2/c_stateblock.h"
#include "../k2/c_host.h"
#include "../k2/c_eventmanager.h"
//=============================================================================

/*====================
  CServerEntityDirectory::~CServerEntityDirectory
  ====================*/
CServerEntityDirectory::~CServerEntityDirectory()
{
    EntMap_it it(m_mapEntities.begin());
    while (it != m_mapEntities.end())
    {
        m_mapUniqueIDs.erase(it->second->GetUniqueID());
        
        SAFE_DELETE(it->second);
        STL_ERASE(m_mapEntities, it);
    }
}


/*====================
  CServerEntityDirectory::CServerEntityDirectory
  ====================*/
CServerEntityDirectory::CServerEntityDirectory() :
m_uiNextUniqueID(0),
m_uiLastGameIndex(0)
{
    m_itLastEntity = m_mapEntities.end();
    m_vAvailableIndexes.resize(1024, uint(-1));
}


/*====================
  CServerEntityDirectory::GetNewEntIndex
  ====================*/
uint    CServerEntityDirectory::GetNewEntIndex(uint uiMinIndex)
{
    PROFILE("CServerEntityDirectory::GetNewEntIndex");

    uint uiIndex(uiMinIndex == INVALID_INDEX ? m_uiLastGameIndex + 1 : MIN(MAX(uiMinIndex, m_uiLastGameIndex + 1), 0x6FFFu));

    while ((m_vAvailableIndexes[uiIndex >> 5] & (1 << (uiIndex & 31))) == 0)
        ++uiIndex;

    if (uiIndex >= 0x7FFF)
    {
        uiIndex = INVALID_INDEX;
        m_uiLastGameIndex = 0;
    }
    else if (uiIndex > 0x6FFF)
    {
        m_uiLastGameIndex = 0;
    }
    else
    {
        m_uiLastGameIndex = uiIndex;
    }

    return uiIndex;
}


/*====================
  CServerEntityDirectory::Clear
  ====================*/
void    CServerEntityDirectory::Clear()
{
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
    {
        if (it->second->IsVisual() && !it->second->IsBit())
            it->second->GetAsVisual()->Unlink();
        else if (it->second->IsSlave())
        {
            IUnitEntity *pOwner(it->second->GetAsSlave()->GetOwner());
            if (pOwner != NULL)
                pOwner->SetInventorySlot(it->second->GetAsSlave()->GetSlot(), NULL);
        }
        
        SAFE_DELETE(it->second);
    }

    m_mapEntities.clear();
    m_mapUniqueIDs.clear();
    m_mapFrameEntities.clear();
    m_lUnits.clear();
    m_uiNextUniqueID = 0;
    m_uiLastGameIndex = 0;
    for (int i = 0; i < 3; ++i)
        m_vBitBuffer[i].clear();
    m_vBitEntities.clear();
    m_vTestBitEntVisible[0].clear();
    m_vTestBitEntVisible[1].clear();
    m_itLastEntity = m_mapEntities.end();
    m_vAvailableIndexes.assign(1024, uint(-1));
}


/*====================
  CServerEntityDirectory::Allocate
  ====================*/
IGameEntity*    CServerEntityDirectory::Allocate(ushort unType, uint uiMinIndex)
{
    PROFILE("CServerEntityDirectory::Allocate");

    try
    {
        uint uiIndex(GetNewEntIndex(uiMinIndex));
        if (uiIndex == INVALID_INDEX)
            EX_FATAL(_T("No free game indexes"));
        if (m_mapEntities.find(uiIndex) != m_mapEntities.end())
            EX_ERROR(_T("Entity #") + XtoA(uiIndex) + _T(" is already allocated"));

        IGameEntity *pNewEntity(EntityRegistry.Allocate(unType));
        if (pNewEntity == NULL)
            EX_ERROR(_T("Allocation failed"));

        //Console << _T("Allocated new entity #") << uiIndex << newl;

        CloseIndex(uiIndex);

        uint uiUniqueID(m_uiNextUniqueID++);

        pNewEntity->SetIndex(uiIndex);
        pNewEntity->SetUniqueID(uiUniqueID);
        m_mapEntities[uiIndex] = pNewEntity;
        m_mapUniqueIDs[uiUniqueID] = pNewEntity;

        if (!pNewEntity->IsBit())
            m_mapFrameEntities[pNewEntity->GetIndex()] = pNewEntity;

        if (pNewEntity->IsUnit() && !pNewEntity->IsBit())
            m_lUnits.push_back(pNewEntity->GetAsUnit());

        if (pNewEntity->IsBit())
        {
            IBitEntity *pBit(pNewEntity->GetAsBit());
            if (pBit != NULL)
            {
                m_vBitEntities.push_back(pBit);
                m_vBitEntities.back()->SetBitIndex(uint(m_vBitEntities.size() - 1));

                for (int i = 0; i < 3; ++i)
                {
                    if (m_vBitEntities.size() > m_vBitBuffer[i].size() * 32)
                        m_vBitBuffer[i].resize((m_vBitEntities.size() - 1) / 32 + 1, uint(-1));
                }
            }
        }

        m_itLastEntity = m_mapEntities.end();
        return pNewEntity;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityDirectory::Allocate() - "), NO_THROW);
        return NULL;
    }
}


/*====================
  CServerEntityDirectory::Delete
  ====================*/
void    CServerEntityDirectory::Delete(uint uiIndex)
{
    PROFILE("CServerEntityDirectory::Delete");

    EntMap_it itFind(m_mapEntities.find(uiIndex));
    if (itFind == m_mapEntities.end())
    {
        Console.Warn << _T("Tried to delete entity #") << uiIndex << _T(", which does not exist") << newl;
        return;
    }

    //Console << _T("Deleting entity #") << uiIndex << newl;

    OpenIndex(uiIndex);
    
    if (itFind->second->IsVisual() && !itFind->second->IsBit())
        itFind->second->GetAsVisual()->Unlink();

    m_mapUniqueIDs.erase(itFind->second->GetUniqueID());

    if (!itFind->second->IsBit())
        m_mapFrameEntities.erase(itFind->first);

    if (itFind->second->IsUnit() && !itFind->second->IsBit())
        m_lUnits.remove(itFind->second->GetAsUnit());

    SAFE_DELETE(itFind->second);
    m_mapEntities.erase(itFind);

    m_itLastEntity = m_mapEntities.end();
}


/*====================
  CServerEntityDirectory::AllocateDynamicEntity
  ====================*/
IGameEntity*    CServerEntityDirectory::AllocateDynamicEntity(const tstring &sName, uint uiMinIndex, uint uiBaseType)
{
    PROFILE("CServerEntityDirectory::AllocateDynamicEntity");

    IGameEntity *pNewEntity(NULL);
    try
    {
        pNewEntity = EntityRegistry.AllocateDynamicEntity(sName, uiBaseType);
        if (pNewEntity == NULL)
            EX_ERROR(_T("Allocation failed for ") + SingleQuoteStr(sName));

        uint uiIndex(GetNewEntIndex(uiMinIndex));
        if (uiIndex == INVALID_INDEX)
            EX_FATAL(_T("No free game indexes"));
        if (m_mapEntities.find(uiIndex) != m_mapEntities.end())
            EX_ERROR(_T("Entity #") + XtoA(uiIndex) + _T(" is already allocated"));

        //Console << _T("Allocated new entity #") << uiIndex << newl;

        CloseIndex(uiIndex);

        uint uiUniqueID(m_uiNextUniqueID++);

        pNewEntity->SetIndex(uiIndex);
        pNewEntity->SetUniqueID(uiUniqueID);
        m_mapEntities[uiIndex] = pNewEntity;
        m_mapUniqueIDs[uiUniqueID] = pNewEntity;

        if (!pNewEntity->IsBit())
            m_mapFrameEntities[pNewEntity->GetIndex()] = pNewEntity;

        if (pNewEntity->IsUnit() && !pNewEntity->IsBit())
            m_lUnits.push_back(pNewEntity->GetAsUnit());

        m_itLastEntity = m_mapEntities.end();
        return pNewEntity;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityDirectory::AllocateDynamicEntity() - "), NO_THROW);
        SAFE_DELETE(pNewEntity);
        return NULL;
    }
}

IGameEntity*    CServerEntityDirectory::AllocateDynamicEntity(ushort unTypeID, uint uiMinIndex, uint uiBaseType)
{
    PROFILE("CServerEntityDirectory::AllocateDynamicEntity");

    IGameEntity *pNewEntity(NULL);
    try
    {
        pNewEntity = EntityRegistry.AllocateDynamicEntity(unTypeID, uiBaseType);
        if (pNewEntity == NULL)
            EX_ERROR(_T("Allocation failed for ") + SingleQuoteStr(EntityRegistry.LookupName(unTypeID)));

        uint uiIndex(GetNewEntIndex(uiMinIndex));
        if (uiIndex == INVALID_INDEX)
            EX_FATAL(_T("No free game indexes"));
        if (m_mapEntities.find(uiIndex) != m_mapEntities.end())
            EX_ERROR(_T("Entity #") + XtoA(uiIndex) + _T(" is already allocated"));

        //Console << _T("Allocated new entity #") << uiIndex << newl;

        CloseIndex(uiIndex);

        uint uiUniqueID(m_uiNextUniqueID++);

        pNewEntity->SetIndex(uiIndex);
        pNewEntity->SetUniqueID(uiUniqueID);
        m_mapEntities[uiIndex] = pNewEntity;
        m_mapUniqueIDs[uiUniqueID] = pNewEntity;

        if (!pNewEntity->IsBit())
            m_mapFrameEntities[pNewEntity->GetIndex()] = pNewEntity;

        if (pNewEntity->IsUnit() && !pNewEntity->IsBit())
            m_lUnits.push_back(pNewEntity->GetAsUnit());

        m_itLastEntity = m_mapEntities.end();
        return pNewEntity;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CEntityDirectory::AllocateDynamicEntity() - "), NO_THROW);
        return NULL;
    }
}


/*====================
  CServerEntityDirectory::GetEntity
  ====================*/
IGameEntity*    CServerEntityDirectory::GetEntity(uint uiIndex)
{
    if (uiIndex == INVALID_INDEX)
        return NULL;

    EntMap_it itFind(m_mapEntities.find(uiIndex));
    if (itFind == m_mapEntities.end())
        return NULL;

    return itFind->second;
}


/*====================
  CServerEntityDirectory::GetEntityFromUniqueID
  ====================*/
IGameEntity*    CServerEntityDirectory::GetEntityFromUniqueID(uint uiUniqueID)
{
    if (uiUniqueID == INVALID_INDEX)
        return NULL;

    EntUIDMap_it itFind(m_mapUniqueIDs.find(uiUniqueID));
    if (itFind == m_mapUniqueIDs.end())
        return NULL;

    return itFind->second;
}


/*====================
  CServerEntityDirectory::GetGameIndexFromUniqueID
  ====================*/
uint    CServerEntityDirectory::GetGameIndexFromUniqueID(uint uiUniqueID)
{
    if (uiUniqueID == INVALID_INDEX)
        return INVALID_INDEX;

    EntUIDMap_it itFind(m_mapUniqueIDs.find(uiUniqueID));
    if (itFind == m_mapUniqueIDs.end())
        return INVALID_INDEX;

    return itFind->second->GetIndex();
}


/*====================
  CServerEntityDirectory::GetFirstEntity
  ====================*/
IGameEntity*    CServerEntityDirectory::GetFirstEntity()
{
    if (m_mapEntities.empty())
    {
        m_itLastEntity = m_mapEntities.end();
        return NULL;
    }
    else
    {
        m_itLastEntity = m_mapEntities.begin();
        return m_mapEntities.begin()->second;
    }
}


/*====================
  CServerEntityDirectory::GetNextEntity
  ====================*/
IGameEntity*    CServerEntityDirectory::GetNextEntity(IGameEntity *pEntity)
{
    if (!pEntity)
        return NULL;

    if (m_itLastEntity != m_mapEntities.end() && m_itLastEntity->second == pEntity)
    {
        ++m_itLastEntity;

        if (m_itLastEntity == m_mapEntities.end())
            return NULL;
        else
            return m_itLastEntity->second;
    }

    EntMap_it itFind(m_mapEntities.find(pEntity->GetIndex()));
    if (itFind == m_mapEntities.end())
        return NULL;

    ++itFind;

    if (itFind == m_mapEntities.end())
        return NULL;
    else
        return itFind->second;
}


/*====================
  CServerEntityDirectory::GetEntityFromName
  ====================*/
IVisualEntity*  CServerEntityDirectory::GetEntityFromName(const tstring &sName)
{
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
    {
        if (it->second->IsVisual() && it->second->GetAsVisual()->GetName() == sName)
            return it->second->GetAsVisual();
    }

    return NULL;
}


/*====================
  CServerEntityDirectory::GetNextEntityFromName
  ====================*/
IVisualEntity*  CServerEntityDirectory::GetNextEntityFromName(IVisualEntity *pEntity)
{
    if (!pEntity)
        return NULL;

    EntMap_it itStart(m_mapEntities.find(pEntity->GetIndex()));
    if (itStart == m_mapEntities.end())
        return NULL;

    ++itStart;

    for (EntMap_it it(itStart); it != m_mapEntities.end(); ++it)
    {
        if (it->second->IsVisual() && it->second->GetAsVisual()->GetName() == pEntity->GetName())
            return it->second->GetAsVisual();
    }

    return NULL;
}


/*====================
  CServerEntityDirectory::GetEntities
  ====================*/
void    CServerEntityDirectory::GetEntities(uivector &vResult, ushort unType)
{
    PROFILE("CServerEntityDirectory::GetEntities");

    vResult.clear();

    for (EntMap_it it(m_mapFrameEntities.begin()), itEnd(m_mapFrameEntities.end()); it != itEnd; ++it)
    {
        if (it->second->GetType() == unType)
            vResult.push_back(it->second->GetIndex());
    }
}


/*====================
  CServerEntityDirectory::GetSnapshot
  ====================*/
void    CServerEntityDirectory::GetSnapshot(CSnapshot &snapshot)
{
    snapshot.SetNumStreams(3);

    CTeamInfo *pTeam[2] = {
        Game.GetTeam(1),
        Game.GetTeam(2)
    };

    for (EntMap_it it(m_mapFrameEntities.begin()), itEnd(m_mapFrameEntities.end()); it != itEnd; ++it)
    {
        IGameEntity *pEntity(it->second);

        if (pEntity->IsStatic() || pEntity->IsServerEntity())
            continue;

        bool bTeam1(true);
        bool bTeam2(true);

        if (pEntity->IsVisual())
        {
            IVisualEntity *pVisual(pEntity->GetAsVisual());
            if (pVisual != NULL)
            {
                bTeam1 = pTeam[0]->CanSee(pVisual);
                bTeam2 = pTeam[1]->CanSee(pVisual);
            }
        }
        else if (pEntity->IsTeamInfo())
        {
            CTeamInfo *pTeamInfo(pEntity->GetAsTeamInfo());
            if (pTeamInfo != NULL)
            {
                if (Game.GetGamePhase() < GAME_PHASE_PRE_MATCH)
                {
                    bTeam1 = true;
                    bTeam2 = true;
                }
                else
                {
                    if (pTeamInfo->GetTeamID() == 1)
                    {
                        bTeam1 = true;
                        bTeam2 = false;
                    }
                    else if (pTeamInfo->GetTeamID() == 2)
                    {
                        bTeam1 = false;
                        bTeam2 = true;
                    }
                    else
                    {
                        bTeam1 = false;
                        bTeam2 = false;
                    }
                }
            }
        }
        else if (pEntity->IsTool())
        {
            IEntityTool *pTool(pEntity->GetAsTool());
            if (pTool != NULL)
            {
                IUnitEntity *pOwner(pTool->GetOwner());
                if (pOwner != NULL)
                {
                    if (pOwner->GetTeam() == 1)
                    {
                        bTeam1 = true;
                        bTeam2 = false;
                    }
                    else if (pOwner->GetTeam() == 2)
                    {
                        bTeam1 = false;
                        bTeam2 = true;
                    }
                    else
                    {
                        bTeam1 = false;
                        bTeam2 = false;
                    }
                }
            }
        }
        else if (pEntity->IsPlayer())
        {
            const CPlayer *pPlayer(pEntity->GetAsPlayer());
            if (pPlayer != NULL)
            {
                // Single draft is blind pick, so we don't want to show enemy info in hero selection
                const CGameInfo *pGameInfo(Game.GetGameInfo());
                if (pGameInfo != NULL && pGameInfo->GetGameMode() == GAME_MODE_SINGLE_DRAFT && Game.GetGamePhase() == GAME_PHASE_HERO_SELECT)
                {
                    if (pPlayer->GetTeam() == 1)
                        bTeam2 = false;
                    else if (pPlayer->GetTeam() == 2)
                        bTeam1 = false;
                    else
                    {
                        bTeam1 = false;
                        bTeam2 = false;
                    }
                }
            }
        }
        
        {
            PoolHandle hPoolHandle(snapshot.PushNewEntity(pEntity->GetIndex()));
            uint uiIndex(pEntity->GetIndex());

            CEntitySnapshot *pEntitySnapshot(CEntitySnapshot::GetByHandle(hPoolHandle));

            const SEntityDesc *pTypeDesc(pEntity->GetTypeDesc());

            // 'Header' data, this is read outside of each entities ReadUpdate function
            pEntitySnapshot->SetIndex(uiIndex);
            pEntitySnapshot->SetType(pEntity->GetType());
            pEntitySnapshot->SetFieldTypes(pTypeDesc->pFieldTypes, pTypeDesc->uiSize);
            pEntitySnapshot->SetBaseline(pTypeDesc->pBaseline);
            pEntitySnapshot->SetUniqueID(pEntity->GetUniqueID());

            snapshot.AddStreamEntity(0, uiIndex, hPoolHandle);
            
            if (bTeam1)
                snapshot.AddStreamEntity(1, uiIndex, hPoolHandle);
            if (bTeam2)
                snapshot.AddStreamEntity(2, uiIndex, hPoolHandle);
    
            pEntity->GetSnapshot(*pEntitySnapshot, 0);
            pEntitySnapshot->SetAllFields();
        }

        if (!bTeam1 || !bTeam2)
        {
            PoolHandle hPoolHandle(snapshot.PushNewEntity(pEntity->GetIndex()));
            uint uiIndex(pEntity->GetIndex());

            CEntitySnapshot *pEntitySnapshot(CEntitySnapshot::GetByHandle(hPoolHandle));

            const SEntityDesc *pTypeDesc(pEntity->GetTypeDesc());

            // 'Header' data, this is read outside of each entities ReadUpdate function
            pEntitySnapshot->SetIndex(pEntity->GetIndex());
            pEntitySnapshot->SetType(pEntity->GetType());
            pEntitySnapshot->SetFieldTypes(pTypeDesc->pFieldTypes, pTypeDesc->uiSize);
            pEntitySnapshot->SetBaseline(pTypeDesc->pBaseline);
            pEntitySnapshot->SetUniqueID(pEntity->GetUniqueID());

            if (!bTeam1)
                snapshot.AddStreamEntity(1, uiIndex, hPoolHandle);
            if (!bTeam2)
                snapshot.AddStreamEntity(2, uiIndex, hPoolHandle);

            pEntity->GetSnapshot(*pEntitySnapshot, SNAPSHOT_HIDDEN);
            pEntitySnapshot->SetAllFields();
        }
    }

    snapshot.SetStreamBitEntityBuffer(0, m_vBitBuffer[0]);

    // resolve team bit ent visibility
    for (int i = 0; i < 2; ++i)
    {
        for (BitEntSet_it it(m_vTestBitEntVisible[i].begin()); it != m_vTestBitEntVisible[i].end();)
        {
            IBitEntity* pBit(*it);
            if (pBit == NULL)
            {
                STL_ERASE(m_vTestBitEntVisible[i], it);
                continue;
            }

            uint uiBitIndex(pBit->GetBitIndex());

            uint uiGroup(uiBitIndex >> 5);
            uint uiBit(uiBitIndex & 31);

            if (pBit->IsVisual())
            {
                IVisualEntity *pVisual(pBit->GetAsVisual());
                if (pVisual != NULL)
                {
                    pBit->SetVisibilityFlags(Game.GetVision(pBit->GetPosition().x,pBit->GetPosition().y));
                    if (pTeam[i]->CanSee(pVisual))
                    {
                        if (m_vBitBuffer[0][uiGroup] & BIT(uiBit))
                            m_vBitBuffer[i + 1][uiGroup] |= BIT(uiBit);
                        else
                            m_vBitBuffer[i + 1][uiGroup] &= ~BIT(uiBit);

                        STL_ERASE(m_vTestBitEntVisible[i], it);
                        continue;
                    }
                }
            }

            ++it;
        }

        snapshot.SetStreamBitEntityBuffer(i + 1, m_vBitBuffer[i + 1]);
    }
}


/*====================
  CServerEntityDirectory::GameStart
  ====================*/
void    CServerEntityDirectory::GameStart()
{
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
        it->second->GameStart();
}


/*====================
  CServerEntityDirectory::MatchStart
  ====================*/
void    CServerEntityDirectory::MatchStart()
{
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
        it->second->MatchStart();
}


/*====================
  CServerEntityDirectory::FlushStats
  ====================*/
void    CServerEntityDirectory::FlushStats()
{
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
        it->second->FlushStats();
}


/*====================
  CServerEntityDirectory::Spawn
  ====================*/
void    CServerEntityDirectory::Spawn()
{
    IModalDialog::NextLoadingJob();
    IModalDialog::Show(_T("loading"));
    IModalDialog::SetTitle(_T("Spawn Entities"));

    uint uiCount(0);
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
    {
        it->second->Spawn();
        IModalDialog::SetProgress(uiCount / float(m_mapEntities.size()));
        IModalDialog::Update();
        ++uiCount;
    }

    IModalDialog::Hide();
}


/*====================
  CServerEntityDirectory::BackgroundFrame
  ====================*/
void    CServerEntityDirectory::BackgroundFrame()
{
    PROFILE("CServerEntityDirectory::BackgroundFrame");

    uiset setRelease;
    for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
    {
        if (it->second->GetDelete())
            setRelease.insert(it->first);
    }

    for (uiset_it it(setRelease.begin()); it != setRelease.end(); ++it)
        Delete(*it);
}


#define SERVER_FRAME_STEP(step) \
void    CServerEntityDirectory::Frame##step() \
{ \
    GAME_PROFILE(_T("CServerEntityDirectory::Frame") _T(#step)); \
    static uivector vRelease; \
    vRelease.clear(); \
    \
    for (EntMap_it it(m_mapFrameEntities.begin()), itEnd(m_mapFrameEntities.end()); it != itEnd; ++it) \
    { \
        if (it->second->GetDelete()) \
        { \
            vRelease.push_back(it->second->GetIndex()); \
            continue; \
        } \
\
        if (!it->second->ServerFrame##step()) \
        { \
            assert(!it->second->IsState()); \
            it->second->SetDelete(true); \
        } \
\
        if (it->second->GetDelete()) \
            vRelease.push_back(it->second->GetIndex()); \
    } \
    for (uivector_it it(vRelease.begin()); it != vRelease.end(); ++it) \
        Delete(*it); \
}

SERVER_FRAME_STEP(Setup)
SERVER_FRAME_STEP(Think)
SERVER_FRAME_STEP(Movement)
SERVER_FRAME_STEP(Action)
SERVER_FRAME_STEP(Cleanup)


/*====================
  CServerEntityDirectory::WriteBitEntityMap
  ====================*/
void    CServerEntityDirectory::WriteBitEntityMap(CStateBlock &block)
{
    block.Clear();

    BitEntVector_it itEnd(m_vBitEntities.end());
    for (BitEntVector_it it(m_vBitEntities.begin()); it != itEnd; ++it)
        block << ushort((*it)->GetIndex()) << ushort((*it)->GetWorldIndex());
}


/*====================
  CServerEntityDirectory::ActivateBitEntity
  ====================*/
void    CServerEntityDirectory::ActivateBitEntity(uint uiIndex)
{
    IGameEntity *pEntity(GetEntity(uiIndex));
    if (pEntity == NULL)
        return;

    IBitEntity *pBit(pEntity->GetAsBit());
    if (pBit == NULL)
        return;

    uint uiBitIndex(pBit->GetBitIndex());

    uint uiGroup(uiBitIndex >> 5);
    uint uiBit(uiBitIndex & 31);

    if (~m_vBitBuffer[0][uiGroup] & BIT(uiBit))
    {
        m_vBitEntities[uiBitIndex]->Activate();
        m_vBitBuffer[0][uiGroup] |= BIT(uiBit);
        m_vBitBuffer[1][uiGroup] |= BIT(uiBit);
        m_vBitBuffer[2][uiGroup] |= BIT(uiBit);
        m_vTestBitEntVisible[0].erase(m_vBitEntities[uiBitIndex]);
        m_vTestBitEntVisible[1].erase(m_vBitEntities[uiBitIndex]);
    }
}


/*====================
  CServerEntityDirectory::DeactivateBitEntity
  ====================*/
void    CServerEntityDirectory::DeactivateBitEntity(uint uiIndex)
{
    IGameEntity *pEntity(GetEntity(uiIndex));
    if (pEntity == NULL)
        return;

    IBitEntity *pBit(pEntity->GetAsBit());
    if (pBit == NULL)
        return;

    uint uiBitIndex(pBit->GetBitIndex());
    if (m_vBitEntities[uiBitIndex] == NULL)
        return;

    uint uiGroup(uiBitIndex >> 5);
    uint uiBit(uiBitIndex & 31);

    if (m_vBitBuffer[0][uiGroup] & BIT(uiBit))
    {
        pBit->SetVisibilityFlags(Game.GetVision(pBit->GetPosition().x,pBit->GetPosition().y));
        pBit->Deactivate();
        m_vBitBuffer[0][uiGroup] &= ~BIT(uiBit);
    }

    // team bit ent visibility
    for (int i = 0; i < 2; ++i)
    {
        if (m_vBitBuffer[0][uiGroup] != m_vBitBuffer[i+1][uiGroup])
        {
            m_vTestBitEntVisible[i].insert(m_vBitEntities[uiBitIndex]);
        }
    }
}


/*====================
  CServerEntityDirectory::ActivateBitEntities
  ====================*/
void    CServerEntityDirectory::ActivateBitEntities()
{
    uint uiSize(uint(m_vBitBuffer[0].size()));
    uint uiNumBitEntities(uint(m_vBitEntities.size()));
    for (uint uiGroup(0); uiGroup < uiSize; ++uiGroup)
    {
        uint uiOld(m_vBitBuffer[0][uiGroup]);
        uint uiNew(uint(-1));

        if ((uiOld ^ uiNew) == 0)
            continue;

        for (uint uiBit(0); uiBit < 32; ++uiBit)
        {
            uint uiIndex(uiGroup * 32 + uiBit);

            if (uiIndex >= uiNumBitEntities)
                break;

            if ((uiOld ^ uiNew) & BIT(uiBit))
                m_vBitEntities[uiIndex]->Activate();
        }

        m_vBitBuffer[0][uiGroup] = uiNew;
        m_vBitBuffer[1][uiGroup] = uiNew;
        m_vBitBuffer[2][uiGroup] = uiNew;
    }
    m_vTestBitEntVisible[0].clear();
    m_vTestBitEntVisible[1].clear();
}


/*====================
  CServerEntityDirectory::DeactivateBitEntities
  ====================*/
void    CServerEntityDirectory::DeactivateBitEntities()
{
    uint uiSize(uint(m_vBitBuffer[0].size()));
    uint uiNumBitEntities(uint(m_vBitEntities.size()));
    for (uint uiGroup(0); uiGroup < uiSize; ++uiGroup)
    {
        uint uiOld(m_vBitBuffer[0][uiGroup]);
        uint uiNew(0);

        if ((uiOld ^ uiNew) == 0)
            continue;

        for (uint uiBit(0); uiBit < 32; ++uiBit)
        {
            uint uiIndex(uiGroup * 32 + uiBit);

            if (uiIndex >= uiNumBitEntities)
                break;

            if ((uiOld ^ uiNew) & BIT(uiBit))
                m_vBitEntities[uiIndex]->Deactivate();
        }

        m_vBitBuffer[0][uiGroup] = uiNew;
        m_vBitBuffer[1][uiGroup] = uiNew;
        m_vBitBuffer[2][uiGroup] = uiNew;
    }
    m_vTestBitEntVisible[0].clear();
    m_vTestBitEntVisible[1].clear();
}


/*====================
  CServerEntityDirectory::ClearBitEntities
  ====================*/
void    CServerEntityDirectory::ClearBitEntities()
{
    m_vBitBuffer[0].clear();
    m_vBitBuffer[1].clear();
    m_vBitBuffer[2].clear();

    m_vBitEntities.clear();

    m_vTestBitEntVisible[0].clear();
    m_vTestBitEntVisible[1].clear();
}


/*====================
  CServerEntityDirectory::UpdateDefinitions
  ====================*/
void    CServerEntityDirectory::UpdateDefinitions(ushort unType)
{
    bool bPrecached(false);

    for (EntMap_it it(m_mapEntities.begin()), itEnd(m_mapEntities.end()); it != itEnd; ++it)
    {
        if (it->second->GetType() != unType)
            continue;

        it->second->UpdateDefinition();

        if (!bPrecached)
        {
            GameServer.Precache(unType, PRECACHE_ALL, _T("All"));
            bPrecached = true;
        }
    }
}
