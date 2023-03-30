// (C)2010 S2 Games
// c_filechangecallback.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_filemanager.h"
#include "c_filechangecallback.h"
//=============================================================================


/*====================
  IFileChangeCallback::IFileChangeCallback
  ====================*/
IFileChangeCallback::IFileChangeCallback(const tstring &sPath) :
m_sPath(FileManager.SanitizePath(sPath)),
m_bExecuting(false)
{
}

