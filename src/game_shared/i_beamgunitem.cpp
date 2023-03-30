// (C)2007 S2 Games
// i_beamgunitem.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_beamgunitem.h"
#include "c_entityeffect.h"

#include "../k2/c_camera.h"
#include "../k2/c_vid.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_host.h"
#include "../k2/c_particlesystem.h"
#include "../k2/c_eventscript.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_clientsnapshot.h"
#include "../k2/c_model.h"
#include "../k2/c_effect.h"
//=============================================================================

/*====================
  IBeamGunItem::CEntityConfig::CEntityConfig
  ====================*/
IBeamGunItem::CEntityConfig::CEntityConfig(const tstring &sName) :
IGunItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(ManaCostPerSecondHit, 0.0f),
INIT_ENTITY_CVAR(ManaCostPerSecondMiss, 0.0f),
INIT_ENTITY_CVAR(ThirdPersonHitEffectPath, _T("")),
INIT_ENTITY_CVAR(ThirdPersonMissEffectPath, _T("")),
INIT_ENTITY_CVAR(FirstPersonHitEffectPath, _T("")),
INIT_ENTITY_CVAR(FirstPersonMissEffectPath, _T("")),
INIT_ENTITY_CVAR(UseHitEffect, false)
{
}


/*====================
  IBeamGunItem::IBeamGunItem
  ====================*/
IBeamGunItem::~IBeamGunItem()
{
	if (m_uiEffectIndex != INVALID_INDEX)
	{
		Game.DeleteEntity(m_uiEffectIndex);
		m_uiEffectIndex = INVALID_INDEX;
	}
}


/*====================
  IBeamGunItem::IBeamGunItem
  ====================*/
IBeamGunItem::IBeamGunItem(CEntityConfig *pConfig) :
IGunItem(pConfig),
m_pEntityConfig(pConfig),

m_uiEffectIndex(INVALID_INDEX)
{
}


/*====================
  IBeamGunItem::StopFire
  ====================*/
void	IBeamGunItem::StopFire()
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return;

	// Third Person Animations
	if (!GetThirdPersonFireAnimName().empty())
		pOwner->StopAnimation(GetThirdPersonFireAnimName(), 1);

	// First person animations
	if (pOwner->IsPlayer())
		pOwner->GetAsPlayerEnt()->StopFirstPersonAnimation(_T("fire"));

	RemoveNetFlags(ITEM_NET_FLAG_ACTIVE);

	if (m_uiEffectIndex != INVALID_INDEX)
	{
		Game.DeleteEntity(m_uiEffectIndex);
		m_uiEffectIndex = INVALID_INDEX;
	}
}


/*====================
  IBeamGunItem::Unselected
  ====================*/
void	IBeamGunItem::Unselected()
{
	StopFire();

	IGunItem::Unselected();
}


/*====================
  IBeamGunItem::Fire
  ====================*/
bool	IBeamGunItem::Fire(int iButtonStatus)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return true;

	// Don't interrupt other actions
	if (pOwner->GetAction() != PLAYER_ACTION_IDLE &&
		pOwner->GetAction() != PLAYER_ACTION_GUN_WARM &&
		pOwner->GetAction() != PLAYER_ACTION_GUN_CHARGE &&
		pOwner->GetAction() != PLAYER_ACTION_GUN_CHARGED &&
		pOwner->GetAction() != PLAYER_ACTION_GUN_FIRE)
	{
		StopFire();
		return false;
	}

	CAxis axis(pOwner->GetAngles());

	const CVec3f &v3Forward(axis.Forward());
	//const CVec3f &v3Offset(GetAttackOffset());
	CVec3f v3Start(pOwner->GetPosition() + V_UP * pOwner->GetViewHeight());

	// Third Person Animations
	if (!GetThirdPersonFireAnimName().empty() && pOwner->GetAction() == PLAYER_ACTION_IDLE)
		pOwner->StartAnimation(GetThirdPersonFireAnimName(), 1);

	// First person animations
	if (pOwner->IsPlayer() && pOwner->GetAction() == PLAYER_ACTION_IDLE)
		pOwner->GetAsPlayerEnt()->StartFirstPersonAnimation(_T("fire"));

	CVec3f v3Dir(v3Forward);
	CVec3f v3End(v3Start + v3Dir * GetRange());

	// Do a trace
	STraceInfo result;
	Game.TraceLine(result, v3Start, v3End, TRACE_PROJECTILE, pOwner->GetWorldIndex());

	IVisualEntity *pEntity(NULL);
	if (result.uiEntityIndex != INVALID_INDEX)
	{
		pEntity = Game.GetEntityFromWorldIndex(result.uiEntityIndex);
		if (pEntity)
		{
			if (!pEntity->Impact(result, pOwner) || !pEntity->CanTakeDamage(DAMAGE_FLAG_GUN, pOwner))
				pEntity = NULL;
		}
	}

	if (!GetAttackTime() || pOwner->GetAction() == PLAYER_ACTION_IDLE || pOwner->GetAction() == PLAYER_ACTION_GUN_WARM)
	{
		// Check ammo
		if (GetAdjustedAmmoCount() > 0 &&
			!pOwner->UseAmmo(m_ySlot))
		{
			StopFire();
			return CoolDown();
		}

		float fFrameTime;
		if (GetAttackTime())
			fFrameTime = GetAttackTime() * SEC_PER_MS;
		else
			fFrameTime = Game.GetFrameLength() * SEC_PER_MS;

		if (pEntity)
		{
			float fManaCost(pOwner->GetGunManaCost(GetManaCostPerSecondHit()) * fFrameTime);

			// Check mana
			if (GetManaCostPerSecondHit() > 0.0f &&
				!pOwner->SpendMana(fManaCost))
			{
				StopFire();
				pOwner->SetMana(0.0f);
				return false;
			}

			if (Game.IsServer())
			{
				float fDamage(M_Randnum(pOwner->GetRangedMinDamage(m_ySlot), pOwner->GetRangedMaxDamage(m_ySlot)) * fFrameTime);
				ICombatEntity *pCombat(pEntity->GetAsCombatEnt());

				//fDamage *= (1.0f + pOwner->GetAttributeBoost(ATTRIBUTE_RANGED_DAMAGE));

				if (pEntity->IsBuilding())
					fDamage *= GetPierceBuilding();
				else if (pCombat != NULL && pCombat->GetIsSiege())
					fDamage *= GetPierceSiege();
				else if (pCombat != NULL && pCombat->GetIsHellbourne())
					fDamage *= GetPierceHellbourne();
				else if (pEntity->IsPlayer() || pEntity->IsNpc() || pEntity->IsPet())
					fDamage *= GetPierceUnit();

				if (pEntity->Damage(fDamage, DAMAGE_FLAG_GUN, pOwner, GetType()) > 0)
					pEntity->Hit(result.v3EndPos, result.plPlane.v3Normal);	// FIXME: Put something meaningful here

				if (pEntity->GetTeam() != pOwner->GetTeam())
				{
					ushort unStateID(0);
					if (!m_pEntityConfig->GetTargetState().empty())
						unStateID = EntityRegistry.LookupID(m_pEntityConfig->GetTargetState());
					if (unStateID != 0)
						pEntity->ApplyState(unStateID, Game.GetGameTime(), m_pEntityConfig->GetTargetStateDuration(), pOwner->GetIndex());
				}
			}
		}
		else
		{
			float fManaCost(pOwner->GetGunManaCost(GetManaCostPerSecondMiss()) * fFrameTime);

			// Check mana
			if (GetManaCostPerSecondMiss() > 0.0f &&
				!pOwner->SpendMana(fManaCost))
			{
				StopFire();
				pOwner->SetMana(0.0f);
				return false;
			}
		}

		if (GetAttackTime())
			pOwner->SetAction(PLAYER_ACTION_GUN_FIRE, (Game.GetGameTime() / GetAttackTime() + 1) * GetAttackTime()); // Round down to the nearest AttackTime (acts like an accumulator)
	}

	SetNetFlags(ITEM_NET_FLAG_ACTIVE);

	if (Game.IsClient())
		return true;

	// Notify all states that an attack is occuring
	pOwner->DoRangedAttack();

	if (m_uiEffectIndex == INVALID_INDEX)
	{
		IGameEntity *pNew(Game.AllocateEntity(Entity_Effect));
		m_uiEffectIndex = pNew ? pNew->GetIndex() : INVALID_INDEX;
	}

	CEntityEffect *pEffect(static_cast<CEntityEffect *>(Game.GetEntity(m_uiEffectIndex)));
	if (pEffect)
	{
		if (!pEntity || !m_pEntityConfig->GetUseHitEffect())
		{
			pEffect->SetSourceEntityIndex(pOwner->GetIndex());
			pEffect->SetTargetEntityIndex(INVALID_INDEX);
			
			pEffect->SetTargetPosition(result.v3EndPos);

			if (!m_pEntityConfig->GetThirdPersonMissEffectPath().empty())
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_THIRD_PERSON, Game.RegisterEffect(m_pEntityConfig->GetThirdPersonMissEffectPath()));
			else
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_THIRD_PERSON, INVALID_RESOURCE);

			if (!m_pEntityConfig->GetFirstPersonMissEffectPath().empty())
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_FIRST_PERSON, Game.RegisterEffect(m_pEntityConfig->GetFirstPersonMissEffectPath()));
			else
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_FIRST_PERSON, INVALID_RESOURCE);
		}
		else
		{
			pEffect->SetSourceEntityIndex(pOwner->GetIndex());
			pEffect->SetTargetEntityIndex(pEntity->GetIndex());

			if (!m_pEntityConfig->GetThirdPersonHitEffectPath().empty())
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_THIRD_PERSON, Game.RegisterEffect(m_pEntityConfig->GetThirdPersonHitEffectPath()));
			else
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_THIRD_PERSON, INVALID_RESOURCE);

			if (!m_pEntityConfig->GetFirstPersonHitEffectPath().empty())
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_FIRST_PERSON, Game.RegisterEffect(m_pEntityConfig->GetFirstPersonHitEffectPath()));
			else
				pEffect->SetEffect(EFFECT_CHANNEL_BEAM_FIRST_PERSON, INVALID_RESOURCE);
		}
	}

	return true;
}


/*====================
  IBeamGunItem::ActivatePrimary
  ====================*/
bool	IBeamGunItem::ActivatePrimary(int iButtonStatus)
{
	ICombatEntity *pOwner(GetOwnerEnt());
	if (!pOwner)
		return false;

	bool bRet(IGunItem::ActivatePrimary(iButtonStatus));

	if (pOwner->GetAction() != PLAYER_ACTION_GUN_FIRE && pOwner->GetAction() != PLAYER_ACTION_GUN_WARM)
		StopFire();

	return bRet;
}


/*====================
  IBeamGunItem::ClientPrecache
  ====================*/
void	IBeamGunItem::ClientPrecache(CEntityConfig *pConfig)
{
	IGunItem::ClientPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetThirdPersonHitEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetThirdPersonHitEffectPath(), RES_EFFECT);

	if (!pConfig->GetThirdPersonMissEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetThirdPersonMissEffectPath(), RES_EFFECT);

	if (!pConfig->GetFirstPersonHitEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetFirstPersonHitEffectPath(), RES_EFFECT);

	if (!pConfig->GetFirstPersonMissEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetFirstPersonMissEffectPath(), RES_EFFECT);
}


/*====================
  IBeamGunItem::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void	IBeamGunItem::ServerPrecache(CEntityConfig *pConfig)
{
	IGunItem::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetThirdPersonHitEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetThirdPersonHitEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetThirdPersonMissEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetThirdPersonMissEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetFirstPersonHitEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetFirstPersonHitEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetFirstPersonMissEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetFirstPersonMissEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}

