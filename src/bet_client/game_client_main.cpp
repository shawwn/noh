// (C)2023 S3 Games
// game_client_main.cpp
//
//=============================================================================
//=============================================================================
// Headers
//=============================================================================
#include "bet_client_common.h"

#include "c_gameclient.h"

#include "../k2/c_clientgamelib.h"
#include "../k2/c_action.h"
//=============================================================================
EXTERN_CVAR_FLOAT(bet_camFov);

CVAR_FLOAT  (bet_camPitch,       -56.0f);
CVAR_FLOAT  (bet_camDistance,    1650.0f);
//=============================================================================

/*--------------------
  Center
  --------------------*/
CMD(Center)
{
    CVec3f v3LookAt;
    if (GameClient.GetLookAtPoint(v3LookAt))
    {
        CVec3f v3Angles(bet_camPitch, 0.0f, 0.0f);

        GameClient.SetCameraAngles(v3Angles);
        GameClient.SetCameraPosition(v3LookAt - M_GetForwardVecFromAngles(v3Angles) * bet_camDistance);
    }

    return true;
}

UI_VOID_CMD(Center, 0)
{
    cmdCenter();
}



/*====================
  CL_Init
  ====================*/
bool    CL_Init(CHostClient *pHostClient)
{
    CGameClient *pNewGameClient(K2_NEW(ctx_Singleton,   CGameClient));
    pNewGameClient->SetGamePointer();
    return pNewGameClient->Init(pHostClient);
}


/*====================
  CL_Frame
  ====================*/
void    CL_Frame()
{
    GameClient.Frame();
}


/*====================
  CL_Shutdown
  ====================*/
void    CL_Shutdown()
{
    Console << _T("Closing client...") << newl;
}


/*====================
  InitLibrary
  ====================*/
K2_DLL_EXPORT void InitLibrary(CClientGameLib &GameLib)
{
    GameLib.SetName(K2System.GetGameName());
    GameLib.SetTypeName(_T("betclient"));
    GameLib.SetMajorVersion(1);
    GameLib.SetMinorVersion(0);
    GameLib.AssignInitFn(CL_Init);
    GameLib.AssignFrameFn(CL_Frame);
    GameLib.AssignShutdownFn(CL_Shutdown);
}



/*--------------------
  Load
  --------------------*/
CMD(Load)
{
    if (vArgList.empty())
    {
        Console << _T("Please specify a world name") << newl;
        return false;
    }

    return GameClient.LoadWorld(vArgList[0]);
}

UI_VOID_CMD(Load, 1)
{
    GameClient.LoadWorld(vArgList[0]->Evaluate());
}


//=============================================================================
// Actions
//=============================================================================

/*--------------------
  CameraPitch
  --------------------*/
ACTION_AXIS(CameraPitch)
{
    GameClient.AdjustCameraPitch(fDelta);
}


/*--------------------
  CameraYaw
  --------------------*/
ACTION_AXIS(CameraYaw)
{
    GameClient.AdjustCameraYaw(fDelta);
}


/*--------------------
  MoveIn
  --------------------*/
ACTION_IMPULSE(MoveIn)
{
    GameClient.ShiftCamera(1.0f);
}


/*--------------------
  MoveOut
  --------------------*/
ACTION_IMPULSE(MoveOut)
{
    GameClient.ShiftCamera(-1.0f);
}

/*--------------------
  Start
  --------------------*/
CMD(Start)
{
    GameClient.Start();
    return true;
}
