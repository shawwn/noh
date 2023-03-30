// (C)2006 S2 Games
// i_game.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "../k2/c_resourcemanager.h"

#include "i_game.h"

#include "c_teaminfo.h"
#include "i_unitentity.h"
#include "i_entityitem.h"
#include "c_replaymanager.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
IGame*	IGame::s_pGame(NULL);

CVAR_INTF(	g_dayStartTime,			MinToMs(3.75f),	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_INTF(	g_dayLength,			MinToMs(15u),	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_INTF(	g_dayTransitionTime,	SecToMs(6u),	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_UINTF(	g_voteRemakeTimeLimit,	MinToMs(5u),	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_UINTF(	g_voteAllowConcedeTime,	MinToMs(15u),	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_UINTF(	g_voteCooldownTime,		SecToMs(60u),	CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  IGame::SetTeam
  ====================*/
void	IGame::SetTeam(uint uiTeamID, CTeamInfo *pTeam)
{
	if (m_mapTeams.find(uiTeamID) != m_mapTeams.end())
	{
		Console.Warn << _T("IGame::SetTeam() - Team ID conflict: ") << uiTeamID << newl;
		return;
	}

	m_mapTeams[uiTeamID] = pTeam;
}


/*====================
  IGame::TraceLine
  ====================*/
bool	IGame::TraceLine(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, int iIgnoreSurface, uint uiIgnoreEntity)
{
	bool bHit(m_pWorld->TraceLine(result, v3Start, v3End, iIgnoreSurface, uiIgnoreEntity));
	return bHit;
}


/*====================
  IGame::TraceBox
  ====================*/
bool	IGame::TraceBox(STraceInfo &result, const CVec3f &v3Start, const CVec3f &v3End, const CBBoxf &bbBounds, int iIgnoreSurface, uint uiIgnoreEntity)
{
	bool bHit(m_pWorld->TraceBox(result, v3Start, v3End, bbBounds, iIgnoreSurface, uiIgnoreEntity));
	return bHit;
}


/*====================
  IGame::IsValidTarget
  ====================*/
bool	IGame::IsValidTarget(uint uiTargetScheme, uint uiEffectType, const IUnitEntity *pInitiator, const IUnitEntity *pTarget, bool bIgnoreInvulnerable)
{
	PROFILE("IGame::IsValidTarget");

	if (pTarget == NULL)
		return false;
	if (!bIgnoreInvulnerable && (pTarget->HasUnitFlags(UNIT_FLAG_INVULNERABLE) || pTarget->GetInvulnerable()))
		return false;
	if (IsImmune(uiEffectType, pTarget->GetAdjustedImmunityType()))
		return false;
	if (pTarget->GetStatus() == ENTITY_STATUS_DORMANT)
		return false;

	const CTargetScheme *pTargetScheme(GetTargetScheme(uiTargetScheme));
	if (pTargetScheme == NULL)
		return false;

	bool bValid(true);

	// Must match one type in the allowed list
	const CTargetScheme::TestRecordVector &vAllow(pTargetScheme->GetAllowList());
	CTargetScheme::TestRecordVector_cit itAllow(vAllow.begin());
	for (; itAllow != vAllow.end(); ++itAllow)
	{
		if (pTarget->IsTargetType(*itAllow, pInitiator))
			break;
	}
	if (itAllow == vAllow.end())
		bValid = false;

	if (bValid)
	{
		// Must match no types in the restricted list
		const CTargetScheme::TestRecordVector &vRestrict(pTargetScheme->GetRestrictList());
		CTargetScheme::TestRecordVector_cit itRestrict(vRestrict.begin());
		for (; itRestrict != vRestrict.end(); ++itRestrict)
		{
			if (pTarget->IsTargetType(*itRestrict, pInitiator))
			{
				bValid = false;
				break;
			}
		}
	}

	if (bValid)
		return true;

	// OR must match one type in the allowed "2" list
	const CTargetScheme::TestRecordVector &vAllow2(pTargetScheme->GetAllow2List());
	CTargetScheme::TestRecordVector_cit itAllow2(vAllow2.begin());
	for (; itAllow2 != vAllow2.end(); ++itAllow2)
	{
		if (pTarget->IsTargetType(*itAllow2, pInitiator))
			break;
	}
	if (itAllow2 == vAllow2.end())
		return false;

	// Allowed "2" list must match no types in the restricted "2" list
	const CTargetScheme::TestRecordVector &vRestrict2(pTargetScheme->GetRestrict2List());
	CTargetScheme::TestRecordVector_cit itRestrict2(vRestrict2.begin());
	for (; itRestrict2 != vRestrict2.end(); ++itRestrict2)
	{
		if (pTarget->IsTargetType(*itRestrict2, pInitiator))
			return false;
	}

	return true;
}


/*====================
  IGame::IsValidTarget
  ====================*/
bool	IGame::CanTargetTrait(uint uiTargetScheme, ETargetTrait eTrait)
{
	const CTargetScheme *pTargetScheme(GetTargetScheme(uiTargetScheme));
	if (pTargetScheme == NULL)
		return false;

	bool bValid(true);

	// Must match one type in the allowed list
	const CTargetScheme::TestRecordVector &vAllow(pTargetScheme->GetAllowList());
	CTargetScheme::TestRecordVector_cit itAllow(vAllow.begin());
	for (; itAllow != vAllow.end(); ++itAllow)
	{
		if (itAllow->m_eTest != CTargetScheme::TARGET_SCHEME_TEST_TRAIT)
			continue;
		if (itAllow->m_eTrait == eTrait)
			break;
	}
	if (itAllow == vAllow.end())
		bValid = false;

	if (bValid)
	{
		// Must match no types in the restricted list
		const CTargetScheme::TestRecordVector &vRestrict(pTargetScheme->GetRestrictList());
		CTargetScheme::TestRecordVector_cit itRestrict(vRestrict.begin());
		for (; itRestrict != vRestrict.end(); ++itRestrict)
		{
			if (itRestrict->m_eTest != CTargetScheme::TARGET_SCHEME_TEST_TRAIT)
				continue;
			if (itRestrict->m_eTrait == eTrait)
			{
				bValid = false;
				break;
			}
		}
	}

	if (bValid)
		return true;

	// OR must match one type in the allowed "2" list
	const CTargetScheme::TestRecordVector &vAllow2(pTargetScheme->GetAllow2List());
	CTargetScheme::TestRecordVector_cit itAllow2(vAllow2.begin());
	for (; itAllow2 != vAllow2.end(); ++itAllow2)
	{
		if (itAllow2->m_eTest != CTargetScheme::TARGET_SCHEME_TEST_TRAIT)
			continue;
		if (itAllow2->m_eTrait == eTrait)
			break;
	}
	if (itAllow2 == vAllow2.end())
		return false;

	// Allowed "2" list must match no types in the restricted "2" list
	const CTargetScheme::TestRecordVector &vRestrict2(pTargetScheme->GetRestrict2List());
	CTargetScheme::TestRecordVector_cit itRestrict2(vRestrict2.begin());
	for (; itRestrict2 != vRestrict2.end(); ++itRestrict2)
	{
		if (itRestrict2->m_eTest != CTargetScheme::TARGET_SCHEME_TEST_TRAIT)
				continue;
		if (itRestrict2->m_eTrait == eTrait)
		{
			bValid = false;
			break;
		}
	}

	return true;
}


/*====================
  IGame::SpawnCliff
  ====================*/
void	IGame::SpawnCliff(CWorldEntity *pWorldEnt)
{
	pWorldEnt->SetModelHandle(RegisterModel(pWorldEnt->GetModelPath()));

	LinkEntity(pWorldEnt->GetIndex(), LINK_RENDER, SURF_CLIFF);
}


/*====================
  IGame::SpawnWater
  ====================*/
void	IGame::SpawnWater(CWorldEntity *pWorldEnt)
{
	AddWaterMarker(pWorldEnt->GetPosition(), pWorldEnt->GetProperty(_T("effecttype")));

	pWorldEnt->SetModelHandle(RegisterModel(pWorldEnt->GetModelPath()));

	LinkEntity(pWorldEnt->GetIndex(), LINK_RENDER, SURF_WATER);
}


/*====================
  IGame::SpawnStaticProp
  ====================*/
void	IGame::SpawnStaticProp(CWorldEntity *pWorldEnt)
{
	pWorldEnt->SetModelHandle(RegisterModel(pWorldEnt->GetModelPath()));

	if (pWorldEnt->HasFlags(WE_NOT_SOLID))
	{
		LinkEntity(pWorldEnt->GetIndex(), LINK_MODEL | LINK_SURFACE | LINK_RENDER, SURF_PROP | SURF_STATIC | SURF_NOT_SOLID);
	}
	else
	{
		LinkEntity(pWorldEnt->GetIndex(), LINK_MODEL | LINK_SURFACE | LINK_RENDER, SURF_PROP | SURF_STATIC);

		vector<PoolHandle> vPathBlockers;
		const vector<CConvexPolyhedron> &cWorldSurfs(pWorldEnt->GetWorldSurfsRef());
		for (vector<CConvexPolyhedron>::const_iterator cit(cWorldSurfs.begin()); cit != cWorldSurfs.end(); ++cit)
			Game.BlockPath(vPathBlockers, NAVIGATION_CLIFF, *cit, 0.0f);
	}
}


/*====================
  IGame::Precache
  ====================*/
void	IGame::Precache(ResHandle hDefinition, EPrecacheScheme eScheme)
{
	CEntityDefinitionResource *pDefRes(g_ResourceManager.Get<CEntityDefinitionResource>(hDefinition));
	if (pDefRes != NULL)
		pDefRes->Precache(eScheme);
}


/*====================
  IGame::GetWaterLevel
  ====================*/
float	IGame::GetWaterLevel(const CVec3f &v3Pos) const
{
	float fBestDistSq(SQR(FAR_AWAY));
	float fBestWaterLevel(-FAR_AWAY);

	const CVec2f v2Pos(v3Pos.xy());

	for (vector<SWaterMarker>::const_iterator it(m_vWaterMarkers.begin()), itEnd(m_vWaterMarkers.end()); it != itEnd; ++it)
	{
		float fDistSq(DistanceSq(it->v3Pos.xy(), v2Pos));

		if (fDistSq < fBestDistSq)
		{
			fBestDistSq = fDistSq;
			fBestWaterLevel = it->v3Pos.z;
		}
	}

	return fBestWaterLevel;
}


/*====================
  IGame::GetCooldownEndTime
  ====================*/
uint	IGame::GetCooldownEndTime(uint uiTime, uint uiDuration) const
{
	if (uiDuration == INVALID_TIME)
		return INVALID_TIME;
	else if (uiTime == INVALID_TIME || uiDuration == 0)
		return 0;
	else
		return uiTime + uiDuration;
}


/*====================
  IGame::CanLeave
  ====================*/
bool	IGame::CanLeave(uint uiTeam) const
{
	if (ReplayManager.IsPlaying())
		return true;

	if (!m_pGameInfo)
		return false;
	else
		return m_pGameInfo->CanLeave(uiTeam);
}


/*====================
  IGame::IsGameOver
  ====================*/
bool	IGame::IsGameOver() const
{
	if (GetGamePhase() <= GAME_PHASE_WAITING_FOR_PLAYERS || GetGamePhase() >= GAME_PHASE_ENDED)
		return true;

	if (GetWinningTeam() != TEAM_INVALID)
		return true;

	CGameInfo *pGameInfo(GetGameInfo());
	if (pGameInfo != NULL)
		if (pGameInfo->HasFlags(GAME_FLAG_CONCEDED))
			return true;

	return false;
}


/*====================
  IGame::SwapItem

  Swap two items between one entity and another
  ====================*/
void	IGame::SwapItem(int iClientNum, IEntityItem *pItem0, IEntityItem *pItem1)
{
	if (pItem0 == NULL || pItem1 == NULL || IsClient())
		return;

	IUnitEntity *pOldOwner0(pItem0->GetOwner());
	if (pOldOwner0 == NULL)
		return;

	if (pOldOwner0->GetOwnerClientNumber() != -1 &&
		pOldOwner0->GetOwnerClientNumber() != iClientNum &&
		!pItem0->BelongsToClient(iClientNum) &&
		!pItem0->GetAllowTransfer())
		return;

	IUnitEntity *pOldOwner1(pItem1->GetOwner());
	if (pOldOwner1 == NULL)
		return;

	if (pOldOwner1->GetOwnerClientNumber() != -1 &&
		pOldOwner1->GetOwnerClientNumber() != iClientNum &&
		!pItem1->BelongsToClient(iClientNum) &&
		!pItem1->GetAllowTransfer())
		return;

	IGameEntity *pNewEntity0(Game.AllocateEntity(pItem0->GetType(), pOldOwner1->GetIndex()));
	if (pNewEntity0 == NULL || !pNewEntity0->IsItem())
	{
		Game.DeleteEntity(pNewEntity0);
		return;
	}

	IEntityItem *pNewItem0(pNewEntity0->GetAsItem());

	pNewItem0->SetAllFlags(pItem0->GetAllFlags());
	pNewItem0->SetActiveModifierKey(pItem0->GetActiveModifierKey());
	pNewItem0->Spawn();
	pNewItem0->SetCharges(pItem0->GetCharges());

	uint uiCooldownTime0(pItem0->GetActualRemainingCooldownTime());
	uint uiDuration0(pItem0->GetCooldownDuration());
	if (uiCooldownTime0 != 0 && uiCooldownTime0 != INVALID_TIME &&
		uiDuration0 != 0 && uiDuration0 != INVALID_TIME)
	{
		// Adjust cooldown for global cooldown modifier differences
		float fOldCooldownSpeed(pOldOwner0->GetCooldownSpeed());
		float fOldCooldownReduction(MIN(pOldOwner0->GetReducedCooldowns() - pOldOwner0->GetIncreasedCooldowns(), 1.0f));

		float fNewCooldownSpeed(pOldOwner1->GetCooldownSpeed());
		float fNewCooldownReduction(MIN(pOldOwner1->GetReducedCooldowns() - pOldOwner1->GetIncreasedCooldowns(), 1.0f));

		float fFactor(fOldCooldownSpeed / fNewCooldownSpeed * (1.0f - fNewCooldownReduction) / (1.0f - fOldCooldownReduction));

		float fPercent(pItem0->GetActualRemainingCooldownPercent());

		uint uiNewDuration(INT_CEIL(uiDuration0 * fFactor));
		uint uiNewStartTime(Game.GetGameTime() - INT_CEIL(uiNewDuration * (1.0f - fPercent)));

		pNewItem0->SetCooldownStartTime(uiNewStartTime);
		pNewItem0->SetCooldownDuration(uiNewDuration);
	}
	else
	{
		pNewItem0->SetCooldownStartTime(pItem0->GetCooldownStartTime());
		pNewItem0->SetCooldownDuration(pItem0->GetCooldownDuration());
	}

	pNewItem0->SetLevel(pItem0->GetLevel());
	pNewItem0->SetPurchaserClientNumber(pItem0->GetPurchaserClientNumber());
	pNewItem0->SetTimer(pNewItem0->GetTimer());

	IGameEntity *pNewEntity1(Game.AllocateEntity(pItem1->GetType(), pOldOwner0->GetIndex()));
	if (pNewEntity1 == NULL || !pNewEntity1->IsItem())
	{
		Game.DeleteEntity(pNewEntity0);
		Game.DeleteEntity(pNewEntity1);
		return;
	}

	IEntityItem *pNewItem1(pNewEntity1->GetAsItem());

	pNewItem1->SetAllFlags(pItem1->GetAllFlags());
	pNewItem1->SetActiveModifierKey(pItem1->GetActiveModifierKey());
	pNewItem1->Spawn();
	pNewItem1->SetCharges(pItem1->GetCharges());
	
	uint uiCooldownTime1(pItem1->GetActualRemainingCooldownTime());
	uint uiDuration1(pItem1->GetCooldownDuration());
	if (uiCooldownTime1 != 0 && uiCooldownTime1 != INVALID_TIME &&
		uiDuration1 != 0 && uiDuration1 != INVALID_TIME)
	{
		// Adjust cooldown for global cooldown modifier differences
		float fOldCooldownSpeed(pOldOwner1->GetCooldownSpeed());
		float fOldCooldownReduction(MIN(pOldOwner1->GetReducedCooldowns() - pOldOwner1->GetIncreasedCooldowns(), 1.0f));

		float fNewCooldownSpeed(pOldOwner0->GetCooldownSpeed());
		float fNewCooldownReduction(MIN(pOldOwner0->GetReducedCooldowns() - pOldOwner0->GetIncreasedCooldowns(), 1.0f));

		float fFactor(fOldCooldownSpeed / fNewCooldownSpeed * (1.0f - fNewCooldownReduction) / (1.0f - fOldCooldownReduction));

		float fPercent(pItem1->GetActualRemainingCooldownPercent());

		uint uiNewDuration(INT_CEIL(uiDuration1 * fFactor));
		uint uiNewStartTime(Game.GetGameTime() - INT_CEIL(uiNewDuration * (1.0f - fPercent)));

		pNewItem1->SetCooldownStartTime(uiNewStartTime);
		pNewItem1->SetCooldownDuration(uiNewDuration);
	}
	else
	{
		pNewItem1->SetCooldownStartTime(pItem1->GetCooldownStartTime());
		pNewItem1->SetCooldownDuration(pItem1->GetCooldownDuration());
	}

	pNewItem1->SetLevel(pItem1->GetLevel());
	pNewItem1->SetPurchaserClientNumber(pItem1->GetPurchaserClientNumber());
	pNewItem1->SetTimer(pItem1->GetTimer());

	int iSlot0(pItem0->GetSlot());
	int iSlot1(pItem1->GetSlot());

	pOldOwner0->RemoveItem(iSlot0);
	pOldOwner1->RemoveItem(iSlot1);

	pOldOwner1->SetInventorySlot(iSlot1, pNewItem0);
	pNewItem0->SetOwnerIndex(pOldOwner1->GetIndex());
	pNewItem0->SetSlot(iSlot1);
	pOldOwner1->CheckRecipes(iSlot1);
	pOldOwner1->ValidateExclusiveAttackModSlot();
	pNewItem0->UpdateApparentCooldown();

	pOldOwner0->SetInventorySlot(iSlot0, pNewItem1);
	pNewItem1->SetOwnerIndex(pOldOwner0->GetIndex());
	pNewItem1->SetSlot(iSlot0);
	pOldOwner0->CheckRecipes(iSlot0);
	pOldOwner0->ValidateExclusiveAttackModSlot();
	pNewItem1->UpdateApparentCooldown();
}


