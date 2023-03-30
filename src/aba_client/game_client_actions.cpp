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

#include "../aba_shared/c_replaymanager.h"
#include "../aba_shared/c_player.h"
#include "../aba_shared/c_replaymanager.h"

#include "../k2/c_actionregistry.h"
#include "../k2/c_camera.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_interface.h"
#include "../k2/c_inputstate.h"
#include "../k2/c_vid.h"
#include "../k2/c_gamebind.h"
#include "../k2/c_voicemanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_FLOATF(	cam_pushSpeed,		4000.0f,	CVAR_SAVECONFIG);
CVAR_BOOLF(		cam_pushInvertX,	false,	CVAR_SAVECONFIG);
CVAR_BOOLF(		cam_pushInvertY,	false,	CVAR_SAVECONFIG);

INPUT_STATE_BOOL(FreeLook);

bool g_bMouseLook(false);
//=============================================================================

//=============================================================================
// Game binds
//=============================================================================
GAMEBIND_BUTTON(AltInfo, BINDTABLE_GAME, BUTTON_ALT, BIND_MOD_NONE);
GAMEBIND_IMPULSE(ToggleAltInfo, BINDTABLE_GAME, BUTTON_MISC7, BIND_MOD_NONE);
GAMEBIND_BUTTON(AltInfoAllies, BINDTABLE_GAME, BUTTON_MISC4, BIND_MOD_NONE);
GAMEBIND_BUTTON(AltInfoEnemies, BINDTABLE_GAME, BUTTON_MISC6, BIND_MOD_NONE);
GAMEBIND_BUTTON(CommanderPrimary, BINDTABLE_GAME, BUTTON_MOUSEL, BIND_MOD_NONE);
GAMEBIND_BUTTON(CommanderSecondary, BINDTABLE_GAME, BUTTON_MOUSER, BIND_MOD_NONE);
GAMEBIND_BUTTON(CommanderModifier1, BINDTABLE_GAME, BUTTON_SHIFT, BIND_MOD_NONE);
GAMEBIND_BUTTON(CommanderModifier2, BINDTABLE_GAME, BUTTON_CTRL, BIND_MOD_NONE);
GAMEBIND_BUTTON(FrontQueueModifier, BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_BUTTON(AlternateModifier, BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);

GAMEBIND_BUTTON(CameraDrag, BINDTABLE_GAME, BUTTON_MOUSEM, BIND_MOD_NONE);
GAMEBIND_IMPULSE(CommanderPing, BINDTABLE_GAME, BUTTON_MOUSEL, BIND_MOD_ALT);
GAMEBIND_BUTTON(Cancel, BINDTABLE_GAME, BUTTON_ESC, BIND_MOD_NONE);
GAMEBIND_IMPULSE(ToggleMenu, BINDTABLE_GAME, BUTTON_F10, BIND_MOD_NONE);
GAMEBIND_IMPULSE(ToggleShop, BINDTABLE_GAME, EButton('B'), BIND_MOD_NONE);

GAMEBIND_IMPULSE(ChatTeam, BINDTABLE_GAME, BUTTON_ENTER, BIND_MOD_NONE);
GAMEBIND_IMPULSE(ChatAll, BINDTABLE_GAME, BUTTON_ENTER, BIND_MOD_SHIFT);
GAMEBIND_BUTTON(ShowChat, BINDTABLE_GAME, EButton('Z'), BIND_MOD_NONE);
GAMEBIND_IMPULSE(SwitchScore, BINDTABLE_GAME, EButton('['), BIND_MOD_NONE);

GAMEBIND_IMPULSE(PrevAttackModifier, BINDTABLE_GAME, BUTTON_COMMA, BIND_MOD_NONE);
GAMEBIND_IMPULSE(NextAttackModifier, BINDTABLE_GAME, BUTTON_PERIOD, BIND_MOD_NONE);

GAMEBIND_BUTTON(Center, BINDTABLE_GAME,	EButton('C'), BIND_MOD_NONE);
GAMEBIND_BUTTON(CenterInfo, BINDTABLE_GAME,	EButton('V'), BIND_MOD_NONE);

GAMEBIND_BUTTON(MoveForward, BINDTABLE_GAME, BUTTON_UP, BIND_MOD_NONE);
GAMEBIND_BUTTON(MoveLeft, BINDTABLE_GAME, BUTTON_LEFT, BIND_MOD_NONE);
GAMEBIND_BUTTON(MoveBack, BINDTABLE_GAME, BUTTON_DOWN, BIND_MOD_NONE);
GAMEBIND_BUTTON(MoveRight, BINDTABLE_GAME, BUTTON_RIGHT, BIND_MOD_NONE);

GAMEBIND_IMPULSE(SelectHero, BINDTABLE_GAME, BUTTON_F1, BIND_MOD_NONE);
GAMEBIND_IMPULSE(OrderAttack, BINDTABLE_GAME, EButton('A'), BIND_MOD_NONE);
GAMEBIND_IMPULSE(OrderStop, BINDTABLE_GAME,	EButton('S'), BIND_MOD_NONE);
GAMEBIND_IMPULSE(OrderHold, BINDTABLE_GAME,	EButton('H'), BIND_MOD_NONE);
GAMEBIND_IMPULSE(OrderCancelAndHold, BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE(OrderMove, BINDTABLE_GAME,	EButton('M'), BIND_MOD_NONE);
GAMEBIND_IMPULSE(OrderPatrol, BINDTABLE_GAME, EButton('P'), BIND_MOD_NONE);

GAMEBIND_IMPULSE(NextUnit, BINDTABLE_GAME, BUTTON_TAB, BIND_MOD_NONE);
GAMEBIND_IMPULSE(PrevUnit, BINDTABLE_GAME, BUTTON_TAB, BIND_MOD_SHIFT);
GAMEBIND_IMPULSE(NextUnitCentered, BINDTABLE_GAME, BUTTON_TAB, BIND_MOD_CTRL);
GAMEBIND_IMPULSE(PrevUnitCentered, BINDTABLE_GAME, BUTTON_TAB, BIND_MOD_SHIFT | BIND_MOD_CTRL);
GAMEBIND_IMPULSE(NextInventoryUnit, BINDTABLE_GAME, BUTTON_MISC3, BIND_MOD_NONE);
GAMEBIND_IMPULSE(PrevInventoryUnit, BINDTABLE_GAME, BUTTON_MISC3, BIND_MOD_SHIFT);
GAMEBIND_IMPULSE(NextInventoryUnitCentered, BINDTABLE_GAME, BUTTON_MISC3, BIND_MOD_CTRL);
GAMEBIND_IMPULSE(PrevInventoryUnitCentered, BINDTABLE_GAME, BUTTON_MISC3, BIND_MOD_SHIFT | BIND_MOD_CTRL);

GAMEBIND_IMPULSE_EX(Ability1, ActivateTool, _T("0"), BINDTABLE_GAME, EButton('Q'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability2, ActivateTool, _T("1"), BINDTABLE_GAME, EButton('W'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability3, ActivateTool, _T("2"), BINDTABLE_GAME, EButton('E'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability4, ActivateTool, _T("3"), BINDTABLE_GAME, EButton('R'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Stats, ActivateTool, _T("4"), BINDTABLE_GAME, EButton('O'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability5, ActivateTool, _T("5"), BINDTABLE_GAME, EButton('D'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability6, ActivateTool, _T("6"), BINDTABLE_GAME, EButton('F'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability7, ActivateTool, _T("7"), BINDTABLE_GAME, EButton('G'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability8, ActivateTool, _T("8"), BINDTABLE_GAME, EButton('N'), BIND_MOD_NONE);

GAMEBIND_IMPULSE_EX(Ability1Secondary, ActivateToolSecondary, _T("0"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability2Secondary, ActivateToolSecondary, _T("1"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability3Secondary, ActivateToolSecondary, _T("2"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability4Secondary, ActivateToolSecondary, _T("3"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability5Secondary, ActivateToolSecondary, _T("5"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability6Secondary, ActivateToolSecondary, _T("6"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability7Secondary, ActivateToolSecondary, _T("7"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Ability8Secondary, ActivateToolSecondary, _T("8"), BINDTABLE_GAME, BUTTON_INVALID, BIND_MOD_NONE);

GAMEBIND_IMPULSE2_EX(Backpack1, ActivateTool, _T("28"), BINDTABLE_GAME, BUTTON_NUM4, BIND_MOD_NONE, EButton('Q'), BIND_MOD_ALT);
GAMEBIND_IMPULSE2_EX(Backpack2, ActivateTool, _T("29"), BINDTABLE_GAME, BUTTON_NUM5, BIND_MOD_NONE, EButton('W'), BIND_MOD_ALT);
GAMEBIND_IMPULSE2_EX(Backpack3, ActivateTool, _T("30"), BINDTABLE_GAME, BUTTON_NUM6, BIND_MOD_NONE, EButton('E'), BIND_MOD_ALT);
GAMEBIND_IMPULSE2_EX(Backpack4, ActivateTool, _T("31"), BINDTABLE_GAME, BUTTON_NUM1, BIND_MOD_NONE, EButton('A'), BIND_MOD_ALT);
GAMEBIND_IMPULSE2_EX(Backpack5, ActivateTool, _T("32"), BINDTABLE_GAME, BUTTON_NUM2, BIND_MOD_NONE, EButton('S'), BIND_MOD_ALT);
GAMEBIND_IMPULSE2_EX(Backpack6, ActivateTool, _T("33"), BINDTABLE_GAME, BUTTON_NUM3, BIND_MOD_NONE, EButton('D'), BIND_MOD_ALT);

GAMEBIND_IMPULSE_EX(Shared1, ActivateTool, _T("25"), BINDTABLE_GAME, EButton('I'), BIND_MOD_NONE);

GAMEBIND_IMPULSE_EX(RecallSelection1, RecallSelection, _T("1"), BINDTABLE_GAME, EButton('1'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection2, RecallSelection, _T("2"), BINDTABLE_GAME, EButton('2'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection3, RecallSelection, _T("3"), BINDTABLE_GAME, EButton('3'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection4, RecallSelection, _T("4"), BINDTABLE_GAME, EButton('4'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection5, RecallSelection, _T("5"), BINDTABLE_GAME, EButton('5'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection6, RecallSelection, _T("6"), BINDTABLE_GAME, EButton('6'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection7, RecallSelection, _T("7"), BINDTABLE_GAME, EButton('7'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection8, RecallSelection, _T("8"), BINDTABLE_GAME, EButton('8'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection9, RecallSelection, _T("9"), BINDTABLE_GAME, EButton('9'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(RecallSelection0, RecallSelection, _T("0"), BINDTABLE_GAME, EButton('0'), BIND_MOD_NONE);

GAMEBIND_IMPULSE_EX(SaveSelection1, SaveSelection, _T("1"), BINDTABLE_GAME, EButton('1'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection2, SaveSelection, _T("2"), BINDTABLE_GAME, EButton('2'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection3, SaveSelection, _T("3"), BINDTABLE_GAME, EButton('3'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection4, SaveSelection, _T("4"), BINDTABLE_GAME, EButton('4'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection5, SaveSelection, _T("5"), BINDTABLE_GAME, EButton('5'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection6, SaveSelection, _T("6"), BINDTABLE_GAME, EButton('6'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection7, SaveSelection, _T("7"), BINDTABLE_GAME, EButton('7'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection8, SaveSelection, _T("8"), BINDTABLE_GAME, EButton('8'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection9, SaveSelection, _T("9"), BINDTABLE_GAME, EButton('9'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(SaveSelection0, SaveSelection, _T("0"), BINDTABLE_GAME, EButton('0'), BIND_MOD_CTRL);

GAMEBIND_IMPULSE_EX(AddToSelection1, AddToSelection, _T("1"), BINDTABLE_GAME, EButton('1'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection2, AddToSelection, _T("2"), BINDTABLE_GAME, EButton('2'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection3, AddToSelection, _T("3"), BINDTABLE_GAME, EButton('3'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection4, AddToSelection, _T("4"), BINDTABLE_GAME, EButton('4'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection5, AddToSelection, _T("5"), BINDTABLE_GAME, EButton('5'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection6, AddToSelection, _T("6"), BINDTABLE_GAME, EButton('6'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection7, AddToSelection, _T("7"), BINDTABLE_GAME, EButton('7'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection8, AddToSelection, _T("8"), BINDTABLE_GAME, EButton('8'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection9, AddToSelection, _T("9"), BINDTABLE_GAME, EButton('9'), BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(AddToSelection0, AddToSelection, _T("0"), BINDTABLE_GAME, EButton('0'), BIND_MOD_SHIFT);

GAMEBIND_IMPULSE_EX(RemoveFromSelection1, RemoveFromSelection, _T("1"), BINDTABLE_GAME, EButton('1'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection2, RemoveFromSelection, _T("2"), BINDTABLE_GAME, EButton('2'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection3, RemoveFromSelection, _T("3"), BINDTABLE_GAME, EButton('3'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection4, RemoveFromSelection, _T("4"), BINDTABLE_GAME, EButton('4'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection5, RemoveFromSelection, _T("5"), BINDTABLE_GAME, EButton('5'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection6, RemoveFromSelection, _T("6"), BINDTABLE_GAME, EButton('6'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection7, RemoveFromSelection, _T("7"), BINDTABLE_GAME, EButton('7'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection8, RemoveFromSelection, _T("8"), BINDTABLE_GAME, EButton('8'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection9, RemoveFromSelection, _T("9"), BINDTABLE_GAME, EButton('9'), BIND_MOD_CTRL | BIND_MOD_SHIFT);
GAMEBIND_IMPULSE_EX(RemoveFromSelection0, RemoveFromSelection, _T("0"), BINDTABLE_GAME, EButton('0'), BIND_MOD_CTRL | BIND_MOD_SHIFT);

GAMEBIND_IMPULSE(ZoomOut, BINDTABLE_GAME, BUTTON_WHEELDOWN, BIND_MOD_NONE);
GAMEBIND_IMPULSE(ZoomIn, BINDTABLE_GAME, BUTTON_WHEELUP, BIND_MOD_NONE);
GAMEBIND_BUTTON(FreeLook, BINDTABLE_GAME, BUTTON_MISC5, BIND_MOD_NONE);

GAMEBIND_IMPULSE(ResetCamera, BINDTABLE_GAME, BUTTON_BACKSPACE, BIND_MOD_NONE);
GAMEBIND_IMPULSE(CameraUp, BINDTABLE_GAME, BUTTON_WHEELDOWN, BIND_MOD_CTRL);
GAMEBIND_IMPULSE(CameraDown, BINDTABLE_GAME, BUTTON_WHEELUP, BIND_MOD_CTRL);

GAMEBIND_BUTTON(VoicePushToTalk, BINDTABLE_GAME, EButton('T'), BIND_MOD_NONE);
GAMEBIND_BUTTON(VoicePushToLaneTalk, BINDTABLE_GAME, EButton('Y'), BIND_MOD_NONE);

GAMEBIND_IMPULSE_EX(ToggleSound, ToggleCvar, _T("sound_mute"), BINDTABLE_GAME, EButton('S'), BIND_MOD_CTRL);
GAMEBIND_IMPULSE_EX(ToggleMusic, ToggleCvar, _T("sound_muteMusic"), BINDTABLE_GAME, EButton('M'), BIND_MOD_CTRL);

//GAMEBIND_BUTTON(Quit, BINDTABLE_GAME, BUTTON_F4, BIND_MOD_ALT);

GAMEBIND_IMPULSE(ToggleLevelup, BINDTABLE_GAME, EButton('L'), BIND_MOD_NONE);

GAMEBIND_IMPULSE(FocusLastEvent, BINDTABLE_GAME, BUTTON_MISC2, BIND_MOD_NONE);

GAMEBIND_IMPULSE_EX(Shop1, Shop, _T("0"), BINDTABLE_GAME_SHOP, EButton('Q'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop2, Shop, _T("1"), BINDTABLE_GAME_SHOP, EButton('W'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop3, Shop, _T("2"), BINDTABLE_GAME_SHOP, EButton('E'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop4, Shop, _T("3"), BINDTABLE_GAME_SHOP, EButton('R'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop5, Shop, _T("4"), BINDTABLE_GAME_SHOP, EButton('T'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop6, Shop, _T("5"), BINDTABLE_GAME_SHOP, EButton('A'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop7, Shop, _T("6"), BINDTABLE_GAME_SHOP, EButton('S'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop8, Shop, _T("7"), BINDTABLE_GAME_SHOP, EButton('D'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop9, Shop, _T("8"), BINDTABLE_GAME_SHOP, EButton('F'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop10, Shop, _T("9"), BINDTABLE_GAME_SHOP, EButton('G'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop11, Shop, _T("10"), BINDTABLE_GAME_SHOP, EButton('Y'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop12, Shop, _T("11"), BINDTABLE_GAME_SHOP, EButton('U'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop13, Shop, _T("12"), BINDTABLE_GAME_SHOP, EButton('I'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop14, Shop, _T("13"), BINDTABLE_GAME_SHOP, EButton('O'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop15, Shop, _T("14"), BINDTABLE_GAME_SHOP, EButton('P'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop16, Shop, _T("15"), BINDTABLE_GAME_SHOP, EButton('H'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop17, Shop, _T("16"), BINDTABLE_GAME_SHOP, EButton('J'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop18, Shop, _T("17"), BINDTABLE_GAME_SHOP, EButton('K'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop19, Shop, _T("18"), BINDTABLE_GAME_SHOP, EButton('L'), BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(Shop20, Shop, _T("19"), BINDTABLE_GAME_SHOP, BUTTON_MISC1, BIND_MOD_NONE);
GAMEBIND_IMPULSE_EX(ShopClosePanel, Shop, _T("-1"), BINDTABLE_GAME_SHOP, BUTTON_BACKSPACE, BIND_MOD_NONE);
GAMEBIND_IMPULSE(RecipeBack, BINDTABLE_GAME_SHOP, BUTTON_MISC4, BIND_MOD_NONE);
GAMEBIND_IMPULSE(RecipeForward, BINDTABLE_GAME_SHOP, BUTTON_MISC6, BIND_MOD_NONE);

GAMEBIND_BUTTON(Sprint, BINDTABLE_GAME, BUTTON_SPACE, BIND_MOD_NONE);

EXTERN_CVAR_BOOL(cg_alwaysShowHealthBars);
//=============================================================================

/*--------------------
  ToggleAltInfo
  --------------------*/
ACTION_IMPULSE(ToggleAltInfo)
{
	cg_alwaysShowHealthBars.Toggle();
}

/*--------------------
  MoveForward
  --------------------*/
ACTION_BUTTON(MoveForward)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_FORWARD, fValue > 0.0f);
}


/*--------------------
  MoveBack
  --------------------*/
ACTION_BUTTON(MoveBack)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_BACK, fValue > 0.0f);
}


/*--------------------
  MoveLeft
  --------------------*/
ACTION_BUTTON(MoveLeft)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_LEFT, fValue > 0.0f);
}


/*--------------------
  MoveRight
  --------------------*/
ACTION_BUTTON(MoveRight)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_RIGHT, fValue > 0.0f);
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
	GameClient.GetCurrentSnapshot()->SetButton(GAME_BUTTON_UP, fValue > 0.0f);
}


/*--------------------
  ZoomOut
  --------------------*/
ACTION_IMPULSE(ZoomOut)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer != NULL)
		pPlayer->ZoomOut(*GameClient.GetCurrentSnapshot());
}


/*--------------------
  ZoomIn
  --------------------*/
ACTION_IMPULSE(ZoomIn)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer != NULL)
		pPlayer->ZoomIn(*GameClient.GetCurrentSnapshot());
}


/*--------------------
  CameraUp
  --------------------*/
ACTION_IMPULSE(CameraUp)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer != NULL)
		pPlayer->AdjustCameraHeight(1.0f);
}


/*--------------------
  CameraDown
  --------------------*/
ACTION_IMPULSE(CameraDown)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer != NULL)
		pPlayer->AdjustCameraHeight(-1.0f);
}


/*--------------------
  CameraYaw
  --------------------*/
ACTION_AXIS(CameraYaw)
{
	if (cam_mode == 0)
	{
		if (!FreeLook)
		{
			CClientSnapshot *pSnapshot(GameClient.GetCurrentSnapshot());
			if (!pSnapshot->IsButtonDown(GAME_CMDR_BUTTON_DRAGSCROLL))
				return;

			fDelta /= Vid.GetScreenW();

			CVec3f v3Pos(pSnapshot->GetCameraPosition());
			float fSign(cam_pushInvertX ? -1.0f : 1.0f);
			v3Pos += CVec3f(fSign, 0.0f, 0.0f) * cam_pushSpeed * fDelta;
			pSnapshot->SetCameraPosition(v3Pos);

			return;
		}

		CPlayer *pPlayer(GameClient.GetLocalPlayer());
		if ((pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_LOCAL)) || ReplayManager.IsPlaying())
			pPlayer->AdjustYaw(fDelta * 0.25f);
	}
	else if (g_bMouseLook)
	{
		CPlayer *pPlayer(GameClient.GetLocalPlayer());
		if (pPlayer != NULL)
			pPlayer->AdjustYaw(fDelta * 0.5f);
	}
}


/*--------------------
  CameraPitch
  --------------------*/
ACTION_AXIS(CameraPitch)
{
	if (cam_mode == 0)
	{
		if (!FreeLook)
		{
			CClientSnapshot *pSnapshot(GameClient.GetCurrentSnapshot());
			if (!pSnapshot->IsButtonDown(GAME_CMDR_BUTTON_DRAGSCROLL))
				return;

			fDelta /= Vid.GetScreenH();

			CVec3f v3Pos(pSnapshot->GetCameraPosition());
			float fSign(cam_pushInvertY ? 1.0f : -1.0f);
			v3Pos += CVec3f(0.0f, fSign, 0.0f) * cam_pushSpeed * fDelta;
			pSnapshot->SetCameraPosition(v3Pos);

			return;
		}

		CPlayer *pPlayer(GameClient.GetLocalPlayer());
		if ((pPlayer != NULL && pPlayer->HasFlags(PLAYER_FLAG_LOCAL)) || ReplayManager.IsPlaying())
			pPlayer->AdjustPitch(fDelta * 0.25f);
	}
	else if (g_bMouseLook)
	{
		CPlayer *pPlayer(GameClient.GetLocalPlayer());
		if (pPlayer != NULL)
			pPlayer->AdjustPitch(fDelta * 0.5f);
	}
}


/*--------------------
  ResetCamera
  --------------------*/
ACTION_IMPULSE(ResetCamera)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer != NULL)
		pPlayer->ResetCamera();
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
	GameClient.ToggleMenu();
}


/*--------------------
  CommanderPrimary
  --------------------*/
ACTION_BUTTON(CommanderPrimary)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	if (fValue)
		pCommander->PrimaryDown(v2Cursor);
	else
		pCommander->PrimaryUp(v2Cursor);
}


/*--------------------
  CommanderSecondary
  --------------------*/
ACTION_BUTTON(CommanderSecondary)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;
	
	if (fValue)
		pCommander->SecondaryDown(v2Cursor);
	else
		pCommander->SecondaryUp(v2Cursor);
}


/*--------------------
  CameraDrag
  --------------------*/
ACTION_BUTTON(CameraDrag)
{
	if (cam_mode == 0)
	{
		CPlayer *pPlayer(GameClient.GetLocalPlayer());
		if (pPlayer == NULL)
			return;
		
		if (fValue)
			pPlayer->StartDrag(GameClient.GetCamera());
		else
			pPlayer->EndDrag();
	}
	else
	{
		g_bMouseLook = fValue != 0.0f;
	}
}


/*--------------------
  CameraScroll
  --------------------*/
ACTION_BUTTON(CameraScroll)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer == NULL)
		return;
	
	if (fValue)
		pPlayer->StartScroll();
	else
		pPlayer->EndScroll();
}


/*--------------------
  CameraPush
  --------------------*/
ACTION_BUTTON(CameraPush)
{
	GameClient.GetCurrentSnapshot()->SetButton(GAME_CMDR_BUTTON_DRAGSCROLL, fValue > 0.0f);
}


/*--------------------
  SelectHero
  --------------------*/
ACTION_IMPULSE(SelectHero)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer == NULL)
		return;

	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->SelectEntity(pPlayer->GetHeroIndex());
}


/*--------------------
  SaveSelection
  --------------------*/
ACTION_IMPULSE(SaveSelection)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	uint uiIndex(AtoI(sParam));
	pCommander->SaveSelectionSet(uiIndex);
}


/*--------------------
  AddToSelection
  --------------------*/
ACTION_IMPULSE(AddToSelection)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	uint uiIndex(AtoI(sParam));
	pCommander->AddToSelectionSet(uiIndex);
}


/*--------------------
  RemoveFromSelection
  --------------------*/
ACTION_IMPULSE(RemoveFromSelection)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	uint uiIndex(AtoI(sParam));
	pCommander->RemoveFromSelectionSet(uiIndex);
}


/*--------------------
  RecallSelection
  --------------------*/
ACTION_IMPULSE(RecallSelection)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	uint uiIndex(AtoI(sParam));
	pCommander->RecallSelectionSet(uiIndex);
}


/*--------------------
  NextUnit
  --------------------*/
ACTION_IMPULSE(NextUnit)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->NextUnit(false);
}


/*--------------------
  PrevUnit
  --------------------*/
ACTION_IMPULSE(PrevUnit)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->PrevUnit(false);
}


/*--------------------
  NextUnitCentered
  --------------------*/
ACTION_IMPULSE(NextUnitCentered)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->NextUnit(true);
}


/*--------------------
  PrevUnitCentered
  --------------------*/
ACTION_IMPULSE(PrevUnitCentered)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->PrevUnit(true);
}


/*--------------------
  NextInventoryUnit
  --------------------*/
ACTION_IMPULSE(NextInventoryUnit)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->NextInventoryUnit(false);
}


/*--------------------
  PrevInventoryUnit
  --------------------*/
ACTION_IMPULSE(PrevInventoryUnit)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->PrevInventoryUnit(false);
}


/*--------------------
  NextInventoryUnitCentered
  --------------------*/
ACTION_IMPULSE(NextInventoryUnitCentered)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->NextInventoryUnit(true);
}


/*--------------------
  PrevInventoryUnitCentered
  --------------------*/
ACTION_IMPULSE(PrevInventoryUnitCentered)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	pCommander->PrevInventoryUnit(true);
}


/*--------------------
  CommanderPing
  --------------------*/
ACTION_IMPULSE(CommanderPing)
{
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
	CClientSnapshot *pSnapshot(GameClient.GetCurrentSnapshot());
	if (!pSnapshot->IsButtonDown(GAME_CMDR_BUTTON_DRAGSCROLL))
		return;

	CVec3f v3Pos(pSnapshot->GetCameraPosition());
	float fSign(cam_pushInvertX ? -1.0f : 1.0f);
	v3Pos += CVec3f(fSign, 0.0f, 0.0f) * cam_pushSpeed * fDelta;
	pSnapshot->SetCameraPosition(v3Pos);
}


/*--------------------
  DragScrollY
  --------------------*/
ACTION_AXIS(DragScrollY)
{
	CClientSnapshot *pSnapshot(GameClient.GetCurrentSnapshot());
	if (!pSnapshot->IsButtonDown(GAME_CMDR_BUTTON_DRAGSCROLL))
		return;

	CVec3f v3Pos(pSnapshot->GetCameraPosition());
	float fSign(cam_pushInvertY ? 1.0f : -1.0f);
	v3Pos += CVec3f(0.0f, fSign, 0.0f) * cam_pushSpeed * fDelta;
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
  FrontQueueModifier
  --------------------*/
ACTION_BUTTON(FrontQueueModifier)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	if (fValue)
		pCommander->SetFrontQueueModifier(true);
	else
		pCommander->SetFrontQueueModifier(false);
}


/*--------------------
  OrderMove
  --------------------*/
ACTION_IMPULSE(OrderMove)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;
	pCommander->SetCommanderState(COMSTATE_MOVE);
}


/*--------------------
  OrderStop
  --------------------*/
ACTION_IMPULSE(OrderStop)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	CBufferFixed<5> buffer;
	buffer << GAME_CMD_ORDER_STOP
		<< (pCommander->GetModifier2() ? pCommander->GetActiveControlEntity() : INVALID_INDEX);
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  OrderHold
  --------------------*/
ACTION_IMPULSE(OrderHold)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	CBufferFixed<6> buffer;
	buffer << GAME_CMD_ORDER_HOLD
		<< (pCommander->GetModifier1() ? (pCommander->GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE)
		<< (pCommander->GetModifier2() ? pCommander->GetActiveControlEntity() : INVALID_INDEX);
	GameClient.SendGameData(buffer, true);
}

/*--------------------
  OrderCancelAndHold
  --------------------*/
ACTION_IMPULSE(OrderCancelAndHold)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	CBufferFixed<6> buffer;
	buffer << GAME_CMD_ORDER_CANCEL_AND_HOLD
		<< (pCommander->GetModifier1() ? (pCommander->GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE)
		<< (pCommander->GetModifier2() ? pCommander->GetActiveControlEntity() : INVALID_INDEX);
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  OrderAttack
  --------------------*/
ACTION_IMPULSE(OrderAttack)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;
	pCommander->SetCommanderState(COMSTATE_ATTACK);
}


/*--------------------
  OrderPatrol
  --------------------*/
ACTION_IMPULSE(OrderPatrol)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;
	pCommander->SetCommanderState(COMSTATE_PATROL);
}


/*--------------------
  ActivateTool
  --------------------*/
ACTION_IMPULSE(ActivateTool)
{
	if (GameClient.GetClientCommander() == NULL)
		return;
	GameClient.GetClientCommander()->ActivateTool(AtoI(sParam), false, GameClient.GetClientCommander()->GetSelectedControlEntity(), false);
}


/*--------------------
  ActivateToolSecondary
  --------------------*/
ACTION_IMPULSE(ActivateToolSecondary)
{
	if (GameClient.GetClientCommander() == NULL)
		return;
	GameClient.GetClientCommander()->ActivateTool(AtoI(sParam), true, GameClient.GetClientCommander()->GetSelectedControlEntity(), false);
}


/*--------------------
  ToggleShop
  --------------------*/
ACTION_IMPULSE(ToggleShop)
{
	GameClient.GetInterfaceManager()->ToggleShopInterface();
}


/*--------------------
  OpenShop
  --------------------*/
ACTION_IMPULSE(OpenShop)
{
	GameClient.GetInterfaceManager()->SetShopVisible(true);
}


/*--------------------
  CloseShop
  --------------------*/
ACTION_IMPULSE(CloseShop)
{
	GameClient.GetInterfaceManager()->SetShopVisible(false);
}


/*--------------------
  PrevAttackModifier
  --------------------*/
ACTION_IMPULSE(PrevAttackModifier)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	IUnitEntity *pUnit(pCommander->GetSelectedControlEntity());
	if (pUnit == NULL)
		return;

	CBufferFixed<5> buffer;
	buffer << GAME_CMD_PREV_ATTACK_MOD_SLOT << pUnit->GetIndex();
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  NextAttackModifier
  --------------------*/
ACTION_IMPULSE(NextAttackModifier)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	IUnitEntity *pUnit(pCommander->GetSelectedControlEntity());
	if (pUnit == NULL)
		return;

	CBufferFixed<5> buffer;
	buffer << GAME_CMD_NEXT_ATTACK_MOD_SLOT << pUnit->GetIndex();
	GameClient.SendGameData(buffer, true);
}

/*--------------------
  VoicePushToTalk
  --------------------*/
ACTION_BUTTON(VoicePushToTalk)
{
	if (fValue)
		VoiceManager.SetTalkPushed(true);
	else
		VoiceManager.SetTalkPushed(false);
}


/*--------------------
  VoicePushToLaneTalk
  --------------------*/
ACTION_BUTTON(VoicePushToLaneTalk)
{
	if (fValue)
		VoiceManager.SetLaneTalkPushed(true);
	else
		VoiceManager.SetLaneTalkPushed(false);
}


/*--------------------
  SwitchScore
  --------------------*/
ACTION_IMPULSE(SwitchScore)
{
	CGameInterfaceManager *pInterfaceManager(GameClient.GetInterfaceManager());
	if (pInterfaceManager == NULL)
		return;
	
	uint uiScoreState = pInterfaceManager->GetScoreState();

	if (uiScoreState <= 1)
		uiScoreState++;
	else
		uiScoreState = 0;

	pInterfaceManager->SetScoreState(uiScoreState);
}


/*--------------------
  ToggleLevelup
  --------------------*/
ACTION_IMPULSE(ToggleLevelup)
{
	GameClient.ToggleLevelup();
}


#if 0
/*--------------------
  Levelup
  --------------------*/
ACTION_IMPULSE(Levelup)
{
	GameClient.SetLevelup(true);
}
#endif


/*--------------------
  Shop
  --------------------*/
ACTION_IMPULSE(Shop)
{
	int iIndex(AtoI(sParam));
	GameClient.Shop(iIndex);
}


/*--------------------
  FocusLastEvent
  --------------------*/
ACTION_IMPULSE(FocusLastEvent)
{
	CVec3f v3Pos(GameClient.GetLastEventPos());

	CPlayer *pPlayer(Game.GetLocalPlayer());

	if (pPlayer != NULL)
		v3Pos[Z] = pPlayer->GetCameraHeight();

	GameClient.GetCurrentSnapshot()->SetCameraPosition(v3Pos);
}


/*--------------------
  Sprint
  --------------------*/
ACTION_BUTTON_EX(Sprint, ACTION_NOREPEAT)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;
	pCommander->UpdateSprint(fValue != 0.0f);
}

