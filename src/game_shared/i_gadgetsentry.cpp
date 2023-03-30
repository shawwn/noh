// (C)2007 S2 Games
// i_gadgetsentry.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gadgetsentry.h"
#include "c_entityclientinfo.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
CVAR_UINTF(		g_sentryResightInterval,	30000,	CVAR_GAMECONFIG);
//=============================================================================

/*====================
  IGadgetSentry::CEntityConfig::CEntityConfig
  ====================*/
IGadgetSentry::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(SightedEffectPath, _T("")),
INIT_ENTITY_CVAR(ExperiencePerSighting, 5.0f),
INIT_ENTITY_CVAR(ExperiencePerSiegeSighting, 10.0f),
INIT_ENTITY_CVAR(ExperiencePerReveal, 10.0f),
INIT_ENTITY_CVAR(ExperiencePrincipal, 1.0f)
{
}


/*====================
  IGadgetSentry::Baseline
  ====================*/
void	IGadgetSentry::Baseline()
{
	IGadgetEntity::Baseline();

	for (int i(0); i < MAX_GADGET_COUNTERS; ++i)
		m_auiCounter[i] = 0;
}


/*====================
  IGadgetSentry::Spawn
  ====================*/
void	IGadgetSentry::Spawn()
{
	IGadgetEntity::Spawn();

	// Store a list of pre-existing neighboring gadgets
	m_setNeighbors.clear();
	m_mapSightList.clear();

	IGameEntity *pNextEntity(Game.GetFirstEntity());
	while (pNextEntity != NULL)
	{
		IGameEntity *pEntity(pNextEntity);
		pNextEntity = Game.GetNextEntity(pEntity);

		if (pEntity->GetUniqueID() == GetUniqueID())
			continue;
		IGadgetEntity *pGadget(pEntity->GetAsGadget());
		if (pGadget == NULL)
			continue;
		IGadgetSentry *pSentry(pGadget->GetAsSentryGadget());
		if (pSentry == NULL)
			continue;
		if (pSentry->GetTeam() != GetTeam())
			continue;

		if (DistanceSq(pSentry->GetPosition().xy(), GetPosition().xy()) > SQR(GetSightRange() + pSentry->GetSightRange()))
			continue;

		m_setNeighbors.insert(pSentry->GetUniqueID());
	}
}


/*====================
  IGadgetSentry::GiveDeploymentExperience
  ====================*/
float	IGadgetSentry::GiveDeploymentExperience()
{
	if (Game.GetGamePhase() != GAME_PHASE_ACTIVE)
		return 0.0f;
	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return 0.0f;

	// Diminish reward for other nearby sentrys
	float fMultiplier(1.0f);
	for (iset_it itNeighbor(m_setNeighbors.begin()); itNeighbor != m_setNeighbors.end(); ++itNeighbor)
	{
		IGameEntity *pNeighbor(Game.GetEntityFromUniqueID(*itNeighbor));
		if (pNeighbor == NULL)
			continue;
		IGadgetEntity *pNeighborGadget(pNeighbor->GetAsGadget());
		if (pNeighborGadget == NULL)
			continue;

		float fMaxDistanceSq(SQR(GetSightRange() + pNeighborGadget->GetSightRange()));
		float fDistSq(DistanceSq(GetPosition().xy(), pNeighborGadget->GetPosition().xy()));
		fMultiplier *= CLAMP(fDistSq / fMaxDistanceSq, 0.0f, 1.0f);
	}

	// Calculate current experience value (compounding interest)
	float fBase(m_pEntityConfig->GetExperiencePrincipal());
	float fTotalPotentialExp(m_pEntityConfig->GetExperiencePerMinute() * MsToMin(GetLifetime()));
	float fRate(pow(1.0f + (fTotalPotentialExp / fBase), 1.0f / GetLifetime()) - 1.0f);
	uint uiTime(Game.GetGameTime() - m_uiSpawnTime);
	float fCurrentValue((fBase * pow(M_E, fRate * uiTime)) - fBase);

	float fExpReward((fCurrentValue - m_fDeploymentExpAccumulator) * fMultiplier);
	if (fExpReward > 0.0f)
	{
		IPlayerEntity *pOwner(Game.GetPlayerEntityFromClientID(GetOwnerClientNumber()));
		if (pOwner != NULL)
			pOwner->GiveExperience(fExpReward);
		m_fTotalExperience += fExpReward;
		m_fDeploymentExpAccumulator = fCurrentValue;
	}

	return fExpReward;
}


/*====================
  IGadgetSentry::ServerFrame
  ====================*/
bool	IGadgetSentry::ServerFrame()
{
	float fStartExperience(m_fTotalExperience);

	if (GetStatus() == ENTITY_STATUS_ACTIVE)
	{
		static uivector vSetResult;
		vSetResult.clear();
		Game.GetEntitiesInRadius(vSetResult, CSphere(m_v3Position, GetSightRange()), 0);
		for (uivector_it it(vSetResult.begin()); it != vSetResult.end(); ++it)
		{
			IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
			if (pEnt == NULL)
				continue;
			ICombatEntity *pCombatEnt(pEnt->GetAsCombatEnt());
			if (pCombatEnt == NULL)
				continue;
			if (pCombatEnt->GetStatus() != ENTITY_STATUS_ACTIVE)
				continue;
			if (pCombatEnt->GetTeam() < 1 || pCombatEnt->GetTeam() == m_iTeam)
				continue;

			bool bOtherSighted(pCombatEnt->HasNetFlags(ENT_NET_FLAG_SIGHTED));
			pCombatEnt->SetNetFlags(ENT_NET_FLAG_REVEALED);

			// If this entity has already been sighted recently, just update the last seen time
			bool bRecentlySighted(false);
			if (m_mapSightList.find(pCombatEnt->GetUniqueID()) != m_mapSightList.end() &&
				Game.GetGameTime() - m_mapSightList[pEnt->GetUniqueID()] < g_sentryResightInterval)
			{
				m_mapSightList[pCombatEnt->GetUniqueID()] = Game.GetGameTime();
				bRecentlySighted = true;
			}

			// Check for the entity being uniquely spotted by this gadget
			bool bUniqueVision(true);
			for (iset_it itNeighbor(m_setNeighbors.begin()); itNeighbor != m_setNeighbors.end(); ++itNeighbor)
			{
				IGameEntity *pNeighbor(Game.GetEntityFromUniqueID(*itNeighbor));
				if (pNeighbor == NULL)
					continue;
				IGadgetEntity *pNeighborGadget(pNeighbor->GetAsGadget());
				if (pNeighborGadget == NULL)
					continue;
				IGadgetSentry *pNeighborSentry(pNeighborGadget->GetAsSentryGadget());
				if (pNeighborSentry == NULL)
					continue;

				if (DistanceSq(pCombatEnt->GetPosition().xy(), pNeighborSentry->GetPosition().xy()) < SQR(pNeighborSentry->GetSightRange()))
					bUniqueVision = false;

				if (pNeighborSentry->GetLastSeenTime(pCombatEnt->GetUniqueID()) - Game.GetGameTime() < g_sentryResightInterval)
					bRecentlySighted = true;

				pNeighborSentry->EntitySpotted(pCombatEnt->GetUniqueID(), Game.GetGameTime());
			}

			if (!bUniqueVision || bOtherSighted)
				continue;

			pCombatEnt->SetNetFlags(ENT_NET_FLAG_SIGHTED);

			if (bRecentlySighted)
				continue;

			// A sighting is registered when target entity has not been outside of this gadget's vision radius for at least
			// g_sentryResightInterval ms, and it is not within the vision radius of any other entity on this team
			// In the case there are multiple sentry objects in the same area, the first placed receives priority
			++m_auiCounter[SENTRY_COUNTER_SIGHTINGS];
			m_fTotalExperience += m_pEntityConfig->GetExperiencePerSighting();

			if (pCombatEnt->GetIsSiege())
			{
				++m_auiCounter[SENTRY_COUNTER_SIEGE_SIGHTINGS];
				m_fTotalExperience += m_pEntityConfig->GetExperiencePerSiegeSighting();
			}
			
			if (pCombatEnt->IsStealthed())
			{
				++m_auiCounter[SENTRY_COUNTER_REVEALS];
				m_fTotalExperience += m_pEntityConfig->GetExperiencePerReveal();
			}

			m_mapSightList[pCombatEnt->GetUniqueID()] = Game.GetGameTime();
		}
	}

	float fExpReward(m_fTotalExperience - fStartExperience);
	if (fExpReward > 0.0f)
	{
		IPlayerEntity *pOwner(Game.GetPlayerEntityFromClientID(GetOwnerClientNumber()));
		if (pOwner != NULL)
			pOwner->GiveExperience(fExpReward);
	}

	return IGadgetEntity::ServerFrame();
}


/*====================
  IGadgetSentry::GetLastSeenTime
  ====================*/
uint	IGadgetSentry::GetLastSeenTime(uint uiUniqueIndex)
{
	map<uint, uint>::iterator it(m_mapSightList.find(uiUniqueIndex));
	if (it == m_mapSightList.end())
		return 0;
	return it->second;
}


/*====================
  IGadgetSentry::EntitySpotted
  ====================*/
void	IGadgetSentry::EntitySpotted(uint uiUniqueIndex, uint uiTime)
{
	map<uint, uint>::iterator it(m_mapSightList.find(uiUniqueIndex));
	if (it == m_mapSightList.end())
		m_mapSightList[uiUniqueIndex] = uiTime;
	else
		it->second = MAX(it->second, uiTime);
}
