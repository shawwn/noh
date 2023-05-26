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
//=============================================================================
class IResourceWatcher
{
    friend class IResourceLibrary;

    // internal for IResourceLibrary
private:
    set<ResHandle>  m_setWatchingResources;

    bool            HasAddedWatcher(ResHandle hResource) const;
    void            MarkHasAddedWatcher(ResHandle hResource);
    void            ClearHasAddedWatcher(ResHandle hResource);

public:
    virtual ~IResourceWatcher();

    virtual void    Rebuild(ResHandle hResource) = 0;
};
//=============================================================================

#endif //__I_RESOURCEWATCHER_H__
