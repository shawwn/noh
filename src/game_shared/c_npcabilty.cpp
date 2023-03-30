// (C)2006 S2 Games
// c_npcability.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_npcability.h"
#include "c_projectilenpcshot.h"
#include "c_statenpcability.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
const CNpcAbilityEffect	*g_pNpcAbilityEffect(NULL);
//=============================================================================

/*====================
  CNpcAbilityEffect::ApplyEffect
  ====================*/
void	CNpcAbilityEffect::ApplyEffect(uint uiTargetIndex, uint uiAttackerIndex) const
{
	if (M_Randnum(0.0f, 1.0f) > m_fProc)
		return;

	IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
	if (pTarget == NULL)
		return;

	IVisualEntity *pAttacker(Game.GetVisualEntity(uiAttackerIndex));
	if (pAttacker == NULL)
		return;

	if (pTarget != pAttacker && pTarget->IsIntangible())
		return;

	if (!pAttacker->IsEnemy(pTarget) ||
		pTarget->GetStatus() != ENTITY_STATUS_ACTIVE ||
		(!pTarget->IsCombat() && !pTarget->IsGadget() && !pTarget->IsBuilding() && !pTarget->IsNpc()))
		return;

	float fMult(1.0f);

	if (pTarget->IsCombat())
		fMult = pTarget->GetAsCombatEnt()->GetSkillResistance();

	float fDamage(M_Randnum(m_fMinDamage, m_fMaxDamage) * fMult);

	if (fDamage > 0.0f)
		pTarget->Damage(fDamage, 0, Game.GetVisualEntity(uiAttackerIndex));

	if (m_bMeleeImpact)
	{
		STraceInfo result;
		Game.TraceLine(result, pAttacker->GetPosition() + pAttacker->GetBounds().GetMid(), pTarget->GetPosition() + pTarget->GetBounds().GetMid(), TRACE_MELEE, pAttacker->GetWorldIndex());

		if (result.uiEntityIndex != INVALID_INDEX)
		{
			pTarget->Hit(result.v3EndPos, V3_ZERO, ENTITY_HIT_BY_MELEE); // TODO: Impact angles
		}
	}

	g_pNpcAbilityEffect = this;

	if (m_uiDuration > 0)
	{
		int iSlot(pTarget->ApplyState(State_NpcAbility, Game.GetGameTime(), m_uiDuration, uiAttackerIndex));

		if (iSlot != -1 && pTarget->GetState(iSlot) != NULL && pTarget->GetState(iSlot)->IsDebuff())
			pTarget->GetState(iSlot)->SetDuration(m_uiDuration * fMult);
	}

	g_pNpcAbilityEffect = NULL;
}


/*====================
  CNpcAbilityEffect::Activated
  ====================*/
void	CNpcAbilityEffect::Activated(CStateNpcAbility *pState) const
{
	pState->SetIcon(m_hIcon);
	pState->SetEffect(m_hEffect);
	pState->SetAnimName(m_sAnimName);
	pState->SetStun(m_bStun);
	pState->SetStack(m_bStack);
	pState->SetSpeedMod(m_modSpeed);
	pState->SetAttackSpeedMod(m_modAttackSpeed);
	pState->SetDamageMod(m_modDamage);
	pState->SetHealthRegenMod(m_modHealthRegen);
	pState->SetManaRegenMod(m_modManaRegen);
	pState->SetStaminaRegenMod(m_modStaminaRegen);
	pState->SetArmorMod(m_modArmor);
	pState->SetAmmoMod(m_modAmmo);
	pState->SetNpcAbilityEffectID(m_unID);
}


/*====================
  CNpcProjectile::SpawnProjectile
  ====================*/
IProjectile*	CNpcProjectile::SpawnProjectile(IGameEntity *pOwner, const CVec3f &v3Start, const CVec3f &v3Forward) const
{
	tstring sProjectileName(_T("Projectile_NpcShot"));

	// Spawn a projectile
	IGameEntity *pNewEnt(Game.AllocateEntity(sProjectileName));
	if (pNewEnt == NULL || pNewEnt->GetAsProjectile() == NULL)
	{
		Console.Warn << _T("Failed to spawn projectile: ") << sProjectileName << newl;
		return NULL;
	}

	CProjectileNpcShot *pProjectile(static_cast<CProjectileNpcShot *>(pNewEnt));

	pProjectile->SetOwner(pOwner->GetIndex());
	pProjectile->SetOrigin(v3Start);
	pProjectile->SetAngles(M_GetAnglesFromForwardVec(v3Forward));
	pProjectile->SetVelocity(v3Forward * m_fSpeed);
	pProjectile->SetOriginTime(Game.GetServerTime() + Game.GetServerFrameLength());
	pProjectile->Spawn();
	
	pProjectile->SetModelHandle(m_hModel);
	pProjectile->SetDeathEffect(m_hDeathEffect);

	if (m_hTrailEffect != INVALID_RESOURCE)
	{
		pProjectile->SetEffect(EFFECT_CHANNEL_PROJECTILE_TRAIL, m_hTrailEffect);
		pProjectile->IncEffectSequence(EFFECT_CHANNEL_PROJECTILE_TRAIL);
	}

	return pProjectile;
}


/*====================
  CNpcAbility::SpawnProjectile
  ====================*/
IProjectile*	CNpcAbility::SpawnProjectile(IGameEntity *pOwner, const CVec3f &v3Start, const CVec3f &v3Forward) const
{
	CProjectileNpcShot *pProjectile(static_cast<CProjectileNpcShot *>(m_cProjectile.SpawnProjectile(pOwner, v3Start, v3Forward)));

	for (EffectVector::const_iterator itEffect(m_vSourceEffects.begin()); itEffect != m_vSourceEffects.end(); ++itEffect)
		pProjectile->AddSourceEffect(*itEffect);

	for (EffectVector::const_iterator itEffect(m_vTargetEffects.begin()); itEffect != m_vTargetEffects.end(); ++itEffect)
		pProjectile->AddTargetEffect(*itEffect);

	return pProjectile;
}


/*====================
  CNpcAbility::Impact
  ====================*/
void	CNpcAbility::Impact(uint uiOwnerIndex, uint uiTargetIndex) const
{
	if (uiTargetIndex != INVALID_INDEX)
	{
		for (EffectVector::const_iterator itEffect(m_vTargetEffects.begin()); itEffect != m_vTargetEffects.end(); ++itEffect)
			itEffect->ApplyEffect(uiTargetIndex, uiOwnerIndex);

		for (EffectVector::const_iterator itEffect(m_vSourceEffects.begin()); itEffect != m_vSourceEffects.end(); ++itEffect)
			itEffect->ApplyEffect(uiOwnerIndex, uiOwnerIndex);
	}
	else if (uiOwnerIndex != INVALID_INDEX)
	{
		for (EffectVector::const_iterator itEffect(m_vTargetEffects.begin()); itEffect != m_vTargetEffects.end(); ++itEffect)
			itEffect->ApplyEffect(uiOwnerIndex, uiOwnerIndex);

		for (EffectVector::const_iterator itEffect(m_vSourceEffects.begin()); itEffect != m_vSourceEffects.end(); ++itEffect)
			itEffect->ApplyEffect(uiOwnerIndex, uiOwnerIndex);
	}

	IVisualEntity *pTarget(Game.GetVisualEntity(uiTargetIndex));
	IVisualEntity *pOwner(Game.GetVisualEntity(uiOwnerIndex));

	if (m_fEffectRadius > 0.0f)
	{
		CSphere	cSphere;

		if (pTarget)
			cSphere = CSphere(pTarget->GetPosition(), m_fEffectRadius);
		else if (pOwner)
			cSphere = CSphere(pOwner->GetPosition(), m_fEffectRadius);
		else
			return;

		uivector	vResult;
		Game.GetEntitiesInRadius(vResult, cSphere, 0);
		for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
		{
			IGameEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
			if (pEnt == NULL || pEnt == pTarget)
				continue;

			for (EffectVector::const_iterator itEffect(m_vTargetEffects.begin()); itEffect != m_vTargetEffects.end(); ++itEffect)
				itEffect->ApplyEffect(pEnt->GetIndex(), uiOwnerIndex);

			for (EffectVector::const_iterator itEffect(m_vSourceEffects.begin()); itEffect != m_vSourceEffects.end(); ++itEffect)
				itEffect->ApplyEffect(uiOwnerIndex, uiOwnerIndex);
		}
	}
}


/*====================
  CNpcAbility::ImpactPosition
  ====================*/
void	CNpcAbility::ImpactPosition(uint uiOwnerIndex, const CVec3f &v3Pos) const
{
	CSphere	cSphere(v3Pos, m_fEffectRadius);

	uivector	vResult;
	Game.GetEntitiesInRadius(vResult, cSphere, 0);
	for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
	{
		IGameEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
		if (pEnt == NULL)
			continue;

		for (EffectVector::const_iterator itEffect(m_vTargetEffects.begin()); itEffect != m_vTargetEffects.end(); ++itEffect)
			itEffect->ApplyEffect(pEnt->GetIndex(), uiOwnerIndex);

		for (EffectVector::const_iterator itEffect(m_vSourceEffects.begin()); itEffect != m_vSourceEffects.end(); ++itEffect)
			itEffect->ApplyEffect(uiOwnerIndex, uiOwnerIndex);
	}
}

