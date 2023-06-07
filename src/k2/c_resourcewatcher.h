// (C)2023 S3 Games
// c_resourcewatcher.h
//
//=============================================================================
#ifndef __C_RESOURCE_WATCHER_H__
#define __C_RESOURCE_WATCHER_H__

//=============================================================================
// Headers
//=============================================================================
#include "i_resourcewatcher2.h"
//=============================================================================

//=============================================================================
// CResourceWatcher
//=============================================================================
class CResourceWatcher : public IResourceWatcher
{
    std::function<void ()>      m_fOnReload;
public:
    CResourceWatcher() = delete;
    CResourceWatcher(ResHandle hResource, std::function<void ()> fOnReload);

    void    Rebuild(ResHandle hResource) override;
};
//=============================================================================

#endif //__C_RESOURCE_WATCHER_H__
