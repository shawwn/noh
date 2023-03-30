// (C)2006 S2 Games
// host_commands.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "k2_common.h"

#include "c_host.h"
#include "c_uicmd.h"
#include "c_eventmanager.h"
#include "c_function.h"
//=============================================================================

/*--------------------
  StartServer
  --------------------*/
CMD(StartServer)
{
	return Host.StartServer(vArgList.empty() ? TSNULL : vArgList[0]);
}

UI_VOID_CMD(Start, 0)
{
	cmdStartServer(vArgList.empty() ? TSNULL : vArgList[0]->Evaluate());
}


/*--------------------
  StartClient
  --------------------*/
CMD(StartClient)
{
	return Host.StartClient(vArgList.empty() ? TSNULL : vArgList[0]);
}

UI_VOID_CMD(StartClient, 0)
{
	Console.AddCmdBuffer(_T("StartClient ") + (vArgList.empty() ? TSNULL : QuoteStr(vArgList[0]->Evaluate())));
}


/*--------------------
  SetActiveClient
  --------------------*/
CMD(SetActiveClient)
{
	if (vArgList.empty())
		return false;

	Host.SetActiveClient(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  NextClient
  --------------------*/
CMD(NextClient)
{
	Host.NextClient();
	return true;
}


/*--------------------
  PrevClient
  --------------------*/
CMD(PrevClient)
{
	Host.PrevClient();
	return true;
}


/*--------------------
  StartLocalGame
  --------------------*/
CMD(StartLocalGame)
{
	if (vArgList.size() < 2)
	{
		Console << _T("Invalid arguments") << newl;
		return false;
	}

	Host.StartGame(_T("local"), vArgList[0], ConcatinateArgs(vArgList.begin() + 1, vArgList.end()));
	return true;
}

UI_VOID_CMD(StartLocalGame, 1)
{
	tstring sArgs;
	for (ScriptTokenVector_cit it(vArgList.begin() + 1); it != vArgList.end(); ++it)
		sArgs += (*it)->Evaluate() + SPACE;
	cmdStartLocalGame(vArgList[0]->Evaluate(), sArgs);
}


/*--------------------
  StartGame
  --------------------*/
CMD(StartGame)
{
	if (vArgList.size() < 3)
	{
		Console << _T("Invalid arguments") << newl;
		return false;
	}

	Host.StartGame(vArgList[0], vArgList[1], ConcatinateArgs(vArgList.begin() + 2, vArgList.end()));
	return true;
}

UI_VOID_CMD(StartGame, 2)
{
	tstring sArgs;
	for (ScriptTokenVector_cit it(vArgList.begin() + 2); it != vArgList.end(); ++it)
		sArgs += (*it)->Evaluate() + SPACE;
	cmdStartGame(vArgList[0]->Evaluate(), vArgList[1]->Evaluate(), sArgs);
}


/*--------------------
  StartReplay
  --------------------*/
CMD(StartReplay)
{
	if (vArgList.empty())
	{
		Console << _T("No replay specified") << newl;
		return false;
	}

	if (!Host.StartReplay(vArgList[0]))
	{
		Console << _T("Failed to start replay") << newl;
		return false;
	}
	
	Host.Connect(_T("localhost"), false, true);
	return true;
}

UI_VOID_CMD(StartReplay, 1)
{
	cmdStartReplay(vArgList[0]->Evaluate());
}


/*--------------------
  StopReplay
  --------------------*/
CMD(StopReplay)
{
	Host.StopReplay();
	return true;
}

UI_VOID_CMD(StopReplay, 0)
{
	cmdStopReplay();
}


/*--------------------
  DownloadReplayCompat
  --------------------*/
CMD(DownloadReplayCompat)
{
	if (vArgList.empty())
	{
		Console << _T("No replay specified") << newl;
		return false;
	}

	return true;
}

UI_VOID_CMD(DownloadReplayCompat, 1)
{
	cmdDownloadReplayCompat(vArgList[0]->Evaluate());
}



/*--------------------
  Connect
  --------------------*/
CMD(Connect)
{
	if (vArgList.empty())
	{
		Console << _T("No address specified") << newl;
		return false;
	}

	Host.Connect(vArgList[0], vArgList.size() > 2 ? AtoB(vArgList[2]) : false);
	return true;
}

UI_VOID_CMD(Connect, 1)
{
	Host.Connect(vArgList[0]->Evaluate(), vArgList.size() > 2 ? AtoB(vArgList[2]->Evaluate()) : false);
}


/*--------------------
  Reconnect
  --------------------*/
CMD(Reconnect)
{
	Host.Reconnect();
	return true;
}


/*--------------------
  Disconnect
  --------------------*/
CMD(Disconnect)
{
	Host.DisconnectNextFrame(_T("CMD(Disconnect)"));
	return true;
}


/*--------------------
  StopServer
  --------------------*/
CMD(StopServer)
{
	Host.StopServer();
	return true;
}


/*--------------------
  StopClient
  --------------------*/
CMD(StopClient)
{
	if (vArgList.empty())
		Host.StopClient();
	else
		Host.StopClient(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  Version
  --------------------*/
CMD(Version)
{
	Host.ToggleVersionStamp();
	return true;
}


/*--------------------
  Quit
  --------------------*/
CMD(Quit)
{
	K2System.RestartOnExit(false);
	K2System.Exit(0);
	return true;
}


/*--------------------
  Restart
  --------------------*/
CMD(Restart)
{
	K2System.RestartOnExit(true);
	K2System.Exit(0);
	return true;
}


/*--------------------
  Beep
  --------------------*/
CMD(Beep)
{
	if (vArgList.empty())
		K2System.Beep(500, 200);
	else
		K2System.Beep(AtoI(vArgList[0]), AtoI(vArgList[1]));
	
	return true;
}


/*--------------------
  SystemMemoryInfo
  --------------------*/
CMD(SystemMemoryInfo)
{
	Console << _T("System:") << newl;
	Console << _T("-------------------") << newl;
	Console << _T("Physical: ") << GetByteString(K2System.GetFreePhysicalMemory()) << _T(" / ") << GetByteString(K2System.GetTotalPhysicalMemory()) << newl;
	Console << _T("Virtual: ") << GetByteString(K2System.GetFreeVirtualMemory()) << _T(" / ") << GetByteString(K2System.GetTotalVirtualMemory()) << newl;
	Console << _T("Page File: ") << GetByteString(K2System.GetFreePageFile()) << _T(" / ") << GetByteString(K2System.GetTotalPageFile()) << newl;
	Console << newl;
	Console << _T("This process:") << newl;
	Console << _T("-------------------") << newl;
	Console << _T("Physical: ") << GetByteString(K2System.GetProcessMemoryUsage()) << _T(" ") << ParenStr(K2System.GetProcessMemoryUsage()) << newl;
	Console << _T("Virtual: ") << GetByteString(K2System.GetProcessVirtualMemoryUsage()) << _T(" ") << ParenStr(K2System.GetProcessVirtualMemoryUsage()) << newl;
	Console << _T("Virtual Limit: ") << GetByteString(K2System.GetVirtualMemoryLimit()) << _T(" ") << ParenStr(K2System.GetVirtualMemoryLimit()) << newl;
	
	return true;
}


/*--------------------
  GetActiveClientIndex
  --------------------*/
UI_CMD(GetActiveClientIndex, 0)
{
	return XtoA(Host.GetActiveClientIndex());
}


/*--------------------
  GetActiveClientIndex
  --------------------*/
FUNCTION(GetActiveClientIndex)
{
	return XtoA(Host.GetActiveClientIndex());
}


/*--------------------
  IsClientConnected
  --------------------*/
UI_CMD(IsClientConnected, 0)
{
	return XtoA(Host.IsConnected());
}


/*--------------------
  PreloadWorld
  --------------------*/
CMD(PreloadWorld)
{
	if (vArgList.size() > 0)
		Host.PreloadWorld(vArgList[0]);
	return true;
}


/*--------------------
  PreloadWorld
  --------------------*/
UI_VOID_CMD(PreloadWorld, 1)
{
	Host.PreloadWorld(vArgList[0]->Evaluate());
}

