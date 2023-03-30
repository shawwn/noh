// (C)2006 S2 Games
// game_client_commands.cpp
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
#include "../game_shared/i_persistantitem.h"
#include "../game_shared/c_teaminfo.h"

#include "../k2/c_eventcmd.h"
#include "../k2/c_buffer.h"
#include "../k2/c_vid.h"
#include "../k2/c_camera.h"
#include "../k2/c_bitmap.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_sample.h"
#include "../k2/c_input.h"
#include "../k2/c_uicmd.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_function.h"
#include "../k2/c_statestring.h"
//=============================================================================

UI_TRIGGER(KarmaList);

/*--------------------
  Unit
  --------------------*/
CMD(Unit)
{
	if (vArgList.empty())
		return false;

	ushort unID(EntityRegistry.LookupID(_T("Player_") + vArgList[0]));
	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("Invalid class name") << newl;
		return false;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_CHANGE_UNIT << unID;
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(Unit, 1)
{
	cmdUnit(vArgList[0]->Evaluate());
}


/*--------------------
  SwapInventory
  --------------------*/
CMD(SwapInventory)
{
	if (vArgList.size() < 2)
		return false;

	int iSlot1 = AtoI(vArgList[0]);
	int iSlot2 = AtoI(vArgList[1]);

	if (iSlot1 == iSlot2)
		return true;

	if (iSlot1 < 1 || iSlot1 > INVENTORY_END_BACKPACK)
		return true;

	if (iSlot2 < 1 || iSlot2 > INVENTORY_END_BACKPACK)
		return true;

	if ((iSlot1 < INVENTORY_START_BACKPACK && iSlot2 >= INVENTORY_START_BACKPACK) ||
		(iSlot2 < INVENTORY_START_BACKPACK && iSlot1 >= INVENTORY_START_BACKPACK))
		return true;

	CBufferDynamic buffer;
	buffer << GAME_CMD_SWAP_INVENTORY << iSlot1 << iSlot2;
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(SwapInventory, 2)
{
	cmdSwapInventory(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  Sacrifice
  --------------------*/
UI_VOID_CMD(Sacrifice, 1)
{
	ushort unID(EntityRegistry.LookupID(_T("Player_") + vArgList[0]->Evaluate()));
	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("Invalid class name") << newl;
		return;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_SACRIFICE << unID;
	GameClient.SendGameData(buffer, true);
}

/*--------------------
  CancelSacrifice
  --------------------*/
UI_VOID_CMD(CancelSacrifice, 0)
{
	CBufferDynamic buffer;
	buffer << GAME_CMD_CANCEL_SACRIFICE;
	GameClient.SendGameData(buffer, true);
}

/*--------------------
  PurchasePersistant
  --------------------*/
CMD(PurchasePersistant)
{
	if (vArgList.empty())
		return false;

	int iVaultNum = AtoI(vArgList[0]);

	if (iVaultNum < 0 || iVaultNum >= MAX_PERSISTANT_ITEMS)
	{
		Console << _T("Invalid vault number") << newl;
		return false;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_PURCHASE_PERSISTANT << iVaultNum;
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(PurchasePersistant, 1)
{
	cmdPurchasePersistant(vArgList[0]->Evaluate());
}


/*--------------------
  PreviewUnit
  --------------------*/
CMD(PreviewUnit)
{
	if (vArgList.empty())
		return false;

	ushort unID(EntityRegistry.LookupID(_T("Player_") + vArgList[0]));
	if (unID == INVALID_ENT_TYPE)
	{
		Console << _T("Invalid class name") << newl;
		return false;
	}

	GameClient.SetPreviewUnit(unID);
	return true;
}


/*--------------------
  Purchase
  --------------------*/
CMD(Purchase)
{
	if (vArgList.empty())
		return false;

	ushort unID(EntityRegistry.LookupID(vArgList[0]));
	if (unID == 0)
	{
		Console << _T("Invalid item name") << newl;
		return false;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_PURCHASE << unID;
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(Purchase, 1)
{
	cmdPurchase(vArgList[0]->Evaluate());
}


/*--------------------
  Sell
  --------------------*/
CMD(Sell)
{
	if (vArgList.size() < 2)
		return false;

	int iSlot = AtoI(vArgList[0]);
	int iAmount = AtoI(vArgList[1]);

	if (iSlot < INVENTORY_START_BACKPACK || iSlot >= INVENTORY_END_BACKPACK)
		return false;

	CBufferDynamic buffer;
	buffer << GAME_CMD_SELL << iSlot << iAmount;
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(Sell, 2)
{
	cmdSell(vArgList[0]->Evaluate(), vArgList[1]->Evaluate());
}


/*--------------------
  Team
  --------------------*/
CMD(Team)
{
	if (vArgList.empty())
		return false;

	CBufferDynamic buffer;
	buffer << GAME_CMD_CHANGE_TEAM << AtoN(vArgList[0]);
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(Team, 1)
{
	cmdTeam(vArgList[0]->Evaluate());
}

/*--------------------
  JoinOpposingTeam
  --------------------*/
CMD(JoinOpposingTeam)
{
	CEntityClientInfo *pClient(GameClient.GetLocalClient());
	short nTeam(0);

	if (pClient != NULL)
		nTeam = (pClient->GetTeam() ^ 3);

	CBufferDynamic buffer;
	buffer << GAME_CMD_CHANGE_TEAM << nTeam;
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(JoinOpposingTeam, 0)
{
	cmdJoinOpposingTeam();
}

/*--------------------
  ToggleInfoScreen
  --------------------*/
UI_VOID_CMD(ToggleInfoScreen, 0)
{
	if (vArgList.empty())
		GameClient.ToggleInfoScreen();
	else
		GameClient.ToggleInfoScreen(vArgList[0]->Evaluate());
}


/*--------------------
  ToggleMenu
  --------------------*/
UI_VOID_CMD(ToggleMenu, 0)
{
	GameClient.ToggleMenu();
}

/*--------------------
  HideMenu
  --------------------*/
UI_VOID_CMD(HideMenu, 0)
{
	GameClient.HideMenu();
}

/*--------------------
  TogglePurchase
  --------------------*/
UI_VOID_CMD(TogglePurchase, 0)
{
	GameClient.TogglePurchase();
}

/*--------------------
  HidePurchase
  --------------------*/
UI_VOID_CMD(HidePurchase, 0)
{
	GameClient.HidePurchase();
}

/*--------------------
  ToggleLobby
  --------------------*/
UI_VOID_CMD(ToggleLobby, 0)
{
	GameClient.ToggleLobby();
}

/*--------------------
  HideLobby
  --------------------*/
UI_VOID_CMD(HideLobby, 0)
{
	GameClient.HideLobby();
}


/*--------------------
  StopBuilding
  --------------------*/
UI_CMD(StopBuilding, 0)
{
	return XtoA(GameClient.StopBuildingPlacement());
}


/*--------------------
  Cancel
  --------------------*/
UI_VOID_CMD(Cancel, 0)
{
	GameClient.Cancel();
}


/*--------------------
  SpendPoint
  --------------------*/
UI_VOID_CMD(SpendPoint, 1)
{
	tstring sName(vArgList[0]->Evaluate());
	int iStat(ATTRIBUTE_NULL);
	for (int i(1); i < NUM_PLAYER_ATTRIBUTES; ++i)
	{
		if (CompareNoCase(g_asPlayerAttibutes[i], sName) == 0)
		{
			iStat = i;
			break;
		}
	}
	if (iStat == ATTRIBUTE_NULL)
		iStat = AtoN(sName);
	if (iStat == ATTRIBUTE_NULL)
	{
		Console << _T("Invalid stat: ") << sName << newl;
		return;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_SPEND_POINT << iStat;
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  Toggle Repairable
  --------------------*/
UI_VOID_CMD(ToggleRepairable, 0)
{
	CBufferDynamic buffer;
	buffer << GAME_CMD_REPAIRABLE;
	GameClient.SendGameData(buffer, true);
}

/*--------------------
  Toggle Repairable
  --------------------*/
CMD(Repairable)
{
	CBufferDynamic buffer;
	buffer << GAME_CMD_REPAIRABLE;
	GameClient.SendGameData(buffer, true);
	return true;
}

/*--------------------
  SpendTeamPoint
  --------------------*/
CMD(SpendTeamPoint)
{
	if (vArgList.empty())
		return false;
	
	int iStat(ATTRIBUTE_NULL);
	for (int i(1); i < NUM_PLAYER_ATTRIBUTES; ++i)
	{
		if (CompareNoCase(g_asPlayerAttibutes[i], vArgList[0]) == 0)
		{
			iStat = i;
			break;
		}
	}
	if (iStat == ATTRIBUTE_NULL)
		iStat = AtoN(vArgList[0]);
	if (iStat == ATTRIBUTE_NULL)
	{
		Console << _T("Invalid stat: ") << vArgList[0] << newl;
		return false;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_SPEND_TEAM_POINT << iStat;
	GameClient.SendGameData(buffer, true);
	return true;
}


/*--------------------
  SpawnWorker
  --------------------*/
UI_VOID_CMD(SpawnWorker, 0)
{
	CBufferFixed<1> buffer;
	buffer << GAME_CMD_SPAWN_WORKER;
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  AllChat
  --------------------*/
CMD(AllChat)
{
	if (vArgList.empty())
		return false;

	const tstring &sText(ConcatinateArgs(vArgList));

	if (sText.empty())
		return false;

	// If they entered a command, pass it to IRC
	if (sText[0] == _T('/'))
	{
		Console.Execute(_T("IRCSendMessage ") + sText);
		return false;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_CHAT_ALL << sText << byte(0);
	GameClient.SendGameData(buffer, true);
	return true;
}


/*--------------------
  TeamChat
  --------------------*/
CMD(TeamChat)
{
	if (vArgList.empty())
		return false;

	const tstring &sText(ConcatinateArgs(vArgList));

	if (sText.empty())
		return false;

	// If they entered a command, pass it to IRC
	if (sText[0] == _T('/'))
	{
		Console.Execute(_T("IRCSendMessage ") + sText);
		return false;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_CHAT_TEAM << sText << byte(0);
	GameClient.SendGameData(buffer, true);
	return true;
}


/*--------------------
  SquadChat
  --------------------*/
CMD(SquadChat)
{
	if (vArgList.empty())
		return false;

	const tstring &sText(ConcatinateArgs(vArgList));

	if (sText.empty())
		return false;

	// If they entered a command, pass it to IRC
	if (sText[0] == _T('/'))
	{
		Console.Execute(_T("IRCSendMessage ") + sText);
		return false;
	}

	CBufferDynamic buffer;
	buffer << GAME_CMD_CHAT_SQUAD << sText << byte(0);
	GameClient.SendGameData(buffer, true);
	return true;
}


/*--------------------
  Chat
  --------------------*/
UI_VOID_CMD(AllChat, 1)
{
	cmdAllChat(vArgList[0]->Evaluate());
}


/*--------------------
  TeamChat
  --------------------*/
UI_VOID_CMD(TeamChat, 1)
{
	cmdTeamChat(vArgList[0]->Evaluate());
}


/*--------------------
  SquadChat
  --------------------*/
UI_VOID_CMD(SquadChat, 1)
{
	cmdSquadChat(vArgList[0]->Evaluate());
}


/*--------------------
  Spawn
  --------------------*/
CMD(Spawn)
{
	CBufferDynamic buffer;
	buffer << GAME_CMD_SPAWN;
	GameClient.SendGameData(buffer, true);
	
	return true;
}

UI_VOID_CMD(Spawn, 0)
{
	cmdSpawn();
}


/*--------------------
  Respawn
  --------------------*/
CMD(Respawn)
{
	CBufferDynamic buffer;
	buffer << GAME_CMD_SPAWN_SELECT << INVALID_INDEX;
	GameClient.SendGameData(buffer, true);
	
	return true;
}


/*--------------------
  eventStartEffect
  --------------------*/
EVENT_CMD(StartEffect)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.StartEffect(vArgList[0], vArgList.size() > 1 ? AtoI(vArgList[1]) : -1, iTimeNudge);
	return true;
}


/*--------------------
  precacheStartEffect
  --------------------*/
CMD_PRECACHE(StartEffect)
{
	if (vArgList.size() < 1)
		return false;

	g_ResourceManager.Register(vArgList[0], RES_EFFECT);
	return true;
}


/*--------------------
  eventStopEffect
  --------------------*/
EVENT_CMD(StopEffect)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.StopEffect(AtoI(vArgList[0]) + NUM_EFFECT_CHANNELS);
	return true;
}


/*--------------------
  eventPlaySound <filename> <falloff> <volume> <channel> <fadein ms> <fadeout start time ms> <fadeout ms> <frequency>
  --------------------*/
EVENT_CMD(PlaySound)
{
	if (vArgList.size() < 1)
		return false;

	if (vArgList.size() > 7 && (AtoF(vArgList[7]) * 1000 >= M_Randnum(0, 1000)))
		return true;


	GameClient.PlaySound(vArgList[0], vArgList.size() > 3 ? AtoI(vArgList[3]) : -1, vArgList.size() > 1 ? AtoF(vArgList[1]) : -1, vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0f, 0, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0, vArgList.size() > 6 ? AtoI(vArgList[6]) : 0);
	return true;
}


/*--------------------
  eventPlaySoundLooping

  PlaySoundLooping <filename> <falloff> <volume> <channel> <fadein ms> <fadeout ms> <override channel> <speed up time> <speed 1> <speed 2> <slow down time>
  --------------------*/
EVENT_CMD(PlaySoundLooping)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.PlaySound(vArgList[0], vArgList.size() > 3 ? AtoI(vArgList[3]) : -1, vArgList.size() > 1 ? AtoF(vArgList[1]) : -1, vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0f, SND_LOOP, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, 0, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0, vArgList.size() > 6 ? AtoB(vArgList[6]) : true, vArgList.size() > 7 ? AtoI(vArgList[7]) : 0, vArgList.size() > 8 ? AtoF(vArgList[8]) : 1.0f, vArgList.size() > 9 ? AtoF(vArgList[9]) : 1.0f, vArgList.size() > 10 ? AtoI(vArgList[10]) : 0);
	return true;
}


/*--------------------
  precachePlaySoundLooping
  --------------------*/
CMD_PRECACHE(PlaySoundLooping)
{
	if (vArgList.size() < 1)
		return false;

	ResHandle hSample(g_ResourceManager.LookUpPath(vArgList[0]));

	if (hSample == INVALID_RESOURCE)
		hSample = g_ResourceManager.Register(K2_NEW(global,   CSample)(vArgList[0], SND_LOOP), RES_SAMPLE);

	return true;
}


/*--------------------
  eventPlaySoundStationary
  --------------------*/
EVENT_CMD(PlaySoundStationary)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.PlaySoundStationary(vArgList[0], vArgList.size() > 3 ? AtoI(vArgList[3]) : -1, vArgList.size() > 1 ? AtoF(vArgList[1]) : -1, vArgList.size() > 2 ? AtoF(vArgList[2]) : 1.0f);
	return true;
}


/*--------------------
  precachePlaySoundStationary
  --------------------*/
CMD_PRECACHE(PlaySoundStationary)
{
	if (vArgList.size() < 1)
		return false;

	ResHandle hSample(g_ResourceManager.LookUpPath(vArgList[0]));

	if (hSample == INVALID_RESOURCE)
		hSample = g_ResourceManager.Register(K2_NEW(global,   CSample)(vArgList[0], 0), RES_SAMPLE);

	return true;
}


/*--------------------
  eventStopSound
  --------------------*/
EVENT_CMD(StopSound)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.StopSound(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  PrecacheSound
  --------------------*/
EVENT_CMD(PrecacheSound)
{
	if (!vArgList.empty())
	{
		Console << "Precaching " << vArgList[0] << newl;
		g_ResourceManager.Register(K2_NEW(global,   CSample)(vArgList[0], 0), RES_SAMPLE);
	}
	return true;
}


/*--------------------
  MinimapLeftClick
  --------------------*/
CMD(MinimapLeftClick)
{
	if (vArgList.size() < 2)
		return false;

	CVec3f v3NewPosition;
	v3NewPosition.x = AtoF(vArgList[0]) * GameClient.GetWorldWidth();
	v3NewPosition.y = AtoF(vArgList[1]) * GameClient.GetWorldHeight();
	v3NewPosition.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;

	GameClient.GetCurrentSnapshot()->SetCameraPosition(v3NewPosition);
	return true;
}


/*--------------------
  MinimapButton
  --------------------*/
CMD(MinimapButton)
{
	CBufferDynamic buffer;
	buffer << GAME_CMD_SPAWN_SELECT << (vArgList.size() > 0 ? AtoI(vArgList[0]) : INVALID_INDEX);
	GameClient.SendGameData(buffer, true);
	return true;
}


/*--------------------
  MinimapDraw
  --------------------*/
CMD(MinimapDraw)
{
	if (vArgList.size() < 2)
		return false;

	CBufferFixed<11> buffer;
	buffer << GAME_CMD_MINIMAP_DRAW << AtoF(vArgList[0]) << AtoF(vArgList[1]) << ((vArgList.size() > 2) ? byte(AtoI(vArgList[2])) : byte(-1));
	GameClient.SendGameData(buffer, false);

	return true;
}


/*--------------------
  MinimapPing
  --------------------*/
CMD(MinimapPing)
{
	if (vArgList.size() < 2)
		return false;

	if (Host.GetTime() - GameClient.GetLastPingTime() < MIN_PING_TIME)
		return false;

	CBufferFixed<11> buffer;
	buffer << GAME_CMD_MINIMAP_PING << AtoF(vArgList[0]) << AtoF(vArgList[1]) << ((vArgList.size() > 2) ? byte(AtoI(vArgList[2])) : byte(-1));
	GameClient.SendGameData(buffer, false);

	return true;
}


/*--------------------
  SelectBuilding
  --------------------*/
UI_VOID_CMD(SelectBuilding, 1)
{
	tstring sName(vArgList[0]->Evaluate());
	ushort unType(EntityRegistry.LookupID(_T("Building_") + sName));
	if (unType == INVALID_ENT_TYPE)
	{
		Console << _T("Unknown building: ") << sName << newl;
		return;
	}

	GameClient.StartBuildingPlacement(unType);
}


/*--------------------
  SelectItem
  --------------------*/
UI_VOID_CMD(SelectItem, 1)
{
	GameClient.SelectItem(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  SetBuildingUpkeepLevel
  --------------------*/
UI_VOID_CMD(SetBuildingUpkeepLevel, 2)
{
	GameClient.SetBuildingUpkeepLevel(AtoI(vArgList[0]->Evaluate()), AtoF(vArgList[1]->Evaluate()));
}


/*--------------------
  ToggleBuildingUpkeep
  --------------------*/
UI_VOID_CMD(ToggleBuildingUpkeep, 1)
{
	GameClient.ToggleBuildingUpkeep(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  PromotePlayer
  --------------------*/
UI_VOID_CMD(PromotePlayer, 1)
{
	GameClient.PromotePlayer(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  DemotePlayer
  --------------------*/
UI_VOID_CMD(DemotePlayer, 1)
{
	GameClient.DemotePlayer(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  Vote
  --------------------*/
UI_VOID_CMD(Vote, 1)
{
	GameClient.SetVote(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  RequestCommand
  --------------------*/
UI_VOID_CMD(RequestCommand, 0)
{
	CBufferFixed<1> buffer;
	buffer << GAME_CMD_REQUEST_COMMAND;
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  Resign
  --------------------*/
UI_VOID_CMD(Resign, 0)
{
	CBufferFixed<1> buffer;
	buffer << GAME_CMD_COMMANDER_RESIGN;
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  DeclineOfficer
  --------------------*/
UI_VOID_CMD(DeclineOfficer, 0)
{
	CBufferFixed<1> buffer;
	buffer << GAME_CMD_DECLINE_OFFICER;
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  JoinSquad
  --------------------*/
CMD(JoinSquad)
{
	if (vArgList.empty())
		return false;

	CBufferFixed<2> buffer;
	buffer << GAME_CMD_JOIN_SQUAD << byte(AtoI(vArgList[0]));
	GameClient.SendGameData(buffer, true);
	return true;
}

UI_VOID_CMD(JoinSquad, 1)
{
	cmdJoinSquad(vArgList[0]->Evaluate());
}


/*--------------------
  Contribute
  --------------------*/
UI_VOID_CMD(Contribute, 1)
{
	ushort unAmount(AtoI(vArgList[0]->Evaluate()));
	
	CBufferFixed<3> buffer;
	buffer << GAME_CMD_CONTRIBUTE << unAmount;
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  RayTrace
  --------------------*/
CMD(RayTrace)
{
	// Determine size of image to produce
	int iWidth(160);
	int iHeight(120);
	if (vArgList.size() >= 2)
	{
		iWidth = AtoI(vArgList[0]);
		iHeight = AtoI(vArgList[1]);
	}
	else if (!vArgList.empty())
	{
		iWidth = AtoI(vArgList[0]);
		iHeight = INT_ROUND(iWidth / Vid.GetAspect());
	}
	iWidth = CLAMP(iWidth, 10, Vid.GetScreenW());
	iHeight = CLAMP(iHeight, 10, Vid.GetScreenH());

	Console << _T("Tracing scene at a resolution of ") << iWidth << _T("x") << iHeight << newl;

	float fRatioX = Vid.GetScreenW() / static_cast<float>(iWidth);
	float fRatioY = Vid.GetScreenH() / static_cast<float>(iHeight);

	CBitmap bmp;
	bmp.Alloc(iWidth, iHeight, BITMAP_RGBA);

	uint uiMsec(K2System.Milliseconds());

	CVec4f	colors[] =
	{
		CVec4f(1.0f, 0.5f, 0.5f, 1.0f),
		CVec4f(0.0f, 1.0f, 0.5f, 1.0f),
		CVec4f(0.5f, 0.5f, 1.0f, 1.0f),
		CVec4f(0.5f, 1.0f, 1.0f, 1.0f),
		CVec4f(1.0f, 0.5f, 1.0f, 1.0f),
		CVec4f(1.0f, 1.0f, 0.5f, 1.0f)
	};

	CWorld *pWorld(GameClient.GetWorldPointer());
	CCamera *pCamera(GameClient.GetCamera());
	bool bTraceBox(vArgList.size() > 2);
	float fBoxSize(bTraceBox ? AtoF(vArgList[2]) : 0.0f);
	CBBoxf bbBounds(CVec3f(-fBoxSize, -fBoxSize, -fBoxSize), CVec3f(fBoxSize, fBoxSize, fBoxSize));
	int iIgnoreSurface(0);

	if (fBoxSize == 0.0f)
		iIgnoreSurface |= SURF_HULL | SURF_SHIELD;

	for (int y(0); y < iHeight; ++y)
	{
		for (int x(0); x < iWidth; ++x)
		{
			// Set up the trace
			CVec3f v3Dir(pCamera->ConstructRay(x * fRatioX, y * fRatioY));
			CVec3f v3End(M_PointOnLine(pCamera->GetOrigin(), v3Dir, 100000.0f));
			STraceInfo trace;

			// Perform the trace
			if (bTraceBox)
				pWorld->TraceBox(trace, pCamera->GetOrigin(), v3End, bbBounds, iIgnoreSurface);
			else
				pWorld->TraceLine(trace, pCamera->GetOrigin(), v3End, iIgnoreSurface);

			// No hit
			if (trace.fFraction >= 1.0f)
			{
				bmp.SetPixel4b(x, y, 255, 0, 0, 255);
				continue;
			}

			float fDot(DotProduct(-v3Dir, trace.plPlane.v3Normal));
			if (fDot < 0.0f)
			{
				bmp.SetPixel4b(x, y, 255, 0, 255, 255);
			}
			else if (trace.uiEntityIndex != INVALID_INDEX)
			{
				float fLight(CLAMP(fDot, 0.0f, 1.0f));
				CVec4f	v4Color(colors[trace.uiEntityIndex % 6]);
				bmp.SetPixel4b(x, y, byte(v4Color[R] * fLight * 255), byte(v4Color[G] * fLight * 255), byte(v4Color[B] * fLight * 255), 255);
			}
			else
			{
				byte yLight(static_cast<byte>(MAX(fDot, 0.0f) * 255));
				bmp.SetPixel4b(x, y, yLight, yLight, (trace.uiSurfFlags & SURF_TERRAIN) ? yLight : 0, 255);
			}
		}
	}

	Console << _T("Raytrace took ") << MsToSec(K2System.Milliseconds() - uiMsec) << _T(" seconds") << newl;
	bmp.WritePNG(_T("~/raytrace.png"));
	bmp.Free();
	return true;
}


/*--------------------
  Remote
  --------------------*/
CMD(Remote)
{
	if (vArgList.size() < 2)
		return false;

	CBufferDynamic buffer;
	buffer << GAME_CMD_REMOTE << vArgList[0] << byte(0) << ConcatinateArgs(vArgList.begin() + 1, vArgList.end()) << byte(0);
	GameClient.SendGameData(buffer, true);

	return true;
}


/*--------------------
  SubmitReplayComment
  --------------------*/
UI_VOID_CMD(SubmitReplayComment, 2)
{
	int iRating;
	tstring sComment;

	iRating = AtoI(vArgList[0]->Evaluate());

	sComment = vArgList[1]->Evaluate();

	CBufferDynamic buffer;
	buffer << GAME_CMD_SUBMIT_REPLAY_COMMENT << iRating << sComment << byte(0);
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  UpdateInterface
  --------------------*/
CMD(UpdateInterface)
{
	GameClient.ForceInterfaceRefresh();
	return true;
}


/*--------------------
  ShowStats
  --------------------*/
CMD(ShowStats)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.GetInterfaceManager()->ShowStats(AtoI(vArgList[0]));
	return true;
}

UI_VOID_CMD(ShowStats, 1)
{
	GameClient.GetInterfaceManager()->ShowStats(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  ShowStatsByTeam
  --------------------*/
CMD(ShowStatsByTeam)
{
	if (vArgList.size() < 2)
		return false;

	GameClient.GetInterfaceManager()->ShowStatsByTeam(AtoI(vArgList[0]), AtoI(vArgList[1]));
	return true;
}

UI_VOID_CMD(ShowStatsByTeam, 2)
{
	GameClient.GetInterfaceManager()->ShowStatsByTeam(AtoI(vArgList[0]->Evaluate()), AtoI(vArgList[1]->Evaluate()));
}


/*--------------------
  StartTalking
  --------------------*/
CMD(StartTalking)
{
	GameClient.StartVoiceRecording();
	return true;
}


/*--------------------
  StopTalking
  --------------------*/
CMD(StopTalking)
{
	GameClient.StopVoiceRecording();
	return true;
}


/*--------------------
  MuteVoice
  --------------------*/
UI_VOID_CMD(MuteVoice, 1)
{
	GameClient.SetVoiceMute(AtoI(vArgList[0]->Evaluate()), true);
}


/*--------------------
  UnmuteVoice
  --------------------*/
UI_VOID_CMD(UnmuteVoice, 1)
{
	GameClient.SetVoiceMute(AtoI(vArgList[0]->Evaluate()), false);
}


/*--------------------
  MuteVoiceByTeam
  --------------------*/
UI_VOID_CMD(MuteVoiceByTeam, 2)
{
	CEntityTeamInfo *pTeam = GameClient.GetTeam(AtoI(vArgList[0]->Evaluate()));

	if (pTeam == NULL)
		return;

	GameClient.SetVoiceMute(pTeam->GetClientIDFromTeamIndex(AtoI(vArgList[1]->Evaluate())), true);
}


/*--------------------
  UnmuteVoiceByTeam
  --------------------*/
UI_VOID_CMD(UnmuteVoiceByTeam, 2)
{
	CEntityTeamInfo *pTeam = GameClient.GetTeam(AtoI(vArgList[0]->Evaluate()));

	if (pTeam == NULL)
		return;

	GameClient.SetVoiceMute(pTeam->GetClientIDFromTeamIndex(AtoI(vArgList[1]->Evaluate())), false);
}


/*--------------------
  PetAttack
  --------------------*/
UI_VOID_CMD(PetAttack, 0)
{
	GameClient.PetCommand(PETCMD_ATTACK);
}


/*--------------------
  PetMove
  --------------------*/
UI_VOID_CMD(PetMove, 0)
{
	GameClient.PetCommand(PETCMD_MOVE);
}


/*--------------------
  PetStop
  --------------------*/
UI_VOID_CMD(PetStop, 0)
{
	GameClient.PetCommand(PETCMD_STOP);
}


/*--------------------
  PetFollow
  --------------------*/
UI_VOID_CMD(PetFollow, 0)
{
	GameClient.PetCommand(PETCMD_FOLLOW);
}


/*--------------------
  PetSpecialAbility
  --------------------*/
UI_VOID_CMD(PetSpecialAbility, 0)
{
	GameClient.PetCommand(PETCMD_SPECIALABILITY);
}


/*--------------------
  PetReturn
  --------------------*/
UI_VOID_CMD(PetReturn, 0)
{
	GameClient.PetCommand(PETCMD_RETURN);
}


/*--------------------
  PetToggleAggro
  --------------------*/
UI_VOID_CMD(PetToggleAggro, 0)
{
	GameClient.PetCommand(PETCMD_TOGGLEAGGRO);
}


/*--------------------
  PetBanish
  --------------------*/
UI_VOID_CMD(PetBanish, 0)
{
	GameClient.PetCommand(PETCMD_BANISH);
}


/*--------------------
  OfficerAttack
  --------------------*/
UI_VOID_CMD(OfficerAttack, 0)
{
	GameClient.OfficerCommand(OFFICERCMD_ATTACK);
}


/*--------------------
  OfficerMove
  --------------------*/
UI_VOID_CMD(OfficerMove, 0)
{
	GameClient.OfficerCommand(OFFICERCMD_MOVE);
}


/*--------------------
  OfficerFollow
  --------------------*/
UI_VOID_CMD(OfficerFollow, 0)
{
	GameClient.OfficerCommand(OFFICERCMD_FOLLOW);
}


/*--------------------
  OfficerDefend
  --------------------*/
UI_VOID_CMD(OfficerDefend, 0)
{
	GameClient.OfficerCommand(OFFICERCMD_DEFEND);
}


/*--------------------
  OfficerRally
  --------------------*/
UI_VOID_CMD(OfficerRally, 0)
{
	GameClient.OfficerCommand(OFFICERCMD_RALLY);
}


/*--------------------
  OfficerPing
  --------------------*/
UI_VOID_CMD(OfficerPing, 0)
{
	GameClient.OfficerCommand(OFFICERCMD_PING);
}


/*--------------------
  SetLoadoutUnit
  --------------------*/
UI_VOID_CMD(SetLoadoutUnit, 1)
{
	GameClient.SetLoadoutUnitMouseover(vArgList[0]->Evaluate());
}


/*--------------------
  OfficerClearOrders
  --------------------*/
UI_VOID_CMD(OfficerClearOrders, 0)
{
	GameClient.OfficerCommand(OFFICERCMD_INVALID);
}


/*--------------------
  ShowEndGameStats
  --------------------*/
UI_VOID_CMD(ShowEndGameStats, 3)
{
	GameClient.GetInterfaceManager()->ShowEndGameStats(AtoI(vArgList[0]->Evaluate()), AtoI(vArgList[1]->Evaluate()), AtoI(vArgList[2]->Evaluate()));
}


/*--------------------
  CommanderSelect
  --------------------*/
CMD(CommanderSelect)
{
	if (vArgList.empty())
		return false;

	if (!GameClient.IsCommander())
		return false;

	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return false;

	pCommander->SelectEntity(AtoI(vArgList[0]));
	return true;
}

UI_VOID_CMD(CommanderSelect, 1)
{
	cmdCommanderSelect(vArgList[0]->Evaluate());
}


/*--------------------
  CommanderSelectSquad
  --------------------*/
UI_VOID_CMD(CommanderSelectSquad, 1)
{
	if (!GameClient.IsCommander())
		return;

	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (!pCommander)
		return;

	pCommander->SelectSquad(AtoI(vArgList[0]->Evaluate()));
}


/*--------------------
  AddVCCategory
  --------------------*/
CMD(AddVCCategory)
{
	EButton button;

	if (vArgList.size() < 3)
		return false;

	button = Input.MakeEButton(vArgList[0]);

	if (button == BUTTON_INVALID || button == BUTTON_ESC)
	{
		Console.Warn << _T("Invalid key specified in voice chat category!") << newl;
		return false;
	}

	if (!GameClient.AddVCCategory(button, vArgList[1], vArgList[2]))
	{
		Console.Warn << _T("Failed to add voice chat category.") << newl;
		return false;
	}

	return true;
}

/*--------------------
  AddVCSubItem
  --------------------*/
CMD(AddVCSubItem)
{
	EButton buttonCat;
	EButton buttonSub;
	VCCategory *pVCC;
	VCSub structVC;

	if (vArgList.size() < 4)
		return false;

	buttonCat = Input.MakeEButton(vArgList[0]);

	if (buttonCat == BUTTON_INVALID || buttonCat == BUTTON_ESC)
	{
		Console.Warn << _T("Invalid key specified for voice chat sub item category!") << newl;
		return false;
	}

	pVCC = GameClient.GetVCCategory(buttonCat);

	if (pVCC == NULL)
	{
		Console.Warn << _T("Invalid key specified for voice chat sub item category!") << newl;
		return false;
	}

	buttonSub = Input.MakeEButton(vArgList[1]);

	if (buttonSub == BUTTON_INVALID || buttonSub == BUTTON_ESC)
	{
		Console.Warn << _T("Invalid key specified for voice chat sub item!") << newl;
		return false;
	}

	if (!GameClient.AddVCSubItem(pVCC, buttonSub, vArgList[2], vArgList[3], vArgList[4]))
	{
		Console.Warn << _T("Failed to add voice chat sub item.") << newl;
		return false;
	}

	return true;
}


/*--------------------
  MouseHidden
  --------------------*/
CMD(MouseHidden)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.ForceMouseHidden(AtoB(vArgList[0]));

	if (vArgList.size() > 1)
		GameClient.IsMouseHidden(AtoB(vArgList[1]));

	return true;
}


/*--------------------
  MouseCentered
  --------------------*/
CMD(MouseCentered)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.ForceMouseCentered(AtoB(vArgList[0]));

	if (vArgList.size() > 1)
		GameClient.IsMouseCentered(AtoB(vArgList[1]));

	return true;
}


/*--------------------
  AllowMouseAim
  --------------------*/
CMD(AllowMouseAim)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.AllowMouseAim(AtoB(vArgList[0]));

	return true;
}


/*--------------------
  AllowMovement
  --------------------*/
CMD(AllowMovement)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.AllowMovement(AtoB(vArgList[0]));

	return true;
}


/*--------------------
  AllowAttacks
  --------------------*/
CMD(AllowAttacks)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.AllowAttacks(AtoB(vArgList[0]));

	return true;
}


/*--------------------
  SendScriptInput
  --------------------*/
CMD(SendScriptInput)
{
	CBufferDynamic buffer;

	// Add command and # of arguments to buffer
	buffer << GAME_CMD_SCRIPT_INPUT << short(INT_FLOOR(vArgList.size() / 2));

	uint uiParam(0);
	while (uiParam + 1 < INT_SIZE(vArgList.size()))
	{
		buffer << vArgList[uiParam] << byte(0) << vArgList[uiParam + 1] << byte(0);
		uiParam += 2;
	}

	GameClient.SendGameData(buffer, true);

	return true;
}

/*--------------------
  SendScriptInput
  --------------------*/
UI_VOID_CMD(SendScriptInput, 0)
{
	CBufferDynamic buffer;

	// Add command and # of arguments to buffer
	buffer << GAME_CMD_SCRIPT_INPUT << short(INT_FLOOR(vArgList.size() / 2));

	uint uiParam(0);
	while (uiParam + 1 < INT_SIZE(vArgList.size()))
	{
		buffer << vArgList[uiParam]->Evaluate() << byte(0) << vArgList[uiParam + 1]->Evaluate() << byte(0);
		uiParam += 2;
	}

	GameClient.SendGameData(buffer, true);
}

/*--------------------
  ResetClientPitch
  --------------------*/
CMD(ResetClientPitch)
{
	CVec3f v3Angles(GameClient.GetCurrentSnapshot()->GetCameraAngles());
	GameClient.GetCurrentSnapshot()->AdjustCameraPitch(v3Angles[PITCH]);
	return true;
}


/*--------------------
  SetSelectedItem
  --------------------*/
CMD(SetSelectedItem)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.SelectItem(AtoI(vArgList[0]) - 1);

	return true;
}


/*--------------------
  SetReplayClient
  --------------------*/
CMD(SetReplayClient)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.SetReplayClient(AtoI(vArgList[0]));

	return true;
}


/*--------------------
  SetReplayClient
  --------------------*/
UI_VOID_CMD(SetReplayClient, 1)
{
	cmdSetReplayClient(vArgList[0]->Evaluate());
}


/*--------------------
  NextReplayClient
  --------------------*/
CMD(NextReplayClient)
{
	GameClient.NextReplayClient();
	return true;
}


/*--------------------
  NextReplayClient
  --------------------*/
UI_VOID_CMD(NextReplayClient, 0)
{
	GameClient.NextReplayClient();
}


/*--------------------
  PrevReplayClient
  --------------------*/
CMD(PrevReplayClient)
{
	GameClient.PrevReplayClient();
	return true;
}


/*--------------------
  PrevReplayClient
  --------------------*/
UI_VOID_CMD(PrevReplayClient, 0)
{
	GameClient.PrevReplayClient();
}


/*--------------------
  RequestServerStatus
  --------------------*/
CMD(RequestServerStatus)
{
	CBufferFixed<1> buffer;
	buffer << GAME_CMD_SERVER_STATUS;
	GameClient.SendGameData(buffer, true);
	return true;
}


/*--------------------
  StartClientGameEffect
  --------------------*/
CMD(StartClientGameEffect)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.StartClientGameEffect(vArgList[0], vArgList.size() > 1 ? AtoI(vArgList[1]) + NUM_CLIENT_GAME_EFFECT_CHANNELS : -1, 0, V3_ZERO);
	return true;
}


/*--------------------
  StopClientGameEffect
  --------------------*/
CMD(StopClientGameEffect)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.StopClientGameEffect(AtoI(vArgList[0]) + NUM_CLIENT_GAME_EFFECT_CHANNELS);
	return true;
}


/*--------------------
  PlayClientGameSound

  PlayClientGameSound <filename> <volume> <channel> <fadein ms> <fadeout start time ms> <fadeout ms> <frequency>
  --------------------*/
CMD(PlayClientGameSound)
{
	if (vArgList.size() < 1)
		return false;

	if (vArgList.size() > 6 && (AtoF(vArgList[6]) * 1000 >= M_Randnum(0, 1000)))
		return true;


	GameClient.PlayClientGameSound(vArgList[0], vArgList.size() > 2 ? AtoI(vArgList[2]) : -1, vArgList.size() > 1 ? AtoF(vArgList[1]) : 1.0f, 0, vArgList.size() > 3 ? AtoI(vArgList[3]) : 0, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, vArgList.size() > 5 ? AtoI(vArgList[5]) : 0);
	return true;
}


/*--------------------
  PlayClientGameSoundLooping

  PlayClientGameSoundLooping <filename> <volume> <channel> <fadein ms> <fadeout ms> <override channel> <speed up time> <speed 1> <speed 2> <slow down time>
  --------------------*/
CMD(PlayClientGameSoundLooping)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.PlayClientGameSound(vArgList[0], vArgList.size() > 2 ? AtoI(vArgList[2]) : -1, vArgList.size() > 1 ? AtoF(vArgList[1]) : 1.0f, SND_LOOP, vArgList.size() > 3 ? AtoI(vArgList[3]) : 0, 0, vArgList.size() > 4 ? AtoI(vArgList[4]) : 0, vArgList.size() > 5 ? AtoB(vArgList[5]) : true, vArgList.size() > 6 ? AtoI(vArgList[6]) : 0, vArgList.size() > 7 ? AtoF(vArgList[7]) : 1.0f, vArgList.size() > 8 ? AtoF(vArgList[8]) : 1.0f, vArgList.size() > 9 ? AtoI(vArgList[9]) : 0);
	return true;
}


/*--------------------
  StopClientGameSound
  --------------------*/
CMD(StopClientGameSound)
{
	if (vArgList.size() < 1)
		return false;

	GameClient.StopClientGameSound(AtoI(vArgList[0]));
	return true;
}


/*--------------------
  SetClientAngles
  --------------------*/
CMD(SetClientAngles)
{
	if (vArgList.size() < 3)
		return false;

	GameClient.GetCurrentSnapshot()->AdjustCameraAngles(CVec3f(AtoF(vArgList[0]),AtoF(vArgList[1]),AtoF(vArgList[2])));

	return true;
}


/*--------------------
  GivePositionOrder
  --------------------*/
UI_VOID_CMD(GivePositionOrder, 2)
{
	CClientCommander *pCommander(GameClient.GetClientCommander());
	if (pCommander == NULL)
		return;

	float fX(CLAMP(AtoF(vArgList[0]->Evaluate()), 0.0f, 1.0f));
	float fY(CLAMP(AtoF(vArgList[1]->Evaluate()), 0.0f, 1.0f));

	fX *= GameClient.GetWorldWidth();
	fY *= GameClient.GetWorldHeight();
	pCommander->GiveOrder(CMDR_ORDER_AUTO, CVec3f(fX, fY, GameClient.GetTerrainHeight(fX, fY)));
}


/*--------------------
  IsDemoAccount
  --------------------*/
UI_CMD(IsDemoAccount, 0)
{
	CEntityClientInfo *pClient(GameClient.GetLocalClient());

	if (pClient == NULL)
		return false;

	return XtoA(pClient->IsDemoAccount(), true);
}


/*--------------------
  DemoTimeRemaining
  --------------------*/
UI_CMD(DemoTimeRemaining, 0)
{
	CEntityClientInfo *pClient(GameClient.GetLocalClient());

	if (pClient->GetDemoTimeRemaining() < GameClient.GetCurrentGameLength())
		return _T("0");

	return XtoA(pClient->GetDemoTimeRemaining() - GameClient.GetCurrentGameLength());
}


/*--------------------
  GetTerrainType
  --------------------*/
FUNCTION(GetTerrainType)
{
	return GameClient.GetTerrainType();
}


/*--------------------
  GetLocalClientNum
  --------------------*/
UI_CMD(GetLocalClientNum, 0)
{
	return XtoA(GameClient.GetLocalClientNum());
}


/*--------------------
  GetLocalClientNum
  --------------------*/
FUNCTION(GetLocalClientNum)
{
	return XtoA(GameClient.GetLocalClientNum());
}


/*--------------------
  GetLocalTeam
  --------------------*/
UI_CMD(GetLocalTeam, 0)
{
	if (GameClient.GetLocalClient() == NULL)
		return _T("0");

	return XtoA(GameClient.GetLocalClient()->GetTeam());
}

/*--------------------
  GetCommanderClientNum
  --------------------*/
UI_CMD(GetCommanderClientNum, 1)
{
	CEntityTeamInfo *pTeam(GameClient.GetTeam(AtoI(vArgList[0]->Evaluate())));

	if (pTeam == NULL)
		return _T("-1");

	return XtoA(pTeam->GetCommanderClientID());
}


/*--------------------
  GetCommanderName
  --------------------*/
UI_CMD(GetCommanderName, 1)
{
	CEntityTeamInfo *pTeam(GameClient.GetTeam(AtoI(vArgList[0]->Evaluate())));

	if (pTeam == NULL)
		return _T("");

	if (GameClient.GetClientInfo(pTeam->GetCommanderClientID()) == NULL)
		return _T("");

	return GameClient.GetClientInfo(pTeam->GetCommanderClientID())->GetName();
}

/*--------------------
  GetLastCommanderClientNum
  --------------------*/
UI_CMD(GetLastCommanderClientNum, 1)
{
	CEntityTeamInfo *pTeam(GameClient.GetTeam(AtoI(vArgList[0]->Evaluate())));

	if (pTeam == NULL)
		return _T("-1");

	return XtoA(pTeam->GetLastCommanderClientID());
}


/*--------------------
  GetLastCommanderName
  --------------------*/
UI_CMD(GetLastCommanderName, 1)
{
	CEntityTeamInfo *pTeam(GameClient.GetTeam(AtoI(vArgList[0]->Evaluate())));

	if (pTeam == NULL)
		return _T("");

	if (GameClient.GetClientInfo(pTeam->GetLastCommanderClientID()) == NULL)
		return _T("");

	return GameClient.GetClientInfo(pTeam->GetLastCommanderClientID())->GetName();
}

/*--------------------
  GetOfficialCommanderName
  --------------------*/
UI_CMD(GetOfficialCommanderName, 1)
{
	CEntityTeamInfo *pTeam(GameClient.GetTeam(AtoI(vArgList[0]->Evaluate())));

	if (pTeam == NULL)
		return _T("");

	if (GameClient.GetClientInfo(pTeam->GetOfficialCommanderClientID()) == NULL)
		return _T("");

	return GameClient.GetClientInfo(pTeam->GetOfficialCommanderClientID())->GetName();
}

/*--------------------
  GetOfficialCommanderClientNum
  --------------------*/
UI_CMD(GetOfficialCommanderClientNum, 1)
{
	CEntityTeamInfo *pTeam(GameClient.GetTeam(AtoI(vArgList[0]->Evaluate())));

	if (pTeam == NULL)
		return _T("-1");

	return XtoA(pTeam->GetOfficialCommanderClientID());
}

/*--------------------
  SubmitCommanderRating
  --------------------*/
UI_VOID_CMD(SubmitCommanderRating, 2)
{
	int iRating(AtoI(vArgList[0]->Evaluate()));
	tstring sComment(vArgList[1]->Evaluate());

	if (iRating < 1 || iRating > 5)
		return;

	CBufferDynamic buffer;

	buffer << GAME_CMD_SUBMIT_COMMANDER_RATING << iRating << sComment << byte(0);
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  SubmitKarmaRating
  --------------------*/
UI_VOID_CMD(SubmitKarmaRating, 3)
{
	tstring sAction(vArgList[0]->Evaluate());
	tstring sPlayer(vArgList[1]->Evaluate());
	tstring sComment(vArgList[2]->Evaluate());

	if (sAction != _T("add") && sAction != _T("remove"))
		return;

	IPlayerEntity *pPlayer(GameClient.GetPlayerByName(sPlayer));

	if (pPlayer == NULL)
		return;
	
	CBufferDynamic buffer;

	buffer << GAME_CMD_SUBMIT_KARMA_RATING << byte(sAction == _T("add") ? 1 : 0) << sComment << byte(0) << pPlayer->GetClientID();
	GameClient.SendGameData(buffer, true);
}


/*--------------------
  SubmitCommanderRating
  --------------------*/
CMD(SubmitCommanderRating)
{
	if (vArgList.size() < 2)
		return false;

	int iRating(AtoI(vArgList[0]));
	tstring sComment(vArgList[1]);

	if (iRating < 1 || iRating > 5)
		return true;

	CBufferDynamic buffer;

	buffer << GAME_CMD_SUBMIT_COMMANDER_RATING << iRating << sComment << byte(0);
	GameClient.SendGameData(buffer, true);

	return true;
}


/*--------------------
  SubmitKarmaRating
  --------------------*/
CMD(SubmitKarmaRating)
{
	if (vArgList.size() < 3)
		return false;

	tstring sAction(vArgList[0]);
	tstring sPlayer(vArgList[1]);
	tstring sComment(vArgList[2]);

	if (sAction != _T("add") && sAction != _T("remove"))
		return true;

	IPlayerEntity *pPlayer(GameClient.GetPlayerByName(sPlayer));

	if (pPlayer == NULL)
		return true;
	
	CBufferDynamic buffer;

	buffer << GAME_CMD_SUBMIT_KARMA_RATING << byte(sAction == _T("add") ? 1 : 0) << sComment << byte(0) << pPlayer->GetClientID();
	GameClient.SendGameData(buffer, true);

	return true;
}


/*--------------------
  UpdateKarmaList
  --------------------*/
UI_CMD(UpdateKarmaList, 0)
{
	// Client related stats
	CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
	if (pLocalClient == NULL || pLocalClient->IsDemoAccount())
		return _T("0");

	// Karma ranking player list
	int iNumAdded(0);
	ClientInfoMap mapClients(GameClient.GetClientMap());

	// Clear the list by sending a blank name
	KarmaList.Trigger(_T(""));

	if (pLocalClient->GetTeam() > 0)
	{
		for (ClientInfoMap_it it(mapClients.begin()); it != mapClients.end(); it++)
		{
			if (it->second->GetTeam() > 0 && it->second->GetClientNumber() != pLocalClient->GetClientNumber() && (it->second->GetClanName() != pLocalClient->GetClanName() || pLocalClient->GetClanName().empty()))
			{
				KarmaList.Trigger(it->second->GetName());
				iNumAdded++;
			}
		}
	}

	return XtoA(iNumAdded);
}

/*--------------------
  GetCommanderRatingTarget
  --------------------*/
UI_CMD(GetCommanderRatingTarget, 0)
{
	// Client related stats
	CEntityClientInfo *pLocalClient(GameClient.GetLocalClient());
	if (pLocalClient == NULL || pLocalClient->IsDemoAccount())
		return _T("0");

	// Karma ranking player list
	int iNumAdded(0);
	ClientInfoMap mapClients(GameClient.GetClientMap());

	// Clear the list by sending a blank name
	KarmaList.Trigger(_T(""));

	if (pLocalClient->GetTeam() > 0)
	{
		for (ClientInfoMap_it it(mapClients.begin()); it != mapClients.end(); it++)
		{
			if (it->second->GetClientNumber() != pLocalClient->GetClientNumber() && (it->second->GetClanName() != pLocalClient->GetClanName() || pLocalClient->GetClanName().empty()))
			{
				KarmaList.Trigger(it->second->GetName());
				iNumAdded++;
			}
		}
	}

	return XtoA(iNumAdded);
}


/*--------------------
  ShowBuildMenu
  --------------------*/
UI_VOID_CMD(ShowBuildMenu, 0)
{
	GameClient.ShowBuildMenu();
}


/*--------------------
  HideBuildMenu
  --------------------*/
UI_VOID_CMD(HideBuildMenu, 0)
{
	GameClient.HideBuildMenu();
}

/*--------------------
  GetNumClients
  --------------------*/
UI_CMD(GetNumClients, 0)
{
	if (vArgList.size() < 1)
		return XtoA(GameClient.GetConnectedClientCount());

	return XtoA(GameClient.GetConnectedClientCount(AtoI(vArgList[0]->Evaluate())));
}
