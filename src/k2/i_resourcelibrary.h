// (C)2005 S2 Games
// i_resourcelibrary.h
//
//=============================================================================
#ifndef __I_RESOURCELIBRARY_H__
#define __I_RESOURCELIBRARY_H__

//=============================================================================
// Headers
//=============================================================================
#include "c_resourcemanager_constants.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
class IResource;
class IResourceWatcher;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
//=============================================================================

//=============================================================================
// IResourceLibrary
//=============================================================================
class IResourceLibrary
{
	typedef hash_map<tstring, ResHandle>				PathHandleMap;
	typedef hash_set<IResourceWatcher*>					ResourceWatcherSet;
	typedef hash_map<ResHandle, ResourceWatcherSet>		ResourceWatcherMap;

protected:
	tstring						m_sName;
	tstring						m_sTypeName;
	uint						m_uiType;
	ResRegAllocFn				m_fnAlloc;
	bool						m_bLoadRaw;
	bool						m_bReplaceResources;
	bool						m_bReloading;

	ResPtrVec					m_vEntries;
	ResNameMap					m_mapResNames;
	ResNameMap					m_mapResPaths;

	PathHandleMap				m_mapReservedHandles;
	stack<ResHandle>			m_stkAvailableHandles;

	ResourceWatcherMap			m_mapWatchers;

	ULONGLONG					m_llLoadTime;
	ULONGLONG					m_llLoadMemory;
	ULONGLONG					m_llLoadCount;

	IResourceLibrary();
	IResourceLibrary(const IResourceLibrary&);
	IResourceLibrary&	operator=(const IResourceLibrary&);

	void	EmbedType(ResHandle &h)	{ h |= (m_uiType << 24); }
	int		GetType(ResHandle h)	{ return ((h & 0xff000000) >> 24); }
	void	MaskType(ResHandle &h)	{ h &= 0x00ffffff; }

public:
	K2_API virtual ~IResourceLibrary();
	K2_API IResourceLibrary(uint uiType, const tstring &sName, const tstring &sTypeName, bool bLoadRaw, ResRegAllocFn fnAlloc);

	const tstring&				GetName() const			{ return m_sName; }

	// returns "{texture}" or "{sample}" or "{effect}" etc
	const tstring&				GetTypeName() const		{ return m_sTypeName; }

	virtual ResHandle			Register(const tstring &sPath, uint uiIgnoreFlags);
	virtual ResHandle			Register(IResource *pResource, uint uiIgnoreFlags);
	virtual void				Unregister(ResHandle hResource, EUnregisterResource eUnregisterOp);
	virtual bool				Reload(ResHandle hResource, uint uiIgnoreFlags);
	virtual void				ReloadByFlag(int iFlag);
	virtual	ResHandle			LookUpPath(const tstring &sPath);
	virtual	ResHandle			LookUpName(const tstring &sName);
	virtual IResource*			LookUpHandle(ResHandle hResource);
	virtual IResource*			Get(ResHandle hResource);

	K2_API void					ReloadAll();
	void						FreeAll();

	K2_API uint					FindResources(ResPtrVec &vResults, const tstring &sWildcard);

	void						SetReplaceResources(bool b)			{ m_bReplaceResources = b; }

	ULONGLONG					GetLoadTime() const					{ return m_llLoadTime; }
	ULONGLONG					GetLoadMemory() const				{ return m_llLoadMemory; }
	ULONGLONG					GetLoadCount() const				{ return m_llLoadCount; }

	void						AddLoadTime(ULONGLONG llTime)		{ m_llLoadTime += llTime; }
	void						AddLoadMemory(ULONGLONG llMemory)	{ m_llLoadMemory += llMemory; }
	void						AddLoadCount(ULONGLONG llCount)		{ m_llLoadCount += llCount; }

	const ResNameMap&			GetResourcePathMap() const			{ return m_mapResPaths; }

	// internal for g_ResourceManager
	void						RemoveResourceWatcher(IResourceWatcher* pWatcher, ResHandle hUnregisterFrom);
	void						AddResourceWatcher(IResourceWatcher* pWatcher, ResHandle hRegisterWith);

	bool						HasResourceWatcher(ResHandle hResource);
	uint						NotifyWatchers(ResHandle hResource);
};
//=============================================================================

#endif //__I_RESOURCELIBRARY_H__
