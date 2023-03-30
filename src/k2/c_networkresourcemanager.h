// (C)2006 S2 Games
// c_networkresourcemanager.h
//
//=============================================================================
#ifndef __C_NETWORKRESOURCEMANAGER_H__
#define __C_NETWORKRESOURCEMANAGER_H__

//=============================================================================
// Declarations
//=============================================================================
extern K2_API class CNetworkResourceManager &g_NetworkResourceManager;
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#ifdef K2_EXPORTS
#define NetworkResourceManager	(*CNetworkResourceManager::GetInstance())
#else
#define NetworkResourceManager	(g_NetworkResourceManager)
#endif

class CNetworkResourceEntry
{
private:
	ResHandle		m_hResource;

public:
	~CNetworkResourceEntry()	{}
	CNetworkResourceEntry() :
	m_hResource(INVALID_RESOURCE)
	{
	}

	CNetworkResourceEntry(ResHandle hResource) :
	m_hResource(hResource)
	{
	}

	void			Set(ResHandle hResource)	{ m_hResource = hResource; }
	
	tstring			GetPath() const;
	EResourceType	GetType() const;
	ResHandle		GetHandle() const	{ return m_hResource; }
};

const ushort INVALID_NETWORK_STRING(-1);

typedef vector<CNetworkResourceEntry>		NetResourceVector;
typedef NetResourceVector::iterator			NetResourceVector_it;
typedef NetResourceVector::const_iterator	NetResourceVector_cit;
//=============================================================================

//=============================================================================
// CNetworkResourceManager
//=============================================================================
class CNetworkResourceManager
{
private:
	SINGLETON_DEF(CNetworkResourceManager);

	NetResourceVector	m_vResources;
	bool				m_bModified;

	tsvector			m_vStrings;
	bool				m_bStringsModified;

public:
	~CNetworkResourceManager();

	// Resources
	K2_API uint			RegisterLocalResource(ResHandle hHandle);
	K2_API ResHandle	RegisterNetworkResource(ResHandle hHandle, uint uiIndex);

	K2_API ResHandle	GetLocalHandle(uint uiIndex);
	K2_API uint			GetNetIndex(ResHandle hHandle);

	void				GetStateString(CStateString &ssResourceList);

	bool				IsModified()							{ return m_bModified; }
	void				SetModified(bool bModified)				{ m_bModified = bModified; }

	// Strings
	ushort					ReserveString()						{ m_vStrings.push_back(TSNULL); return ushort(m_vStrings.size() - 1); }
	K2_API void				SetString(ushort unIndex, const tstring &sString, bool bGrow = false);
	K2_API const tstring&	GetString(ushort unIndex);
	K2_API void				ClearStrings();

	bool					IsStringListModified() const		{ return m_bStringsModified; }
	void					SetStringListModified(bool b)		{ m_bStringsModified = b; }

	void					UpdateEntityStateString(CStateString &ssStrings) const;
	K2_API void				ApplyUpdateFromStateString(const CStateString &ssStrings);

	K2_API void				Clear();

	void					ListResources() const;
};
//=============================================================================

#endif //__C_NETWORKRESOURCEMANAGER_H__
