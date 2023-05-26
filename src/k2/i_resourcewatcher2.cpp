// (C)2005 S2 Games
// i_resourcewatcher2.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_resourcemanager.h"

#include "i_resourcewatcher2.h"
//=============================================================================


/*====================
  IResourceWatcher::~IResourceWatcher
  ====================*/
IResourceWatcher::~IResourceWatcher()
{
    auto setResources(m_setWatchingResources); // copy, since RemoveResourceWatcher mutates m_setWatchingResources
    for (ResHandle hResource : setResources)
        g_ResourceManager.RemoveResourceWatcher(this, hResource);
}


/*====================
  IResourceWatcher::HasAddedWatcher
  ====================*/
bool IResourceWatcher::HasAddedWatcher(ResHandle hResource) const
{
    return m_setWatchingResources.contains(hResource);
}


/*====================
  IResourceWatcher::MarkHasAddedWatcher
  ====================*/
void IResourceWatcher::MarkHasAddedWatcher(ResHandle hResource)
{
    if (hResource != INVALID_RESOURCE)
        m_setWatchingResources.insert(hResource);
}


/*====================
  IResourceWatcher::ClearHasAddedWatcher
  ====================*/
void IResourceWatcher::ClearHasAddedWatcher(ResHandle hResource)
{
    if (auto it = m_setWatchingResources.find(hResource); it != m_setWatchingResources.end())
        m_setWatchingResources.erase(it);
}
