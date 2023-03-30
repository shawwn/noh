// (C)2005 S2 Games
// i_resourcewatcher2.h
//
//=============================================================================
#ifndef __I_RESOURCEWATCHER_H__
#define __I_RESOURCEWATCHER_H__

//=============================================================================
// Definitions
//=============================================================================
class IResource;
//=============================================================================

//=============================================================================
// IResourceWatcher
//		***NOTE***:  BE SURE you call g_ResourceManager.RemoveResourceWatcher(this, m_hResource)
//		in the destructor of your derived class!
//=============================================================================
class IResourceWatcher
{
	friend class IResourceLibrary;

	// internal for IResourceLibrary
private:
	bool			m_bHasAddedWatcher;

	void			MarkHasAddedWatcher()		{ m_bHasAddedWatcher = true; }
	void			ClearHasAddedWatcher()		{ m_bHasAddedWatcher = false; }
	bool			HasAddedWatcher()			{ return m_bHasAddedWatcher; }

public:
	IResourceWatcher()
		: m_bHasAddedWatcher(false)
	{ }

	virtual ~IResourceWatcher()
	{ }

	virtual void	Rebuild(ResHandle hResource) = 0;
};
//=============================================================================

#endif //__I_RESOURCEWATCHER_H__
