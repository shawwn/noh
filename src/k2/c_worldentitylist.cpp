// (C)2005 S2 Games
// c_worldentitylist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_worldentitylist.h"
#include "c_worldentity.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
#include "c_world.h"
#include "c_model.h"
#include "c_skin.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CWorldEntityList::~CWorldEntityList
  ====================*/
CWorldEntityList::~CWorldEntityList()
{
    Release();
}


/*====================
  CWorldEntityList::CWorldEntityList
  ====================*/
CWorldEntityList::CWorldEntityList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("EntityList")),
m_poolWorldEntities(DEFAULT_WORLDENTS),
m_uiMinFreeIndex(0)
{
    m_vEntities.reserve(DEFAULT_WORLDENTS);
}


/*====================
  CWorldEntityList::Load
  ====================*/
bool    CWorldEntityList::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        Release();

        CFileHandle hEntList(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hEntList.IsOpen())
            EX_ERROR(_T("Couldn't open file"));

        XMLManager.Process(hEntList, _T("entitylist"), this);
        m_bChanged = true;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldEntityList::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldEntityList::Generate
  ====================*/
bool    CWorldEntityList::Generate(const CWorld *pWorld)
{
    try
    {
        // TODO: Create a few generic entities, sufficient to load up the world and play in it
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldEntityList::Generate() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldEntityList::Release
  ====================*/
void    CWorldEntityList::Release()
{
    m_poolWorldEntities.Clear();
    m_vEntities.clear();
    m_uiMinFreeIndex = 0;
}


/*====================
  CWorldEntityList::Serialize
  ====================*/
bool    CWorldEntityList::Serialize(IBuffer *pBuffer)
{
    CXMLDoc xml;
    xml.NewNode("entitylist");
    WorldEntList_cit cit(m_vEntities.begin()), citEnd(m_vEntities.end());
    for (; cit != citEnd; ++cit)
    {
        if (*cit == INVALID_POOL_HANDLE)
            continue;

        CWorldEntity *pWorldEnt(m_poolWorldEntities.GetReferenceByHandle(*cit));
        xml.NewNode("entity");

        // Add generic properties
        WEPropertyMap mapProperties(pWorldEnt->GetPropertyMap());
        
        // Update current world entity properties
        mapProperties[_T("index")] = XtoA(pWorldEnt->GetIndex());
        
        mapProperties[_T("seed")] = XtoA(pWorldEnt->GetSeed());
        mapProperties[_T("name")] = XtoA(pWorldEnt->GetName());
        mapProperties[_T("type")] = XtoA(pWorldEnt->GetType());
        mapProperties[_T("team")] = XtoA(pWorldEnt->GetTeam());
        mapProperties[_T("position")] = XtoA(pWorldEnt->GetPosition());
        mapProperties[_T("angles")] = XtoA(pWorldEnt->GetAngles());
        mapProperties[_T("scale")] = XtoA(pWorldEnt->GetScale());
        mapProperties[_T("model")] = XtoA(g_ResourceManager.GetPath(pWorldEnt->GetModelHandle()));
        
        CModel *pModel(g_ResourceManager.GetModel(pWorldEnt->GetModelHandle()));
        if (pModel != NULL)
        {
            IModel *pIModel(pModel->GetModelFile());
            if (pIModel != NULL)
                mapProperties[_T("skin")] = XtoA(pIModel->GetSkin(pWorldEnt->GetSkin())->GetName());
        }
        
        for (WEPropertyMap::const_iterator itP(mapProperties.begin()); itP != mapProperties.end(); ++itP)
            xml.AddProperty(itP->first, itP->second);
        
        xml.EndNode();
    }
    xml.EndNode();

    pBuffer->Clear();
    pBuffer->Write(xml.GetBuffer()->Get(), xml.GetBuffer()->GetLength());
    return true;
}


/*====================
  CWorldEntityList::Restore
  ====================*/
void    CWorldEntityList::Restore(CArchive &archive)
{
    try
    {
        Release();

        CFileHandle hEntList(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hEntList.IsOpen())
            EX_ERROR(_T("Couldn't open file"));

        XMLManager.Process(hEntList, _T("entitylist"), this);
        m_bChanged = true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldEntityList::Restore() - "), NO_THROW);
    }
}


/*====================
  CWorldEntityList::AllocateNewEntity
  ====================*/
uint    CWorldEntityList::AllocateNewEntity(uint uiIndex)
{
    PROFILE("CWorldEntityList::AllocateNewEntity");

    try
    {
        if (uiIndex == INVALID_INDEX)
        {
            m_uiMinFreeIndex = CLAMP<uint>(m_uiMinFreeIndex, 0, uint(m_vEntities.size()));

            uiIndex = m_uiMinFreeIndex;
            WorldEntList_cit cit(m_vEntities.begin() + m_uiMinFreeIndex), citEnd(m_vEntities.end());
            for (; cit != citEnd && *cit != INVALID_POOL_HANDLE; ++cit, ++uiIndex);

            if (cit == citEnd)
            {
                uiIndex = uint(m_vEntities.size());
                m_vEntities.push_back(INVALID_POOL_HANDLE);
            }

            m_uiMinFreeIndex = uiIndex + 1;
        }
        else
        {
            if (uiIndex < m_vEntities.size() && m_vEntities[uiIndex] != INVALID_POOL_HANDLE)
            {
                Console.Warn << _T("Overwriting object #") << uiIndex << newl;
                m_poolWorldEntities.Free(m_vEntities[uiIndex]);
                m_vEntities[uiIndex] = INVALID_POOL_HANDLE;
            }

            while (uiIndex >= m_vEntities.size())
                m_vEntities.push_back(INVALID_POOL_HANDLE);
        }

        PoolHandle hHandle(m_poolWorldEntities.New(CWorldEntity()));
        if (hHandle == INVALID_POOL_HANDLE)
            EX_ERROR(_T("Failed to allocate new entity"));

        // Must set the index after initialization - the operator= of CWorldEntity doesn't copy the world index
        CWorldEntity *pWorldEnt(m_poolWorldEntities.GetReferenceByHandle(hHandle));
        pWorldEnt->SetIndex(uiIndex);

        m_vEntities[uiIndex] = hHandle;
        m_bChanged = true;
        return uiIndex;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldEntityList::AllocateNewEntity() - "));
        return INVALID_INDEX;
    }
}


/*====================
  CWorldEntityList::FreeEntity
  ====================*/
void    CWorldEntityList::FreeEntity(PoolHandle hHandle)
{
    CWorldEntity *pWorldEnt(m_poolWorldEntities.GetReferenceByHandle(hHandle));

    if (pWorldEnt == NULL)
        return;

    uint uiIndex(pWorldEnt->GetIndex());
    if (uiIndex < m_vEntities.size())
    {
        m_vEntities[uiIndex] = INVALID_POOL_HANDLE;

        while (!m_vEntities.empty() && m_vEntities.back() == INVALID_POOL_HANDLE)
            m_vEntities.pop_back();

        m_uiMinFreeIndex = MIN(m_uiMinFreeIndex, uiIndex);
    }

    m_poolWorldEntities.Free(hHandle);
}


/*====================
  CWorldEntityList::DeleteEntity
  ====================*/
void    CWorldEntityList::DeleteEntity(uint uiIndex)
{
    try
    {
        if (uiIndex >= m_vEntities.size() || m_vEntities[uiIndex] == INVALID_POOL_HANDLE)
            EX_WARN(_T("Entity with index: ") + XtoA(uiIndex) + _T(" not found"));

        m_poolWorldEntities.Free(m_vEntities[uiIndex]);
        m_vEntities[uiIndex] = INVALID_POOL_HANDLE;
        m_bChanged = true;

        while (!m_vEntities.empty() && m_vEntities.back() == INVALID_POOL_HANDLE)
            m_vEntities.pop_back();

        m_uiMinFreeIndex = MIN(m_uiMinFreeIndex, uiIndex);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldEntityList::DeleteEntity() - "), NO_THROW);
    }
}


/*====================
  CWorldEntityList::Exists
  ====================*/
bool    CWorldEntityList::Exists(uint uiIndex)
{
    return uiIndex < m_vEntities.size() && m_vEntities[uiIndex] != INVALID_POOL_HANDLE;
}
