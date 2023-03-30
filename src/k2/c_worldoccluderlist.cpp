// (C)2006 S2 Games
// c_worldoccluderlist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_worldoccluderlist.h"
#include "c_occluder.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CWorldOccluderList::~CWorldOccluderList
  ====================*/
CWorldOccluderList::~CWorldOccluderList()
{
    Release();
}


/*====================
  CWorldOccluderList::CWorldOccluderList
  ====================*/
CWorldOccluderList::CWorldOccluderList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("WorldOccluderList"))
{
}


/*====================
  CWorldOccluderList::Load
  ====================*/
bool    CWorldOccluderList::Load(CArchive &archive, const CWorld *pWorld)
{
    try
    {
        CFileHandle hOccluderList(m_sName, FILE_READ | FILE_BINARY, archive);
        if (!hOccluderList.IsOpen())
            EX_ERROR(_T("Couldn't open file"));

        XMLManager.Process(hOccluderList, _T("occluderlist"), this);
        m_bChanged = true;
        return true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldOccluderList::Load() - "), NO_THROW);
        return false;
    }
}


/*====================
  CWorldOccluderList::Generate
  ====================*/
bool    CWorldOccluderList::Generate(const CWorld *pWorld)
{
    return true;
}


/*====================
  CWorldOccluderList::Release
  ====================*/
void    CWorldOccluderList::Release()
{
    for (OccluderMap_it it(m_mapOccluders.begin()); it != m_mapOccluders.end(); ++it)
    {
        if (it->second != NULL)
            K2_DELETE(it->second);
    }

    m_mapOccluders.clear();
}


/*====================
  CWorldOccluderList::Serialize
  ====================*/
bool    CWorldOccluderList::Serialize(IBuffer *pBuffer)
{
    CXMLDoc xml;
    xml.NewNode("occluderlist");
    for (OccluderMap_it it(m_mapOccluders.begin()); it != m_mapOccluders.end(); ++it)
    {
        xml.NewNode("occluder");
        xml.AddProperty("index", it->first);
        for (uint n(0); n < it->second->GetNumPoints(); ++n)
        {
            xml.NewNode("point");
                xml.AddProperty("position", it->second->GetPoint(n));
            xml.EndNode();
        }
        xml.EndNode();
    }
    xml.EndNode();

    pBuffer->Clear();
    pBuffer->Write(xml.GetBuffer()->Get(), xml.GetBuffer()->GetLength());
    return true;
}


/*====================
  CWorldOccluderList::AllocateNewOccluder
  ====================*/
uint    CWorldOccluderList::AllocateNewOccluder(uint uiIndex)
{
    try
    {
        if (uiIndex == INVALID_INDEX)
        {
            uiIndex = 0;
            OccluderMap_it findit(m_mapOccluders.find(uiIndex));
            while (findit != m_mapOccluders.end() && uiIndex != INVALID_INDEX)
                findit = m_mapOccluders.find(++uiIndex);
        }
        else
        {
            OccluderMap_it findit(m_mapOccluders.find(uiIndex));
            if (findit != m_mapOccluders.end())
            {
                Console.Warn << _T("Overwriting occluder #") << uiIndex << newl;
                if (findit->second != NULL)
                    K2_DELETE(findit->second);
                m_mapOccluders.erase(findit);
            }
        }

        if (uiIndex == INVALID_INDEX)
            EX_ERROR(_T("No available index for new occluder"));

        COccluder *pNewOccluder(K2_NEW(ctx_World,  COccluder));
        if (pNewOccluder == NULL)
            EX_ERROR(_T("Failed to allocate new occluder"));

        m_mapOccluders[uiIndex] = pNewOccluder;
        m_bChanged = true;
        return uiIndex;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldOccluderList::AllocateNewEntity() - "));
        return INVALID_INDEX;
    }
}


/*====================
  CWorldOccluderList::GetOccluder
  ====================*/
COccluder*  CWorldOccluderList::GetOccluder(uint uiIndex, bool bThrow)
{
    try
    {
        OccluderMap_it findit(m_mapOccluders.find(uiIndex));
        if (findit == m_mapOccluders.end())
            EX_ERROR(_T("Occluder with index ") + XtoA(uiIndex) + _T(" not found"));

        return findit->second;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldOccluderList::GetEntity() - "), bThrow);
        return NULL;
    }
}


/*====================
  CWorldOccluderList::DeleteOccluder
  ====================*/
void    CWorldOccluderList::DeleteOccluder(uint uiIndex)
{
    try
    {
        OccluderMap_it findit(m_mapOccluders.find(uiIndex));
        if (findit == m_mapOccluders.end())
            EX_WARN(_T("Occluder with index") + XtoA(uiIndex) + _T(" not found"));

        K2_DELETE(findit->second);
        m_mapOccluders.erase(findit);
        m_bChanged = true;
    }
    catch (CException &ex)
    {
        ex.Process(_T("CWorldOccluderList::DeleteOccluder() - "));
    }
}
