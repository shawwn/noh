// (C)2006 S2 Games
// c_gadgetimmolate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetimmolate.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, Immolate);
//=============================================================================

/*====================
  CGadgetImmolate::ServerFrame
  ====================*/
bool	CGadgetImmolate::ServerFrame()
{
	ushort unStateID(EntityRegistry.LookupID(m_pEntityConfig->GetStateName()));
	if (GetStatus() != ENTITY_STATUS_DEAD &&
		unStateID != 0)
	{
		CBBoxf bbRegion;
		bbRegion.SetSphere(m_pEntityConfig->GetStateRadius());
		bbRegion.Offset(GetPosition());

		uivector vSetResult;
		Game.GetEntitiesInRegion(vSetResult, bbRegion, 0);
		for (uivector_it it(vSetResult.begin()); it != vSetResult.end(); ++it)
		{
			IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
			if (pEnt == NULL)
				continue;
				
			if (pEnt->GetStatus() == ENTITY_STATUS_DEAD)
				continue;

			if (pEnt->GetTeam() == m_iTeam && pEnt->GetIndex() != m_uiOwnerIndex)
				pEnt->ApplyState(unStateID, Game.GetGameTime(), m_pEntityConfig->GetStateDuration(), m_uiOwnerIndex);
		}

		IGameEntity *pOwnerEnt(Game.GetEntity(m_uiOwnerIndex));
		if (pOwnerEnt == NULL)
			return false;
		IVisualEntity *pOwner(pOwnerEnt->GetAsVisualEnt());
		if (pOwner == NULL)
			return false;

		SetPosition(pOwner->GetPosition());
		SetAngles(pOwner->GetAngles());

		if (pOwner->GetStatus() != ENTITY_STATUS_ACTIVE)
		{
			//FIXME: Disable the skill... Hardcoded is not good, but will have
			//to do for now.
			/*for (int i = INVENTORY_SKILL1; i < INVENTORY_START_BACKPACK; ++i)
				if (pOwner->GetItem(i)->GetName() == _T("Immolate"))
					pOwner->GetItem(i)->FinishedAction(1);*/
		}
	}

	return true;
}
