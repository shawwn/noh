// (C)2008 S2 Games
// c_entityneutralcampspawner.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entityneutralcampspawner.h"

#include "i_unitentity.h"
#include "c_entityneutralcampcontroller.h"
#include "i_neutralentity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Entity, NeutralCampSpawner)
//=============================================================================

/*====================
  CEntityNeutralCampSpawner::ApplyWorldEntity
  ====================*/
void	CEntityNeutralCampSpawner::ApplyWorldEntity(const CWorldEntity &ent)
{
	IVisualEntity::ApplyWorldEntity(ent);
	m_sSpawnName = ent.GetProperty(_T("target0"));
	Game.Precache(m_sSpawnName, PRECACHE_ALL);
}


/*====================
  CEntityNeutralCampSpawner::Trigger
  ====================*/
void	CEntityNeutralCampSpawner::Trigger(IGameEntity *pActivator)
{
	PROFILE("CEntityNeutralCampSpawner::Trigger");

	IUnitEntity *pUnit(Game.AllocateDynamicEntity<IUnitEntity>(m_sSpawnName));
	if (pUnit == NULL)
		return;

	m_pSpawnedUnit = pUnit;
	
	pUnit->SetTeam(TEAM_NEUTRAL);
	pUnit->SetLevel(1);
	pUnit->SetPosition(GetPosition());
	pUnit->SetAngles(GetAngles());
	pUnit->Spawn();
	pUnit->ValidatePosition(TRACE_UNIT_SPAWN);

	pUnit->GetBrain().AddCommand(UNITCMD_GUARD, false, GetPosition().xy(), INVALID_INDEX, uint(-1), true);

	if (pUnit->IsNeutral() && pActivator != NULL && pActivator->IsType<CEntityNeutralCampController>())
		pUnit->GetAsNeutral()->SetSpawnControllerUID(pActivator->GetUniqueID());
}
