// (C)2007 S2 Games
// c_spellhailstorm.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellhailstorm.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, HailStorm)
//=============================================================================

/*====================
  CSpellHailStorm::TryImpact
  ====================*/
bool	CSpellHailStorm::TryImpact()
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	// Client does not predict the execution
	if (Game.IsClient())
		return false;

	// Information for impact event
	CVec3f v3ImpactPos;
	CVec3f v3ImpactAngles;

	// Validate the target
	CVec3f v3Origin(m_v3TargetPosition);
	m_v3TargetPosition.Clear();
	v3ImpactPos = v3Origin;
	v3ImpactAngles[YAW] = pOwner->GetAngles()[YAW];

	// Spawn gadget
	IGameEntity *pNewEnt(Game.AllocateEntity(_T("Gadget_Hail")));
	if (pNewEnt == NULL || pNewEnt->GetAsGadget() == NULL)
	{
		Console.Warn << _T("Failed to spawn gadget: Hail") << newl;
		return false;
	}

	v3ImpactAngles[PITCH] = 0.0f;
	v3ImpactPos[Z] = Game.GetTerrainHeight(v3ImpactPos[X], v3ImpactPos[Y]);
	pNewEnt->GetAsGadget()->SetOwner(pOwner->GetIndex());
	pNewEnt->GetAsGadget()->SetTeam(pOwner->GetTeam());
	pNewEnt->GetAsGadget()->SetPosition(v3ImpactPos);
	pNewEnt->GetAsGadget()->SetAngles(v3ImpactAngles);
	pNewEnt->Spawn();

	// Impact event
	if (!m_pEntityConfig->GetImpactEffectPath().empty())
	{
		CGameEvent evImpact;
		evImpact.SetSourcePosition(v3ImpactPos);
		evImpact.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetImpactEffectPath()));
		Game.AddEvent(evImpact);
	}

	return true;
}
