// (C)2005 S2 Games
// i_resourcelibrary.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "i_resourcelibrary.h"
#include "i_resource.h"
#include "i_resourcewatcher2.h"
#include "c_statestring.h"
#include "c_networkresourcemanager.h"
#include "c_eventmanager.h"
#include "c_resourcemanager.h"
#include "c_restorevalue.h"
#include "c_resourceinfo.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_STRING(host_vidDriver);
EXTERN_CVAR_STRING(host_language);
//=============================================================================


/*====================
  IResourceLibrary::~IResourceLibrary
  ====================*/
IResourceLibrary::~IResourceLibrary()
{
	for (ResPtrVec::iterator it(m_vEntries.begin()); it != m_vEntries.end(); ++it)
	{
		if (*it)
		{
			// TODO:
			//(*it)->Free();
			//delete *it;
		}
	}

	CResourceManager::GetInstance()->UnregisterLibrary(m_uiType);
}


/*====================
  IResourceLibrary::IResourceLibrary
  ====================*/
IResourceLibrary::IResourceLibrary(uint uiType, const tstring &sName, const tstring &sTypeName, bool bLoadRaw, ResRegAllocFn fnAlloc) :
m_sName(sName),
m_sTypeName(sTypeName),
m_uiType(uiType),
m_fnAlloc(fnAlloc),
m_bLoadRaw(bLoadRaw),
m_bReplaceResources(false),
m_llLoadTime(0),
m_llLoadMemory(0),
m_llLoadCount(0),
m_bReloading(false)
{
	CResourceManager::GetInstance()->RegisterLibrary(uiType, this);
}


/*====================
  IResourceLibrary::Register
  ====================*/
ResHandle	IResourceLibrary::Register(const tstring &sDirtyPath, uint uiIgnoreFlags)
{
	IResource *pNewResource(NULL);
	ResHandle hReservedHandle(INVALID_RESOURCE);

	tstring sPath(FileManager.SanitizePath(sDirtyPath));
	tstring sOldDir(FileManager.GetWorkingDirectory());

	// Check for an already registered resource
	ResHandle hFind(LookUpPath(sPath));
	if (hFind != INVALID_RESOURCE)
	{
		IResource *pOldResource(Get(hFind));

		bool bShouldReload(false);
		if ((pOldResource->GetIgnoreFlags() ^ uiIgnoreFlags) & ~uiIgnoreFlags)
			bShouldReload = true;

		if (bShouldReload)
		{
			if (!Reload(hFind, uiIgnoreFlags))
				return INVALID_RESOURCE;
		}

		CGraphResource::LinkExistingChild(hFind);
		return hFind;
	}

	g_ResourceManager.StartResLoadProfile(m_uiType, K2System.GetTicks(), K2System.GetProcessVirtualMemoryUsage());

	CGraphResource cGraphResource;
	cGraphResource.SetDebugPath(sPath);

	const char *pData(NULL);
	uint uiSize(0);
	CFileHandle hFile;

	tstring sLocalizedPath(sPath);
	bool bLocalized(true);
	if (m_bLoadRaw && !sPath.empty() && sPath[0] != _T('$') && sPath[0] != _T('!') && sPath[0] != _T('*') && sPath.find(_CWS("%"), 0) == tstring::npos) // check for virtual resources (like procedural textures)
	{
		if (!host_language.empty())
		{
			sLocalizedPath = Filename_StripExtension(sLocalizedPath) + _CWS("_") + host_language + _CWS(".") + Filename_GetExtension(sLocalizedPath);
			if (!FileManager.Exists(sLocalizedPath, FILE_READ))
			{
				bLocalized = false;
				sLocalizedPath = sPath;
			}
		}
#if 0
		if (!hFile.Open(sPath, FILE_READ | FILE_BINARY))
			EX_WARN(_T("Failed to open file"));
#else
		hFile.Open(sLocalizedPath, FILE_READ | FILE_BINARY);
#endif
		pData = hFile.GetBuffer(uiSize);
	}

	try
	{
		// Allocate a new resource object
		if (m_fnAlloc == NULL)
			EX_ERROR(_CWS("No allocator provided for this resource type"));

		pNewResource = m_fnAlloc(sPath);
		if (pNewResource == NULL)
			EX_WARN(_CWS("Failed to allocate resource"));

		if (!sPath.empty() && sPath[0] != _T('$') && sPath[0] != _T('!') && sPath[0] != _T('*'))
			FileManager.SetWorkingDirectory(Filename_GetPath(FileManager.SanitizePath(sPath)));

		if (bLocalized)
		{
			pNewResource->AddFlags(RES_FLAG_LOCALIZED);
			pNewResource->SetLocalizedPath(sLocalizedPath);
		}

		// Load the resource
		pNewResource->AddFlags(pNewResource->Load(uiIgnoreFlags, pData, uiSize));
		const tstring &sName(pNewResource->GetName());

		// Check for a name conflict
		if (!sName.empty())
		{
			if (LookUpName(sName) != INVALID_RESOURCE)
				EX_ERROR(_CWS("Name collision for ") + QuoteStr(sName));
		}
	}
	catch (CException &ex)
	{
		cGraphResource.Reset();

		assert(pNewResource);
		if (pNewResource)
		{
			pNewResource->AddFlags(RES_LOAD_FAILED);
			pNewResource->LoadNull();
		}

		ex.Process(_CWS("IResourceLibrary::Register(") + sPath + _CWS(") - "), NO_THROW);
	}

	CResourceManager::GetInstance()->EndResLoadProfile(m_uiType, K2System.GetTicks(), K2System.GetProcessVirtualMemoryUsage());

	FileManager.SetWorkingDirectory(sOldDir);

	assert(pNewResource);
	if (!pNewResource)
		return INVALID_RESOURCE;

	// assign a handle to the resource.
	if (hReservedHandle == INVALID_RESOURCE)
	{
		// if the path is reserved for a specific handle, use it.
		PathHandleMap::iterator itFind(m_mapReservedHandles.find(sPath));
		if (itFind != m_mapReservedHandles.end())
		{
			hReservedHandle = itFind->second;

			// this is optional, but since memory is so tight, we should probably do this.
			m_mapReservedHandles.erase(itFind);
		}
		else if (!m_stkAvailableHandles.empty())
		{
			// otherwise, try to use any available handle.
			hReservedHandle = m_stkAvailableHandles.top();
			m_stkAvailableHandles.pop();
		}
		else
		{
			// if no handles are available, then allocate a new one.
			hReservedHandle = (ResHandle)m_vEntries.size();
			m_vEntries.push_back(NULL);
		}
	}
	assert(hReservedHandle != INVALID_RESOURCE);

	// Assign this entry a handle and add it to the map
	ResHandle hIdx(hReservedHandle);
	m_vEntries[hIdx] = pNewResource;
	EmbedType(hReservedHandle);
	if (!pNewResource->GetName().empty())
		m_mapResNames[pNewResource->GetName()] = hReservedHandle;
	if (!pNewResource->GetPath().empty())
		m_mapResPaths[pNewResource->GetPath()] = hReservedHandle;
	pNewResource->SetHandle(hReservedHandle);
	pNewResource->PostLoad();

	cGraphResource.SetHandle(hReservedHandle);
	cGraphResource.Done();

	// notify any watchers that this resource has been registered again.
	NotifyWatchers(hReservedHandle);

	return hReservedHandle;
}

ResHandle	IResourceLibrary::Register(IResource *pResource, uint uiIgnoreFlags)
{
	tstring sOldDir(FileManager.GetWorkingDirectory());
	ResHandle hReservedHandle(INVALID_RESOURCE);

	ResHandle	hFind(INVALID_RESOURCE);

	assert(!(pResource->GetPath().empty() && pResource->GetName().empty()));

	// Check for an already registered resource
	// External resources must have a unique name
	const tstring &sName(pResource->GetName());
	if (!sName.empty())
		hFind = LookUpName(sName);

	// Check for an already registered resource
	// For paths, there can be only one...
	const tstring &sPath(pResource->GetPath());
	if (!sPath.empty() && hFind == INVALID_RESOURCE)
	{
		hFind = LookUpPath(sPath);

		// Link the new name to the existing resource
		if (!sName.empty() && hFind != INVALID_RESOURCE)
			m_mapResNames[sName] = hFind;
	}

	// Delete the input resource if we already have a resource with this name registered
	if (hFind != INVALID_RESOURCE)
	{
		if (m_bReplaceResources)
		{
			Unregister(hFind, UNREG_RESERVE_HANDLE);
		}
		else
		{
			IResource *pOldResource(Get(hFind));
			bool bShouldReload(false);
			if ((pOldResource->GetIgnoreFlags() ^ uiIgnoreFlags) & ~uiIgnoreFlags)
				bShouldReload = true;

			if (bShouldReload)
			{
				if (!Reload(hFind, uiIgnoreFlags))
				{
					SAFE_DELETE(pResource);
					return INVALID_RESOURCE;
				}
			}

			SAFE_DELETE(pResource);
			CGraphResource::LinkExistingChild(hFind);
			return hFind;
		}
	}

	if (!sPath.empty() && sPath[0] != _T('$') && sPath[0] != _T('!') && sPath[0] != _T('*'))
		FileManager.SetWorkingDirectory(Filename_GetPath(FileManager.SanitizePath(sPath)));

	CResourceManager::GetInstance()->StartResLoadProfile(m_uiType, K2System.GetTicks(), K2System.GetProcessVirtualMemoryUsage());

	CGraphResource cGraphResource;
	cGraphResource.SetDebugPath(sPath);

	try
	{
		uint uiResFlags(RES_EXTERNAL);

		// prevent procedural textures from being reloaded.
		if (!sPath.empty() && sPath[0] == '$')
			uiResFlags = 0;

		// Load the resource
		pResource->AddFlags(pResource->Load(uiIgnoreFlags, pResource->GetData(), pResource->GetSize()) | uiResFlags);
	}
	catch (CException &ex)
	{
		cGraphResource.Reset();

		tstring sPath(pResource->GetPath());

		pResource->AddFlags(RES_LOAD_FAILED | RES_EXTERNAL);
		pResource->LoadNull();

		ex.Process(_TS("IResourceLibrary::Register(") + sPath + _TS(") - "), NO_THROW);
	}

	CResourceManager::GetInstance()->EndResLoadProfile(m_uiType, K2System.GetTicks(), K2System.GetProcessVirtualMemoryUsage());

	FileManager.SetWorkingDirectory(sOldDir);

	// assign a handle to the resource.
	if (hReservedHandle == INVALID_RESOURCE)
	{
		PathHandleMap::iterator itFind(m_mapReservedHandles.find(sPath));
		if (itFind != m_mapReservedHandles.end())
		{
			// if the path is reserved for a specific handle, use it.
			hReservedHandle = itFind->second;

			// this is optional, but since memory is so tight, we should probably do this.
			m_mapReservedHandles.erase(itFind);
		}
		else if (!m_stkAvailableHandles.empty()) 
		{
			// otherwise, try to use any available handle.
			hReservedHandle = m_stkAvailableHandles.top();
			m_stkAvailableHandles.pop();
		}
		else
		{
			// if no handles are available, then allocate a new one.
			hReservedHandle = (ResHandle)m_vEntries.size();
			m_vEntries.push_back(NULL);
		}
	}
	assert(hReservedHandle != INVALID_RESOURCE);

	// Assign this entry a handle and add it to the map
	ResHandle hIdx(hReservedHandle);
	m_vEntries[hIdx] = pResource;
	EmbedType(hReservedHandle);
	if (!pResource->GetName().empty())
		m_mapResNames[pResource->GetName()] = hReservedHandle;
	if (!pResource->GetPath().empty())
		m_mapResPaths[pResource->GetPath()] = hReservedHandle;
	pResource->SetHandle(hReservedHandle);
	pResource->PostLoad();
	cGraphResource.SetHandle(hReservedHandle);
	cGraphResource.Done();

	// notify any watchers that this resource has been registered again.
	NotifyWatchers(hReservedHandle);

	return hReservedHandle;
}


/*====================
  IResourceLibrary::Unregister
  ====================*/
void	IResourceLibrary::Unregister(ResHandle hResource, EUnregisterResource eUnregisterOp)
{
	if (GetType(hResource) != m_uiType)
	{
		Console.Warn << _T("IResourceLibrary::Unregister(") << hResource << _T(") - Handle is of wrong type") << newl;
		return;
	}
	ResHandle hIdx(hResource);
	MaskType(hIdx);

	if (hIdx >= m_vEntries.size())
	{
		Console.Warn << _T("IResourceLibrary::Unregister(") << hResource << _T(") - Handle out of range") << newl;
		return;
	}

	g_ResourceInfo.OnResourceUnregistered(hResource);

	IResource* pResource(m_vEntries[hIdx]);
	assert(pResource != NULL);
	if (pResource == NULL)
		return;

	ResNameMap::iterator findit;

	if (eUnregisterOp == UNREG_DELETE_HANDLE)
	{
		// free the handle for use by any other resource of the same type.
		m_stkAvailableHandles.push(hIdx);

		// if something was watching this resource, which we just deleted, then this is a very bad thing!
		bool bHasResourceWatcher(HasResourceWatcher(hResource));
		assert(!bHasResourceWatcher);
		if (bHasResourceWatcher)
			Console.Warn << _T("IResourceLibrary::Unregister(") << hResource << _T(") - Deleted a resource handle which was being watched!") << newl;
	}
	else
	{
		assert(eUnregisterOp == UNREG_RESERVE_HANDLE);

		// reserve the handle for resources with this path only.
		const tstring &sPath(pResource->GetPath());
		m_mapReservedHandles[sPath] = hIdx;
	}

	findit = m_mapResNames.find(pResource->GetName());
	if (findit != m_mapResNames.end())
		m_mapResNames.erase(findit);

	findit = m_mapResPaths.find(pResource->GetPath());
	if (findit != m_mapResPaths.end())
		m_mapResPaths.erase(findit);

	pResource->Free();
	SAFE_DELETE(m_vEntries[hIdx]);

	if (!m_bReloading)
	{
		// notify the resource watchers that this resource has been unregistered.
		NotifyWatchers(hResource);
	}
}


/*====================
  IResourceLibrary::ReloadByFlag
  ====================*/
void	IResourceLibrary::ReloadByFlag(int iFlag)
{
	for (ResPtrVec::iterator it(m_vEntries.begin()); it != m_vEntries.end(); it++)
	{
		if (!*it)
			continue;

		if ((*it)->HasFlags(iFlag))
			Reload((*it)->GetHandle(), 0xffffffff);
	}
}


/*====================
  IResourceLibrary::Reload
  ====================*/
bool	IResourceLibrary::Reload(ResHandle hResource, uint uiIgnoreFlags)
{
	IResource *pResource(Get(hResource));
	ResHandle hIdx(hResource);
	MaskType(hIdx);

	// Validate handle
	if (pResource == NULL)
	{
		Console.Warn << _T("Invalid resource handle") << newl;
		return false;
	}

	Console << _T("Reloading ");
	g_ResourceManager.PrintResource(Console.DefaultStream(), pResource);
	Console << newl;
			
	// Save pertinent data
	tstring sSavedPath(pResource->GetPath());
	uint uiSavedIgnoreFlags(pResource->GetIgnoreFlags());
	set<ResHandle>	setDependents(pResource->GetDependents());

	try
	{
		CRestoreValue<bool> cSetReloading(m_bReloading, true);

		if (pResource->HasFlags(RES_EXTERNAL))
		{
			const tstring &sPath(pResource->GetPath());

			if (!sPath.empty() && sPath[0] == _T('*'))
				return true;

			// External resources with valid paths set should just Free+Load
			pResource->Free();
			g_ResourceInfo.OnResourceUnregistered(hResource);
			CGraphResource cGraphResource;
			cGraphResource.SetHandle(hResource);
			pResource->Load(pResource->GetIgnoreFlags() & uiIgnoreFlags, pResource->GetData(), pResource->GetSize());
		}
		else
		{
			// Reload
			if (sSavedPath[0] != '$')
			{
				g_ResourceManager.Unregister(hResource, UNREG_RESERVE_HANDLE);
				if (g_ResourceManager.Register(sSavedPath, m_uiType, uiSavedIgnoreFlags & uiIgnoreFlags) != hResource)
					EX_ERROR(_T("Resource handle changed"));
			}
			pResource = Get(hResource);
		}
	}
	catch (CException &ex)
	{
		g_ResourceInfo.OnResourceUnregistered(hResource);
		CGraphResource cGraphResource;
		cGraphResource.SetHandle(hResource);

		pResource->AddFlags(RES_LOAD_FAILED);
		pResource->LoadNull();

		ex.Process(_TS("IResourceLibrary::Reload(") + sSavedPath + _TS(") - "), NO_THROW);
	}

	// Handle the dependents
	for (set<ResHandle>::iterator it(setDependents.begin()); it != setDependents.end(); ++it)
	{
		IResource *pChildResource(g_ResourceManager.Get(*it));
		if (pChildResource == NULL)
		{
			Console.Warn << _T("Couldn't retrieve a dependant resource") << newl;
			continue;
		}

		// If there is no path, a Free()/Load() should handle everything,
		// otherwise just do a standard Reload
		if (pChildResource->GetPath().empty())
		{
			pChildResource->Free();
			g_ResourceInfo.OnResourceUnregistered(hResource);
			CGraphResource cGraphResource;
			cGraphResource.SetHandle(hResource);
			pChildResource->Load(pChildResource->GetIgnoreFlags(), pChildResource->GetData(), pChildResource->GetSize());
		}
		else
		{
			g_ResourceManager.Reload(*it);
		}

		pResource->AddDependent(*it);
	}

	// notify the resource watchers that this resource has been reloaded.
	NotifyWatchers(hResource);

	pResource->Reloaded();
	return true;
}


/*====================
  IResourceLibrary::LookUpName
  ====================*/
ResHandle	IResourceLibrary::LookUpName(const tstring &sName)
{
	ResNameMap::iterator findit(m_mapResNames.find(sName));
	if (findit == m_mapResNames.end())
		return INVALID_RESOURCE;

	return findit->second;
}


/*====================
  IResourceLibrary::LookUpPath
  ====================*/
ResHandle	IResourceLibrary::LookUpPath(const tstring &sPath)
{
	if (sPath.empty())
		return INVALID_RESOURCE;

	ResNameMap::iterator findit(m_mapResPaths.find(FileManager.SanitizePath(sPath)));
	if (findit == m_mapResPaths.end())
		return INVALID_RESOURCE;

	return findit->second;
}


/*====================
  IResourceLibrary::LookUpHandle
  ====================*/
IResource*	IResourceLibrary::LookUpHandle(ResHandle hResource)
{
	try
	{
		if (GetType(hResource) != m_uiType)
			throw CException(_T("Handle is of wrong type"), E_ERROR);

		MaskType(hResource);

		assert(hResource < m_vEntries.size());
		if (hResource >= m_vEntries.size())
			return NULL;

		// resource has been unregistered.
		if (!m_vEntries[hResource])
			return NULL;

		return m_vEntries[hResource];
	}
	catch (CException &ex)
	{
		ex.Process(_TS("IResourceLibrary(") + XtoA(m_uiType) + _T(")::LookUpHandle(") + XtoA(hResource, 0, 0, 16) + _T(") - "), NO_THROW);
		return NULL;
	}
}


/*====================
  IResourceLibrary::Get
  ====================*/
IResource*	IResourceLibrary::Get(ResHandle hResource)
{
	try
	{
		if (GetType(hResource) != m_uiType)
			throw CException(_T("Handle is of wrong type"), E_ERROR);

		MaskType(hResource);

		if (hResource >= m_vEntries.size())
			throw CException(_T("Handle out of range"), E_ERROR);

		if (!m_vEntries[hResource])
			throw CException(_T("Handle has been unregistered"), E_ERROR);

		return m_vEntries[hResource];
	}
	catch (CException &ex)
	{
		ex.Process(_TS("IResourceLibrary(") + XtoA(m_uiType) + _T(")::Get(") + XtoA(hResource, 0, 0, 16) + _T(") - "), NO_THROW);
		return NULL;
	}
}


/*====================
  IResourceLibrary::ReloadAll
  ====================*/
void	IResourceLibrary::ReloadAll()
{
	if (m_uiType == RES_FONTMAP)
	{
		for (ResPtrVec::const_iterator it(m_vEntries.begin()); it != m_vEntries.end(); ++it)
			(*it)->Free();
	}

	if (host_vidDriver.IsModified())
	{
		for (ResPtrVec::const_iterator it(m_vEntries.begin()); it != m_vEntries.end(); ++it)
			Reload((*it)->GetHandle(), 0xffffffff);
	}
	else if (m_uiType == RES_MODEL)
	{
		class CReloadFunctions : public CLoadJob<ResPtrVec>::IFunctions
		{
		private:
			IResourceLibrary*	m_pLib;
	
		public:
			CReloadFunctions(IResourceLibrary *pLib) : m_pLib(pLib)	{}
			virtual ~CReloadFunctions() {}
			
			float	Frame(ResPtrVec::iterator &it, float f)	const
			{
				if (m_pLib == NULL)
					return 0.0f;
				SetTitle(_T("Reloading ") + m_pLib->GetName());
				SetProgress(f);
				return 0.0f;
			}
			
			float	PostFrame(ResPtrVec::iterator &it, float f)	const
			{
				if (m_pLib != NULL && *it != NULL)
				{
					m_pLib->Reload((*it)->GetHandle(), 0xffffffff);

					g_ResourceManager.PrecacheSkin((*it)->GetHandle(), uint(-1));
				}
				++it;
				return 1.0f;
			}
		};
		class CReloadFunctions fnReload(this);
		CLoadJob<ResPtrVec>	jobReload(m_vEntries, &fnReload, LOADING_DISPLAY_LOGO);
		jobReload.Execute(m_vEntries.size());
	}
	else
	{
		class CReloadFunctions : public CLoadJob<ResPtrVec>::IFunctions
		{
		private:
			IResourceLibrary*	m_pLib;
	
		public:
			CReloadFunctions(IResourceLibrary *pLib) : m_pLib(pLib)	{}
			virtual ~CReloadFunctions() {}
			
			float	Frame(ResPtrVec::iterator &it, float f)	const
			{
				if (m_pLib == NULL)
					return 0.0f;
				SetTitle(_T("Reloading ") + m_pLib->GetName());
				SetProgress(f);
				return 0.0f;
			}
			
			float	PostFrame(ResPtrVec::iterator &it, float f)	const
			{
				if (m_pLib != NULL && *it != NULL)
					m_pLib->Reload((*it)->GetHandle(), 0xffffffff);
				++it;
				return 1.0f;
			}
		};
		class CReloadFunctions fnReload(this);
		CLoadJob<ResPtrVec>	jobReload(m_vEntries, &fnReload, LOADING_DISPLAY_LOGO);
		jobReload.Execute(m_vEntries.size());
	}
}


/*====================
  IResourceLibrary::FreeAll
  ====================*/
void	IResourceLibrary::FreeAll()
{
	// TODO: Make sure this plays nicely with the resource graphing.
	assert(m_uiType == RES_SAMPLE);
	for (ResPtrVec::const_iterator it(m_vEntries.begin()); it != m_vEntries.end(); ++it)
		(*it)->Free();
}


/*====================
  IResourceLibrary::FindResources
  ====================*/
uint	IResourceLibrary::FindResources(ResPtrVec &vResults, const tstring &sWildcard)
{
	uint uiTotal(0);

	for (ResPtrVec::const_iterator it(m_vEntries.begin()); it != m_vEntries.end(); ++it)
	{
		IResource* pResource(*it);
		if (pResource == NULL)
			continue;

		// skip invalid resources.
		ResHandle hRes(pResource->GetHandle());
		if (hRes == INVALID_RESOURCE)
			continue;

		const tstring &sPath(pResource->GetPath());
		const tstring &sName(pResource->GetName());

		// match the wildcard against the resource path or name.
		if (!EqualsWildcards(sWildcard, sPath) &&
			!EqualsWildcards(sWildcard, sName))
		{
			continue;
		}

		vResults.push_back(pResource);
		++uiTotal;
	}

	return uiTotal;
}


/*====================
  IResourceLibrary::RemoveResourceWatcher
  ====================*/
void	IResourceLibrary::RemoveResourceWatcher(IResourceWatcher* pWatcher, ResHandle hUnregisterFrom)
{
	if (pWatcher == NULL || hUnregisterFrom == INVALID_RESOURCE)
		return;

	ResHandle hIdx(hUnregisterFrom);
	MaskType(hIdx);
	try
	{
		if (GetType(hUnregisterFrom) != m_uiType)
			EX_ERROR(_T("Handle is of wrong type"));

		if (hIdx >= m_vEntries.size())
			EX_ERROR(_T("Handle out of range"));
	}
	catch (CException& ex)
	{
		ex.Process(_TS("IResourceLibrary::RemoveResourceWatcher(IResourceWatcher*, ") + XtoA(hUnregisterFrom) + _TS(") - "), NO_THROW);
		return;
	}

	ResourceWatcherMap::iterator itFind(m_mapWatchers.find(hIdx));
	assert(itFind != m_mapWatchers.end());
	if (itFind == m_mapWatchers.end())
		return;

	ResourceWatcherSet& setWatchers(itFind->second);

	// if the watcher was already removed, we're done.
	if (setWatchers.find(pWatcher) == setWatchers.end())
		return;

	setWatchers.erase(pWatcher);

	assert(pWatcher->HasAddedWatcher());
	pWatcher->ClearHasAddedWatcher();

	if (setWatchers.empty())
		m_mapWatchers.erase(itFind);
}


/*====================
  IResourceLibrary::AddResourceWatcher
  ====================*/
void	IResourceLibrary::AddResourceWatcher(IResourceWatcher* pWatcher, ResHandle hRegisterWith)
{
	if (pWatcher == NULL || hRegisterWith == INVALID_RESOURCE)
		return;

	ResHandle hIdx(hRegisterWith);
	MaskType(hIdx);
	try
	{
		if (GetType(hRegisterWith) != m_uiType)
			EX_ERROR(_T("Handle is of wrong type"));

		if (hIdx >= m_vEntries.size())
			EX_ERROR(_T("Handle out of range"));
	}
	catch (CException& ex)
	{
		ex.Process(_TS("IResourceLibrary::AddResourceWatcher(IResourceWatcher*, ") + XtoA(hRegisterWith) + _TS(") - "), NO_THROW);
		return;
	}

	ResourceWatcherMap::iterator itFind(m_mapWatchers.find(hIdx));
	if (itFind == m_mapWatchers.end())
		itFind = m_mapWatchers.insert(std::make_pair(hIdx, ResourceWatcherSet())).first;

	ResourceWatcherSet& setWatchers(itFind->second);

	// if the watcher was already added, we're done.
	if (setWatchers.find(pWatcher) != setWatchers.end())
		return;

	// verify that the dependent hasn't already added a watcher.
	assert(!pWatcher->HasAddedWatcher());
	if (pWatcher->HasAddedWatcher())
		Console.Err << _T("PROGRAMMER ERROR: IResourceWatcher has added multiple watchers!") << newl;

	// register the watcher.
	pWatcher->MarkHasAddedWatcher();
	setWatchers.insert(pWatcher);
}


/*====================
  IResourceLibrary::HasResourceWatcher
  ====================*/
bool	IResourceLibrary::HasResourceWatcher(ResHandle hResource)
{
	if (hResource == INVALID_RESOURCE)
		return false;

	ResHandle hIdx(hResource);
	MaskType(hIdx);
	try
	{
		if (GetType(hResource) != m_uiType)
			EX_ERROR(_T("Handle is of wrong type"));

		if (hIdx >= m_vEntries.size())
			EX_ERROR(_T("Handle out of range"));
	}
	catch (CException& ex)
	{
		ex.Process(_TS("IResourceLibrary::HasResourceWatcher(IResourceWatcher*, ") + XtoA(hResource) + _TS(") - "), NO_THROW);
		return false;
	}

	ResourceWatcherMap::iterator itFind(m_mapWatchers.find(hIdx));
	if (itFind == m_mapWatchers.end())
		return false;

	return true;
}


/*====================
  IResourceLibrary::NotifyWatchers
  ====================*/
uint	IResourceLibrary::NotifyWatchers(ResHandle hResource)
{
	if (hResource == INVALID_RESOURCE)
		return 0;

	ResHandle hIdx(hResource);
	MaskType(hIdx);
	try
	{
		if (GetType(hResource) != m_uiType)
			EX_ERROR(_T("Handle is of wrong type"));

		if (hIdx >= m_vEntries.size())
			EX_ERROR(_T("Handle out of range"));
	}
	catch (CException& ex)
	{
		ex.Process(_TS("IResourceLibrary::HasResourceWatcher(IResourceWatcher*, ") + XtoA(hResource) + _TS(") - "), NO_THROW);
		return 0;
	}

	ResourceWatcherMap::iterator itFind(m_mapWatchers.find(hIdx));
	if (itFind == m_mapWatchers.end())
		return 0;

	uint uiNotifiedCount(0);

	// since the set of watchers could change as they get rebuilt, copy them.
	ResourceWatcherSet setWatchers(itFind->second);
	for (ResourceWatcherSet::const_iterator it(setWatchers.begin()); it != setWatchers.end(); ++it)
	{
		IResourceWatcher* pWatcher(*it);
		pWatcher->Rebuild(hResource);
	}

	return uiNotifiedCount;
}

