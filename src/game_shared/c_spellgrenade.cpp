// (C)2006 S2 Games
// c_spellgrenade.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellgrenade.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, Grenade);

CVAR_STRINGF	(Spell_Grenade_Projectile,		"Projectile_BuilderGrenade",	CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF		(Spell_Grenade_MinDamage,		0.0f,				CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF		(Spell_Grenade_MaxDamage,		0.0f,				CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF		(Spell_Grenade_PierceUnit,		1.0f,				CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF		(Spell_Grenade_PierceHellbourne,1.0f,				CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF		(Spell_Grenade_PierceSiege,		1.0f,				CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF		(Spell_Grenade_PierceBuilding,	1.0f,				CVAR_GAMECONFIG | CVAR_TRANSMIT);
CVAR_FLOATF		(Spell_Grenade_DamageRadius,	0.0f,				CVAR_GAMECONFIG | CVAR_TRANSMIT);
//============================================================================


/*====================
  CSpellGrenade::TryImpact
  ====================*/
bool	CSpellGrenade::TryImpact()
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	// Client does not predict the execution
	if (Game.IsClient())
		return false;

	if (Spell_Grenade_Projectile.empty())
	{
		return ISpellItem::TryImpact();
	}
	else
	{
		CAxis axis(pOwner->GetAngles());
		CVec3f v3Start(pOwner->GetPosition() + V_UP * pOwner->GetViewHeight());

		// Spawn a projectile
		IGameEntity *pNewEnt(Game.AllocateEntity(Spell_Grenade_Projectile));
		if (pNewEnt == NULL || pNewEnt->GetAsProjectile() == NULL)
		{
			Console.Warn << _T("Failed to spawn projectile: ") << Spell_Grenade_Projectile << newl;
			return false;
		}

		IProjectile *pProjectile(pNewEnt->GetAsProjectile());

		pProjectile->SetOwner(pOwner->GetIndex());
		pProjectile->SetWeaponOrigin(GetType());
		pProjectile->SetTeam(pOwner->GetTeam());
		pProjectile->SetAngles(pOwner->GetAngles());

		if (pProjectile->GetGravity() > 0.0f)
		{
			float fGravity(-pProjectile->GetGravity() * p_gravity);

			CVec3f v3Velocity;

			float fTime(Distance(v3Start, m_v3TargetPosition) / pProjectile->GetSpeed());

			v3Velocity = (m_v3TargetPosition - v3Start) / fTime;
			v3Velocity.z = ((m_v3TargetPosition.z - v3Start.z) - 0.5f * fGravity * fTime * fTime) / fTime;

			pProjectile->SetOrigin(v3Start);
			pProjectile->SetAngles(pOwner->GetAngles());
			pProjectile->SetVelocity(v3Velocity);
		}
		else
		{
			pProjectile->SetOrigin(m_v3TargetPosition + CVec3f(0.0f, 0.0f, 3000.0f));
			pProjectile->SetAngles(CVec3f(-90.0f, 0.0f, 0.0f));
			pProjectile->SetVelocity(CVec3f(0.0f, 0.0f, -1000.0f));
		}

		pProjectile->SetOriginTime(Game.GetServerTime() + Game.GetServerFrameLength());
		pProjectile->SetDamage(Spell_Grenade_MinDamage, Spell_Grenade_MaxDamage, m_pEntityConfig->GetTargetRadius(),
							Spell_Grenade_PierceUnit, Spell_Grenade_PierceHellbourne, Spell_Grenade_PierceSiege, Spell_Grenade_PierceBuilding);
		pNewEnt->Spawn();
	}

	return true;
}
