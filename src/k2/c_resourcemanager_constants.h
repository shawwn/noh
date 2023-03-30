// (C)2010 S2 Games
// c_resourcemanager_constants.h
//
//=============================================================================
#ifndef __C_RESOURCEMANAGER_CONSTANTS_H__
#define __C_RESOURCEMANAGER_CONSTANTS_H__

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
enum EUnregisterResource
{
    // delete the resource.  If a resource with the same path is again registered
    // in the future, it will be assigned the exact same handle.
    UNREG_RESERVE_HANDLE,

    // delete the resource without caring whether the handle is reassigned to
    // a resource with a different path.  This is typically used in situations
    // where the available resource handles could become exhausted over time.
    // For example, downloading player icons --- since there is no upper limit
    // on the number of player icons that could be downloaded over time, then
    // old icons should be unregistered using DELETE_HANDLE.
    UNREG_DELETE_HANDLE
};
//=============================================================================

#endif //__C_RESOURCEMANAGER_CONSTANTS_H__

