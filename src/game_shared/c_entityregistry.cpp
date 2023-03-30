// (C)2006 S2 Games
// c_entityregistry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entityregistry.h"

#include "../k2/c_entitysnapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CEntityRegistry	&g_EntityRegistry(*CEntityRegistry::GetInstance());
SINGLETON_INIT(CEntityRegistry);
//=============================================================================


/*====================
  IEntityAllocator::~IEntityAllocator
  ====================*/
IEntityAllocator::~IEntityAllocator()
{
	SAFE_DELETE(m_pBaseline);
}


/*====================
  IEntityAllocator::IEntityAllocator
  ====================*/
IEntityAllocator::IEntityAllocator(const tstring &sName, ushort unID) :
m_sName(sName),
m_unID(unID),
m_pBaseline(NULL)
{
	EntityRegistry.Register(this);
}


/*====================
  CEntityRegistry::CEntityRegistry
  ====================*/
CEntityRegistry::CEntityRegistry()
{
}


/*====================
  CEntityRegistry::Register
  ====================*/
void	CEntityRegistry::Register(IEntityAllocator* pAllocator)
{
	try
	{
		// Make sure that the name is not in use
		EntAllocatorNameMap_it	itFindName(m_mapAllocatorNames.find(LowerString(pAllocator->GetName())));
		if (itFindName != m_mapAllocatorNames.end())
			EX_FATAL(_T("Entity with this name already has an allocator registered: ") + pAllocator->GetName());

		// Assign an ID to this entity and associate it with the allocator
		m_mapAllocatorNames.insert(EntAllocatorNameEntry(LowerString(pAllocator->GetName()), pAllocator));
		m_mapAllocatorIDs.insert(EntAllocatorIDEntry(pAllocator->GetID(), pAllocator));
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::Register() - "), NO_THROW);
	}
}


/*====================
  CEntityRegistry::Allocate
  ====================*/
IGameEntity*	CEntityRegistry::Allocate(ushort unType)
{
	try
	{
		EntAllocatorIDMap_it	itFind(m_mapAllocatorIDs.find(unType));
		if (itFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity type: ") + XtoA(unType, FMT_PADZERO, 6, 16));

		IGameEntity *pNewEnt(itFind->second->Allocate());
		return pNewEnt;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::Allocate() - "), NO_THROW);
		return NULL;
	}
}

IGameEntity*	CEntityRegistry::Allocate(const tstring &sName)
{
	try
	{
		EntAllocatorNameMap_it	itFind(m_mapAllocatorNames.find(LowerString(sName)));
		if (itFind == m_mapAllocatorNames.end())
			EX_ERROR(_T("No allocator found for entity: ") + sName);

		IGameEntity *pNewEnt(itFind->second->Allocate());
		return pNewEnt;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::Allocate() - "), NO_THROW);
		return NULL;
	}
}


/*====================
  CEntityRegistry::GetTypeVector
  ====================*/
const vector<SDataField>*	CEntityRegistry::GetTypeVector(ushort unType) const
{
	try
	{
		EntAllocatorIDMap_cit	citFind(m_mapAllocatorIDs.find(unType));
		if (citFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity type: ") + XtoA(unType, FMT_PADZERO, 6, 16));

		return citFind->second->GetTypeVector();
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::GetTypeVector() - "), NO_THROW);
		return NULL;
	}
}


/*====================
  CEntityRegistry::GetBaseline
  ====================*/
const CEntitySnapshot*	CEntityRegistry::GetBaseline(ushort unType) const
{
	try
	{
		EntAllocatorIDMap_cit	citFind(m_mapAllocatorIDs.find(unType));
		if (citFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity type: ") + XtoA(unType, FMT_PADZERO, 6, 16));

		return citFind->second->GetBaseline();
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::GetBaseline() - "), NO_THROW);
		return NULL;
	}
}


/*====================
  CEntityRegistry::ServerPrecache
  ====================*/
void	CEntityRegistry::ServerPrecache(ushort unType) const
{
	try
	{
		EntAllocatorIDMap_cit	citFind(m_mapAllocatorIDs.find(unType));
		if (citFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity type: ") + XtoA(unType, FMT_PADZERO, 6, 16));

		citFind->second->ServerPrecache();
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::ServerPrecache() - "), NO_THROW);
	}
}


/*====================
  CEntityRegistry::ClientPrecache
  ====================*/
void	CEntityRegistry::ClientPrecache(ushort unType) const
{
	try
	{
		EntAllocatorIDMap_cit	citFind(m_mapAllocatorIDs.find(unType));
		if (citFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity type: ") + XtoA(unType, FMT_PADZERO, 6, 16));

		citFind->second->ClientPrecache();
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::ClientPrecache() - "), NO_THROW);
	}
}


/*====================
  CEntityRegistry::LookupID
  ====================*/
ushort	CEntityRegistry::LookupID(const tstring &sName)
{
	EntAllocatorNameMap_it itFind(m_mapAllocatorNames.find(LowerString(sName)));
	if (itFind == m_mapAllocatorNames.end())
		return INVALID_ENT_TYPE;

	return itFind->second->GetID();
}


/*====================
  CEntityRegistry::LookupName
  ====================*/
tstring	CEntityRegistry::LookupName(ushort unType)
{
	EntAllocatorIDMap_it itFind(m_mapAllocatorIDs.find(unType));
	if (itFind == m_mapAllocatorIDs.end())
		return _T("<INVALID ENTITY>");

	return itFind->second->GetName();
}


/*====================
  CEntityRegistry::GetAllocator
  ====================*/
const IEntityAllocator*	CEntityRegistry::GetAllocator(ushort unType)
{
	EntAllocatorIDMap_it itFind(m_mapAllocatorIDs.find(unType));
	if (itFind == m_mapAllocatorIDs.end())
		return NULL;

	return itFind->second;
}


/*====================
  CEntityRegistry::GetGameSetting
  ====================*/
ICvar*	CEntityRegistry::GetGameSetting(ushort unType, const tstring &sSetting) const
{
	try
	{
		EntAllocatorIDMap_cit	citFind(m_mapAllocatorIDs.find(unType));
		if (citFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity ID: ") + SHORT_HEX_STR(unType) + _T(", could not retrieve setting \"") + sSetting + _T("\""));

		return citFind->second->GetGameSetting(sSetting);
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::GetGameSetting() - "), NO_THROW);
		return NULL;
	}
}


/*====================
  CEntityRegistry::GetGameSettingFloat
  ====================*/
float	CEntityRegistry::GetGameSettingFloat(ushort unType, const tstring &sSetting, float fDefault) const
{
	try
	{
		EntAllocatorIDMap_cit	citFind(m_mapAllocatorIDs.find(unType));
		if (citFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity ID: ") + SHORT_HEX_STR(unType) + _T(", could not retrieve setting \"") + sSetting + _T("\""));

		ICvar *pCvar(citFind->second->GetGameSetting(sSetting));
		if (pCvar)
			return pCvar->GetFloat();
		else
			return fDefault;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::GetGameSettingFloat() - "), NO_THROW);
		return fDefault;
	}
}


/*====================
  CEntityRegistry::GetGameSettingString
  ====================*/
tstring	CEntityRegistry::GetGameSettingString(ushort unType, const tstring &sSetting, const tstring &sDefault) const
{
	try
	{
		EntAllocatorIDMap_cit	citFind(m_mapAllocatorIDs.find(unType));
		if (citFind == m_mapAllocatorIDs.end())
			EX_ERROR(_T("No allocator found for entity ID: ") + SHORT_HEX_STR(unType) + _T(", could not retrieve setting \"") + sSetting + _T("\""));

		ICvar *pCvar(citFind->second->GetGameSetting(sSetting));
		if (pCvar)
			return pCvar->GetString();
		else
			return sDefault;
	}
	catch (CException &ex)
	{
		ex.Process(_T("CEntityRegistry::GetGameSettingString() - "), NO_THROW);
		return sDefault;
	}
}


/*--------------------
  EntityList
  --------------------*/
CMD(EntityList)
{
	const EntAllocatorNameMap &mapAllocatorNames(EntityRegistry.GetAllocatorNames());
	for (EntAllocatorNameMap::const_iterator cit(mapAllocatorNames.begin()); cit != mapAllocatorNames.end(); ++cit)
		Console << cit->second->GetName() << _T(",") << newl;

	return true;
}
