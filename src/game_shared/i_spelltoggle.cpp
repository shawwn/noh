// (C)2007 S2 Games
// i_spelltoggle.h
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_spelltoggle.h"
//=============================================================================

/*====================
  ISpellToggle::CEntityConfig::CEntityConfig
  ====================*/
ISpellToggle::CEntityConfig::CEntityConfig(const tstring &sName) :
ISpellItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(EndAnimName, _T("")),
INIT_ENTITY_CVAR(ActiveEffectPath, _T("")),
INIT_ENTITY_CVAR(FinishTime, 0),
INIT_ENTITY_CVAR(SelfState, _T("")),
INIT_ENTITY_CVAR(ManaCostPerSecond, 0.0f),
INIT_ENTITY_CVAR(MaxTime, 0),
INIT_ENTITY_CVAR(AllowManaRegen, false),
INIT_ENTITY_CVAR(ActiveIconPath, _T(""))
{
}

/*====================
  ISpellToggle::ActiveFrame
  ====================*/
void	ISpellToggle::ActiveFrame()
{
	if (Game.IsClient())
		return;

	if (!HasNetFlags(ITEM_NET_FLAG_ACTIVE))
		return;

	ICombatEntity *pOwner(GetOwnerEnt());
	if (pOwner == NULL)
		return;

	if (!GetSelfState().empty() && (m_iSelfStateSlot == -1 || pOwner->GetState(m_iSelfStateSlot) == NULL || pOwner->GetState(m_iSelfStateSlot)->GetType() != EntityRegistry.LookupID(GetSelfState())))
		m_iSelfStateSlot = pOwner->ApplyState(EntityRegistry.LookupID(GetSelfState()), Game.GetGameTime(), (GetMaxTime() == 0 ? INVALID_TIME : (Game.GetGameTime() + GetMaxTime()) - m_uiActivationTime), pOwner->GetIndex());

	float fManaCost(GetManaCostPerSecond() * MsToSec(Game.GetFrameLength()));
	if (!pOwner->SpendMana(fManaCost))
	{
		Deactivate();
		return;
	}

	if (GetMaxTime() > 0 && Game.GetGameTime() - m_uiActivationTime >= GetMaxTime())
	{
		Deactivate();
		return;
	}
}

/*====================
  ISpellToggle::Deactivate
  ====================*/
void	ISpellToggle::Deactivate()
{
	RemoveNetFlags(ITEM_NET_FLAG_ACTIVE);
	SetCooldownTimer(Game.GetGameTime(), GetCooldownTime());

	m_uiActivationTime = INVALID_TIME;

	ICombatEntity *pOwner(GetOwnerEnt());

	if (!pOwner)
		return;

	if (!GetAllowManaRegen())
		pOwner->RemoveNetFlags(ENT_NET_FLAG_NO_MANA_REGEN);

	if (m_iSelfStateSlot != -1 && pOwner->GetState(m_iSelfStateSlot) != NULL && pOwner->GetState(m_iSelfStateSlot)->GetType() == EntityRegistry.LookupID(GetSelfState()))
		pOwner->RemoveState(m_iSelfStateSlot);

	m_iSelfStateSlot = -1;

	if (!GetActiveEffectPath().empty() && pOwner->GetEffect(EFFECT_CHANNEL_SPELL_TOGGLE) == Game.RegisterEffect(GetActiveEffectPath()))
	{
		pOwner->SetEffect(EFFECT_CHANNEL_SPELL_TOGGLE, INVALID_RESOURCE);
		pOwner->IncEffectSequence(EFFECT_CHANNEL_SPELL_TOGGLE);
	}

	pOwner->SetAction(PLAYER_ACTION_SKILL, Game.GetGameTime() + GetFinishTime());

	if (GetEndAnimName().empty())
		pOwner->StartAnimation(_T("idle"), 1);
	else
		pOwner->StartAnimation(GetEndAnimName(), 1);
}

/*====================
  ISpellToggle::ActivatePrimary
  ====================*/
bool	ISpellToggle::ActivatePrimary(int iButtonStatus)
{
	if (!ISpellItem::ActivatePrimary(iButtonStatus))
		return false;

	if (HasNetFlags(ITEM_NET_FLAG_ACTIVE))
	{
		Deactivate();
		return true;
	}

	SetNetFlags(ITEM_NET_FLAG_ACTIVE);
	m_uiActivationTime = Game.GetGameTime();

	ICombatEntity *pOwner(GetOwnerEnt());

	if (!pOwner)
		return true;

	if (!GetAllowManaRegen())
		pOwner->SetNetFlags(ENT_NET_FLAG_NO_MANA_REGEN);

	if (!GetSelfState().empty())
		m_iSelfStateSlot = pOwner->ApplyState(EntityRegistry.LookupID(GetSelfState()), Game.GetGameTime(), (GetMaxTime() == 0 ? INVALID_TIME : GetMaxTime()), pOwner->GetIndex());

	if (!GetActiveEffectPath().empty())
	{
		pOwner->SetEffect(EFFECT_CHANNEL_SPELL_TOGGLE, Game.RegisterEffect(GetActiveEffectPath()));
		pOwner->IncEffectSequence(EFFECT_CHANNEL_SPELL_TOGGLE);
	}

	return true;
}

/*====================
  ISpellToggle::GetIconImageList
  ====================*/
const tstring&	ISpellToggle::GetIconImageList()
{
	if (m_pEntityConfig && HasNetFlags(ITEM_NET_FLAG_ACTIVE) && !m_pEntityConfig->GetActiveIconPath().empty())
		return m_pEntityConfig->GetActiveIconPath().GetValue();

	return ISpellItem::GetIconImageList();
}

/*====================
  ISpellToggle::GetCurrentIconPath
  ====================*/
const tstring&	ISpellToggle::GetCurrentIconPath()
{
	if (m_pEntityConfig && HasNetFlags(ITEM_NET_FLAG_ACTIVE) && !m_pEntityConfig->GetActiveIconPath().empty())
		return m_pEntityConfig->GetActiveIconPath().GetValue();

	return ISpellItem::GetCurrentIconPath();
}

/*====================
  ISpellToggle::ClientPrecache
  ====================*/
void	ISpellToggle::ClientPrecache(CEntityConfig *pConfig)
{
	ISpellItem::ClientPrecache(pConfig);

	if (!pConfig)
		return;
	
	if (!pConfig->GetActiveEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetActiveEffectPath(), RES_EFFECT);

	if (!pConfig->GetActiveIconPath().empty())
		g_ResourceManager.Register(pConfig->GetActiveIconPath(), RES_TEXTURE);

	if (!pConfig->GetSelfState().empty())
		EntityRegistry.ClientPrecache(EntityRegistry.LookupID(pConfig->GetSelfState()));
}


/*====================
  ISpellToggle::ServerPrecache
  ====================*/
void	ISpellToggle::ServerPrecache(CEntityConfig *pConfig)
{
	ISpellItem::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetActiveEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetActiveEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));

	if (!pConfig->GetSelfState().empty())
		EntityRegistry.ServerPrecache(EntityRegistry.LookupID(pConfig->GetSelfState()));
}
