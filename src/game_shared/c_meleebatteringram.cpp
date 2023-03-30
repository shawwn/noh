// (C)2006 S2 Games
// c_meleebatteringram.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_meleebatteringram.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Melee, BatteringRam);
//=============================================================================

/*====================
  CMeleeBatteringRam::CEntityConfig::CEntityConfig
  ====================*/
CMeleeBatteringRam::CEntityConfig::CEntityConfig(const tstring &sName) :
IMeleeItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(HitBuildingEffectPath, _T(""))
{
}


/*====================
  CBatteringRam::Impact
  ====================*/
void	CMeleeBatteringRam::Impact()
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return;

	// Don't predict hits
	if (Game.IsClient())
		return;

	CMeleeAttackEvent& attack(pOwner->GetAttackEvent());
	attack.SetActive();

	// Trace for hits
	uiset setIndices;
	vector<CVec3f> vImpacts;
	TraceArc(attack, setIndices, vImpacts);
	
	// Process each entity that this attack hit
	vector<CVec3f>::iterator itImpacts(vImpacts.begin());
	for (uiset_it itEntIndex(setIndices.begin()); itEntIndex != setIndices.end(); ++itEntIndex, ++itImpacts)
	{
		IVisualEntity *pEntity(Game.GetVisualEntity(*itEntIndex));

		if (pEntity == NULL)
			continue;

		// Melee hits never affect same team
		if (pEntity->GetTeam() == pOwner->GetTeam())
			continue;

		// Apply any states carried by this attack
		const svector &vStates(attack.GetStateVector());
		const uivector &vStateDurations(attack.GetStateDurationVector());
		for (size_t z(0); z < vStates.size(); ++z)
			pEntity->ApplyState(EntityRegistry.LookupID(vStates[z]), Game.GetGameTime() + vStateDurations[z], pOwner->GetIndex());

		// Melee push
		CAxis axis(pOwner->GetAngles());
		CVec3f v3AttackPush(attack.GetPush());
		CVec3f v3Push(axis.Forward2d());
		v3AttackPush *= pEntity->GetPushMultiplier();
		v3Push *= v3AttackPush.x;
		v3Push += axis.Right() * v3AttackPush.y;
		v3Push.z = v3AttackPush.z;
		pEntity->ApplyVelocity(v3Push);

		// Do the damage
		float fDamage(attack.GetDamage());
		IPlayerEntity *pPlayer = pEntity->GetAsPlayerEnt();

		if (pEntity->IsBuilding())
			fDamage *= m_pEntityConfig->GetPierceBuilding();
		else if (pPlayer != NULL && pPlayer->GetIsSiege())
			fDamage *= m_pEntityConfig->GetPierceSiege();
		else if (pPlayer != NULL && pPlayer->GetIsHellbourne())
			fDamage *= m_pEntityConfig->GetPierceHellbourne();
		else if (pEntity->IsPlayer() || pEntity->IsNpc() || pEntity->IsPet())
			fDamage *= m_pEntityConfig->GetPierceUnit();

		fDamage = pEntity->Damage(fDamage, attack.GetDamageFlags(), pOwner, GetType());
		pEntity->Hit(*itImpacts, V3_ZERO, ENTITY_HIT_BY_MELEE); // TODO: Impact angles

		if (pEntity->IsBuilding() && !m_pEntityConfig->GetHitBuildingEffectPath().empty())
		{
			CGameEvent evImpact;
			evImpact.SetSourcePosition(*itImpacts);
			evImpact.SetSourceAngles(V3_ZERO); // TODO: Impact angles
			evImpact.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetHitBuildingEffectPath()));
			Game.AddEvent(evImpact);
		}

		if (fDamage > 0.0f && !pEntity->IsBuilding() && !(pEntity->IsPlayer() && pEntity->GetAsPlayerEnt()->GetIsVehicle()))
		{
			// Start blood effect on the weapon
			pOwner->SetEffect(2, Game.RegisterEffect(_T("/shared/effects/bloodspray_small.effect")));
			pOwner->IncEffectSequence(2);
		}
	}

	attack.SetInactive();
}

/*====================
  CMeleeBatteringRam::ClientPrecache
  ====================*/
void	CMeleeBatteringRam::ClientPrecache(CEntityConfig *pConfig)
{
	IMeleeItem::ClientPrecache(pConfig);

	if (pConfig == NULL)
		return;

	if (!pConfig->GetHitBuildingEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetHitBuildingEffectPath(), RES_EFFECT);
}


/*====================
  CMeleeBatteringRam::ServerPrecache
  ====================*/
void	CMeleeBatteringRam::ServerPrecache(CEntityConfig *pConfig)
{
	IMeleeItem::ClientPrecache(pConfig);

	if (pConfig == NULL)
		return;

	if (!pConfig->GetHitBuildingEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetHitBuildingEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}
