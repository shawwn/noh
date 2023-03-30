// (C)2005 S2 Games
// misc_cmds.cpp
//
// miscellaneous console commands
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "shell_common.h"

#include "../k2/c_vid.h"
#include "../k2/c_bitmap.h"
#include "../k2/c_action.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_STRINGF(host_screenshotFormat,		"jpg",	CVAR_SAVECONFIG);
CVAR_INTF	(host_screenshotQuality,	90,		CVAR_SAVECONFIG);
//=============================================================================

/*--------------------
  Echo
  --------------------*/
CMD(Echo)
{
	Console << ConcatinateArgs(vArgList) << newl;
	return true;
}


/*--------------------
  ListMods
  --------------------*/
CMD(ListMods)
{
	FileManager.ListMods();
	return true;
}


/*--------------------
  PushMod
  --------------------*/
CMD(PushMod)
{
	if (vArgList.size() < 1)
		return false;

	FileManager.PushMod(vArgList[0]);
	return true;
}


/*--------------------
  Action: Screenshot
  --------------------*/
ACTION_IMPULSE(Screenshot)
{
	tstring sFilename(FileManager.GetNextFileIncrement(4, _T("~/screenshots/shot"), host_screenshotFormat));

	CBitmap screenshot;
	Vid.GetFrameBuffer(screenshot);
	if (host_screenshotFormat == _T("png"))
		screenshot.WritePNG(sFilename);
	else if (host_screenshotFormat == _T("jpg"))
		screenshot.WriteJPEG(sFilename, host_screenshotQuality);
	else
		return;

	Console << _T("Screenshot saved to: ") << sFilename << newl;
}


/*--------------------
  SystemError
  --------------------*/
CMD(SystemError)
{
	K2System.Error(ConcatinateArgs(vArgList));
	return true;
}
