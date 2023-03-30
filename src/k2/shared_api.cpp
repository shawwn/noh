// (C)2005 S2 Games
// shared_api.cpp
//
// Main file for shared dll
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "shared_common.h"

#include "shared_api.h"
#include "c_filemanager.h"
#include "c_input.h"
#include "c_actionregistry.h"
//=============================================================================

//=============================================================================
// Globals
//=============================================================================
SCoreAPI	Core;
//=============================================================================


/*====================
  Shared_GetCoreAPI

  Use this for now to get access to core functions as things are migrated
  to the new dll, hopefully we won't need this at all by the time everything
  is properly arranged
 ====================*/
void	Shared_SetCoreAPI(SCoreAPI *pAPI)
{
	Core = *pAPI;
}


/*====================
  Shared_Release

  Perform any cleanup here, note that after calling this function, all of
  shared's singletons will be invalid
  ====================*/
void	Shared_Release()
{
	pActionRegistry->Release();
	pInput->Release();
	pFileManager->Release();
	pSystem->Release();
	pConsole->Release();

	MemManager.Release();
}
