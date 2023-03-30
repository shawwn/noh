// (C)2006 S2 Games
// c_spelleruption.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spelleruption.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, Eruption);

CCvarf	CSpellEruption::s_cvarDamage(_T("Spell_Eruption_Damage"),	100.0f,		CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarui	CSpellEruption::s_cvarStunLength(_T("Spell_Eruption_StunLength"),	2000,		CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CSpellEruption::TryImpact
  ====================*/
bool	CSpellEruption::TryImpact()
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
	if (!m_pEntityConfig->GetSnapcast())
	{
		CVec3f v3Origin(m_v3TargetPosition);
		m_v3TargetPosition.Clear();
		v3ImpactPos = v3Origin;
		v3ImpactAngles[YAW] = pOwner->GetAngles()[YAW];

		CBBoxf bbRegion;
		bbRegion.SetSphere(m_pEntityConfig->GetTargetRadius());
		bbRegion.Offset(v3Origin);

		uivector	vResult;
		Game.GetEntitiesInRegion(vResult, bbRegion, 0);
		for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
		{
			IGameEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
			if (pEnt == NULL)
				continue;

			if (!IsValidTarget(pEnt, true))
				continue;

			float fMult(1.0f);

			if (pEnt->IsCombat())
				fMult = pEnt->GetAsCombatEnt()->GetSpellResistance();
			
			pEnt->Damage(s_cvarDamage.GetValue() * fMult, 0, pOwner, GetType());

			if (pEnt->IsPlayer())
				pEnt->GetAsPlayerEnt()->Stun(Game.GetGameTime() + (s_cvarStunLength.GetValue() * fMult));
		}

		// Impact event
		if (!m_pEntityConfig->GetImpactEffectPath().empty())
		{
			CGameEvent evImpact;
			evImpact.SetSourcePosition(v3ImpactPos);
			evImpact.SetSourceAngles(v3ImpactAngles);
			evImpact.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetImpactEffectPath()));
			Game.AddEvent(evImpact);
		}
	}

	return true;
}
