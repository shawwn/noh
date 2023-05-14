// (C)2005 S2 Games
// c_clifflist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_cliffsetlist.h"
#include "c_world.h"
#include "c_cliffdefinitionresource.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CCliffList::~CCliffList
  ====================*/
CCliffList::~CCliffList()
{
    Release();
}


/*====================
  CCliffList::CCliffList
  ====================*/
CCliffList::CCliffList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("CliffSetList"))
{
}

/*====================
  CCliffList::Load
  ====================*/
bool    CCliffList::Load(CArchive &archive, const CWorld *pWorld)
{
    /*
    PROFILE("CCliffList::Load");

    try
    {
        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Invalid CWorld"));

        CFileHandle hCliffList(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hCliffList.IsOpen())
            EX_ERROR(_T("No CliffList found in archive"));

        XMLManager.Process(hCliffList, _T("Clifflist"), this);

        m_bChanged = false;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CCliffList::Load() - "), NO_THROW);
        return false;
    }

    return false;
    */
    return true;
}


/*====================
  CCliffList::Generate
  ====================*/
bool    CCliffList::Generate(const CWorld *pWorld)
{
    try
    {
        Release();

        m_pWorld = pWorld;
        if (m_pWorld == nullptr)
            EX_ERROR(_T("Invalid CWorld"));

        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CCliffList::Generate() - "), NO_THROW);
        return false;
    }
}


/*====================
  CCliffList::Serialize
  ====================*/
bool    CCliffList::Serialize(IBuffer *pBuffer)
{
    /*
    CXMLDoc xmlCliffList;
    xmlCliffList.NewNode("Clifflist");
    for (CliffHandleMap::iterator it(m_mapCliffs.begin()); it != m_mapCliffs.end(); ++it)
    {
        xmlCliffList.NewNode("CliffDefinition");
        xmlCliffList.AddProperty("id", it->first);
        xmlCliffList.AddProperty("path", g_ResourceManager.GetPath(it->second));
        xmlCliffList.EndNode();
    }
    xmlCliffList.EndNode();
    pBuffer->Clear();
    pBuffer->Write(xmlCliffList.GetBuffer()->Get(), xmlCliffList.GetBuffer()->GetLength());

    if (pBuffer->GetFaults())
        return false;
    */
    return true;

}


/*====================
  CCliffList::Release
  ====================*/
void    CCliffList::Release()
{
    m_pWorld = nullptr;

    m_mapCliffs.clear();
    m_mapResHandles.clear();
}


/*====================
  CCliffList::AddCliff
  ====================*/
uint    CCliffList::AddCliff(ResHandle hCliff)
{
    try
    {
        CliffHandleMap::iterator findit(m_mapResHandles.find(hCliff));
        if (findit != m_mapResHandles.end())
            return findit->second;

        uint uiID(0);
        while (m_mapCliffs.find(uiID) != m_mapCliffs.end())
            ++uiID;

        m_mapCliffs[uiID] = hCliff;
        m_mapResHandles[hCliff] = uiID;
        return uiID;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CCliffList::AddCliff() - "), NO_THROW);
        return 0;
    }
}


/*====================
  CCliffList::GetCliffHandle
  ====================*/
ResHandle   CCliffList::GetCliffHandle(uint uiID)
{
    try
    {
        CliffIDMap::iterator findit(m_mapCliffs.find(uiID));
        if (findit == m_mapCliffs.end())
            return INVALID_RESOURCE;

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CCliffList::GetCliffHandle() - "), NO_THROW);
        return INVALID_RESOURCE;
    }
}


/*====================
  CCliffList::GetCliffID
  ====================*/
uint    CCliffList::GetCliffID(ResHandle hCliff)
{
    try
    {
        CliffHandleMap::iterator findit(m_mapResHandles.find(hCliff));
        if (findit == m_mapResHandles.end())
            EX_ERROR(_T("Invalid resource handle: ") + XtoA(hCliff));

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CCliffList::GetCliffID() - "), NO_THROW);
        return 0;
    }
}


/*====================
  CCliffList::AddCliff
  ====================*/
void    CCliffList::AddCliff(uint uiID, const tstring &sCliff)
{
    tstring sShortName(Filename_GetName(sCliff));

    ResHandle   hCliff(g_ResourceManager.Register(K2_NEW(ctx_World,  CCliffDefinitionResource)(sCliff), RES_CLIFFDEF));

    m_mapCliffs[uiID] = hCliff;
    m_mapResHandles[hCliff] = uiID;
}


/*====================
  CCliffList::GetCliffIDList
  ====================*/
void    CCliffList::GetCliffIDList(vector<uint> &vuiID)
{
    for (CliffIDMap::iterator it = m_mapCliffs.begin(); it != m_mapCliffs.end(); it++)
        vuiID.push_back((*it).first);
}