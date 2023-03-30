// (C)2006 S2 Games
// game_client_actions.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameclient.h"
#include "c_clientcommander.h"
#include "c_gameinterfacemanager.h"

#include "../game_shared/c_playercommander.h"
#include "../game_shared/c_entityclientinfo.h"

#include "../k2/c_actionregistry.h"
#include "../k2/c_camera.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_interface.h"
#include "../k2/c_uitrigger.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_FLOAT(	cg_camSensitivityX,		0.1f);
CVAR_FLOAT(	cg_camSensitivityY,		0.1f);

CVAR_FLOAT(	cg_camDragSpeed,		40.0f);
CVAR_FLOAT(	cg_camEdgeScrollSpeed,	1600.0f);

UI_TRIGGER(PetCommands);
UI_TRIGGER(OfficerCommands);
//=============================================================================

/*--------------------
  CameraPitch
  --------------------*/
ACTION_AXIS(CameraPitch)
{
	if (!GameClient.AllowMouseAim())
		return;

	float fScaleFov(1.0f);
	IPlayerEntity *pLocalPlayer(GameClient.GetLocalPlayer());
	if (pLocalPlayer != NULL)
		fScaleFov = pLocalPlayer->GetFovScale();

	GameClient.GetCurrentSnapshot()->AdjustCameraPitch(fDelta * cg_camSensitivityY * fScaleFov);
}


/*--------------------
  CameraYaw
  --------------------*/
ACTION_AXIS(CameraYaw)
{
	if (!GameClient.AllowMouseAim())
		return;

	float fScaleFov(1.0f);
	IPlayerEntity *pLocalPlayer(GameClient.GetLocalPlayer());
	if (pLocalPlayer != NULL)
		fScaleFov = pLocalPlayer->GetFovScale();

	GameClient.GetCurrentSnapshot()->AdjustCameraYaw(fDelta * cg_camSensitivityX * fScaleFov);
}


/*--------------------
  MoveForward
  --------------------*/
ACTION_BUTTON(MoveForward)
{
	if (fValue > 0.0f)
		GameClient.BreakAutoRun();
	
	if (!GameClient.AllowMovement())
	{
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_FORWARD, false);
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_DASH, false);
		return;
	}
	
	if (fDelta == 1.0f && GameClient.CheckDash())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_DASH, true);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_DASH, false);

	if (!GameClient.GetAutoRun())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_FORWARD, fValue > 0.0f);
}


/*--------------------
  MoveBack
  --------------------*/
ACTION_BUTTON(MoveBack)
{
	if (fValue > 0.0f)
		GameClient.BreakAutoRun();

	if (GameClient.AllowMovement())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_BACK, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_BACK, 0.0f);
}


/*--------------------
  MoveLeft
  --------------------*/
ACTION_BUTTON(MoveLeft)
{
	if (GameClient.AllowMovement())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_LEFT, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_LEFT, 0.0f);
}


/*--------------------
  MoveRight
  --------------------*/
ACTION_BUTTON(MoveRight)
{
	if (GameClient.AllowMovement())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_RIGHT, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_RIGHT, 0.0f);
}


/*--------------------
  Sprint
  --------------------*/
ACTION_BUTTON(Sprint)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_SPRINT, fValue > 0.0f);
}

/*--------------------
  Shift
  --------------------*/
ACTION_BUTTON(Shift)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_SHIFT, fValue > 0.0f);
}

/*--------------------
  Ctrl
  --------------------*/
ACTION_BUTTON(Ctrl)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_CTRL, fValue > 0.0f);
}


/*--------------------
  Jump
  --------------------*/
ACTION_BUTTON(Jump)
{
	if (GameClient.AllowMovement())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_UP, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_UP, 0.0f);
}


/*--------------------
  ZoomOut
  --------------------*/
ACTION_IMPULSE(ZoomOut)
{
	CEntityClientInfo *pClient(GameClient.GetLocalClient());
	if (pClient == NULL || !pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
		return;

	IPlayerEntity *pPlayer(pClient->GetPlayerEntity());
	if (pPlayer == NULL)
		return;

	CPlayerCommander *pCommander(pPlayer->GetAsCommander());
	if (pCommander != NULL)
		pCommander->ZoomOut(GameClient.GetCurrentSnapshot());
}


/*--------------------
  ZoomIn
  --------------------*/
ACTION_IMPULSE(ZoomIn)
{
	CEntityClientInfo *pClient(GameClient.GetLocalClient());
	if (pClient == NULL || !pClient->HasFlags(CLIENT_INFO_IS_COMMANDER))
		return;

	IPlayerEntity *pPlayer(pClient->GetPlayerEntity());
	if (pPlayer == NULL)
		return;

	CPlayerCommander *pCommander(pPlayer->GetAsCommander());
	if (pCommander != NULL)
		pCommander->ZoomIn(GameClient.GetCurrentSnapshot());
}


/*--------------------
  InvNext
  --------------------*/
ACTION_IMPULSE(InvNext)
{
	if (GameClient.AllowAttacks())
		GameClient.InvNext();
}


/*--------------------
  InvPrev
  --------------------*/
ACTION_IMPULSE(InvPrev)
{
	if (GameClient.AllowAttacks())
		GameClient.InvPrev();
}


/*--------------------
  InvSelect
  --------------------*/
ACTION_IMPULSE(InvSelect)
{
	GameClient.SelectItem(AtoI(sParam));
}


/*--------------------
  QuickAttack
  --------------------*/
ACTION_BUTTON(QuickAttack)
{
	if (GameClient.AllowAttacks())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_ACTIVATE_PRIMARY, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_ACTIVATE_PRIMARY, 0.0f);
}


/*--------------------
  StrongAttack
  --------------------*/
ACTION_BUTTON(StrongAttack)
{
	if (GameClient.AllowAttacks())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_ACTIVATE_SECONDARY, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_ACTIVATE_SECONDARY, 0.0f);
}


/*--------------------
  Block
  --------------------*/
ACTION_BUTTON(Block)
{
	if (GameClient.AllowAttacks())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_ACTIVATE_TERTIARY, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_ACTIVATE_TERTIARY, 0.0f);
}


/*--------------------
  InventoryX
  --------------------*/
#define INVENTORY_IMPULSE(slot) \
ACTION_IMPULSE(Inventory##slot) \
{ \
	if (!GameClient.AllowAttacks())\
		return;\
\
	int iSlot(slot); \
	GameClient.GetCurrentSnapshot()->SetActivate(iSlot); \
}

// Main inventory
INVENTORY_IMPULSE(0)
INVENTORY_IMPULSE(1)
INVENTORY_IMPULSE(2)
INVENTORY_IMPULSE(3)
INVENTORY_IMPULSE(4)
INVENTORY_IMPULSE(5)
INVENTORY_IMPULSE(6)
INVENTORY_IMPULSE(7)
INVENTORY_IMPULSE(8)
INVENTORY_IMPULSE(9)

// Backpack
INVENTORY_IMPULSE(10)
INVENTORY_IMPULSE(11)
INVENTORY_IMPULSE(12)
INVENTORY_IMPULSE(13)
INVENTORY_IMPULSE(14)
INVENTORY_IMPULSE(15)


/*--------------------
  Use
  --------------------*/
ACTION_BUTTON(Use)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_USE, fValue > 0.0f);
}


/*--------------------
  Repair
  --------------------*/
ACTION_BUTTON(Repair)
{
	if (GameClient.AllowAttacks())
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_REPAIR, fValue > 0.0f);
	else
		GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_REPAIR, 0.0f);
}


/*--------------------
  VoiceChat
  --------------------*/
ACTION_BUTTON(VoiceChat)
{
	if (fValue > 0.0f)
		GameClient.StartVoiceRecording();
	else
		GameClient.StopVoiceRecording();
}


/*--------------------
  VoiceCommands
  --------------------*/
ACTION_IMPULSE(VoiceCommands)
{
	GameClient.VCMenuActive(true);
}


/*--------------------
  VCCancel
  --------------------*/
ACTION_IMPULSE(VCCancel)
{
	GameClient.VCMenuActive(false);
	GameClient.VCSubMenuActive(BUTTON_INVALID);
}


/*--------------------
  VCMain
  --------------------*/
ACTION_IMPULSE(VCMain)
{
	GameClient.VCSubMenuActive(Input.MakeEButton(sParam));
}


/*--------------------
  VCSub
  --------------------*/
ACTION_IMPULSE(VCSub)
{
	GameClient.DoVoiceCommand(Input.MakeEButton(sParam));
}


/*--------------------
  ChatTeam
  --------------------*/
ACTION_IMPULSE(ChatTeam)
{
	CInterface *pInterface(UIManager.GetActiveInterface());
	if (pInterface == NULL)
		return;

	tstring sTargetWidget(pInterface->GetName() + _T("_chat_team_input"));
	if (pInterface->GetWidget(sTargetWidget) == NULL)
		return;

	pInterface->SetActiveWidget(NULL);
	pInterface->SetActiveWidget(pInterface->GetWidget(sTargetWidget));
}

/*--------------------
  ChatSquad
  --------------------*/
ACTION_IMPULSE(ChatSquad)
{
	CInterface *pInterface(UIManager.GetActiveInterface());
	if (pInterface == NULL)
		return;

	tstring sTargetWidget(pInterface->GetName() + _T("_chat_squad_input"));
	if (pInterface->GetWidget(sTargetWidget) == NULL)
		return;

	pInterface->SetActiveWidget(NULL);
	pInterface->SetActiveWidget(pInterface->GetWidget(sTargetWidget));
}


/*--------------------
  ChatAll
  --------------------*/
ACTION_IMPULSE(ChatAll)
{
	CInterface *pInterface(UIManager.GetActiveInterface());
	if (pInterface == NULL)
		return;

	tstring sTargetWidget(pInterface->GetName() + _T("_chat_all_input"));
	if (pInterface->GetWidget(sTargetWidget) == NULL)
		return;

	pInterface->SetActiveWidget(NULL);
	pInterface->SetActiveWidget(pInterface->GetWidget(sTargetWidget));
}

/*--------------------
  ShowChat
  --------------------*/
ACTION_BUTTON(ShowChat)
{
	CInterface *pInterface(UIManager.GetActiveInterface());
	if (pInterface == NULL)
		return;

	tstring sTargetWidget(pInterface->GetName() + _T("_chat_box_popup"));

	IWidget *pWidget(pInterface->GetWidget(sTargetWidget));

	if (pWidget == NULL)
		return;

	if (fValue > 0.0f)
		pWidget->Show();
	else
		pWidget->Hide();
}


/*--------------------
  Cancel
  --------------------*/
ACTION_BUTTON(Cancel)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_CANCEL, fValue > 0.0f);
	
	if (fValue > 0.0f)
		GameClient.Cancel();
}


/*--------------------
  StartPing
  --------------------*/
ACTION_IMPULSE(StartPing)
{
	GameClient.StartPinging();
}


/*--------------------
  ToggleInfoScreen
  --------------------*/
ACTION_IMPULSE(ToggleInfoScreen)
{
	GameClient.ToggleInfoScreen(sParam);
}


/*--------------------
  ShowInfoScreen
  --------------------*/
ACTION_IMPULSE(ShowInfoScreen)
{
	GameClient.ShowInfoScreen(sParam);
}


/*--------------------
  ToggleMenu
  --------------------*/
ACTION_IMPULSE(ToggleMenu)
{
	GameClient.ToggleMenu();
}


/*--------------------
  ToggleLobby
  --------------------*/
ACTION_IMPULSE(ToggleLobby)
{
	GameClient.ToggleLobby();
}


/*--------------------
  CommanderPrimary
  --------------------*/
ACTION_BUTTON(CommanderPrimary)
{
	if (fValue)
		GameClient.TryBuildingPlacement();

	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	if (fValue)
		pCommander->PrimaryDown();
	else
		pCommander->PrimaryUp();
}


/*--------------------
  CommanderSecondary
  --------------------*/
ACTION_BUTTON(CommanderSecondary)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
	{
		GameClient.SetBuildingRotate(fValue > 0.0f);
		return;
	}
	
	if (fValue)
		pCommander->SecondaryDown();
	else
		pCommander->SecondaryUp();
}


/*--------------------
  CommanderTertiary
  --------------------*/
ACTION_BUTTON(CommanderTertiary)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_CMDR_BUTTON_DRAGSCROLL, fValue > 0.0f);
}


/*--------------------
  CommanderPing
  --------------------*/
ACTION_BUTTON(CommanderPing)
{
	if (fValue == 0.0f)
		return;

	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->Ping();
}


/*--------------------
  DragScrollX
  --------------------*/
ACTION_AXIS(DragScrollX)
{
	if (GameClient.GetBuildingRotate())
	{
		GameClient.RotateBuildingPlacement(fDelta);
		return;
	}

	if (!GameClient.IsCommander())
		return;

	CClientSnapshot *pSnapshot(GameClient.GetCurrentSnapshot());
	if (!pSnapshot->IsButtonDown(GAME_CMDR_BUTTON_DRAGSCROLL))
		return;

	CVec3f v3Pos(pSnapshot->GetCameraPosition());
	v3Pos += CVec3f(1.0f, 0.0f, 0.0f) * cg_camDragSpeed * fDelta;
	pSnapshot->SetCameraPosition(v3Pos);
}


/*--------------------
  DragScrollY
  --------------------*/
ACTION_AXIS(DragScrollY)
{
	if (!GameClient.IsCommander())
		return;

	CClientSnapshot *pSnapshot(GameClient.GetCurrentSnapshot());
	if (!pSnapshot->IsButtonDown(GAME_CMDR_BUTTON_DRAGSCROLL))
		return;

	CVec3f v3Pos(pSnapshot->GetCameraPosition());
	v3Pos += CVec3f(0.0f, -1.0f, 0.0f) * cg_camDragSpeed * fDelta;
	pSnapshot->SetCameraPosition(v3Pos);
}


/*--------------------
  CommanderModifier1
  --------------------*/
ACTION_BUTTON(CommanderModifier1)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	if (fValue)
		pCommander->SetModifier1(true);
	else
		pCommander->SetModifier1(false);
}


/*--------------------
  CommanderModifier2
  --------------------*/
ACTION_BUTTON(CommanderModifier2)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	if (fValue)
		pCommander->SetModifier2(true);
	else
		pCommander->SetModifier2(false);
}


/*--------------------
  CommanderModifier3
  --------------------*/
ACTION_BUTTON(CommanderModifier3)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	if (fValue)
		pCommander->SetModifier3(true);
	else
		pCommander->SetModifier3(false);
}


/*--------------------
  AutoRun
  --------------------*/
ACTION_IMPULSE_EX(AutoRun, ACTION_NOREPEAT)
{
	if (GameClient.AllowMovement())
		GameClient.ToggleAutoRun();
}


/*--------------------
  PetCommands
  --------------------*/
ACTION_IMPULSE(PetCommands)
{
	PetCommands.Trigger(SNULL);
}


/*--------------------
  OfficerCommands
  --------------------*/
ACTION_IMPULSE(OfficerCommands)
{
	OfficerCommands.Trigger(SNULL);
}


/*--------------------
  ToggleScoreOverlay
  --------------------*/
ACTION_BUTTON(ToggleScoreOverlay)
{
	CGameInterfaceManager *pInterfaceManger(GameClient.GetInterfaceManager());
	if (pInterfaceManger != NULL)
		pInterfaceManger->ShowScoreOverlay(fValue > 0.0f);
}
