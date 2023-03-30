// (C)2006 S2 Games
// c_worldlightlist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_worldlightlist.h"
#include "c_worldlight.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CWorldLightList::~CWorldLightList
  ====================*/
CWorldLightList::~CWorldLightList()
{
	Release();
}


/*====================
  CWorldLightList::CWorldLightList
  ====================*/
CWorldLightList::CWorldLightList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("LightList"))
{
}


/*====================
  CWorldLightList::Load
  ====================*/
bool	CWorldLightList::Load(CArchive &archive, const CWorld *pWorld)
{
	try
	{
		CFileHandle hEntList(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!hEntList.IsOpen())
			EX_ERROR(_T("Couldn't open file"));

		XMLManager.Process(hEntList, _T("lightlist"), this);
		m_bChanged = true;
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorldLightList::Load() - "), NO_THROW);
		return false;
	}
}


/*====================
  CWorldLightList::Generate
  ====================*/
bool	CWorldLightList::Generate(const CWorld *pWorld)
{
	return true;
}


/*====================
  CWorldLightList::Release
  ====================*/
void	CWorldLightList::Release()
{
	for (WorldLightsMap_it it(m_mapLights.begin()); it != m_mapLights.end(); ++it)
	{
		if (it->second != NULL)
			K2_DELETE(it->second);
	}

	m_mapLights.clear();
}


/*====================
  CWorldLightList::Serialize
  ====================*/
bool	CWorldLightList::Serialize(IBuffer *pBuffer)
{
	CXMLDoc xml;
	xml.NewNode("lightlist");
	for (WorldLightsMap_it it(m_mapLights.begin()); it != m_mapLights.end(); ++it)
	{
		xml.NewNode("light");
		xml.AddProperty("index", it->first);
		xml.AddProperty("position", it->second->GetPosition());
		xml.AddProperty("color", it->second->GetColor());
		xml.AddProperty("falloffstart", it->second->GetFalloffStart());
		xml.AddProperty("falloffend", it->second->GetFalloffEnd());
		xml.EndNode();
	}
	xml.EndNode();

	pBuffer->Clear();
	pBuffer->Write(xml.GetBuffer()->Get(), xml.GetBuffer()->GetLength());
	return true;
}


/*====================
  CWorldLightList::AllocateNewLight
  ====================*/
uint	CWorldLightList::AllocateNewLight(uint uiIndex)
{
	try
	{
		if (uiIndex == INVALID_INDEX)
		{
			uiIndex = 0;
			WorldLightsMap_it findit(m_mapLights.find(uiIndex));
			while (findit != m_mapLights.end() && uiIndex != INVALID_INDEX)
				findit = m_mapLights.find(++uiIndex);
		}
		else
		{
			WorldLightsMap_it findit(m_mapLights.find(uiIndex));
			if (findit != m_mapLights.end())
			{
				Console.Warn << _T("Overwriting light #") << uiIndex << newl;
				if (findit->second != NULL)
					K2_DELETE(findit->second);
				m_mapLights.erase(findit);
			}
		}

		if (uiIndex == INVALID_INDEX)
			EX_ERROR(_T("No available index for new light"));

		CWorldLight *pNewLight(K2_NEW(ctx_World,  CWorldLight));
		if (pNewLight == NULL)
			EX_ERROR(_T("Failed to allocate new light"));

		pNewLight->SetIndex(uiIndex);
		m_mapLights[uiIndex] = pNewLight;
		m_bChanged = true;
		return uiIndex;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorldLightList::AllocateNewEntity() - "));
		return INVALID_INDEX;
	}
}


/*====================
  CWorldLightList::GetLight
  ====================*/
CWorldLight*	CWorldLightList::GetLight(uint uiIndex, bool bThrow)
{
	try
	{
		WorldLightsMap_it findit(m_mapLights.find(uiIndex));
		if (findit == m_mapLights.end())
			EX_ERROR(_T("Light with index ") + XtoA(uiIndex) + _T(" not found"));

		return findit->second;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorldLightList::GetEntity() - "), bThrow);
		return NULL;
	}
}


/*====================
  CWorldLightList::DeleteLight
  ====================*/
void	CWorldLightList::DeleteLight(uint uiIndex)
{
	try
	{
		WorldLightsMap_it findit(m_mapLights.find(uiIndex));
		if (findit == m_mapLights.end())
			EX_WARN(_T("Light with index") + XtoA(uiIndex) + _T(" not found"));

		K2_DELETE(findit->second);
		m_mapLights.erase(findit);
		m_bChanged = true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CWorldLightList::DeleteLight() - "));
	}
}
