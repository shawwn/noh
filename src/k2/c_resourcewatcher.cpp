// (C)2023 S3 Games
// c_resourcewatcher.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_resourcewatcher.h"

#include "c_resourcemanager.h"
//=============================================================================

/*====================
  CResourceWatcher::CResourceWatcher
  ====================*/
CResourceWatcher::CResourceWatcher(ResHandle hResource, std::function<void ()> fOnReload)
: m_fOnReload(std::move(fOnReload))
{
    g_ResourceManager.AddResourceWatcher(this, hResource);
}

/*====================
  CResourceWatcher::Rebuild
  ====================*/
void CResourceWatcher::Rebuild(ResHandle hResource)
{
    m_fOnReload();
}
