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
#include "../game_shared/c_entityclientinfo.h"

#include "../k2/c_snapshot.h"
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
}


/*====================
  CServerEntityDirectory::GetNewEntIndex
  ====================*/
uint	CServerEntityDirectory::GetNewEntIndex(uint uiMinIndex)
{
	uint uiIndex(uiMinIndex == INVALID_INDEX ? m_uiLastGameIndex + 1 : MIN(MAX(uiMinIndex, m_uiLastGameIndex + 1), 0x6FFFu));
	EntMap_it it(m_mapEntities.begin());

	while (it != m_mapEntities.end() && it->first < uiIndex)
		++it;

	while (it != m_mapEntities.end() && it->first == uiIndex)
	{
		++it;
		++uiIndex;
	}

	if (uiIndex >= 0x7FFF)
	{
		uiIndex = INVALID_INDEX;
		m_uiLastGameIndex = uiIndex;
	}
	else if (uiIndex > 0x6FFF)
	{
		m_uiLastGameIndex = uint(-1);
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
void	CServerEntityDirectory::Clear()
{
	EntMap_it it(m_mapEntities.begin());
	while (it != m_mapEntities.end())
	{
		if (it->second->IsVisual())
			it->second->GetAsVisualEnt()->Unlink();
		
		m_mapUniqueIDs.erase(it->second->GetUniqueID());
		
		SAFE_DELETE(it->second);
		STL_ERASE(m_mapEntities, it);
	}

	m_mapEntities.clear();
	m_mapUniqueIDs.clear();
	m_mapNamedEntities.clear();
	m_uiNextUniqueID = 0;
}


/*====================
  CServerEntityDirectory::Allocate
  ====================*/
IGameEntity*	CServerEntityDirectory::Allocate(ushort unType, uint uiMinIndex)
{
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

		uint uiUniqueID(m_uiNextUniqueID++);

		pNewEntity->SetIndex(uiIndex);
		pNewEntity->SetUniqueID(uiUniqueID);
		m_mapEntities[uiIndex] = pNewEntity;
		m_mapUniqueIDs[uiUniqueID] = pNewEntity;
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
void	CServerEntityDirectory::Delete(uint uiIndex)
{
	EntMap_it itFind(m_mapEntities.find(uiIndex));
	if (itFind == m_mapEntities.end())
	{
		Console.Warn << _T("Tried to delete entity #") << uiIndex << _T(", which does not exist") << newl;
		return;
	}

	//Console << _T("Deleting entity #") << uiIndex << newl;
	
	if (itFind->second->IsVisual())
		itFind->second->GetAsVisualEnt()->Unlink();

	m_mapUniqueIDs.erase(itFind->second->GetUniqueID());

	SAFE_DELETE(itFind->second);
	m_mapEntities.erase(itFind);

	TriggerManager.ClearEntityScripts(uiIndex);
}


/*====================
  CServerEntityDirectory::GetEntity
  ====================*/
IGameEntity*	CServerEntityDirectory::GetEntity(uint uiIndex)
{
	if (uiIndex == INVALID_INDEX)
		return NULL;

	EntMap_it itFind(m_mapEntities.find(uiIndex));
	if (itFind == m_mapEntities.end())
		return NULL;

	return itFind->second;
}


/*====================
  CServerEntityDirectory::GetPlayerEntityFromClientID
  ====================*/
IPlayerEntity*	CServerEntityDirectory::GetPlayerEntityFromClientID(int iClientNum)
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
  CServerEntityDirectory::GetEntityFromUniqueID
  ====================*/
IGameEntity*	CServerEntityDirectory::GetEntityFromUniqueID(uint uiUniqueID)
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
uint	CServerEntityDirectory::GetGameIndexFromUniqueID(uint uiUniqueID)
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
IGameEntity*	CServerEntityDirectory::GetFirstEntity()
{
	if (m_mapEntities.empty())
		return NULL;
	else
		return m_mapEntities.begin()->second;
}


/*====================
  CServerEntityDirectory::GetNextEntity
  ====================*/
IGameEntity*	CServerEntityDirectory::GetNextEntity(IGameEntity *pEntity)
{
	if (!pEntity)
		return NULL;

	EntMap_it itFind(m_mapEntities.find(pEntity->GetIndex()));
	if (itFind == m_mapEntities.end())
		return NULL;

	++itFind;

	if (itFind == m_mapEntities.end())
		return NULL;

	return itFind->second;
}


/*====================
  CServerEntityDirectory::GetEntityFromName
  ====================*/
IVisualEntity*	CServerEntityDirectory::GetEntityFromName(const tstring &sName)
{
	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
	{
		if (it->second->IsVisual() && it->second->GetAsVisualEnt()->GetName() == sName)
			return it->second->GetAsVisualEnt();
	}

	return NULL;
}


/*====================
  CServerEntityDirectory::GetNextEntityFromName
  ====================*/
IVisualEntity*	CServerEntityDirectory::GetNextEntityFromName(IVisualEntity *pEntity)
{
	if (!pEntity)
		return NULL;

	EntMap_it itStart(m_mapEntities.find(pEntity->GetIndex()));
	if (itStart == m_mapEntities.end())
		return NULL;

	++itStart;

	for (EntMap_it it(itStart); it != m_mapEntities.end(); ++it)
	{
		if (it->second->IsVisual() && it->second->GetAsVisualEnt()->GetName() == pEntity->GetName())
			return it->second->GetAsVisualEnt();
	}

	return NULL;
}


/*====================
  CServerEntityDirectory::GetSnapshot
  ====================*/
void	CServerEntityDirectory::GetSnapshot(CSnapshot &snapshot)
{
	EntMap_it it(m_mapEntities.begin()), itEnd(m_mapEntities.end());

	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
	{
		IGameEntity &cGameEntity(*(it->second));

		if (cGameEntity.IsStatic())
			continue;

		CEntitySnapshot *pEntitysnapshot(snapshot.PushNewEntity(cGameEntity.GetIndex()));

		// 'Header' data, this is read outside of each entities ReadUpdate function
		pEntitysnapshot->SetIndex(cGameEntity.GetIndex());
		pEntitysnapshot->SetType(cGameEntity.GetType());
		pEntitysnapshot->SetPrivateClient(cGameEntity.GetPrivateClient());
		pEntitysnapshot->SetFieldTypes(EntityRegistry.GetTypeVector(cGameEntity.GetType()));
		pEntitysnapshot->SetBaseline(EntityRegistry.GetBaseline(cGameEntity.GetType()));
		pEntitysnapshot->SetUniqueID(cGameEntity.GetUniqueID());
		
		cGameEntity.GetSnapshot(*pEntitysnapshot);
		pEntitysnapshot->SetAllFields();
	}
}


/*====================
  CServerEntityDirectory::WarmupStart
  ====================*/
void	CServerEntityDirectory::WarmupStart()
{
	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		it->second->WarmupStart();
}



/*====================
  CServerEntityDirectory::GameStart
  ====================*/
void	CServerEntityDirectory::GameStart()
{
	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		it->second->GameStart();
}


/*====================
  CServerEntityDirectory::Spawn
  ====================*/
void	CServerEntityDirectory::Spawn()
{
	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		it->second->Spawn();
}


/*====================
  CServerEntityDirectory::BackgroundFrame
  ====================*/
void	CServerEntityDirectory::BackgroundFrame()
{
	PROFILE("CServerEntityDirectory::BackgroundFrame");

	uiset setRelease;
	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		if (it->second->GetDelete())
			setRelease.insert(it->first);

	for (uiset_it it(setRelease.begin()); it != setRelease.end(); ++it)
		Delete(*it);
}


/*====================
  CServerEntityDirectory::Frame
  ====================*/
void	CServerEntityDirectory::Frame()
{
	PROFILE("CServerEntityDirectory::Frame");

	{
		PROFILE("Visibility");

		// Clear visibility flags
		for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		{
			if (it->second->IsVisual())
				it->second->GetAsVisualEnt()->RemoveNetFlags(ENT_NET_FLAG_REVEALED | ENT_NET_FLAG_SIGHTED);
		}
		
		// Set visibility (excluding gadgets)
		for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		{
			IVisualEntity *pEntity(it->second->GetAsVisualEnt());
			if (pEntity == NULL)
				continue;
			if (pEntity->GetTeam() < 1)
				continue;
			if (!pEntity->IsCombat() && !pEntity->IsBuilding() && !pEntity->IsGadget())
				continue;

			for (EntMap_it itViewer(m_mapEntities.begin()); itViewer != m_mapEntities.end(); ++itViewer)
			{
				IVisualEntity *pViewer(itViewer->second->GetAsVisualEnt());
				if (pViewer == NULL)
					continue;
				if (pViewer->IsGadget())
					continue;
				if (!pViewer->IsCombat() && !pViewer->IsBuilding())
					continue;
				if (pViewer->GetTeam() < 1)
					continue;
				if (pViewer->GetTeam() == pEntity->GetTeam())
					continue;
				
				if (pViewer->CanSee(pEntity))
				{
					pEntity->SetNetFlags(ENT_NET_FLAG_SIGHTED);
					break;
				}
			}
		}
	}

	uiset setRelease;

	{
		PROFILE("Gadgets");

		// Process gadgets
		for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		{
			IGadgetEntity *pGadget(it->second->GetAsGadget());
			if (pGadget == NULL)
				continue;

			if (pGadget->GetDelete())
			{
				setRelease.insert(it->first);
				continue;
			}

			if (!pGadget->ServerFrame())
				pGadget->SetDelete(true);
		}
	}

	{
		PROFILE("General entities");
		// Process all other entities
		for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
		{
			if (it->second->IsGadget())
				continue;

			if (!it->second->GetDelete())
			{
				if (!it->second->ServerFrame())
				{
					if (it->second->IsState())
						Console.Warn << "Orphaned entity state " << SingleQuoteStr(it->second->GetTypeName()) << newl;

					it->second->SetDelete(true);
				}
			}
			else
			{
				setRelease.insert(it->first);
			}
		}

		for (uiset_it it(setRelease.begin()); it != setRelease.end(); ++it)
			Delete(*it);
	}
}


/*====================
  CServerEntityDirectory::Reset
  ====================*/
void	CServerEntityDirectory::Reset()
{
	uiset setRelease;
	for (EntMap_it it(m_mapEntities.begin()); it != m_mapEntities.end(); ++it)
	{
		if (it->second->GetDelete() || !it->second->Reset())
			setRelease.insert(it->first);
	}

	for (uiset_it it(setRelease.begin()); it != setRelease.end(); ++it)
		Delete(*it);
}
