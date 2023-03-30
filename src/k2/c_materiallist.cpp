// (C)2005 S2 Games
// c_materiallist.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_materiallist.h"
#include "c_world.h"
#include "c_buffer.h"
#include "c_xmlmanager.h"
#include "c_xmldoc.h"
#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CMaterialList::~CMaterialList
  ====================*/
CMaterialList::~CMaterialList()
{
	Release();
}


/*====================
  CMaterialList::CMaterialList
  ====================*/
CMaterialList::CMaterialList(EWorldComponent eComponent) :
IWorldComponent(eComponent, _T("MaterialList"))
{
}


/*====================
  CMaterialList::Load
  ====================*/
bool	CMaterialList::Load(CArchive &archive, const CWorld *pWorld)
{
	PROFILE("CMaterialList::Load");

	try
	{
		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("Invalid CWorld"));

		CFileHandle	hMaterialList(m_sName, FILE_READ | FILE_BINARY, archive);
		if (!hMaterialList.IsOpen())
			EX_ERROR(_T("No MaterialList found in archive"));

		return XMLManager.Process(hMaterialList, _T("materiallist"), this);
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMaterialList::Load() - "), NO_THROW);
		return false;
	}
}


/*====================
  CMaterialList::Generate
  ====================*/
bool	CMaterialList::Generate(const CWorld *pWorld)
{
	PROFILE("CMaterialList::Generate");

	try
	{
		Release();

		m_pWorld = pWorld;
		if (m_pWorld == NULL)
			EX_ERROR(_T("Invalid CWorld"));

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMaterialList::Generate() - "), NO_THROW);
		return false;
	}
}


/*====================
  CMaterialList::Serialize
  ====================*/
bool	CMaterialList::Serialize(IBuffer *pBuffer)
{
	CXMLDoc xmlMaterialList;
	xmlMaterialList.NewNode("materiallist");
	for (MaterialHandleMap::iterator it(m_mapMaterials.begin()); it != m_mapMaterials.end(); ++it)
	{
		xmlMaterialList.NewNode("material");
		xmlMaterialList.AddProperty("name", g_ResourceManager.GetPath(it->second.hMaterial));
		xmlMaterialList.AddProperty("id", it->second.uiID);
		xmlMaterialList.EndNode();
	}
	xmlMaterialList.EndNode();
	pBuffer->Clear();
	pBuffer->Write(xmlMaterialList.GetBuffer()->Get(), xmlMaterialList.GetBuffer()->GetLength());

	if (pBuffer->GetFaults())
		return false;

	return true;
}


/*====================
  CMaterialList::Release
  ====================*/
void	CMaterialList::Release()
{
	m_pWorld = NULL;

	m_mapMaterials.clear();
	m_mapResHandles.clear();
}


/*====================
  CMaterialList::AddMaterial
  ====================*/
uint	CMaterialList::AddMaterial(ResHandle hMaterial)
{
	try
	{
		if (g_ResourceManager.Get(hMaterial) == NULL)
			EX_ERROR(_T("Invalid resource"));

		MaterialHandleMap::iterator findit(m_mapResHandles.find(hMaterial));
		if (findit != m_mapResHandles.end())
			return findit->second.uiID;

		SMaterialListEntry newEntry;
		newEntry.uiID = uint(m_mapMaterials.size());
		newEntry.hMaterial = hMaterial;

		m_mapMaterials[newEntry.uiID] = newEntry;
		m_mapResHandles[hMaterial] = newEntry;
		return newEntry.uiID;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMaterialList::AddMaterial() - "), NO_THROW);
		return 0;
	}
}


/*====================
  CMaterialList::GetMaterialHandle
  ====================*/
ResHandle	CMaterialList::GetMaterialHandle(uint uiID)
{
	try
	{
		MaterialIDMap::iterator findit(m_mapMaterials.find(uiID));
		if (findit == m_mapMaterials.end())
			EX_ERROR(_T("Invalid material id: ") + XtoA(uiID));

		return findit->second.hMaterial;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMaterialList::GetMaterialHandle() - "), NO_THROW);
		return m_mapMaterials[0].hMaterial;
	}
}


/*====================
  CMaterialList::GetMaterialID
  ====================*/
uint	CMaterialList::GetMaterialID(ResHandle hMaterial)
{
	try
	{
		MaterialHandleMap::iterator findit(m_mapResHandles.find(hMaterial));
		if (findit == m_mapResHandles.end())
			EX_ERROR(_T("Invalid resource handle: ") + XtoA(hMaterial));

		return findit->second.uiID;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CMaterialList::GetMaterialID() - "), NO_THROW);
		return 0;
	}
}


/*====================
  CMaterialList::AddMaterial
  ====================*/
void	CMaterialList::AddMaterial(uint uiID, const tstring &sMaterial)
{
	ResHandle	hMaterial = g_ResourceManager.Register(sMaterial, RES_MATERIAL);

	SMaterialListEntry newEntry;
	newEntry.uiID = uiID;
	newEntry.hMaterial = hMaterial;

	m_mapMaterials[uiID] = newEntry;
	m_mapResHandles[hMaterial] = newEntry;
}
