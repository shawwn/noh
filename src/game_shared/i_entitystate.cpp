// (C)2006 S2 Games
// i_entitystate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_entitystate.h"
#include "c_entityclientinfo.h"

#include "../k2/c_texture.h"
#include "../k2/c_model.h"
#include "../k2/c_effect.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	IEntityState::s_pvFields;
//=============================================================================

/*====================
  IEntityState::CEntityConfig::CEntityConfig
  ====================*/
IEntityState::CEntityConfig::CEntityConfig(const tstring &sName) :
IGameEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(Name, _T("NAME")),
INIT_ENTITY_CVAR(Description, _T("DESCRIPTION")),
INIT_ENTITY_CVAR(IconPath, _T("")),
INIT_ENTITY_CVAR(EffectPath, _T("")),
INIT_ENTITY_CVAR(AnimName, _T("")),
INIT_ENTITY_CVAR(IsDebuff, false),
INIT_ENTITY_CVAR(DisplayState, true),
INIT_ENTITY_CVAR(IsSecret, false),
INIT_ENTITY_CVAR(Skin, _T("")),
INIT_ENTITY_CVAR(ModelPath, _T("")),
INIT_ENTITY_CVAR(Stack, false),
INIT_ENTITY_CVAR(DisplayTimer, true),
INIT_ENTITY_CVAR(DisableSkills, false),
INIT_ENTITY_CVAR(IsStealth, false),
INIT_ENTITY_CVAR(IsInvulnerable, false),
INIT_ENTITY_CVAR(IsIntangible, false),
INIT_ENTITY_CVAR(IsDispellable, true),
INIT_ENTITY_CVAR(RemoveOnDamage, false),
INIT_ENTITY_CVAR(AssistCredit, 0.0f),
INIT_ENTITY_CVAR(AllowDash, true)
{
}


/*====================
  IEntityState::~IEntityState
  ====================*/
IEntityState::~IEntityState()
{
	if (m_uiOwnerIndex != INVALID_INDEX)
	{
		IVisualEntity *pEntity(Game.GetVisualEntity(m_uiOwnerIndex));
		if (pEntity)
		{
			pEntity->ClearState(this);
		}

		m_uiOwnerIndex = INVALID_INDEX;
	}
}


/*====================
  IEntityState::IEntityState
  ====================*/
IEntityState::IEntityState(CEntityConfig *pConfig) :
IGameEntity(pConfig),
m_pEntityConfig(pConfig),

m_uiOwnerIndex(INVALID_INDEX),
m_uiInflictorIndex(INVALID_INDEX),
m_unDamageID(INVALID_ENT_TYPE),

m_bValid(true),

m_uiStartTime(INVALID_TIME),
m_uiDuration(INVALID_TIME),
m_uiLastIncomeTime(INVALID_TIME),

m_hModel(INVALID_RESOURCE),
m_iDisguiseTeam(-1),
m_iDisguiseClient(-1),
m_unDisguiseItem(INVALID_ENT_TYPE)
{
}


/*====================
  IEntityState::GetTypeVector
  ====================*/
const vector<SDataField>&	IEntityState::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->clear();
		const vector<SDataField> &vBase(IGameEntity::GetTypeVector());
		s_pvFields->insert(s_pvFields->begin(), vBase.begin(), vBase.end());

		s_pvFields->push_back(SDataField(_T("m_uiOwnerIndex"), FIELD_PUBLIC, TYPE_GAMEINDEX));
		s_pvFields->push_back(SDataField(_T("m_uiStartTime"), FIELD_PRIVATE, TYPE_INT));
		s_pvFields->push_back(SDataField(_T("m_uiDuration"), FIELD_PRIVATE, TYPE_INT));
	}

	return *s_pvFields;
}


/*====================
  IEntityState::GetSnapshot
  ====================*/
void	IEntityState::GetSnapshot(CEntitySnapshot &snapshot) const
{
	IGameEntity::GetSnapshot(snapshot);

	snapshot.AddGameIndex(m_uiOwnerIndex);
	snapshot.AddField(m_uiStartTime);
	snapshot.AddField(m_uiDuration);
}


/*====================
  IEntityState::ReadSnapshot
  ====================*/
bool	IEntityState::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		IGameEntity::ReadSnapshot(snapshot);

		snapshot.ReadNextGameIndex(m_uiOwnerIndex);
		snapshot.ReadNextField(m_uiStartTime);
		snapshot.ReadNextField(m_uiDuration);

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IEntityState::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  IEntityState::Baseline
  ====================*/
void	IEntityState::Baseline()
{
	IGameEntity::Baseline();

	m_uiOwnerIndex = INVALID_INDEX;
	m_uiStartTime = INVALID_TIME;
	m_uiDuration = INVALID_TIME;
}


/*====================
  IEntityState::GetPrivateClient
  ====================*/
int		IEntityState::GetPrivateClient()
{
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner && pOwner->IsPlayer())
		return pOwner->GetAsPlayerEnt()->GetClientID();
	else
		return -1;
}


/*====================
  IEntityState::Activated
  ====================*/
void	IEntityState::Activated()
{
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;

	if (!GetModelPath().empty())
		m_hModel = Game.RegisterModel(GetModelPath());

	if (!GetEffectPath().empty())
	{
		CGameEvent evStateEffect;
		evStateEffect.SetEffect(Game.RegisterEffect(GetEffectPath()));
		evStateEffect.SetSourceEntity(GetIndex());
		Game.AddEvent(evStateEffect);
	}

	if (!GetAnimName().empty())
		pOwner->StartAnimation(GetAnimName(), -1);

	if (m_uiLastIncomeTime == INVALID_TIME)
		m_uiLastIncomeTime = Game.GetGameTime();
}


/*====================
  IEntityState::Expired
  ====================*/
void	IEntityState::Expired()
{
	IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;

	if (!GetAnimName().empty())
		pOwner->StopAnimation(GetAnimName(), -1);
}


/*====================
  IEntityState::Spawn
  ====================*/
void	IEntityState::Spawn()
{
	IGameEntity::Spawn();

	if (!GetModelPath().empty())
		m_hModel = Game.RegisterModel(GetModelPath());
}


/*====================
  IEntityState::SetMod
  ====================*/
void	IEntityState::SetMod(uint uiMod, const FloatMod &modValue)
{
	UShortMod modUShort;

	switch (uiMod)
	{
	case MOD_MANA:
		SetManaMod(modValue);
		break;

	case MOD_INCOME:
		modUShort.SetMult(modValue.GetMult());
		modUShort.SetAdd((ushort)(modValue.GetAdd()));

		SetIncomeMod(modUShort);
		break;

	case MOD_EVASION:
		// TODO: Evasion?
		break;

	case MOD_ARMOR:
		SetArmorMod(modValue);
		break;

	case MOD_HEALTH:
		SetHealthMod(modValue);
		break;

	case MOD_SPEED:
		SetSpeedMod(modValue);
		break;

	case MOD_STAMINA:
		SetStaminaMod(modValue);
		break;

	case MOD_DAMAGE:
		SetDamageMod(modValue);
		break;

	case MOD_REGEN_HEALTH:
		SetHealthRegenMod(modValue);
		break;

	case MOD_REGEN_INCOME:
		modUShort.SetMult(modValue.GetMult());
		modUShort.SetAdd((ushort)(modValue.GetAdd()));

		SetIncomeGenerationMod(modUShort);
		break;

	case MOD_REGEN_MANA:
		SetManaRegenMod(modValue);
		break;

	case MOD_REGEN_STAMINA:
		SetStaminaRegenMod(modValue);
		break;

	case MOD_GUNMANACOST:
		SetGunManaCostMod(modValue);
		break;

	case MOD_SPELLRESIST:
		SetSpellResistMod(modValue);
		break;

	case MOD_SKILLRESIST:
		SetSkillResistMod(modValue);
		break;

	case MOD_NULL:
	case MOD_REGEN_NULL:
	default:
		break;
	}
}


/*====================
  IEntityState::StateFrame
  ====================*/
void	IEntityState::StateFrame()
{
	if (Game.GetGameTime() - m_uiLastIncomeTime <= 15000)
		return;

	IGameEntity *pOwner(Game.GetEntity(m_uiOwnerIndex));
	if (pOwner == NULL)
		return;

	IPlayerEntity *pPlayer(pOwner->GetAsPlayerEnt());
	if (pPlayer == NULL)
		return;

	CEntityClientInfo *pClient(Game.GetClientInfo(pPlayer->GetClientID()));
	if (pClient == NULL)
		return;

	// FIXME: Possible bug... if Modify() causes unGold to drop below 0,
	// may give player a large amount of gold. Not a concern for now, as
	// only positive values will be used for the modifier.
	ushort unGold(m_modIncomeGen.Modify(pClient->GetGold()));
	if (unGold > pClient->GetGold())
	{
		ushort unDiff(unGold - pClient->GetGold());
		Game.MatchStatEvent(pClient->GetClientNumber(), PLAYER_MATCH_GOLD_EARNED, int(unDiff), pPlayer->GetClientID(), GetType());
		pClient->GiveGold(unDiff);
	}

	m_uiLastIncomeTime = Game.GetGameTime();
	return;
}


/*====================
  IEntityState::OwnerDamaged
  ====================*/
float	IEntityState::OwnerDamaged(float fDamage, int iFlags, IVisualEntity *pAttacker)
{
	if (fDamage <= 0.0f || GetIsInvulnerable())
		return 0.0f;

	if (GetRemoveOnDamage())
		Invalidate();

	return fDamage;
}


/*====================
  IEntityState::IsMatch
  ====================*/
bool	IEntityState::IsMatch(ushort unType)
{
	return m_unType == unType;
}


/*====================
  IEntityState::ClientPrecache
  ====================*/
void	IEntityState::ClientPrecache(CEntityConfig *pConfig)
{
	IGameEntity::ClientPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetModelPath().empty())
	{
		ResHandle hModel(g_ResourceManager.Register(pConfig->GetModelPath(), RES_MODEL));

		if (hModel != INVALID_RESOURCE)
			g_ResourceManager.PrecacheSkin(hModel, -1);
	}

	if (!pConfig->GetEffectPath().empty())
		g_ResourceManager.Register(pConfig->GetEffectPath(), RES_EFFECT);

	if (!pConfig->GetIconPath().empty())
		g_ResourceManager.Register(K2_NEW(global,   CTexture)(pConfig->GetIconPath(), TEXTURE_2D, TEX_FULL_QUALITY, TEXFMT_A8R8G8B8), RES_TEXTURE);
}


/*====================
  IEntityState::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void	IEntityState::ServerPrecache(CEntityConfig *pConfig)
{
	IGameEntity::ServerPrecache(pConfig);

	if (!pConfig)
		return;

	if (!pConfig->GetModelPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetModelPath(), RES_MODEL, RES_MODEL_IGNORE_VID | RES_MODEL_IGNORE_EVENTS));

	if (!pConfig->GetEffectPath().empty())
		g_NetworkResourceManager.GetNetIndex(g_ResourceManager.Register(pConfig->GetEffectPath(), RES_EFFECT, RES_EFFECT_IGNORE_ALL));
}
