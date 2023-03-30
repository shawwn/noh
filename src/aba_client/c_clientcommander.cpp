// (C)2006 S2 Games
// c_clientcommander.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_client_common.h"

#include "c_gameclient.h"
#include "c_clientcommander.h"
#include "c_gameinterfacemanager.h"

#include "../aba_shared/c_teaminfo.h"
#include "../aba_shared/i_buildingentity.h"
#include "../aba_shared/i_propentity.h"
#include "../aba_shared/i_entityitem.h"
#include "../aba_shared/i_entityability.h"
#include "../aba_shared/i_heroentity.h"
#include "../aba_shared/i_shopentity.h"
#include "../aba_shared/c_shopdefinition.h"

#include "../k2/c_camera.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_uitrigger.h"
#include "../k2/c_draw2d.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_input.h"
#include "../k2/c_vid.h"
#include "../k2/c_soundmanager.h"
#include "../k2/c_sample.h"
#include "../k2/c_stringtable.h"
#include "../k2/c_inputstate.h"
#include "../k2/c_uimanager.h"
#include "../k2/c_interface.h"
#include "../k2/c_resourcemanager.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
EXTERN_CVAR_BOOL(cg_unitVoiceResponses);
EXTERN_CVAR_UINT(cg_unitVoiceResponsesDelay);
EXTERN_CVAR_BOOL(cg_cameraCenterOnRespawn);
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_UINTF(		cg_cmdrDoubleSelectTime,		500,		CVAR_SAVECONFIG);
CVAR_FLOATF(	cg_cmdrDoubleSelectRadius,		800.0f,		CVAR_SAVECONFIG);
CVAR_FLOATF(	cg_cmdrVectorTargetMinDistance,	8.0f,		CVAR_SAVECONFIG);
CVAR_BOOLF(		cg_enableMinimapRightclick,		true,		CVAR_SAVECONFIG);
CVAR_BOOLF(		cg_moveHeroToSpawnOnDisconnect,	true,		CVAR_SAVECONFIG);
CVAR_BOOLF(		cg_cmdrDoubleActivate,			true,		CVAR_SAVECONFIG);
CVAR_UINTF(		cg_cmdrDoubleActivateTime,		500,		CVAR_SAVECONFIG);
CVAR_UINTF(		cg_sprintToggle,				false,		CVAR_SAVECONFIG);

UI_TRIGGER(ErrorMessage);

INPUT_STATE_BOOL(TurboPing);
//=============================================================================

/*====================
  CClientCommander::CClientCommander
  ====================*/
CClientCommander::CClientCommander() :
m_pCamera(NULL),
m_bModifier1(false),
m_bModifier2(false),
m_bModifier3(false),
m_bFrontQueueModifier(false),
m_bAlternateModifier(false),
m_eState(COMSTATE_HOVER),
m_v3TraceEndPos(V3_ZERO),
m_uiHoverEnt(INVALID_INDEX),
m_uiFirstEntityIndex(INVALID_INDEX),

m_uiCameraWatchIndex(INVALID_INDEX),

m_iActiveSlot(-1),
m_uiLastPrimaryTime(0),
m_uiLastSelectTime(0),
m_uiSelectTime(0),

m_uiLastSelectionSetRecallIndex(-1),
m_uiLastSelectionSetRecallTime(INVALID_TIME),

m_hHoverSound(INVALID_RESOURCE),
m_bControlSelection(true),

m_uiActiveControlEntity(INVALID_INDEX),

m_uiLastSetControlTime(0),
m_uiLastSetControlEntity(INVALID_INDEX),

m_uiLastVoiceEntity(INVALID_INDEX),
m_uiVoiceSequence(0),

m_hArrow(INVALID_RESOURCE),
m_hArrowAlly(INVALID_RESOURCE),
m_hArrowEnemy(INVALID_RESOURCE),

m_hActionValid(INVALID_RESOURCE),
m_hActionInvalid(INVALID_RESOURCE),
m_hActionAllyValid(INVALID_RESOURCE),
m_hActionAllyInvalid(INVALID_RESOURCE),
m_hActionEnemyValid(INVALID_RESOURCE),
m_hActionEnemyInvalid(INVALID_RESOURCE),

m_hScrollDown(INVALID_RESOURCE),
m_hScrollLeft(INVALID_RESOURCE),
m_hScrollLeftDown(INVALID_RESOURCE),
m_hScrollLeftUp(INVALID_RESOURCE),
m_hScrollRight(INVALID_RESOURCE),
m_hScrollRightDown(INVALID_RESOURCE),
m_hScrollRightUp(INVALID_RESOURCE),
m_hScrollUp(INVALID_RESOURCE),

m_hShop(INVALID_RESOURCE),

m_bVectorTargeting(false),
m_uiLastDoubleActivateTime(INVALID_TIME),
m_pLastDoubleActivateTool(NULL),

m_bSprinting(false)
{
}


/*====================
  CClientCommander::LoadResources
  ====================*/
void	CClientCommander::LoadResources(ResHandle hClientSoundsStringTable)
{
	m_hArrow = g_ResourceManager.LookUpName(_T("arrow"), RES_K2CURSOR);
	m_hArrowAlly = g_ResourceManager.LookUpName(_T("arrow_ally"), RES_K2CURSOR);
	m_hArrowEnemy = g_ResourceManager.LookUpName(_T("arrow_enemy"), RES_K2CURSOR);

	m_hActionValid = g_ResourceManager.LookUpName(_T("action_valid"), RES_K2CURSOR);
	m_hActionInvalid = g_ResourceManager.LookUpName(_T("action_invalid"), RES_K2CURSOR);
	m_hActionAllyValid = g_ResourceManager.LookUpName(_T("action_ally_valid"), RES_K2CURSOR);
	m_hActionAllyInvalid = g_ResourceManager.LookUpName(_T("action_ally_invalid"), RES_K2CURSOR);
	m_hActionEnemyValid = g_ResourceManager.LookUpName(_T("action_enemy_valid"), RES_K2CURSOR);
	m_hActionEnemyInvalid = g_ResourceManager.LookUpName(_T("action_enemy_invalid"), RES_K2CURSOR);

	m_hScrollDown = g_ResourceManager.LookUpName(_T("scroll_down"), RES_K2CURSOR);
	m_hScrollLeft = g_ResourceManager.LookUpName(_T("scroll_left"), RES_K2CURSOR);
	m_hScrollLeftDown = g_ResourceManager.LookUpName(_T("scroll_left_down"), RES_K2CURSOR);
	m_hScrollLeftUp = g_ResourceManager.LookUpName(_T("scroll_left_up"), RES_K2CURSOR);
	m_hScrollRight = g_ResourceManager.LookUpName(_T("scroll_right"), RES_K2CURSOR);
	m_hScrollRightDown = g_ResourceManager.LookUpName(_T("scroll_right_down"), RES_K2CURSOR);
	m_hScrollRightUp = g_ResourceManager.LookUpName(_T("scroll_right_up"), RES_K2CURSOR);
	m_hScrollUp = g_ResourceManager.LookUpName(_T("scroll_up"), RES_K2CURSOR);

	m_hShop = g_ResourceManager.LookUpName(_T("shop"), RES_K2CURSOR);

	CStringTable *pClientSounds(g_ResourceManager.GetStringTable(hClientSoundsStringTable));
	if (pClientSounds == NULL)
		return;

	GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_SELECTED_SAMPLE, pClientSounds->Get(_T("select_unit")), RES_SAMPLE);
	GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_ABILITY_ACTIVATE_ERROR_SAMPLE, pClientSounds->Get(_T("ability_activate_error")), RES_SAMPLE);
	GameClient.AddResourceToLoadingQueue(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE, pClientSounds->Get(_T("ability_target_error")), RES_SAMPLE);
}


/*====================
  CClientCommander::Reinitialize
  ====================*/
void	CClientCommander::Reinitialize()
{
	m_bModifier1 = false;
	m_bModifier2 = false;
	m_bModifier3 = false;
	m_bFrontQueueModifier = false;
	m_bAlternateModifier = false;
	m_eState = COMSTATE_HOVER;
	m_v3TraceEndPos = V3_ZERO;
	m_uiHoverEnt = INVALID_INDEX;
	m_uiFirstEntityIndex = INVALID_INDEX;

	m_uiCameraWatchIndex = INVALID_INDEX;

	m_iActiveSlot = -1;
	m_uiLastSelectTime = 0;
	m_uiLastPrimaryTime = 0;
	m_uiSelectTime = 0;

	m_uiLastSelectionSetRecallIndex = -1;
	m_uiLastSelectionSetRecallTime = INVALID_TIME;

	m_hHoverSound = INVALID_RESOURCE;
	m_bControlSelection = true;

	m_uiActiveControlEntity = INVALID_INDEX;

	m_uiLastSetControlTime = 0;
	m_uiLastSetControlEntity = INVALID_INDEX;

	m_uiLastVoiceEntity = INVALID_INDEX;
	m_uiVoiceSequence = 0;

	m_setInfoSelection.clear();
	m_setControlSelection.clear();
	m_setHoverSelection.clear();
	m_setTransmittedControlSelection.clear();

	for (uint ui(0); ui < MAX_SAVED_SELECTION_SETS; ++ui)
	{
		m_aSavedSelection[ui].setEntries.clear();
		m_aSavedSelection[ui].uiFocus = INVALID_INDEX;
	}

	m_bVectorTargeting = false;
	m_bSprinting = false;
}


/*====================
  CClientCommander::UpdateSelection
  ====================*/
void	CClientCommander::UpdateSelection()
{
	if (m_setControlSelection == m_setTransmittedControlSelection)
		return;

	CBufferDynamic buffer;
	buffer << GAME_CMD_SELECTION;

	for (uiset::iterator it(m_setControlSelection.begin()); it != m_setControlSelection.end(); ++it)
		buffer << ushort(*it);

	buffer << ushort(-1);

	GameClient.SendGameData(buffer, true);

	m_setTransmittedControlSelection = m_setControlSelection;
}


/*====================
  CClientCommander::CanDoubleActivate
  ====================*/
bool	CClientCommander::CanDoubleActivate()
{
	if (cg_cmdrDoubleActivate)
	{
		if (m_uiLastDoubleActivateTime != INVALID_TIME)
		{
			if ((Host.GetTime() - m_uiLastDoubleActivateTime) <= cg_cmdrDoubleActivateTime)
				return true;
		}
	}
	return false;
}


/*====================
  CClientCommander::CancelDoubleActivate
  ====================*/
void	CClientCommander::CancelDoubleActivate()
{
	m_uiLastDoubleActivateTime = INVALID_TIME;
	m_pLastDoubleActivateTool = NULL;
}


/*====================
  CClientCommander::UpdateDoubleActivate
  ====================*/
void	CClientCommander::UpdateDoubleActivate()
{
	m_uiLastDoubleActivateTime = Host.GetTime();
}


/*====================
  CClientCommander::ValidateSelections
  ====================*/
void	CClientCommander::ValidateSelections()
{
	if (m_pPlayer == NULL)
		return;

	// Remove invalid selections
	uiset_it itInfo(m_setInfoSelection.begin());
	while (itInfo != m_setInfoSelection.end())
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(*itInfo));
		if (pUnit == NULL ||
			(pUnit->GetStatus() != ENTITY_STATUS_ACTIVE && !(pUnit->IsHero() && m_pPlayer->GetTeam() == TEAM_SPECTATOR)) ||
			!m_pPlayer->CanSee(pUnit))
			STL_ERASE(m_setInfoSelection, itInfo);
		else
			++itInfo;
	}

	bool bChanged(false);
	uiset_it itControl(m_setControlSelection.begin());
	while (itControl != m_setControlSelection.end())
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(*itControl));
		if (pUnit == NULL ||
			((!m_pPlayer->CanSee(pUnit) ||
			!pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum())) &&
			pUnit->GetIndex() != m_pPlayer->GetHeroIndex()))
		{
			STL_ERASE(m_setControlSelection, itControl);
			bChanged = true;
		}
		else
			++itControl;
	}

	// If for some reason no control entity is selected, try to select the hero
	if (m_setControlSelection.empty() && m_pPlayer->GetHeroIndex() != INVALID_INDEX)
	{
		m_setControlSelection.insert(m_pPlayer->GetHeroIndex());
		bChanged = true;
	}

	itControl = m_setControlSelection.begin();
	for (; itControl != m_setControlSelection.end(); ++itControl)
	{
		if (*itControl == m_uiActiveControlEntity)
			break;
	}
	if (itControl == m_setControlSelection.end())
		m_uiActiveControlEntity = INVALID_INDEX;

	if (m_uiActiveControlEntity == INVALID_INDEX)
	{
		if (m_setControlSelection.size() > 0)
			m_uiActiveControlEntity = *m_setControlSelection.begin();
	}

	if (bChanged)
	{
		UpdateSelection();
		UIManager.RefreshCursor();
	}
}


/*====================
  CClientCommander::Frame
  ====================*/
void	CClientCommander::Frame()
{
	TransmitGameplayOptions();

	if (m_iActiveSlot != -1 &&
		(GetCommanderState() == COMSTATE_TARGET_ENTITY ||
		GetCommanderState() == COMSTATE_TARGET_AREA ||
		GetCommanderState() == COMSTATE_TARGET_DUAL || 
		GetCommanderState() == COMSTATE_TARGET_DUAL_POSITION || 
		GetCommanderState() == COMSTATE_TARGET_VECTOR))
	{
		// Get current selected unit
		IUnitEntity *pUnit(GetSelectedControlEntity());
		if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()))
		{
			SetCommanderState(COMSTATE_HOVER);
			m_iActiveSlot = -1;
		}
		else
		{
			// Get the units item
			IEntityTool *pItem(pUnit->GetTool(m_iActiveSlot));
			if (pItem == NULL || !pItem->CanOrder())
			{
				SetCommanderState(COMSTATE_HOVER);
				m_iActiveSlot = -1;
			}
		}
	}

	uint uiIgnoreSurfaces(HOVER_TRACE);
	if (GetCommanderState() == COMSTATE_TARGET_ENTITY || GetCommanderState() == COMSTATE_TARGET_DUAL || GetCommanderState() == COMSTATE_TARGET_DUAL_POSITION)
	{
		uiIgnoreSurfaces = ACTIVATE_TRACE;

		if (m_iActiveSlot != -1)
		{
			// Get current selected unit
			IUnitEntity *pUnit(GetSelectedControlEntity());
			if (pUnit != NULL)
			{
				// Get the units item
				IEntityTool *pItem(pUnit->GetTool(m_iActiveSlot));
				if (pItem != NULL)
				{
					if (!Game.CanTargetTrait(pItem->GetTargetScheme(), TARGET_TRAIT_TREE))
						uiIgnoreSurfaces |= SURF_TREE;
				}
			}
		}
	}

	UpdateCursorTrace(Input.GetCursorPos(), uiIgnoreSurfaces);
	UpdateHoverSelection();
	//UpdateSelectedPlayerWaypoints();

	ValidateSelections();

	GameClient.GetInterfaceManager()->SetCursorHidden(false);

	CClientSnapshot &cSnapshot(GameClient.GetCurrentClientSnapshot());

	IUnitEntity *pHover(GameClient.GetUnitEntity(m_uiHoverEnt));

	// HACK: UI cursor is being set and overriding everything
	if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_DOWN) && cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_LEFT))
		Input.SetCursor(CURSOR_GAME, m_hScrollLeftDown);
	else if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_DOWN) && cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_RIGHT))
		Input.SetCursor(CURSOR_GAME, m_hScrollRightDown);
	else if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_UP) && cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_LEFT))
		Input.SetCursor(CURSOR_GAME, m_hScrollLeftUp);
	else if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_UP) && cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_RIGHT))
		Input.SetCursor(CURSOR_GAME, m_hScrollRightUp);
	else if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_DOWN))
		Input.SetCursor(CURSOR_GAME, m_hScrollDown);
	else if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_LEFT))
		Input.SetCursor(CURSOR_GAME, m_hScrollLeft);
	else if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_RIGHT))
		Input.SetCursor(CURSOR_GAME, m_hScrollRight);
	else if (cSnapshot.IsButtonDown(GAME_CMDR_BUTTON_EDGESCROLL_UP))
		Input.SetCursor(CURSOR_GAME, m_hScrollUp);
	else if (GetCommanderState() == COMSTATE_MOVE || GetCommanderState() == COMSTATE_PATROL)
	{
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));
		if (pTarget == NULL)
			Input.SetCursor(CURSOR_GAME, m_hActionValid);
		else if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, GetSelectedControlEntity()))
			Input.SetCursor(CURSOR_GAME, m_hActionEnemyValid);
		else
			Input.SetCursor(CURSOR_GAME, m_hActionAllyValid);
	}
	else if (GetCommanderState() == COMSTATE_TARGET_AREA || GetCommanderState() == COMSTATE_TARGET_VECTOR)
	{
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));
		if (pTarget == NULL)
			Input.SetCursor(CURSOR_GAME, m_hActionValid);
		else if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, GetSelectedControlEntity()))
			Input.SetCursor(CURSOR_GAME, m_hActionEnemyValid);
		else
			Input.SetCursor(CURSOR_GAME, m_hActionAllyValid);
	}
	else if (GetCommanderState() == COMSTATE_ATTACK)
	{
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));
		if (pUnit == NULL || pTarget == NULL)
			Input.SetCursor(CURSOR_GAME, m_hActionInvalid);
		else if (Game.IsValidTarget(pUnit->GetAttackTargetScheme(), 0, pUnit, pTarget, false))
		{
			if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, pUnit))
				Input.SetCursor(CURSOR_GAME, m_hActionEnemyValid);
			else
				Input.SetCursor(CURSOR_GAME, m_hActionAllyValid);
		}
		else
		{
			if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, pUnit))
				Input.SetCursor(CURSOR_GAME, m_hActionEnemyInvalid);
			else
				Input.SetCursor(CURSOR_GAME, m_hActionAllyInvalid);
		}
	}
	else if (GetCommanderState() == COMSTATE_TARGET_ENTITY)
	{		
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IEntityTool *pTool(pUnit ? pUnit->GetTool(m_iActiveSlot) : NULL);
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));

		if (pTarget != NULL && pTool != NULL && pTool->IsValidTarget(pTarget))
		{
			if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, pUnit))
				Input.SetCursor(CURSOR_GAME, m_hActionEnemyValid);
			else
				Input.SetCursor(CURSOR_GAME, m_hActionAllyValid);
		}
		else
		{
			if (pTarget == NULL)
				Input.SetCursor(CURSOR_GAME, m_hActionInvalid);
			else if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, pUnit))
				Input.SetCursor(CURSOR_GAME, m_hActionEnemyInvalid);
			else
				Input.SetCursor(CURSOR_GAME, m_hActionAllyInvalid);
		}
	}
	else if (GetCommanderState() == COMSTATE_TARGET_DUAL || GetCommanderState() == COMSTATE_TARGET_DUAL_POSITION)
	{		
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IEntityTool *pTool(pUnit ? pUnit->GetTool(m_iActiveSlot) : NULL);
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));

		if (pTarget != NULL && pTool != NULL)
		{
			if (pTool->IsValidTarget(pTarget))
			{
				if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, pUnit))
					Input.SetCursor(CURSOR_GAME, m_hActionEnemyValid);
				else
					Input.SetCursor(CURSOR_GAME, m_hActionAllyValid);
			}
			else
			{
				if (pTarget == NULL)
					Input.SetCursor(CURSOR_GAME, m_hActionInvalid);
				else if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, pUnit))
					Input.SetCursor(CURSOR_GAME, m_hActionEnemyInvalid);
				else
					Input.SetCursor(CURSOR_GAME, m_hActionAllyInvalid);
			}
		}
		else
		{
			if (pTarget == NULL)
				Input.SetCursor(CURSOR_GAME, m_hActionValid);
			else if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, GetSelectedControlEntity()))
				Input.SetCursor(CURSOR_GAME, m_hActionEnemyValid);
			else
				Input.SetCursor(CURSOR_GAME, m_hActionAllyValid);
		}
	}
	else if (pHover != NULL && pHover->IsBuilding() && pHover->GetAsBuilding()->GetIsShop())
	{
		Input.SetCursor(CURSOR_GAME, m_hShop);
	}
	else
	{
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));

		if (GetCommanderState() == COMSTATE_SELECT || pTarget == NULL)
			Input.SetCursor(CURSOR_GAME, m_hArrow);
		else if (pTarget->IsTargetType(TARGET_TRAIT_ENEMY, pUnit))
			Input.SetCursor(CURSOR_GAME, m_hArrowEnemy);
		else
			Input.SetCursor(CURSOR_GAME, m_hArrowAlly);
	}

	// if the item cursor is over an ally, change it to the "action ally valid" cursor.
	IUnitEntity *pHoverTarget(Game.GetUnitEntity(m_uiHoverEnt));
	if (GameClient.GetItemCursorIndex() != INVALID_INDEX && pHoverTarget != NULL && !pHoverTarget->IsBuilding())
	{
		if (pHoverTarget->IsTargetType(TARGET_TRAIT_ALLY, GetSelectedControlEntity()))
			Input.SetCursor(CURSOR_GAME, m_hActionAllyValid);
	}


	// Proxy targeting area
	if (GetCommanderState() == COMSTATE_TARGET_ENTITY ||
		GetCommanderState() == COMSTATE_TARGET_AREA ||
		GetCommanderState() == COMSTATE_TARGET_DUAL ||
		GetCommanderState() == COMSTATE_TARGET_DUAL_POSITION)
	{
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IEntityTool *pTool(pUnit ? pUnit->GetTool(m_iActiveSlot) : NULL);
		if (pTool != NULL && pTool->GetUseProxy() && pTool->GetProxySelectionRadius() > 0.0f)
		{
			ResHandle hMaterial(pTool->GetProxyTargetMaterial());
			if (hMaterial != INVALID_RESOURCE)
			{
				CSceneEntity scProxyTargetSprite;
				scProxyTargetSprite.Clear();
				scProxyTargetSprite.objtype = OBJTYPE_GROUNDSPRITE;
				scProxyTargetSprite.angle = 0.0f;
				scProxyTargetSprite.hRes = hMaterial;
				scProxyTargetSprite.SetPosition(pUnit->GetPosition());
				scProxyTargetSprite.width = scProxyTargetSprite.height = pTool->GetProxySelectionRadius();
				SceneManager.AddEntity(scProxyTargetSprite);
			}
		}
	}

	// Targeting sprites
	if (GetCommanderState() == COMSTATE_TARGET_AREA ||
		GetCommanderState() == COMSTATE_TARGET_ENTITY ||
		GetCommanderState() == COMSTATE_TARGET_DUAL ||
		GetCommanderState() == COMSTATE_TARGET_DUAL_POSITION)
	{
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IEntityTool *pTool(pUnit ? pUnit->GetTool(m_iActiveSlot) : NULL);
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));
		ResHandle hMaterial(pTool ? pTool->GetTargetMaterial() : INVALID_RESOURCE);
		if (pTool != NULL &&
			pTool->GetTargetRadius() > 0.0f &&
			hMaterial != INVALID_RESOURCE &&
			(pTool->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
			pTool->GetActionType() == TOOL_ACTION_TARGET_CURSOR ||
			pTarget != NULL))
		{
			if (pTool->GetActionType() == TOOL_ACTION_TARGET_POSITION ||
				pTool->GetActionType() == TOOL_ACTION_TARGET_CURSOR)
				GameClient.GetInterfaceManager()->SetCursorHidden(true);

			CSceneEntity scTargetSprite;
			scTargetSprite.Clear();
			scTargetSprite.objtype = OBJTYPE_GROUNDSPRITE;
			scTargetSprite.angle = 0.0f;
			scTargetSprite.hRes = hMaterial;
			if (pTool->GetActionType() == TOOL_ACTION_TARGET_ENTITY && pTarget != NULL)
				scTargetSprite.SetPosition(pTarget->GetPosition());
			else
				scTargetSprite.SetPosition(m_v3TraceEndPos);
			scTargetSprite.width = scTargetSprite.height = pTool->GetTargetRadius();
			SceneManager.AddEntity(scTargetSprite);
		}
	}

	// Vector targeting sprite
	if (GetCommanderState() == COMSTATE_TARGET_VECTOR)
	{
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IEntityTool *pTool(pUnit ? pUnit->GetTool(m_iActiveSlot) : NULL);
		IUnitEntity *pTarget(Game.GetUnitEntity(m_uiHoverEnt));
		ResHandle hMaterial(pTool ? pTool->GetTargetMaterial() : INVALID_RESOURCE);
		if (pTool != NULL &&
			pTool->GetTargetRadius() > 0.0f &&
			hMaterial != INVALID_RESOURCE &&
			(pTool->GetActionType() == TOOL_ACTION_TARGET_VECTOR || pTarget != NULL))
		{
			if (m_bVectorTargeting && DistanceSq(m_v3VectorTargetPos.xy(), m_v3TraceEndPos.xy()) > SQR<float>(cg_cmdrVectorTargetMinDistance))
			{
				//GameClient.GetInterfaceManager()->SetCursorHidden(true);

				CSceneEntity scTargetSprite;
				scTargetSprite.Clear();
				scTargetSprite.objtype = OBJTYPE_GROUNDSPRITE;
				scTargetSprite.angle = CVec3f(0.0f, 0.0f, M_YawToPosition(m_v3VectorTargetPos, m_v3TraceEndPos));
				scTargetSprite.hRes = hMaterial;
				scTargetSprite.SetPosition(m_v3VectorTargetPos);
				scTargetSprite.width = scTargetSprite.height = pTool->GetTargetRadius();
				SceneManager.AddEntity(scTargetSprite);
			}
		}
	}

	for (list<uint>::iterator it(m_vPendingPets.begin()); it != m_vPendingPets.end();)
	{
		uint uiPetIndex(*it);

		// if the client hasn't created the pet yet, wait for it.
		IUnitEntity* pPet(GameClient.GetUnitEntity(uiPetIndex));
		if (pPet == NULL)
		{
			++it;
			continue;
		}

		// try to insert our new pet into our keybindings. 
		uint uiPetType(pPet->GetType());
		ReplaceUnitInSelectionSets(uiPetIndex, uiPetType);
		it = m_vPendingPets.erase(it);
	}
}


/*====================
  CClientCommander::GetSelectionRect
  ====================*/
CRectf	CClientCommander::GetSelectionRect()
{
	CVec2f v2Start, v2End;

	v2Start = m_v2StartCursorPos;
	v2End = Input.GetCursorPos();

	CVec2f v2Min(
		MIN(v2Start.x, v2End.x),
		MIN(v2Start.y, v2End.y));
	CVec2f v2Max(
		MAX(v2Start.x, v2End.x),
		MAX(v2Start.y, v2End.y));

	return CRectf(v2Min, v2Max);
}


/*====================
  CClientCommander::UpdateHoverSelection
  ====================*/
void	CClientCommander::UpdateHoverSelection()
{
	m_setHoverSelection.clear();

	if (m_pPlayer == NULL)
		return;

	if (m_uiHoverEnt != INVALID_INDEX)
		m_setHoverSelection.insert(m_uiHoverEnt);

	if (GetCommanderState() != COMSTATE_SELECT)
		return;

	m_setHoverSelection.clear();

	CRectf rect(GetSelectionRect());

	WorldEntList &vEntities(Game.GetWorldEntityList());
	
	// First just get a potential set
	set<IUnitEntity*> setPotential;
	
	if (m_uiHoverEnt != INVALID_INDEX && Game.GetEntity(m_uiHoverEnt) != NULL)
		setPotential.insert(Game.GetUnitEntity(m_uiHoverEnt));

	for (WorldEntList::iterator it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
	{
		if (*it == INVALID_POOL_HANDLE)
			continue;

		uint uiIndex(it - vEntities.begin());

		IUnitEntity *pEntity(GameClient.GetUnitEntity(GameClient.GetGameIndexFromWorldIndex(uiIndex)));
		if (pEntity == NULL)
			continue;
		if (!pEntity->GetIsSelectable())
			continue;
		if (pEntity->GetStatus() != ENTITY_STATUS_ACTIVE)
			continue;
		if (!m_pPlayer->CanSee(pEntity) && !pEntity->GetAlwaysTargetable())
			continue;

		CVec2f v2Screen;
		if (!m_pCamera->WorldToScreen(pEntity->GetPosition(), v2Screen))
			continue;

		v2Screen += CVec2f(m_pCamera->GetX(), m_pCamera->GetY());

		if (rect.Contains(v2Screen) || pEntity->GetIndex() == m_uiFirstEntityIndex)
			setPotential.insert(pEntity);
	}

	if (m_pPlayer->GetTeam() != TEAM_SPECTATOR)
	{	
		// Add only units this player controls
		m_bControlSelection = true;
		for (set<IUnitEntity*>::iterator it(setPotential.begin()); it != setPotential.end(); ++it)
		{
			IUnitEntity *pEntity(*it);
			
			if (pEntity == NULL)
				continue;
			
			if (!pEntity->GetIsControllable())
				continue;
			if (!pEntity->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) && !m_bModifier3)
				continue;
			if (pEntity->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) && m_bModifier3)
				continue;

			m_setHoverSelection.insert(pEntity->GetIndex());
		}
		if (!m_setHoverSelection.empty())
			return;
	}

	// If there are no controllable units in the potential selection, add the rest
	m_bControlSelection = false;
	for (set<IUnitEntity*>::iterator it(setPotential.begin()); it != setPotential.end(); ++it)
		m_setHoverSelection.insert((*it)->GetIndex());
}


/*====================
  CClientCommander::Draw
  ====================*/
void	CClientCommander::Draw()
{
	if (IsSelectionActive())
	{
		CVec4f	v4Border(1.0f, 1.0f, 1.0f, 1.0f);
		CVec4f	v4Fill(0.3f, 0.7f, 1.0f, 0.2f);

		CRectf rec(GetSelectionRect());

		Draw2D.SetColor(v4Fill);
		Draw2D.Rect(rec.left, rec.top, rec.GetWidth(), rec.GetHeight());

		Draw2D.RectOutline(rec, 1.0f, v4Border);
	}
}


/*====================
  CClientCommander::ProximitySearch
  ====================*/
void	 CClientCommander::ProximitySearch(const CVec3f &v3Pos)
{
	if (GetCommanderState() != COMSTATE_TARGET_ENTITY)
		return;

	IUnitEntity *pUnit(GetSelectedControlEntity());
	IEntityTool *pTool(pUnit ? pUnit->GetTool(m_iActiveSlot) : NULL);

	if (m_uiHoverEnt == INVALID_INDEX && pTool != NULL && pTool->GetSearchRadius() > 0.0f)
	{
		float fShortestDistanceSq(FAR_AWAY);
		uint uiClosestIndex(INVALID_INDEX);

		// If the radius is large enough, use the entire unit list
		if (pTool->GetSearchRadius() >= 9999.0f)
		{
			const UnitList &lUnits(Game.GetUnitList());
			for (UnitList_cit itEntity(lUnits.begin()), itEntityEnd(lUnits.end()); itEntity != itEntityEnd; ++itEntity)
			{
				if (!pTool->IsValidTarget(*itEntity))
					continue;

				float fDistanceSq(DistanceSq((*itEntity)->GetPosition().xy(), v3Pos.xy()));
				if (fDistanceSq < fShortestDistanceSq)
				{
					fShortestDistanceSq = fDistanceSq;
					uiClosestIndex = (*itEntity)->GetIndex();
				}
			}
		}
		else
		{
			uivector vEntities;
			Game.GetEntitiesInRadius(vEntities, v3Pos.xy(), pTool->GetSearchRadius(), REGION_UNIT);

			for (uivector_it it(vEntities.begin()), itEnd(vEntities.end()); it != itEnd; ++it)
			{
				IVisualEntity *pSearchEntity(Game.GetEntityFromWorldIndex(*it));
				if (pSearchEntity == NULL)
					continue;

				IUnitEntity *pSearchUnit(pSearchEntity->GetAsUnit());
				if (pSearchUnit == NULL || !pTool->IsValidTarget(pSearchUnit))
					continue;

				float fDistanceSq(DistanceSq(pSearchUnit->GetPosition().xy(), v3Pos.xy()));
				if (fDistanceSq < fShortestDistanceSq)
				{
					fShortestDistanceSq = fDistanceSq;
					uiClosestIndex = Game.GetGameIndexFromWorldIndex(*it);
				}
			}
		}

		if (uiClosestIndex != INVALID_INDEX)
			m_uiHoverEnt = uiClosestIndex;
	}
}


/*====================
  CClientCommander::UpdateCursorTrace
  ====================*/
void	 CClientCommander::UpdateCursorTrace(const CVec2f &v2Cursor, uint uiIgnoreSurface, uint uiIgnoreEntity)
{
	if (m_pPlayer == NULL)
		return;

	bool bFullTrace(GetCommanderState() != COMSTATE_SELECT &&
		GetCommanderState() != COMSTATE_TARGET_AREA);
	bool bPlayHoverSound(false);

	// We don't want to hit building bounds (uses model instead)
	bool bTraceBuildings((~uiIgnoreSurface & SURF_BUILDING) != 0);

	uiIgnoreSurface |= SURF_BUILDING;
	STraceInfo trace;

	// ignore units when vector targetting.
	if (GetCommanderState() == COMSTATE_TARGET_VECTOR)
		uiIgnoreSurface |= SURF_UNIT;
	
	CVec3f v3Dir(GameClient.GetCamera()->ConstructRay(v2Cursor - GameClient.GetCamera()->GetXY()));
	CVec3f v3End(M_PointOnLine(GameClient.GetCamera()->GetOrigin(), v3Dir, FAR_AWAY));

	Game.TraceLine(trace, GameClient.GetCamera()->GetOrigin(), v3End, bFullTrace ? uiIgnoreSurface : TRACE_TERRAIN, uiIgnoreEntity);

	m_v3TraceEndPos = trace.v3EndPos;

	float fSize(1.0f);

	while (trace.uiEntityIndex == INVALID_INDEX && fSize <= 32.0f)
	{
		Game.TraceBox(trace, GameClient.GetCamera()->GetOrigin(), v3End, CBBoxf(-fSize, fSize), bFullTrace ? uiIgnoreSurface : TRACE_TERRAIN, uiIgnoreEntity);
		fSize *= 2.0f;
	}

	STraceInfo cBuildingTrace;
	if (bTraceBuildings && bFullTrace)
	{
		uiIgnoreSurface &= ~SURF_BUILDING;
		uiIgnoreSurface |= SURF_BOUNDS;

		Game.TraceLine(cBuildingTrace, GameClient.GetCamera()->GetOrigin(), v3End, uiIgnoreSurface, uiIgnoreEntity);

		if (cBuildingTrace.uiEntityIndex != INVALID_INDEX && (cBuildingTrace.fFraction < trace.fFraction || trace.uiEntityIndex == INVALID_INDEX))
			trace = cBuildingTrace;
	}

	if (bFullTrace)
	{
		uint uiHoverEnt(Game.GetGameIndexFromWorldIndex(trace.uiEntityIndex));

		if (uiHoverEnt != INVALID_INDEX && uiHoverEnt != m_uiHoverEnt)
			bPlayHoverSound = true;

		m_uiHoverEnt = uiHoverEnt;

		// if an item is being transferred (held by the cursor) then don't hover over any buildings.
		// (this fixes a bug where you can't click on an inventory slot when a building is behind it.)
		IUnitEntity *pHoverTarget(Game.GetUnitEntity(uiHoverEnt));
		if (GameClient.GetItemCursorIndex() != INVALID_INDEX && pHoverTarget != NULL && pHoverTarget->IsBuilding() && !pHoverTarget->GetAsBuilding()->GetIsShop())
		{
			m_uiHoverEnt = INVALID_INDEX;
		}
	}

	if (!bFullTrace)
		m_uiHoverEnt = INVALID_INDEX;

	IUnitEntity *pTarget(GameClient.GetUnitEntity(m_uiHoverEnt));
	if (pTarget == NULL || (!m_pPlayer->CanSee(pTarget) && !pTarget->GetAlwaysTargetable()))
		m_uiHoverEnt = INVALID_INDEX;

	if ((GetCommanderState() == COMSTATE_TARGET_ENTITY || GetCommanderState() == COMSTATE_TARGET_DUAL || GetCommanderState() == COMSTATE_TARGET_DUAL_POSITION) && pTarget != NULL && pTarget->IsType(Prop_Tree))
	{
		IUnitEntity *pUnit(GetSelectedControlEntity());
		IEntityTool *pTool(pUnit ? pUnit->GetTool(m_iActiveSlot) : NULL);
		if (pTool == NULL || !pTool->IsValidTarget(pTarget))
			m_uiHoverEnt = INVALID_INDEX;
	}

	if (bPlayHoverSound && m_uiHoverEnt != INVALID_INDEX && m_hHoverSound != INVALID_RESOURCE)
		K2SoundManager.Play2DSound(m_hHoverSound);

	ProximitySearch(m_v3TraceEndPos);
}


/*====================
  CClientCommander::PrimaryDown
  ====================*/
void	CClientCommander::PrimaryDown(const CVec2f &v2Cursor)
{
	IUnitEntity *pControl(GetSelectedControlEntity());

	CancelDoubleActivate();

	// Ignore a single controlled unit when attacking, moving, or dropping items
	if (GetCommanderState() == COMSTATE_ATTACK || GetCommanderState() == COMSTATE_MOVE || GameClient.GetItemCursorIndex() != INVALID_INDEX)
		UpdateCursorTrace(v2Cursor, HOVER_TRACE, pControl != NULL ? pControl->GetWorldIndex() : INVALID_INDEX);
	else
		UpdateCursorTrace(v2Cursor, HOVER_TRACE);

	if (GameClient.GetItemCursorIndex() != INVALID_INDEX)
	{
		UpdateCursorTrace(Input.GetCursorPos(), HOVER_TRACE | SURF_ITEM, pControl != NULL ? pControl->GetWorldIndex() : INVALID_INDEX);

		if (m_uiHoverEnt != INVALID_INDEX)
		{
			IUnitEntity *pEnt(GameClient.GetUnitEntity(m_uiHoverEnt));

			if (pEnt != NULL && pEnt->IsBuilding() && pEnt->GetAsBuilding()->GetIsShop())
			{
				IGameEntity *pItem(Game.GetEntity(GameClient.GetItemCursorIndex()));
				if (pItem == NULL)
				{
					GameClient.SetItemCursorIndex(INVALID_INDEX);
					return;
				}

				int iSlot(pItem->IsItem() ? pItem->GetAsItem()->GetSlot() : -1);
				if (iSlot == -1)
				{
					GameClient.SetItemCursorIndex(INVALID_INDEX);
					return;
				}

				uint uiUnitIndex(GetSelectedControlEntityIndex());
				if (uiUnitIndex == INVALID_INDEX)
				{
					GameClient.SetItemCursorIndex(INVALID_INDEX);
					return;
				}

				CBufferFixed<9> buffer;
				buffer << GAME_CMD_SELL << uiUnitIndex << iSlot;
				GameClient.SendGameData(buffer, true);

				GameClient.SetItemCursorIndex(INVALID_INDEX);
				return;
			}

			if (pEnt && pEnt->GetCanCarryItems())
			{
				StartCommand(m_v3TraceEndPos.xy(), GetModifier1() ? (GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE, CMDR_ORDER_GIVEITEM, GameClient.GetItemCursorIndex());
				GameClient.SetItemCursorIndex(INVALID_INDEX);
			}

			return;
		}
		else
		{
			StartCommand(m_v3TraceEndPos.xy(), GetModifier1() ? (GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE, CMDR_ORDER_GIVEITEM, GameClient.GetItemCursorIndex());
			GameClient.SetItemCursorIndex(INVALID_INDEX);
			return;
		}
	}

	switch (GetCommanderState())
	{
	case COMSTATE_HOVER:
		StartSelect(v2Cursor);
		break;

	case COMSTATE_SELECT:
		break;

	case COMSTATE_MOVE:
		StartCommand(m_v3TraceEndPos.xy(), GetModifier1() ? (GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE, CMDR_ORDER_MOVE);
		break;

	case COMSTATE_ATTACK:
		StartCommand(m_v3TraceEndPos.xy(), GetModifier1() ? (GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE, CMDR_ORDER_ATTACK);
		break;

	case COMSTATE_PATROL:
		StartCommand(m_v3TraceEndPos.xy(), GetModifier1() ? (GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE, CMDR_ORDER_PATROL);
		break;

	case COMSTATE_TARGET_ENTITY:
	case COMSTATE_TARGET_DUAL:
	case COMSTATE_TARGET_DUAL_POSITION:
		Cast(m_v3TraceEndPos.xy(), V2_ZERO, true);
		break;

	case COMSTATE_TARGET_AREA:
		Cast(m_v3TraceEndPos.xy(), V2_ZERO, false);
		break;

	case COMSTATE_TARGET_VECTOR:
		m_v3VectorTargetPos = m_v3TraceEndPos;
		m_bVectorTargeting = true;
		break;
	}
}


/*====================
  CClientCommander::PrimaryUp
  ====================*/
void	CClientCommander::PrimaryUp(const CVec2f &v2Cursor)
{
	switch (GetCommanderState())
	{
	case COMSTATE_HOVER:
		break;

	case COMSTATE_SELECT:
		ApplySelect(v2Cursor);

		// make a hero unit our active control unit if our selection contains one.
		if (SelectionContainsHeroes())
		{
			IUnitEntity *pActive(GameClient.GetUnitEntity(m_uiActiveControlEntity));
			if (pActive == NULL || !pActive->IsHero())
			{
				for (uiset::iterator it(m_setControlSelection.begin()); it != m_setControlSelection.end(); ++it)
				{
					IUnitEntity *pUnit(GameClient.GetUnitEntity(*it));
					if (pUnit != NULL && pUnit->IsHero())
					{
						m_uiActiveControlEntity = *it;
						break;
					}
				}
			}
		}

		break;

	case COMSTATE_MOVE:
		break;

	case COMSTATE_ATTACK:
		break;

	case COMSTATE_PATROL:
		break;

	case COMSTATE_TARGET_ENTITY:
		break;

	case COMSTATE_TARGET_AREA:
		break;

	case COMSTATE_TARGET_DUAL:
		break;

	case COMSTATE_TARGET_DUAL_POSITION:
		break;

	case COMSTATE_TARGET_VECTOR:
		if (DistanceSq(m_v3VectorTargetPos.xy(), m_v3TraceEndPos.xy()) > SQR<float>(cg_cmdrVectorTargetMinDistance))
		{
			Cast(m_v3VectorTargetPos.xy(), m_v3TraceEndPos.xy() - m_v3VectorTargetPos.xy());
		}
		m_bVectorTargeting = false;
		break;
	}
}

/*====================
  CClientCommander::SelectionContainsHeroes
  ====================*/
bool	CClientCommander::SelectionContainsHeroes() const
{
	for (uiset::const_iterator it(m_setControlSelection.begin()); it != m_setControlSelection.end(); ++it)
	{
		IUnitEntity *pUnit(GameClient.GetUnitEntity(*it));
		if (pUnit == NULL)
			continue;
		if (pUnit->IsHero())
		{
			return true;
		}
	}
	return false;
}


/*====================
  CClientCommander::SecondaryDown
  ====================*/
void	CClientCommander::SecondaryDown(const CVec2f &v2Cursor)
{
	IUnitEntity *pControl(GetSelectedControlEntity());

	// Ignore a single controlled unit when right-clicking
	UpdateCursorTrace(v2Cursor, HOVER_TRACE, pControl != NULL ? pControl->GetWorldIndex() : INVALID_INDEX);

	if (GameClient.GetItemCursorIndex() != INVALID_INDEX)
	{
		GameClient.SetItemCursorIndex(INVALID_INDEX);
		return;
	}

	switch (GetCommanderState())
	{
	case COMSTATE_HOVER:
		StartCommand(m_v3TraceEndPos.xy(), GetModifier1() ? (GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE);
		break;

	case COMSTATE_SELECT:
		Cancel(v2Cursor);
		StartCommand(m_v3TraceEndPos.xy(), GetModifier1() ? (GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK) : QUEUE_NONE);
		break;

	case COMSTATE_MOVE:
		SetCommanderState(COMSTATE_HOVER);
		break;

	case COMSTATE_ATTACK:
		SetCommanderState(COMSTATE_HOVER);
		break;

	case COMSTATE_PATROL:
		SetCommanderState(COMSTATE_HOVER);
		break;

	case COMSTATE_TARGET_ENTITY:
	case COMSTATE_TARGET_AREA:
	case COMSTATE_TARGET_DUAL:
	case COMSTATE_TARGET_DUAL_POSITION:
		m_iActiveSlot = -1;
		SetCommanderState(COMSTATE_HOVER);
		break;

	case COMSTATE_TARGET_VECTOR:
		m_bVectorTargeting = false;
		m_iActiveSlot = -1;
		SetCommanderState(COMSTATE_HOVER);
		break;
	}
}


/*====================
  CClientCommander::SecondaryUp
  ====================*/
void	CClientCommander::SecondaryUp(const CVec2f &v2Cursor)
{
	switch (GetCommanderState())
	{
	case COMSTATE_HOVER:
		break;

	case COMSTATE_SELECT:
		break;

	case COMSTATE_MOVE:
		break;

	case COMSTATE_ATTACK:
		break;

	case COMSTATE_PATROL:
		break;

	case COMSTATE_TARGET_ENTITY:
		break;

	case COMSTATE_TARGET_AREA:
		break;

	case COMSTATE_TARGET_DUAL:
		break;

	case COMSTATE_TARGET_DUAL_POSITION:
		break;

	case COMSTATE_TARGET_VECTOR:
		break;
	}
}


/*====================
  CClientCommander::Cancel
  ====================*/
bool	CClientCommander::Cancel(const CVec2f &v2Cursor)
{
	if (GetCommanderState() != COMSTATE_HOVER)
	{
		SetCommanderState(COMSTATE_HOVER);
		m_iActiveSlot = -1;
		m_bVectorTargeting = false;
		return true;
	}

	if (!m_setInfoSelection.empty())
	{
		m_setInfoSelection.clear();
		return true;
	}

	return false;
}


/*====================
  CClientCommander::StartSelect
  ====================*/
void	CClientCommander::StartSelect(const CVec2f &v2Cursor)
{
	SetCommanderState(COMSTATE_SELECT);
	m_v2StartCursorPos = v2Cursor;
	m_v3StartCursorPos = m_v3TraceEndPos;
	m_uiFirstEntityIndex = m_uiHoverEnt;
	m_uiSelectTime = Host.GetTime();
}


/*====================
  CClientCommander::ApplySelect
  ====================*/
void	CClientCommander::ApplySelect(const CVec2f &v2Cursor)
{
	UpdateHoverSelection();

	if (m_setHoverSelection.empty())
	{
		m_uiFirstEntityIndex = INVALID_INDEX;
		SetCommanderState(COMSTATE_HOVER);
		m_v3StartCursorPos.Clear();
		return;
	}

	uiset *pSelection(m_bControlSelection ? &m_setControlSelection : &m_setInfoSelection);

	uint uiLastSelect(pSelection->size() == 1 ? *pSelection->begin() : INVALID_INDEX);

	// If SelectEntity was recently called for this same entity, select other of same type nearby
	if (((m_uiSelectTime - m_uiLastPrimaryTime < cg_cmdrDoubleSelectTime && m_uiFirstEntityIndex == uiLastSelect) || m_bModifier2) && m_uiFirstEntityIndex != INVALID_INDEX)
	{
		m_uiLastPrimaryTime = 0;

		IUnitEntity *pUnit(Game.GetUnitEntity(m_uiFirstEntityIndex));
		if (pUnit != NULL)
		{
			int iLocalClientNumber(GameClient.GetLocalClientNum());
			bool bControlSelection(pUnit->CanActOnOrdersFrom(iLocalClientNumber));
			bool bFoundUnits(false);

			uivector vResult;
			GameClient.GetEntitiesInRadius(vResult, pUnit->GetPosition().xy(), cg_cmdrDoubleSelectRadius, REGION_UNIT);
			for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
			{
				uint uiIndex(GameClient.GetGameIndexFromWorldIndex(*it));
				IUnitEntity *pOther(GameClient.GetUnitEntity(uiIndex));
				if (pOther == NULL)
					continue;
				if (pOther->GetType() != pUnit->GetType())
					continue;
				if (bControlSelection && pOther->IsIllusion() != pUnit->IsIllusion())
					continue;
				if (bControlSelection != pOther->CanActOnOrdersFrom(iLocalClientNumber))
					continue;

				m_setHoverSelection.insert(uiIndex);
				bFoundUnits = true;
			}

			if (bFoundUnits)
			{
				uiset *pSelection(bControlSelection ? &m_setControlSelection : &m_setInfoSelection);
				m_setInfoSelection.clear();
				
				pSelection->clear();
				for (uiset_it it(m_setHoverSelection.begin()); it != m_setHoverSelection.end(); ++it)
					pSelection->insert(*it);

				PlaySelectSound(m_uiFirstEntityIndex);
				m_uiFirstEntityIndex = pUnit->GetIndex();
			}
		}
	}
	
	// Clear info selection on new control selections
	if (m_bControlSelection)
		m_setInfoSelection.clear();

	if (!m_bModifier1)
		pSelection->clear();

	for (uiset_it it(m_setHoverSelection.begin()); it != m_setHoverSelection.end(); ++it)
	{
		if (m_bModifier1 && pSelection->find(*it) != pSelection->end() && m_setHoverSelection.size() == 1)
			pSelection->erase(*it);
		else
			pSelection->insert(*it);
	}

	// Open shop
	if (!m_bControlSelection && m_setInfoSelection.size() == 1)
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(*m_setInfoSelection.begin()));
		if (pUnit != NULL && pUnit->IsBuilding() && pUnit->GetAsBuilding()->GetIsShop())
		{
			GameClient.GetInterfaceManager()->SetShopVisible(true);
			GameClient.SetActiveShop(pUnit->GetAsBuilding()->GetDefaultShop());

			m_setInfoSelection.clear();
		}
	}

	if (pSelection->size() > 0)
		PlaySelectSound(*pSelection->begin());

	SetCommanderState(COMSTATE_HOVER);
	m_v3StartCursorPos.Clear();

	UpdateSelection();

	m_uiLastPrimaryTime = Host.GetTime();
}


/*====================
  CClientCommander::GiveOrder
  ====================*/
void	CClientCommander::GiveOrder(ECommanderOrder eOrder, uint uiTargetIndex, byte yQueue, uint uiParam)
{
	if (!GetActiveUnits() || GameClient.HasFlags(GAME_FLAG_PAUSED))
		return;

	IUnitEntity *pTarget(GameClient.GetUnitEntity(uiTargetIndex));
	if (pTarget == NULL)
		return;

#if 0
	IUnitEntity *pControl(GetSelectedControlEntity());

	if (eOrder == CMDR_ORDER_ATTACK)
	{
		if ((pControl != NULL && !Game.IsValidTarget(pControl->GetAttackTargetScheme(), pControl->GetAttackEffectType(), pControl, pTarget)) ||
			(pControl == NULL && pTarget->HasUnitFlags(UNIT_FLAG_INVULNERABLE)))
		{
			if (GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE) != INVALID_RESOURCE)
				K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE));

			ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_target_invulnerable")));
			return;
		}
	}
#endif

	CBufferFixed<16> buffer;
	buffer
		<< GAME_CMD_ORDER_ENTITY
		<< byte(eOrder)
		<< uiTargetIndex
		<< uiParam
		<< yQueue
		<< (GetModifier2() ? m_uiActiveControlEntity : INVALID_INDEX)
		<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
	GameClient.SendGameData(buffer, true);
}

void	CClientCommander::GiveOrder(ECommanderOrder eOrder, const CVec2f &v2Pos, byte yQueue, uint uiParam)
{
	if (!GetActiveUnits() || GameClient.HasFlags(GAME_FLAG_PAUSED))
		return;

	if (eOrder == CMDR_ORDER_MOVE || eOrder == CMDR_ORDER_ATTACK || eOrder == CMDR_ORDER_FOLLOW)
	{
		bool bMobile(false);
		for (uiset::iterator it(m_setControlSelection.begin()); it != m_setControlSelection.end(); ++it)
		{
			IUnitEntity *pUnit(GameClient.GetUnitEntity(*it));
			if (pUnit == NULL)
				continue;
			if (pUnit->GetIsMobile())
			{
				bMobile = true;
				break;
			}
		}

		if (!bMobile)
			return;
	}

	CGameEvent ev;
	ev.SetSourcePosition(Game.GetTerrainPosition(v2Pos));

	switch (eOrder)
	{
	default:
	case CMDR_ORDER_MOVE:
		ev.SetEffect(GameClient.GetResource(CLIENT_RESOURCE_MOVE_INDICATOR_EFFECT));
		break;

	case CMDR_ORDER_ATTACK:		
		ev.SetEffect(GameClient.GetResource(CLIENT_RESOURCE_ATTACK_INDICATOR_EFFECT));
		break;

	case CMDR_ORDER_GIVEITEM:
		ev.SetEffect(GameClient.GetResource(CLIENT_RESOURCE_MOVE_INDICATOR_EFFECT));
		break;
	}

	if (ev.GetEffect() != INVALID_RESOURCE)
	{
		ev.Spawn();
		Game.AddEvent(ev);
	}

	CBufferFixed<20> buffer;
	buffer
		<< GAME_CMD_ORDER_POSITION
		<< byte(eOrder)
		<< v2Pos
		<< uiParam
		<< yQueue
		<< (GetModifier2() ? m_uiActiveControlEntity : INVALID_INDEX)
		<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
	GameClient.SendGameData(buffer, true);
}


/*====================
  CClientCommander::StartCommand
  ====================*/
void	CClientCommander::StartCommand(const CVec2f &v2Pos, byte yQueue, ECommanderOrder eOrder, uint uiParam)
{
	// Determine order
	if (eOrder == CMDR_ORDER_AUTO)
	{
		if (m_uiHoverEnt != INVALID_INDEX)
		{
			IUnitEntity *pTarget(GameClient.GetUnitEntity(m_uiHoverEnt));

			if (pTarget != NULL &&
				(pTarget->GetType() == Entity_Chest ||
				pTarget->IsPowerup() ||
				(pTarget->IsBuilding() && pTarget->GetAsBuilding()->GetIsShop()) ||
				pTarget->GetPreferTouch())
			)
				eOrder = CMDR_ORDER_TOUCH;
			else if (pTarget != NULL && m_pPlayer->IsEnemy(pTarget))
				eOrder = CMDR_ORDER_ATTACK;
			else
				eOrder = CMDR_ORDER_FOLLOW;
		}
		else
		{
			eOrder = CMDR_ORDER_MOVE;
		}
	}

	if (m_uiHoverEnt != INVALID_INDEX)
		GiveOrder(eOrder, m_uiHoverEnt, yQueue, uiParam);
	else
		GiveOrder(eOrder, v2Pos, yQueue, uiParam);

	SetCommanderState(COMSTATE_HOVER);
}


/*====================
  CClientCommander::ActivateTool
  ====================*/
void	CClientCommander::ActivateTool(int iSlot, bool bSecondary, IUnitEntity *pUnit, bool bClick)
{
#if 0
	if (GameClient.GetLevelup())
	{
		GameClient.LevelupAbility(iSlot);
		return;
	}
#endif

	CPlayer *pLocal(Game.GetLocalPlayer());

	if (pUnit == NULL)
		return;

	if (!pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) && 
		(pLocal == NULL || iSlot < INVENTORY_START_SHARED_ABILITIES || iSlot > INVENTORY_END_SHARED_ABILITIES || pUnit->GetTeam() != pLocal->GetTeam()))
		return;

	IUnitEntity *pOwnerUnit(pUnit);
	if (iSlot >= INVENTORY_START_SHARED_ABILITIES && iSlot <= INVENTORY_END_SHARED_ABILITIES)
	{
		CTeamInfo *pTeam(Game.GetTeam(pUnit->GetTeam()));
		if (pTeam != NULL)
		{
			IUnitEntity *pBase(Game.GetUnitEntity(pTeam->GetBaseBuildingIndex()));
			if (pBase != NULL)
			{
				pOwnerUnit = pBase;
			}
		}
	}

	// Get the units item
	int iAbilitySlot(-1);
	for (int i(INVENTORY_START_ABILITIES); i <= INVENTORY_END_ABILITIES; ++i)
	{
		IEntityAbility *pAbility(pOwnerUnit->GetAbility(i));
		if (pAbility == NULL || pAbility->GetKeySlot() == -1)
			continue;
		if (pAbility->GetKeySlot() == iSlot)
			iAbilitySlot = i;
	}
	if (iAbilitySlot == -1)
		iAbilitySlot = iSlot;

	IEntityTool *pTool(pOwnerUnit->GetTool(iAbilitySlot));
	if (pTool == NULL)
		return;

	if (pTool->IsItem() && !pTool->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
	{
		ushort unShop(GameClient.GetShop(pTool->GetTypeName()));
		CShopDefinition *pShop(EntityRegistry.GetDefinition<CShopDefinition>(unShop));
		if (pShop != NULL)
		{
			if (GameClient.GetInterfaceManager()->IsShopVisible() &&
				GameClient.GetActiveShop() == EntityRegistry.LookupName(unShop) &&
				GameClient.GetActiveRecipe() == pTool->GetTypeName())
			{
				GameClient.GetInterfaceManager()->SetShopVisible(false);
				//GameClient.SetActiveShop(TSNULL);
			}
			else
			{
				GameClient.GetInterfaceManager()->SetShopVisible(true);
				GameClient.SetActiveShop(EntityRegistry.LookupName(unShop));
				GameClient.SetActiveRecipe(pTool->GetTypeName(), false);
			}
		}

		return;
	}

	if (pTool->IsAbility() && pTool->GetAsAbility()->GetSubSlot() != -1)
	{
		iAbilitySlot = pTool->GetAsAbility()->GetSubSlot();
		pTool = pOwnerUnit->GetTool(iAbilitySlot);

		if (pTool == NULL)
			return;
	}

	// Don't process abilities that aren't leveled
	if (pTool->IsAbility() && pTool->GetLevel() < 1 && pTool->GetMaxLevel() > 0)
		return;

	if (bSecondary && (pTool->GetActionType() == TOOL_ACTION_ATTACK || pTool->GetAllowAutoCast()))
	{
		CBufferFixed<7> buffer;
		buffer << GAME_CMD_ABILITY2 << pUnit->GetIndex() << byte(iAbilitySlot) << (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE);
		GameClient.SendGameData(buffer, true);
		return;
	}
	else if (bSecondary)
	{
		return;
	}

	if (pTool->GetActionType() == TOOL_ACTION_ATTACK_TOGGLE)
	{
		CBufferFixed<7> buffer;
		buffer << GAME_CMD_ABILITY << pUnit->GetIndex() << byte(iAbilitySlot) << (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE);
		GameClient.SendGameData(buffer, true);
		return;
	}

	if (!pTool->CanOrder())
	{
		CClientEntity *pClEntity(GameClient.GetClientEntity(pUnit->GetIndex()));
		if (cg_unitVoiceResponses && pClEntity != NULL)
		{
			ResHandle hSample(!pTool->IsReady() ? pUnit->GetCooldownSound() : pOwnerUnit->GetMana() < pTool->GetCurrentManaCost() ? pUnit->GetNoManaSound() : INVALID_RESOURCE);
			if (hSample != INVALID_RESOURCE)
				pClEntity->PlaySound(hSample, 1.0f, 0.0f, 7, 128, SND_LINEARFALLOFF, 0, 0, 0, false, 0, 1.0f, 1.0f, 0, 17000.0f);

			ResetVoiceSequence();
		}

		tsmapts mapTokens;
		mapTokens[_T("tool")] = pTool->GetDisplayName();

		if (!pTool->IsReady() || pTool->HasFlag(ENTITY_TOOL_FLAG_INVALID_COST))
			ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_not_ready"), mapTokens));
		else if (pUnit->GetMana() < pTool->GetCurrentManaCost())
			ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_low_mana"), mapTokens));
		else if (pTool->GetUseProxy() && pTool->SelectProxy() == NULL)
			ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_no_proxy"), mapTokens));

		if (GameClient.GetResource(CLIENT_RESOURCE_ABILITY_ACTIVATE_ERROR_SAMPLE) != INVALID_RESOURCE)
			K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_ABILITY_ACTIVATE_ERROR_SAMPLE));

		return;
	}

	if (pTool != m_pLastDoubleActivateTool)
		CancelDoubleActivate();

	m_pLastDoubleActivateTool = pTool;

	switch (pTool->GetActionType())
	{
	case TOOL_ACTION_PASSIVE:
		break;

	case TOOL_ACTION_TOGGLE:
	case TOOL_ACTION_NO_TARGET:
	case TOOL_ACTION_GLOBAL:
	case TOOL_ACTION_TARGET_SELF:
	case TOOL_ACTION_FACING:
	case TOOL_ACTION_SELF_POSITION:
	case TOOL_ACTION_ATTACK_TOGGLE:
		{
			CBufferFixed<7> buffer;
			buffer << GAME_CMD_ABILITY << pUnit->GetIndex() << byte(iAbilitySlot) << (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE);
			GameClient.SendGameData(buffer, true);
		}
		break;

	case TOOL_ACTION_ATTACK:
	case TOOL_ACTION_TARGET_ENTITY:
		m_iActiveSlot = iAbilitySlot;
		SetCommanderState(COMSTATE_TARGET_ENTITY);
		if (CanDoubleActivate())
		{
			DoubleActivate();
			CancelDoubleActivate();
		}
		UpdateDoubleActivate();
		break;

	case TOOL_ACTION_TARGET_POSITION:
		{
			m_iActiveSlot = iAbilitySlot;
			SetCommanderState(COMSTATE_TARGET_AREA);
			if (CanDoubleActivate())
			{
				DoubleActivate();
				CancelDoubleActivate();
			}
			UpdateDoubleActivate();
		}
		break;

	case TOOL_ACTION_TARGET_CURSOR:
		{
			if (bClick)
			{
				m_iActiveSlot = iAbilitySlot;
				SetCommanderState(COMSTATE_TARGET_AREA);
				if (CanDoubleActivate())
				{
					DoubleActivate();
					CancelDoubleActivate();
				}
				UpdateDoubleActivate();
			}
			else
			{
				m_iActiveSlot = iAbilitySlot;
				Cast(V2_ZERO, V2_ZERO, true);
				m_iActiveSlot = -1;
			}
		}
		break;

	case TOOL_ACTION_TARGET_DUAL:
		m_iActiveSlot = iAbilitySlot;
		SetCommanderState(COMSTATE_TARGET_DUAL);
		break;

	case TOOL_ACTION_TARGET_DUAL_POSITION:
		m_iActiveSlot = iAbilitySlot;
		SetCommanderState(COMSTATE_TARGET_DUAL_POSITION);
		break;

	case TOOL_ACTION_TARGET_VECTOR:
		m_iActiveSlot = iAbilitySlot;
		SetCommanderState(COMSTATE_TARGET_VECTOR);
		break;

	case TOOL_ACTION_INVALID:
		Console << _T("Invalid action type for tool: ") << pTool->GetDisplayName() << newl;
		break;
	}
}


/*====================
  CClientCommander::Cast
  ====================*/
void	CClientCommander::Cast(const CVec2f &v2Pos, const CVec2f &v2Delta, bool bUpdateTrace)
{
	// Spawner
	if (!m_sClickCmd.empty())
	{
		GameClient.SendRemoteCommand(m_sClickCmd + _T(" ") + XtoA(v2Pos.x) + _T(" ") + XtoA(v2Pos.y));

		if (!m_bModifier2)
		{
			m_sClickCmd.clear();
			SetCommanderState(COMSTATE_HOVER);
			m_iActiveSlot = -1;
		}
		return;
	}

	// Get current selected unit
	IUnitEntity *pUnit(GetSelectedControlEntity());
	if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()))
	{
		SetCommanderState(COMSTATE_HOVER);
		m_iActiveSlot = -1;
		return;
	}

	// Get the units item
	IEntityTool *pItem(pUnit->GetTool(m_iActiveSlot));
	if (pItem == NULL)
	{
		SetCommanderState(COMSTATE_HOVER);
		m_iActiveSlot = -1;
		return;
	}
	if (!pItem->CanOrder())
	{
		SetCommanderState(COMSTATE_HOVER);
		m_iActiveSlot = -1;
		return;
	}

	switch (pItem->GetActionType())
	{
	case TOOL_ACTION_PASSIVE:
	case TOOL_ACTION_TOGGLE:
	case TOOL_ACTION_NO_TARGET:
	case TOOL_ACTION_TARGET_SELF:
	case TOOL_ACTION_FACING:
	case TOOL_ACTION_SELF_POSITION:
	case TOOL_ACTION_ATTACK_TOGGLE:
		break;

	case TOOL_ACTION_ATTACK:
	case TOOL_ACTION_TARGET_ENTITY:
		{
			uint uiIgnoreSurface(ACTIVATE_TRACE);
			if (!Game.CanTargetTrait(pItem->GetTargetScheme(), TARGET_TRAIT_TREE))
				uiIgnoreSurface |= SURF_TREE;

			if (bUpdateTrace)
				UpdateCursorTrace(Input.GetCursorPos(), uiIgnoreSurface);

			IUnitEntity *pTarget(GameClient.GetUnitEntity(m_uiHoverEnt));
			if (pTarget == NULL)
			{
				if (GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE) != INVALID_RESOURCE)
					K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE));

				if (pItem->IsAbility())
					ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_not_unit_ability")));
				else if (pItem->IsItem())
					ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_not_unit_item")));
				else
					ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_not_unit")));

				return;
			}

			if (!pItem->IsValidTarget(pTarget))
			{
				if (GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE) != INVALID_RESOURCE)
					K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE));

				ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_invalid_target")));

				return;
			}

			CBufferFixed<12> buffer;
			buffer
				<< GAME_CMD_ABILITY_ENTITY
				<< pUnit->GetIndex()
				<< byte(m_iActiveSlot)
				<< m_uiHoverEnt
				<< (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE)
				<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
			GameClient.SendGameData(buffer, true);
		}
		break;

	case TOOL_ACTION_TARGET_CURSOR:
	case TOOL_ACTION_TARGET_POSITION:
		{
			CVec2f v2Pos2(v2Pos);

			if (bUpdateTrace)
			{
				UpdateCursorTrace(Input.GetCursorPos(), TRACE_TERRAIN);
				v2Pos2 = m_v3TraceEndPos.xy();
			}

			if (Distance(pUnit->GetPosition().xy(), v2Pos2) < pItem->GetMinRange() || !Game.IsInBounds(v2Pos2.x, v2Pos2.y))
				return;

			if (pItem->GetNeedVision() && !GameClient.IsVisible(v2Pos2.x, v2Pos2.y))
				return;

			CBufferFixed<16> buffer;
			buffer
				<< GAME_CMD_ABILITY_POSITION
				<< pUnit->GetIndex()
				<< byte(m_iActiveSlot)
				<< v2Pos2
				<< (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE)
				<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
			GameClient.SendGameData(buffer, true);

			CGameEvent ev;
			ev.SetSourcePosition(Game.GetTerrainPosition(v2Pos2));
			ev.SetEffect(GameClient.GetResource(CLIENT_RESOURCE_CAST_INDICATOR_EFFECT));

			if (ev.GetEffect() != INVALID_RESOURCE)
			{
				ev.Spawn();
				Game.AddEvent(ev);
			}
		}
		break;

	case TOOL_ACTION_TARGET_DUAL:
	case TOOL_ACTION_TARGET_DUAL_POSITION:
		{
			uint uiIgnoreSurfaces2 = ACTIVATE_TRACE;

			// Get current selected unit
			IUnitEntity *pUnit(GetSelectedControlEntity());
			if (pUnit != NULL)
			{
				// Get the units item
				IEntityTool *pItem(pUnit->GetTool(m_iActiveSlot));
				if (pItem != NULL)
				{
					if (!Game.CanTargetTrait(pItem->GetTargetScheme(), TARGET_TRAIT_TREE))
						uiIgnoreSurfaces2 |= SURF_TREE;
				}
			}

			if (bUpdateTrace)
				UpdateCursorTrace(Input.GetCursorPos(), uiIgnoreSurfaces2);

			IUnitEntity *pTarget(GameClient.GetUnitEntity(m_uiHoverEnt));
			if (pTarget == NULL)
			{
				if (Distance(pUnit->GetPosition().xy(), v2Pos) < pItem->GetMinRange() || !Game.IsInBounds(v2Pos.x, v2Pos.y))
					return;

				if (pItem->GetNeedVision() && !GameClient.IsVisible(v2Pos.x, v2Pos.y))
					return;

				CBufferFixed<16> buffer;
				buffer
					<< GAME_CMD_ABILITY_POSITION
					<< pUnit->GetIndex()
					<< byte(m_iActiveSlot)
					<< v2Pos
					<< (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE)
					<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
				GameClient.SendGameData(buffer, true);

				CGameEvent ev;
				ev.SetSourcePosition(Game.GetTerrainPosition(v2Pos));
				ev.SetEffect(GameClient.GetResource(CLIENT_RESOURCE_CAST_INDICATOR_EFFECT));

				if (ev.GetEffect() != INVALID_RESOURCE)
				{
					ev.Spawn();
					Game.AddEvent(ev);
				}

				SetCommanderState(COMSTATE_HOVER);
				m_iActiveSlot = -1;

				return;
			}

			if (!pItem->IsValidTarget(pTarget))
			{
				if (GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE) != INVALID_RESOURCE)
					K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE));

				ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_invalid_target")));

				return;
			}

			CBufferFixed<12> buffer;
			buffer
				<< GAME_CMD_ABILITY_ENTITY
				<< pUnit->GetIndex()
				<< byte(m_iActiveSlot)
				<< m_uiHoverEnt
				<< (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE)
				<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
			GameClient.SendGameData(buffer, true);
		}
		break;

	case TOOL_ACTION_TARGET_VECTOR:
		{
			if (Distance(pUnit->GetPosition().xy(), v2Pos) < pItem->GetMinRange() || !Game.IsInBounds(v2Pos.x, v2Pos.y))
				return;

			CBufferFixed<24> buffer;
			buffer
				<< GAME_CMD_ABILITY_VECTOR
				<< pUnit->GetIndex()
				<< byte(m_iActiveSlot)
				<< v2Pos
				<< v2Delta
				<< (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE)
				<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
			GameClient.SendGameData(buffer, true);

			CGameEvent ev;
			ev.SetSourcePosition(Game.GetTerrainPosition(v2Pos));
			ev.SetEffect(GameClient.GetResource(CLIENT_RESOURCE_CAST_INDICATOR_EFFECT));

			if (ev.GetEffect() != INVALID_RESOURCE)
			{
				ev.Spawn();
				Game.AddEvent(ev);
			}
		}
		break;
	}

	SetCommanderState(COMSTATE_HOVER);
	m_iActiveSlot = -1;
}


/*====================
  CClientCommander::DoubleActivate
  ====================*/
void	CClientCommander::DoubleActivate()
{
	// Get current selected unit
	IUnitEntity *pUnit(GetSelectedControlEntity());
	if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()))
	{
		SetCommanderState(COMSTATE_HOVER);
		m_iActiveSlot = -1;
		return;
	}

	// Get the units item
	IEntityTool *pItem(pUnit->GetTool(m_iActiveSlot));
	if (pItem == NULL)
	{
		SetCommanderState(COMSTATE_HOVER);
		m_iActiveSlot = -1;
		return;
	}

	// if the item cannot be double activated, ignore it.
	if (!pItem->GetDoubleActivate())
		return;

	if (!pItem->CanOrder())
	{
		SetCommanderState(COMSTATE_HOVER);
		m_iActiveSlot = -1;
		return;
	}

	switch (pItem->GetActionType())
	{
	case TOOL_ACTION_TARGET_POSITION:
	case TOOL_ACTION_TARGET_CURSOR:
	case TOOL_ACTION_TARGET_ENTITY:
		{
			CBufferFixed<16> buffer;
			buffer
				<< GAME_CMD_DOUBLE_ACTIVATE_ABILITY
				<< pUnit->GetIndex()
				<< byte(m_iActiveSlot)
				<< (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE)
				<< byte(GetModifier3() ? CMDR_ORDER_FLAG_DIRECT_PATHING : 0);
			GameClient.SendGameData(buffer, true);
		}
		break;
	}

	SetCommanderState(COMSTATE_HOVER);
	m_iActiveSlot = -1;
}


/*====================
  CClientCommander::Ping
  ====================*/
void	CClientCommander::Ping()
{
	if (GetCommanderState() != COMSTATE_HOVER || GameClient.GetItemCursorIndex() != INVALID_INDEX)
	{
		PrimaryDown(Input.GetCursorPos());
		return;
	}

	UpdateCursorTrace(Input.GetCursorPos(), HOVER_TRACE);

	CBufferFixed<3> buffer;
	buffer << GAME_CMD_MINIMAP_PING
		<< byte((m_v3TraceEndPos.x / GameClient.GetWorldWidth()) * UCHAR_MAX)
		<< byte((m_v3TraceEndPos.y / GameClient.GetWorldHeight()) * UCHAR_MAX);
	GameClient.SendGameData(buffer, false);
}


/*====================
  CClientCommander::GetSelectedControlEntityIndex
  ====================*/
uint	CClientCommander::GetSelectedControlEntityIndex()
{
	if (m_uiActiveControlEntity != INVALID_INDEX)
		return m_uiActiveControlEntity;
	if (m_setControlSelection.size() == 1)
		return *m_setControlSelection.begin();
	else
		return INVALID_INDEX;
}


/*====================
  CClientCommander::GetSelectedInfoEntityIndex
  ====================*/
uint	CClientCommander::GetSelectedInfoEntityIndex()
{
	if (m_setInfoSelection.size() == 1)
		return *m_setInfoSelection.begin();
	else
		return INVALID_INDEX;
}


/*====================
  CClientCommander::SelectEntity
  ====================*/
void	CClientCommander::SelectEntity(uint uiIndex)
{
	if (uiIndex == INVALID_INDEX)
		return;

	IUnitEntity *pUnit(Game.GetUnitEntity(uiIndex));
	if (pUnit == NULL)
		return;

	// Selecting a unit while in target mode uses that unit
	if (GetCommanderState() == COMSTATE_MOVE)
	{
		IUnitEntity *pTarget(GameClient.GetUnitEntity(uiIndex));
		if (pTarget != NULL)
		{
			GiveOrder(CMDR_ORDER_MOVE, uiIndex, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE);
			return;
		}
	}
	else if (GetCommanderState() == COMSTATE_PATROL)
	{
		IUnitEntity *pTarget(GameClient.GetUnitEntity(uiIndex));
		if (pTarget != NULL)
		{
			GiveOrder(CMDR_ORDER_PATROL, uiIndex, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE);
			return;
		}
		return;
	}
	else if (GetCommanderState() == COMSTATE_ATTACK)
	{
		IUnitEntity *pTarget(GameClient.GetUnitEntity(uiIndex));
		if (pTarget != NULL)
		{
			GiveOrder(CMDR_ORDER_ATTACK, uiIndex, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE);
			return;
		}
		return;
	}
	else if (GetCommanderState() == COMSTATE_TARGET_ENTITY || GetCommanderState() == COMSTATE_TARGET_DUAL || GetCommanderState() == COMSTATE_TARGET_DUAL_POSITION)
	{
		IUnitEntity *pTarget(GameClient.GetUnitEntity(uiIndex));
		
		// Get current selected unit
		IUnitEntity *pUnit(GetSelectedControlEntity());
		if (pUnit != NULL && pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()))
		{
			// Get the units item
			IEntityTool *pItem(pUnit->GetTool(m_iActiveSlot));
			if (pItem != NULL && pItem->CanOrder())
			{
				if (pTarget != NULL)
				{
					if (pItem->IsValidTarget(pTarget))
					{
						CBufferFixed<11> buffer;
						buffer << GAME_CMD_ABILITY_ENTITY << pUnit->GetIndex() << byte(m_iActiveSlot) << uiIndex << (GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE);
						GameClient.SendGameData(buffer, true);

						SetCommanderState(COMSTATE_HOVER);
						m_iActiveSlot = -1;
					}
					else
					{
						if (GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE) != INVALID_RESOURCE)
							K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_ABILITY_TARGET_ERROR_SAMPLE));

						ErrorMessage.Trigger(_T("Add ") + GameClient.GetGameMessage(_T("error_invalid_target")));
					}

					return;
				}
			}
		}
	}
	
	uiset *pSelectionSet(&m_setInfoSelection);
	if (pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) || uiIndex == m_pPlayer->GetHeroIndex())
		pSelectionSet = &m_setControlSelection;

	if (pSelectionSet == &m_setControlSelection)
		m_setInfoSelection.clear();

	// Deselect on shift+click
	if (m_bModifier1 && pSelectionSet->find(uiIndex) != pSelectionSet->end())
	{
		pSelectionSet->erase(uiIndex);
		UpdateSelection();
		ValidateSelections();
		return;
	}

	if (!m_bModifier1)
		pSelectionSet->clear();

	pSelectionSet->insert(uiIndex);


	// If SelectEntity was recently called for this same entity, select other of same type nearby
	if ((Host.GetTime() - m_uiLastSelectTime < cg_cmdrDoubleSelectTime && pSelectionSet->size() == 1 && *pSelectionSet->begin() == uiIndex))
	{
		// Center on this entity
		CVec3f v3Target(pUnit->GetPosition());
		v3Target.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
		GameClient.GetCurrentSnapshot()->SetCameraPosition(v3Target);
		m_uiLastSelectTime = 0;
		return;
	}

	m_uiLastSelectTime = Host.GetTime();
	PlaySelectSound(uiIndex);
	UpdateSelection();
	ValidateSelections();
}


/*====================
  CClientCommander::SaveSelectionSet
  ====================*/
void	CClientCommander::SaveSelectionSet(uint uiSetIndex)
{
	if (uiSetIndex >= MAX_SAVED_SELECTION_SETS)
		return;

	uiset *pSelectionSet(&m_setControlSelection);
	if (m_bModifier3)
		pSelectionSet = &m_setInfoSelection;

	m_aSavedSelection[uiSetIndex].setEntries.clear();
	
	for (uiset::iterator it(pSelectionSet->begin()), itEnd(pSelectionSet->end()); it != itEnd; ++it)
	{
		uint uiIndex(*it);

		IUnitEntity* pUnit(GameClient.GetUnitEntity(uiIndex));
		assert(pUnit != NULL);
		if (pUnit != NULL)
		{
			uint uiType(pUnit->GetType());
			m_aSavedSelection[uiSetIndex].setEntries.insert(SSelectionEntry(uiIndex, uiType));
		}
	}
	m_aSavedSelection[uiSetIndex].uiFocus = m_uiActiveControlEntity;
}


/*====================
  CClientCommander::AddToSelectionSet
  ====================*/
void	CClientCommander::AddToSelectionSet(uint uiSetIndex)
{
	if (uiSetIndex >= MAX_SAVED_SELECTION_SETS)
		return;

	uiset *pSelectionSet(&m_setControlSelection);
	if (m_bModifier3)
		pSelectionSet = &m_setInfoSelection;

	for (uiset::iterator it(pSelectionSet->begin()), itEnd(pSelectionSet->end()); it != itEnd; ++it)
	{
		uint uiIndex(*it);

		IUnitEntity* pUnit(GameClient.GetUnitEntity(uiIndex));
		assert(pUnit != NULL);
		if (pUnit != NULL)
		{
			uint uiType(pUnit->GetType());
			m_aSavedSelection[uiSetIndex].setEntries.insert(SSelectionEntry(uiIndex, uiType));
		}
	}
}


/*====================
  CClientCommander::RemoveFromSelectionSet
  ====================*/
void	CClientCommander::RemoveFromSelectionSet(uint uiSetIndex)
{
	if (uiSetIndex >= MAX_SAVED_SELECTION_SETS)
		return;

	uiset *pSelectionSet(&m_setControlSelection);
	if (m_bModifier3)
		pSelectionSet = &m_setInfoSelection;

	for (uiset_it it(pSelectionSet->begin()), itEnd(pSelectionSet->end()); it != itEnd; ++it)
	{
		uint uiEntityIndex(*it);

		m_aSavedSelection[uiSetIndex].setEntries.erase(SSelectionEntry(uiEntityIndex, -1));

		if (m_aSavedSelection[uiSetIndex].uiFocus == uiEntityIndex && !m_aSavedSelection[uiSetIndex].setEntries.empty())
			m_aSavedSelection[uiSetIndex].uiFocus = (*m_aSavedSelection[uiSetIndex].setEntries.begin()).index;
	}
}


/*====================
  CClientCommander::RecallSelectionSet
  ====================*/
void	CClientCommander::RecallSelectionSet(uint uiSetIndex)
{
	if (uiSetIndex >= MAX_SAVED_SELECTION_SETS)
		return;
	if (m_aSavedSelection[uiSetIndex].setEntries.empty())
		return;

	uiset *pSelectionSet(&m_setInfoSelection);
	for (SSelectionSet::Set::iterator it(m_aSavedSelection[uiSetIndex].setEntries.begin()); it != m_aSavedSelection[uiSetIndex].setEntries.end(); ++it)
	{
		const SSelectionEntry& entry(*it);
		IUnitEntity *pUnit(Game.GetUnitEntity(entry.index));
		if (pUnit == NULL)
			continue;
		
		if (pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) || entry.index == m_pPlayer->GetHeroIndex())
		{
			pSelectionSet = &m_setControlSelection;
			break;
		}
	}

	pSelectionSet->clear();

	if (pSelectionSet == &m_setControlSelection)
		m_setInfoSelection.clear();

	for (SSelectionSet::Set::iterator it(m_aSavedSelection[uiSetIndex].setEntries.begin()), itEnd(m_aSavedSelection[uiSetIndex].setEntries.end()); it != itEnd; ++it)
	{
		const SSelectionEntry& entry(*it);
		uint uiIndex(entry.index);
		pSelectionSet->insert(uiIndex);
	}

	m_uiActiveControlEntity = m_aSavedSelection[uiSetIndex].uiFocus;

	PlaySelectSound(m_uiActiveControlEntity);

	if (m_uiLastSelectionSetRecallIndex == uiSetIndex && 
		m_uiLastSelectionSetRecallTime != INVALID_TIME &&
		Host.GetTime() - m_uiLastSelectionSetRecallTime <= cg_cmdrDoubleSelectTime)
	{
		if (pSelectionSet == &m_setControlSelection)
		{
			IUnitEntity *pUnit(Game.GetUnitEntity(m_uiActiveControlEntity));
			if (pUnit != NULL)
			{
				CVec3f v3CenterPos(pUnit->GetPosition());

				v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
				GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
			}
		}

		return;
	}

	m_uiLastSelectionSetRecallIndex = uiSetIndex;
	m_uiLastSelectionSetRecallTime = Host.GetTime();

	UpdateSelection();
}


/*====================
  CClientCommander::ReplaceUnitInSelectionSets
  ====================*/
uint	CClientCommander::ReplaceUnitInSelectionSets(uint uiUnitIndex, uint uiUnitType)
{
	// sanity check parameters.
	IUnitEntity *pReplacingUnit(GameClient.GetUnitEntity(uiUnitIndex));
	assert(pReplacingUnit != NULL);
	assert(pReplacingUnit->GetType() == uiUnitType);
	if (pReplacingUnit == NULL || pReplacingUnit->GetType() != uiUnitType)
		return false;

	// index of the unit we're currently replacing.
	uint uiBeingReplacedIndex(INVALID_INDEX);

	// try to find a suitable (invalid/dead) pet of the same type.
	for (uint uiSetIndex = 0; uiSetIndex < MAX_SAVED_SELECTION_SETS; ++uiSetIndex)
	{
		if (m_aSavedSelection[uiSetIndex].setEntries.empty())
			continue;

		for (SSelectionSet::Set::iterator it(m_aSavedSelection[uiSetIndex].setEntries.begin()); it != m_aSavedSelection[uiSetIndex].setEntries.end(); ++it)
		{
			const SSelectionEntry& entry(*it);

			// if the entry is of the same type, then it is potentially replaceable.
			if (entry.type == uiUnitType)
			{
				// if the unit is invalid or dead, then replace it.
				IUnitEntity *pUnit(Game.GetUnitEntity(entry.index));
				if (pUnit == NULL || !pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()))
				{
					// if we're in the procses of replacing a unit, and this is not it,
					// then skip it.
					if (uiBeingReplacedIndex != INVALID_INDEX)
						if (entry.index != uiBeingReplacedIndex)
							continue;

					// do the actual replacing.
					uiBeingReplacedIndex = entry.index;

					if ( m_aSavedSelection[uiSetIndex].uiFocus == entry.index )
						m_aSavedSelection[uiSetIndex].uiFocus = uiUnitIndex;

					m_aSavedSelection[uiSetIndex].setEntries.erase(it);
					m_aSavedSelection[uiSetIndex].setEntries.insert(SSelectionEntry(uiUnitIndex, uiUnitType));
					break;
				}
			}
		}
	}

	return uiBeingReplacedIndex;
}


/*====================
  CClientCommander::UpdateSelectedPlayerWaypoints
  ====================*/
void	CClientCommander::UpdateSelectedPlayerWaypoints()
{
	for (int i(0); i < MAX_PLAYER_WAYPOINTS; ++i)
		m_aSelectedPlayerWaypoints[i].bActive = false;

	for (uiset::iterator it(m_setControlSelection.begin()); it != m_setControlSelection.end(); ++it)
	{
		IUnitEntity *pUnit(GameClient.GetUnitEntity(*it));
		if (pUnit == NULL)
			continue;
		if (pUnit->GetTeam() != m_pPlayer->GetTeam())
			continue;

#if 0
		if (pUnit->GetCurrentOrder() == CMDR_ORDER_CLEAR)
			continue;

		int i;
		for (i = 0; i < MAX_PLAYER_WAYPOINTS; ++i)
		{
			if (m_aSelectedPlayerWaypoints[i].uiEvent != INVALID_INDEX &&
				m_aSelectedPlayerWaypoints[i].uiOrderEntIndex != INVALID_INDEX &&
				m_aSelectedPlayerWaypoints[i].uiOrderEntIndex == pPlayer->GetCurrentOrderEntIndex())
			{
				m_aSelectedPlayerWaypoints[i].bActive = true;
				break;
			}
			else if (m_aSelectedPlayerWaypoints[i].uiEvent != INVALID_INDEX &&
				m_aSelectedPlayerWaypoints[i].uiOrderEntIndex == INVALID_INDEX &&
				m_aSelectedPlayerWaypoints[i].v3OrderPos == pPlayer->GetCurrentOrderPos())
			{
				m_aSelectedPlayerWaypoints[i].bActive = true;
				break;
			}
		}

		if (i == MAX_PLAYER_WAYPOINTS)
		{
			for (i = 0; i < MAX_PLAYER_WAYPOINTS; ++i)
			{
				if (m_aSelectedPlayerWaypoints[i].uiEvent == INVALID_INDEX)
				{
					m_aSelectedPlayerWaypoints[i].bActive = true;
					m_aSelectedPlayerWaypoints[i].uiOrderEntIndex = pPlayer->GetCurrentOrderEntIndex();
					m_aSelectedPlayerWaypoints[i].v3OrderPos = pPlayer->GetCurrentOrderPos();

					switch (pPlayer->GetCurrentOrder())
					{
					case CMDR_ORDER_MOVE:
						{				
							CGameEvent ev;
							ev.SetSourcePosition(pPlayer->GetCurrentOrderPos());
							ev.SetSourceEntity(pPlayer->GetCurrentOrderEntIndex());
							ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/waypoint.effect"), RES_EFFECT));
							ev.Spawn();
							
							m_aSelectedPlayerWaypoints[i].uiEvent = Game.AddEvent(ev);
						}
						break;
					case CMDR_ORDER_ATTACK:
						{
							CGameEvent ev;
							ev.SetSourcePosition(pPlayer->GetCurrentOrderPos());
							ev.SetSourceEntity(pPlayer->GetCurrentOrderEntIndex());
							ev.SetEffect(g_ResourceManager.Register(_T("/shared/effects/attack_waypoint.effect"), RES_EFFECT));
							ev.Spawn();
							
							m_aSelectedPlayerWaypoints[i].uiEvent = Game.AddEvent(ev);
						}
						break;
					default:
						m_aSelectedPlayerWaypoints[i].uiEvent = INVALID_INDEX;
						break;
					}
					break;
				}
			}
		}
#endif
	}

	for (int i(0); i < MAX_PLAYER_WAYPOINTS; ++i)
	{
		if (!m_aSelectedPlayerWaypoints[i].bActive && m_aSelectedPlayerWaypoints[i].uiEvent != INVALID_INDEX)
		{
			Game.DeleteEvent(m_aSelectedPlayerWaypoints[i].uiEvent);
			m_aSelectedPlayerWaypoints[i].uiEvent = INVALID_INDEX;
		}
	}
}


/*====================
  CClientCommander::GetCameraDistance
  ====================*/
float	CClientCommander::GetCameraDistance() const
{
	if (m_pPlayer)
		return m_pPlayer->GetPosition().z;
	else
		return 0.0f;
}


/*====================
  CClientCommander::NextUnit
  ====================*/
void	CClientCommander::NextUnit(bool bCentered)
{
	if (m_pPlayer == NULL)
		return;

	if (m_pPlayer->GetTeam() == TEAM_SPECTATOR)
	{
		if (m_setInfoSelection.size() < 2)
		{
			IUnitEntity *pInfo(GetSelectedInfoEntity());

			IGameEntity *pNext(Game.GetNextEntity(pInfo));

			if (pNext == NULL)
				pNext = Game.GetFirstEntity();

			while (pNext != pInfo)
			{
				if (pNext == NULL)
					pNext = Game.GetFirstEntity();

				if (pNext == NULL)
					break;

				if (pNext->IsHero() && !pNext->GetAsUnit()->GetNoGlobalSelect())
					break;

				pNext = Game.GetNextEntity(pNext);
			}

			if (pNext != NULL && pNext != pInfo)
			{
				m_setInfoSelection.clear();
				m_setInfoSelection.insert(pNext->GetIndex());

				UpdateSelection();
			}
		}

		if (bCentered)
		{
			IUnitEntity *pUnit(GetSelectedInfoEntity());
			if (pUnit != NULL)
			{
				CVec3f v3CenterPos(pUnit->GetPosition());

				v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
				GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
			}
		}

		return;
	}

	if (m_setControlSelection.size() < 2)
	{
		IUnitEntity *pControl(GetSelectedControlEntity());

		IGameEntity *pNext(Game.GetNextEntity(pControl));
		while (pNext != pControl)
		{
			if (pNext == NULL)
				pNext = Game.GetFirstEntity();

			if (pNext == NULL)
				break;

			if (pNext->IsUnit() &&
				pNext->GetAsUnit()->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) &&
				!pNext->GetAsUnit()->GetNoGlobalSelect() &&
				pNext->GetAsUnit()->GetOwnerClientNumber() == GameClient.GetLocalClientNum())
				break;

			pNext = Game.GetNextEntity(pNext);
		}

		if (pNext != NULL && pNext != pControl)
		{
			m_uiActiveControlEntity = pNext->GetIndex();
			m_setControlSelection.clear();
			m_setControlSelection.insert(pNext->GetIndex());

			UpdateSelection();
		}
	}
	else
	{
		uiset_it it(m_setControlSelection.begin());
		for (; it != m_setControlSelection.end(); ++it)
		{
			if (*it == m_uiActiveControlEntity)
				break;
		}
		if (it == m_setControlSelection.end())
			return;

		++it;
		if (it == m_setControlSelection.end())
			m_uiActiveControlEntity = *m_setControlSelection.begin();
		else
			m_uiActiveControlEntity = *it;

	}

	if (bCentered)
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(m_uiActiveControlEntity));
		if (pUnit != NULL)
		{
			CVec3f v3CenterPos(pUnit->GetPosition());

			v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
			GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
		}
	}
}


/*====================
  CClientCommander::PrevUnit
  ====================*/
void	CClientCommander::PrevUnit(bool bCentered)
{
	if (m_pPlayer == NULL)
		return;

	if (m_pPlayer->GetTeam() == TEAM_SPECTATOR)
	{
		if (GetSelectedInfoEntity() == NULL)
		{
			NextUnit(bCentered);
			return;
		}
		else if (m_setInfoSelection.size() < 2)
		{
			IUnitEntity *pInfo(GetSelectedInfoEntity());

			IGameEntity *pNext(Game.GetNextEntity(pInfo));
			IGameEntity *pPrev(pInfo);
			while (pNext != pInfo)
			{
				if (pNext == NULL)
					pNext = Game.GetFirstEntity();

				if (pNext == NULL)
					break;

				if (pNext->IsHero() && !pNext->GetAsUnit()->GetNoGlobalSelect())
					pPrev = pNext;

				pNext = Game.GetNextEntity(pNext);
			}

			if (pPrev != NULL && pPrev != pInfo)
			{
				m_setInfoSelection.clear();
				m_setInfoSelection.insert(pPrev->GetIndex());

				UpdateSelection();
			}
		}

		if (bCentered)
		{
			IUnitEntity *pUnit(GetSelectedInfoEntity());
			if (pUnit != NULL)
			{
				CVec3f v3CenterPos(pUnit->GetPosition());

				v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
				GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
			}
		}

		return;
	}

	if (m_setControlSelection.size() < 2)
	{
		IUnitEntity *pControl(GetSelectedControlEntity());

		IGameEntity *pNext(Game.GetNextEntity(pControl));
		IGameEntity *pPrev(pControl);
		while (pNext != pControl)
		{
			if (pNext == NULL)
				pNext = Game.GetFirstEntity();

			if (pNext == NULL)
				break;

			if (pNext->IsUnit() &&
				pNext->GetAsUnit()->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) &&
				!pNext->GetAsUnit()->GetNoGlobalSelect() &&
				pNext->GetAsUnit()->GetOwnerClientNumber() == GameClient.GetLocalClientNum())
				pPrev = pNext;

			pNext = Game.GetNextEntity(pNext);
		}

		if (pPrev != NULL && pPrev != pControl)
		{
			m_uiActiveControlEntity = pPrev->GetIndex();
			m_setControlSelection.clear();
			m_setControlSelection.insert(pPrev->GetIndex());

			UpdateSelection();
		}
	}
	else
	{
		uiset_it it(m_setControlSelection.begin());
		for (; it != m_setControlSelection.end(); ++it)
		{
			if (*it == m_uiActiveControlEntity)
				break;
		}
		if (it == m_setControlSelection.end())
			return;

		if (it == m_setControlSelection.begin())
			it = m_setControlSelection.end();

		--it;
		m_uiActiveControlEntity = *it;
	}

	if (bCentered)
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(m_uiActiveControlEntity));
		if (pUnit != NULL)
		{
			CVec3f v3CenterPos(pUnit->GetPosition());

			v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
			GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
		}
	}
}


/*====================
  CClientCommander::NextInventoryUnit
  ====================*/
void	CClientCommander::NextInventoryUnit(bool bCentered)
{
	IUnitEntity *pControl(GetSelectedControlEntity());

	IGameEntity *pNext(Game.GetNextEntity(pControl));
	while (pNext != pControl)
	{
		if (pNext == NULL)
			pNext = Game.GetFirstEntity();

		if (pNext == NULL)
			break;

		if (pNext->IsUnit() &&
			pNext->GetAsUnit()->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) &&
			pNext->GetAsUnit()->GetCanCarryItems())
			break;

		pNext = Game.GetNextEntity(pNext);
	}

	if (pNext != NULL && pNext != pControl)
	{
		m_uiActiveControlEntity = pNext->GetIndex();
		m_setControlSelection.clear();
		m_setControlSelection.insert(pNext->GetIndex());

		UpdateSelection();
	}

	if (bCentered)
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(m_uiActiveControlEntity));
		if (pUnit != NULL)
		{
			CVec3f v3CenterPos(pUnit->GetPosition());

			v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
			GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
		}
	}
}


/*====================
  CClientCommander::PrevInventoryUnit
  ====================*/
void	CClientCommander::PrevInventoryUnit(bool bCentered)
{
	IUnitEntity *pControl(GetSelectedControlEntity());

	IGameEntity *pNext(Game.GetNextEntity(pControl));
	IGameEntity *pPrev(pControl);
	while (pNext != pControl)
	{
		if (pNext == NULL)
			pNext = Game.GetFirstEntity();

		if (pNext == NULL)
			break;

		if (pNext->IsUnit() &&
			pNext->GetAsUnit()->CanActOnOrdersFrom(GameClient.GetLocalClientNum()) &&
			pNext->GetAsUnit()->GetCanCarryItems())
			pPrev = pNext;

		pNext = Game.GetNextEntity(pNext);
	}

	if (pPrev != NULL && pPrev != pControl)
	{
		m_uiActiveControlEntity = pPrev->GetIndex();
		m_setControlSelection.clear();
		m_setControlSelection.insert(pPrev->GetIndex());

		UpdateSelection();
	}

	if (bCentered)
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(m_uiActiveControlEntity));
		if (pUnit != NULL)
		{
			CVec3f v3CenterPos(pUnit->GetPosition());

			v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
			GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
		}
	}
}


/*====================
  CClientCommander::SetControlUnit
  ====================*/
void	CClientCommander::SetControlUnit(int iSlot, bool bCentered)
{
	if (!m_setInfoSelection.empty())
	{
		if (iSlot < 0 || iSlot >= int(m_setInfoSelection.size()))
			return;

		uiset_it it(m_setInfoSelection.begin());
		for (; it != m_setInfoSelection.end() && iSlot > 0; ++it, --iSlot) {}

		if (it == m_setInfoSelection.end())
			return;

		if (m_uiLastSetControlEntity == *it &&
			Host.GetTime() - m_uiLastSetControlTime < cg_cmdrDoubleSelectTime)
		{
			SelectEntity(*it);
			return;
		}

		m_uiLastSetControlTime = Host.GetTime();
		m_uiLastSetControlEntity = *it;
	}
	else
	{
		if (m_setControlSelection.empty())
			return;

		if (iSlot < 0 || iSlot >= int(m_setControlSelection.size()))
			return;

		uiset_it it(m_setControlSelection.begin());
		for (; it != m_setControlSelection.end() && iSlot > 0; ++it, --iSlot) {}

		if (it == m_setControlSelection.end())
			return;

		m_uiActiveControlEntity = *it;

		if (m_uiLastSetControlEntity == m_uiActiveControlEntity &&
			Host.GetTime() - m_uiLastSetControlTime < cg_cmdrDoubleSelectTime)
		{
			SelectEntity(m_uiActiveControlEntity);
			return;
		}

		m_uiLastSetControlTime = Host.GetTime();
		m_uiLastSetControlEntity = m_uiActiveControlEntity;

		if (bCentered)
		{
			IUnitEntity *pUnit(Game.GetUnitEntity(m_uiActiveControlEntity));
			if (pUnit != NULL)
			{
				CVec3f v3CenterPos(pUnit->GetPosition());

				v3CenterPos.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
				GameClient.GetCurrentSnapshot()->SetCameraPosition(v3CenterPos);
			}
		}
	}
}


/*====================
  CClientCommander::DeselectUnit
  ====================*/
void	CClientCommander::DeselectUnit(int iSlot)
{
	if (!m_setInfoSelection.empty())
	{
		if (iSlot < 0 || iSlot >= int(m_setInfoSelection.size()))
			return;

		uiset_it it(m_setInfoSelection.begin());
		for (; it != m_setInfoSelection.end() && iSlot > 0; ++it, --iSlot) {}

		if (it == m_setInfoSelection.end())
			return;

		m_setInfoSelection.erase(it);
	}
	else
	{
		if (m_setControlSelection.empty())
			return;

		if (iSlot < 0 || iSlot >= int(m_setControlSelection.size()))
			return;

		uiset_it it(m_setControlSelection.begin());
		for (; it != m_setControlSelection.end() && iSlot > 0; ++it, --iSlot) {}

		if (it == m_setControlSelection.end())
			return;

		m_setControlSelection.erase(it);

		UpdateSelection();
	}
}


/*====================
  CClientCommander::PlaySelectSound
  ====================*/
void	CClientCommander::PlaySelectSound(uint uiIndex)
{
	CClientEntity *pClEntity(GameClient.GetClientEntity(uiIndex));
	if (pClEntity == NULL)
		return;

	IUnitEntity *pUnit(GameClient.GetUnitEntity(uiIndex));
	if (pUnit == NULL)
		return;

	if (!cg_unitVoiceResponses)
		return;

	if (pUnit->CanActOnOrdersFrom(Game.GetLocalClientNum()))
	{
		if (pUnit->GetIndex() != m_uiLastVoiceEntity)
		{
			m_uiLastVoiceEntity = pUnit->GetIndex();
			m_uiVoiceSequence = 0;
		}

		ResHandle hUnitSelectedSound(INVALID_RESOURCE);

		if (m_uiVoiceSequence >= 3)
		{
			ResHandle hUnitFlavorSound(pUnit->GetSelectedFlavorSound());

			uint uiSequence(m_uiVoiceSequence - 3);

			CSample *pSample(g_ResourceManager.GetSample(hUnitFlavorSound));
			if (pSample != NULL && uiSequence < pSample->GetNumSamples())
			{
				hUnitSelectedSound = pSample->GetSample(uiSequence);
			}
			else
			{
				hUnitSelectedSound = pUnit->GetSelectedSound();
				m_uiVoiceSequence = 0;
			}
		}	
		else
			hUnitSelectedSound = pUnit->GetSelectedSound();

		if (hUnitSelectedSound != INVALID_RESOURCE)
		{
			if (Game.GetGameTime() - GameClient.GetLastSelectedSoundTime() > cg_unitVoiceResponsesDelay)
			{
				if (pClEntity->PlaySound(hUnitSelectedSound, 1.0f, 0.0f, 7, 128, SND_LINEARFALLOFF, 0, 0, 0, false, 0, 1.0f, 1.0f, 0, 17000.0f) != -1)
					++m_uiVoiceSequence;
					
				GameClient.SetLastSelectedSoundTime(Game.GetGameTime());
			}
		}
		else
		{
			if (K2SoundManager.IsChannelAvailable(CHANNEL_CMDR_UNIT_SELECT))
				K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_SELECTED_SAMPLE), 1.0f, CHANNEL_CMDR_UNIT_SELECT);
		}
	}
	else
	{
		m_uiLastVoiceEntity = INVALID_INDEX;

		if (K2SoundManager.IsChannelAvailable(CHANNEL_CMDR_UNIT_SELECT))
			K2SoundManager.Play2DSound(GameClient.GetResource(CLIENT_RESOURCE_SELECTED_SAMPLE), 1.0f, CHANNEL_CMDR_UNIT_SELECT);
	}
}


/*====================
  CClientCommander::PrepareClientSnapshot
  ====================*/
void	CClientCommander::PrepareClientSnapshot(CClientSnapshot &snapshot)
{
	CPlayer *pPlayer(GameClient.GetLocalPlayer());
	if (pPlayer != NULL)
	{
		IHeroEntity *pHero(pPlayer->GetHero());
		if (pHero != NULL)
		{
			uint uiHeroIndex(pHero->GetIndex());
			if (pHero->GetStatus() != ENTITY_STATUS_ACTIVE)
				uiHeroIndex = INVALID_INDEX;

			if (m_uiCameraWatchIndex != uiHeroIndex)
			{						
				if (uiHeroIndex != INVALID_INDEX && cg_cameraCenterOnRespawn)
				{
					SelectEntity(uiHeroIndex);

					// Center on this entity
					CVec3f v3Target(pHero->GetPosition());
					v3Target.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;
					GameClient.GetCurrentSnapshot()->SetCameraPosition(v3Target);

					PlaySelectSound(uiHeroIndex);
				}

				m_uiCameraWatchIndex = uiHeroIndex;
			}
		}
	}
}


/*====================
  CClientCommander::MinimapPrimaryClick
  ====================*/
bool	CClientCommander::MinimapPrimaryClick(const CVec2f &v2Pos)
{
	m_uiHoverEnt = INVALID_INDEX;

	if (GameClient.GetItemCursorIndex() != INVALID_INDEX)
	{
		StartCommand(v2Pos, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, CMDR_ORDER_GIVEITEM, GameClient.GetItemCursorIndex());
		GameClient.SetItemCursorIndex(INVALID_INDEX);
		return true;
	}

	switch (GetCommanderState())
	{
	case COMSTATE_HOVER:
		{
			CVec3f v3NewPosition;
			v3NewPosition.x = v2Pos.x;
			v3NewPosition.y = v2Pos.y;
			v3NewPosition.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;

			GameClient.GetCurrentSnapshot()->SetCameraPosition(v3NewPosition);
		}
		return false;

	case COMSTATE_SELECT:
		return true;

	case COMSTATE_MOVE:
		GiveOrder(CMDR_ORDER_MOVE, v2Pos, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, 0);
		SetCommanderState(COMSTATE_HOVER);
		return true;

	case COMSTATE_ATTACK:
		GiveOrder(CMDR_ORDER_ATTACK, v2Pos, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, 0);
		SetCommanderState(COMSTATE_HOVER);
		return true;

	case COMSTATE_PATROL:
		GiveOrder(CMDR_ORDER_PATROL, v2Pos, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, 0);
		SetCommanderState(COMSTATE_HOVER);
		return true;

	case COMSTATE_TARGET_ENTITY:
	case COMSTATE_TARGET_AREA:
	case COMSTATE_TARGET_DUAL:
	case COMSTATE_TARGET_DUAL_POSITION:
		ProximitySearch(Game.GetTerrainPosition(v2Pos));
		Cast(v2Pos, V2_ZERO, false);
		return true;

	case COMSTATE_TARGET_VECTOR:
		return true;
	}

	return false;
}


/*====================
  CClientCommander::MinimapSecondaryClick
  ====================*/
bool	CClientCommander::MinimapSecondaryClick(const CVec2f &v2Pos)
{
	IUnitEntity *pControl(GetSelectedControlEntity());

	// Ignore a single controlled unit when right-clicking
	UpdateCursorTrace(Input.GetCursorPos(), HOVER_TRACE, pControl != NULL ? pControl->GetWorldIndex() : INVALID_INDEX);

	if (GameClient.GetItemCursorIndex() != INVALID_INDEX)
	{
		GameClient.SetItemCursorIndex(INVALID_INDEX);
		return true;
	}

	if (!cg_enableMinimapRightclick)
	{
		return false;
	}

	switch (GetCommanderState())
	{
	case COMSTATE_HOVER:
		GiveOrder(CMDR_ORDER_MOVE, v2Pos, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, 0);
		SetCommanderState(COMSTATE_HOVER);
		return true;

	case COMSTATE_SELECT:
		Cancel(Input.GetCursorPos());
		GiveOrder(CMDR_ORDER_MOVE, v2Pos, GetModifier1() ? GetFrontQueueModifier() ? QUEUE_FRONT : QUEUE_BACK : QUEUE_NONE, 0);
		SetCommanderState(COMSTATE_HOVER);
		return true;

	case COMSTATE_MOVE:
	case COMSTATE_ATTACK:
	case COMSTATE_PATROL:
	case COMSTATE_TARGET_ENTITY:
	case COMSTATE_TARGET_AREA:
	case COMSTATE_TARGET_DUAL:
	case COMSTATE_TARGET_DUAL_POSITION:
	case COMSTATE_TARGET_VECTOR:
		{
			CVec3f v3NewPosition;
			v3NewPosition.x = v2Pos.x;
			v3NewPosition.y = v2Pos.y;
			v3NewPosition.z = GameClient.GetCurrentSnapshot()->GetCameraPosition().z;

			GameClient.GetCurrentSnapshot()->SetCameraPosition(v3NewPosition);
		}
		return false;
	}

	return false;
}


/*====================
  CClientCommander::StartClickCmdPos
  ====================*/
void	CClientCommander::StartClickCmdPos(const tstring &sCmd)
{
	SetCommanderState(COMSTATE_TARGET_AREA);
	m_sClickCmd = sCmd;
}


/*====================
  CClientCommander::PrintErrorMessage
  ====================*/
void	CClientCommander::PrintErrorMessage(const tstring &sMessage)
{
	ErrorMessage.Trigger(_T("Add ") + sMessage);
}


/*====================
  CClientCommander::SetDefaultActiveShop
  ====================*/
void	CClientCommander::SetDefaultActiveShop()
{
	IUnitEntity *pControl(GetSelectedControlEntity());
	if (pControl != NULL)
	{
		const tstring &sActiveShop(GameClient.GetActiveShop());
		if (sActiveShop.empty() && GameClient.GetInterfaceManager()->IsShopVisible())
		{
			tsvector vsShopAccess(TokenizeString(pControl->GetShopAccess(), _T(' ')));
			if (vsShopAccess.size() == 1)
				GameClient.SetActiveShop(vsShopAccess.front());
		}
	}
}


/*====================
  CClientCommander::IsTargetingEntity
  ====================*/
bool	CClientCommander::IsTargetingEntity() const
{
	return (m_eState == COMSTATE_TARGET_ENTITY || m_eState == COMSTATE_TARGET_DUAL || m_eState == COMSTATE_TARGET_DUAL_POSITION);
}


/*====================
  CClientCommander::GetActiveUnits
  ====================*/
bool	CClientCommander::GetActiveUnits() const
{
	for (uiset_cit it(m_setControlSelection.begin()); it != m_setControlSelection.end(); ++it)
	{
		IUnitEntity *pUnit(Game.GetUnitEntity(*it));
		if (pUnit == NULL)
			continue;
		if (pUnit->CanActOnOrdersFrom(GameClient.GetLocalClientNum()))
			return true;
	}

	return false;
}


/*====================
  CClientCommander::ForceResendGameplayOptions
  ====================*/
void	CClientCommander::ForceResendGameplayOptions()
{
	cg_moveHeroToSpawnOnDisconnect.SetModified(true);
}


/*====================
  CClientCommander::TransmitGameplayOptions
  ====================*/
void	CClientCommander::TransmitGameplayOptions()
{
	if (Host.IsReplay())
		return;

	if (cg_moveHeroToSpawnOnDisconnect.IsModified())
	{
		CBufferDynamic buffer;
		buffer << GAME_CMD_GAMEPLAY_OPTION << TStringToUTF8(_T("move_hero_on_disconnect")) << byte(0)
			<< TStringToUTF8(XtoA(cg_moveHeroToSpawnOnDisconnect.GetBool())) << byte(0);
		GameClient.SendGameData(buffer, true);
		cg_moveHeroToSpawnOnDisconnect.SetModified(false);
	}
}


/*====================
  CClientCommander::OnPetAdded
  ====================*/
void	CClientCommander::OnPetAdded(uint uiPetIndex)
{
	assert(uiPetIndex != INVALID_INDEX);
	if (uiPetIndex == INVALID_INDEX)
		return;

	// delay processing the pet until the client has created it.
	m_vPendingPets.push_back(uiPetIndex);
}


/*====================
  CClientCommander::UpdateSprint
  ====================*/
void	CClientCommander::UpdateSprint(bool bValue)
{
	if (cg_sprintToggle)
	{
		if (bValue)
		{
			if (!m_bSprinting)
			{
				m_bSprinting = true;

				CBufferFixed<1> buffer;
				buffer << GAME_CMD_START_SPRINT;
				GameClient.SendGameData(buffer, true);
			}
			else if (m_bSprinting)
			{
				m_bSprinting = false;

				CBufferFixed<1> buffer;
				buffer << GAME_CMD_STOP_SPRINT;
				GameClient.SendGameData(buffer, true);
			}
		}
	}
	else
	{
		if (!m_bSprinting && bValue)
		{
			m_bSprinting = true;

			CBufferFixed<1> buffer;
			buffer << GAME_CMD_START_SPRINT;
			GameClient.SendGameData(buffer, true);
		}
		else if (m_bSprinting && !bValue)
		{
			m_bSprinting = false;

			CBufferFixed<1> buffer;
			buffer << GAME_CMD_STOP_SPRINT;
			GameClient.SendGameData(buffer, true);
		}
	}
}
