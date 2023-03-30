// (C)2006 S2 Games
// c_networkresourcemanager.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_networkresourcemanager.h"
#include "c_statestring.h"
#include "i_resource.h"
#include "c_hostserver.h"
#include "c_hostclient.h"
#include "c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
SINGLETON_INIT(CNetworkResourceManager);

CNetworkResourceManager &g_NetworkResourceManager(*CNetworkResourceManager::GetInstance());
//=============================================================================


/*====================
  CNetworkResourceEntry::RegisterLocal
  ====================*/
tstring     CNetworkResourceEntry::GetPath() const
{
    return m_hResource == INVALID_RESOURCE ? TSNULL : g_ResourceManager.GetPath(m_hResource);
}


/*====================
  CNetworkResourceEntry::RegisterLocal
  ====================*/
EResourceType   CNetworkResourceEntry::GetType() const
{
    return m_hResource == INVALID_RESOURCE ? RES_UNKNOWN : Res_GetType(m_hResource);
}


/*====================
  CNetworkResourceManager::~CNetworkResourceManager
  ====================*/
CNetworkResourceManager::~CNetworkResourceManager()
{
}


/*====================
  CNetworkResourceManager::CNetworkResourceManager
  ====================*/
CNetworkResourceManager::CNetworkResourceManager() :
m_bModified(false),
m_bStringsModified(false)
{
}


/*====================
  CNetworkResourceManager::RegisterLocalResource
  ====================*/
uint    CNetworkResourceManager::RegisterLocalResource(ResHandle hHandle)
{
    try
    {
        // Check to see if this is already registered
        uint uiIndex(0);
        for (NetResourceVector_it itFind(m_vResources.begin()); itFind != m_vResources.end(); ++itFind, ++uiIndex)
        {
            if (itFind->GetHandle() == hHandle)
                return uiIndex;
        }

        m_bModified = true;
        m_vResources.push_back(CNetworkResourceEntry(hHandle));
        
        return uint(m_vResources.size() - 1);
    }
    catch (CException &ex)
    {
        ex.Process(_T("CNetworkResourceManager::RegisterLocalResource() - "));
        return INVALID_INDEX;
    }
}


/*====================
  CNetworkResourceManager::RegisterNetworkResource
  ====================*/
ResHandle   CNetworkResourceManager::RegisterNetworkResource(ResHandle hHandle, uint uiIndex)
{
    assert(hHandle != INVALID_RESOURCE);
    if (hHandle == INVALID_RESOURCE)
        return INVALID_RESOURCE;

    try
    {
        if (m_vResources.size() < uiIndex + 1)
            m_vResources.resize(uiIndex + 1);

        m_vResources[uiIndex].Set(hHandle);

        IResource *pResource(g_ResourceManager.Get(hHandle));
        if (pResource != NULL)
            pResource->SetNetIndex(uiIndex);

        return m_vResources[uiIndex].GetHandle();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CNetworkResourceManager::RegisterNetworkResource() - "));
        return INVALID_RESOURCE;
    }
}


/*====================
  CNetworkResourceManager::GetLocalHandle
  ====================*/
ResHandle   CNetworkResourceManager::GetLocalHandle(uint uiIndex)
{
    try
    {
        if (uiIndex == INVALID_INDEX)
            return INVALID_RESOURCE;

        if (uiIndex >= m_vResources.size())
        {
            Console.Err << _T("Invalid network index: ") << uiIndex << newl;
            return INVALID_RESOURCE;
        }

        return m_vResources[uiIndex].GetHandle();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CNetworkResourceManager::GetLocalHandle() - "));
        return INVALID_RESOURCE;
    }
}


/*====================
  CNetworkResourceManager::GetNetIndex
  ====================*/
uint    CNetworkResourceManager::GetNetIndex(ResHandle hHandle)
{
    try
    {
        if (hHandle == INVALID_RESOURCE)
            return INVALID_INDEX;

        IResource *pResource(g_ResourceManager.Get(hHandle));

        if (!pResource)
            return INVALID_INDEX;

        if (pResource->GetNetIndex() == INVALID_INDEX)
            pResource->SetNetIndex(RegisterLocalResource(hHandle));

        return pResource->GetNetIndex();
    }
    catch (CException &ex)
    {
        ex.Process(_T("CNetworkResourceManager::GetNetIndex() - "));
        return INVALID_RESOURCE;
    }
}


/*====================
  CNetworkResourceManager::GetStateString
  ====================*/
void    CNetworkResourceManager::GetStateString(CStateString &ssResourceList)
{
    uint uiIndex(0);
    for (NetResourceVector_cit cit(m_vResources.begin()); cit != m_vResources.end(); ++cit, ++uiIndex)
        ssResourceList.Set(XtoA(uiIndex), XtoA(cit->GetType(), FMT_PADZERO, 3) + cit->GetPath());
}


/*====================
  CNetworkResourceManager::SetString
  ====================*/
void    CNetworkResourceManager::SetString(ushort unIndex, const tstring &sString, bool bGrow)
{
    if (unIndex >= m_vStrings.size())
    {
        if (!bGrow)
            return;

        m_vStrings.resize(unIndex + 1);
    }

    m_vStrings[unIndex] = sString;
    m_bStringsModified = true;
}


/*====================
  CNetworkResourceManager::GetString
  ====================*/
const tstring&  CNetworkResourceManager::GetString(ushort unIndex)
{
    if (unIndex >= m_vStrings.size())
        return TSNULL;

    return m_vStrings[unIndex];
}


/*====================
  CNetworkResourceManager::ClearStrings
  ====================*/
void    CNetworkResourceManager::ClearStrings()
{
    m_vStrings.clear();
}


/*====================
  CNetworkResourceManager::UpdateEntityStateString
  ====================*/
void    CNetworkResourceManager::UpdateEntityStateString(CStateString &ssStrings) const
{
    uint uiIndex(0);
    for (tsvector_cit cit(m_vStrings.begin()); cit != m_vStrings.end(); ++cit, ++uiIndex)
        ssStrings.Set(XtoA(uiIndex), *cit);
}


/*====================
  UpdateNetworkString
  ====================*/
void    UpdateNetworkString(const string &sStateUTF8, const string &sValueUTF8)
{
    NetworkResourceManager.SetString(AtoI(UTF8ToString(sStateUTF8)), UTF8ToTString(sValueUTF8), true);
}


/*====================
  CNetworkResourceManager::ApplyUpdateFromStateString
  ====================*/
void    CNetworkResourceManager::ApplyUpdateFromStateString(const CStateString &ssStrings)
{
    ssStrings.ForEachState(UpdateNetworkString, false);
}


/*====================
  CNetworkResourceManager::Clear
  ====================*/
void    CNetworkResourceManager::Clear()
{
    m_bModified = true;

    for (NetResourceVector_it it(m_vResources.begin()); it != m_vResources.end(); ++it)
    {
        IResource *pResource(g_ResourceManager.Get(it->GetHandle()));
        if (pResource == NULL)
            continue;

        pResource->SetNetIndex(INVALID_INDEX);
    }

    m_vResources.clear();
}


/*====================
  CNetworkResourceManager::ListResources
  ====================*/
void    CNetworkResourceManager::ListResources() const
{
    Console << _T("Network Resources") << newl
            << _T("---------") << newl;

    for (NetResourceVector::const_iterator it(m_vResources.begin()); it != m_vResources.end(); ++it)
    {
        Console << _T("^c#") << XtoA(uint(it - m_vResources.begin()), FMT_PADZERO, 3)
                << _T("^y ") << it->GetPath();

        Console << newl;
    }
}


/*--------------------
  ListNetworkResources
  --------------------*/
CMD(ListNetworkResources)
{
    NetworkResourceManager.ListResources();
    return true;
}


