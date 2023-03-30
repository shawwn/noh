// (C)2007 S2 Games
// c_entitychest.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitychest.h"

#include "i_entityitem.h"

#include "../k2/c_texture.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
ResHandle			CEntityChest::s_hModel(INVALID_RESOURCE);

CVAR_STRINGF(Entity_Chest_ImmunityType, "", CVAR_GAMECONFIG | CVAR_TRANSMIT);

DEFINE_ENT_ALLOCATOR2(Entity, Chest)

DEFINE_ENTITY_DESC(CEntityChest, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(ctx_Game,    TypeVector)();
	s_cDesc.pFieldTypes->clear();
	const TypeVector &vBase(IUnitEntity::GetTypeVector());
	s_cDesc.pFieldTypes->insert(s_cDesc.pFieldTypes->begin(), vBase.begin(), vBase.end());
}
//=============================================================================


/*====================
  CEntityChest::CEntityConfig::CEntityConfig
  ====================*/
CEntityChest::CEntityConfig::CEntityConfig(const tstring &sName) :
IUnitEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(DisplayName, _T("")),
INIT_ENTITY_CVAR(ModelPath, _T("")),
INIT_ENTITY_CVAR(PreGlobalScale, 1.0f),
INIT_ENTITY_CVAR(ModelScale, 1.0f),
INIT_ENTITY_CVAR(SelectionRadius, 32.0f),
INIT_ENTITY_CVAR(BoundsRadius, 16.0f),
INIT_ENTITY_CVAR(BoundsHeight, 32.0f),
INIT_ENTITY_CVAR(MaxHealth, 40.0f),
INIT_ENTITY_CVAR(CorpseTime, 0),
INIT_ENTITY_CVAR(DeathTime, 0),
INIT_ENTITY_CVAR(CorpseFadeTime, 0),
INIT_ENTITY_CVAR(DeathAnim, _T("")),
INIT_ENTITY_CVAR(DeathNumAnims, 0),
INIT_ENTITY_CVAR(AltDeathAnim, _T("")),
INIT_ENTITY_CVAR(AltDeathNumAnims, 0),
INIT_ENTITY_CVAR(NoBlockNeutralSpawn, true)
{
}


/*====================
  CEntityChest::~CEntityChest
  ====================*/
CEntityChest::~CEntityChest()
{
	if (IGame::GetCurrentGamePointer() == NULL)
		return;

	if (m_uiWorldIndex != INVALID_INDEX && Game.WorldEntityExists(m_uiWorldIndex))
	{
		Game.UnlinkEntity(m_uiWorldIndex);
		Game.DeleteWorldEntity(m_uiWorldIndex);
	}
}


/*====================
  CEntityChest::CEntityChest
  ====================*/
CEntityChest::CEntityChest() :
m_pEntityConfig(GetEntityConfig())
{
	m_uiLinkFlags = SURF_ITEM;
}


/*====================
  CEntityChest::Baseline
  ====================*/
void	CEntityChest::Baseline()
{
	IUnitEntity::Baseline();
}


/*====================
  CEntityChest::GetSnapshot
  ====================*/
void	CEntityChest::GetSnapshot(CEntitySnapshot &snapshot, uint uiFlags) const
{
	IUnitEntity::GetSnapshot(snapshot, uiFlags);
}


/*====================
  CEntityChest::ReadSnapshot
  ====================*/
bool	CEntityChest::ReadSnapshot(CEntitySnapshot &snapshot, uint uiVersion)
{
	try
	{
		// Base entity info
		if (!IUnitEntity::ReadSnapshot(snapshot, 1))
			return false;

		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("ICreepEntity::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  CEntityChest::Spawn
  ====================*/
void	CEntityChest::Spawn()
{
	IUnitEntity::Spawn();
}


/*====================
  CEntityChest::Touch
  ====================*/
void	CEntityChest::Touch(IGameEntity *pActivator, int iIssuedClientNumber)
{
	if (!Game.IsServer())
		return;

	if (m_yStatus != ENTITY_STATUS_ACTIVE)
		return;

	IUnitEntity *pUnit(pActivator->GetAsUnit());
	if (pUnit == NULL)
		return;

	for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
	{
		if (!m_apInventory[i])
			continue;

		if (m_apInventory[i]->IsItem())
		{
			if (!CanGiveItem(m_apInventory[i]->GetAsItem(), pUnit))
				continue;
		}

		ushort unType(m_apInventory[i]->GetType());
		int iSlot(pUnit->TransferItem(pUnit->GetOwnerClientNumber(), m_apInventory[i]->GetAsItem()));

		if (iSlot != -1)
		{
			CBufferFixed<6> cBuffer;
			cBuffer << GAME_CMD_PICKUP_ITEM << ushort(pUnit->GetIndex()) << byte(iSlot) << unType;
			Game.SendGameData(pUnit->GetOwnerClientNumber(), cBuffer, false);

			Game.LogItem(GAME_LOG_ITEM_PICKUP, pUnit->GetItem(iSlot));

			SetDelete(true);
		}
	}

	bool bRemaining(false);
	for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
	{
		if (m_apInventory[i])
		{
			bRemaining = true;
			break;
		}
	}
	
	if (!bRemaining)
		SetDelete(true);
}


/*====================
  CEntityChest::Die
  ====================*/
void	CEntityChest::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
	if (GetStatus() != ENTITY_STATUS_ACTIVE)
		return;

	// in item drop mode, items are unkillable.
	bool bDropItems(Game.HasGameOptions(GAME_OPTION_DROP_ITEMS));

	// if we contain an item which is unkillable, then don't allow the chest to die.
	for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
	{
		IEntityItem *pItem(GetItem(i));
		if (pItem == NULL)
			continue;

		if (pItem->GetUnkillable())
			return;

		if (bDropItems)
			return;
	}

	m_fHealth = 0.0f;

	Unlink();
	SetStatus(ENTITY_STATUS_CORPSE);
	Link();

	m_uiCorpseTime = Game.GetGameTime() + GetCorpseTime();

	for (int i(0); i < NUM_ANIM_CHANNELS; ++i)
		m_auiAnimLockTime[i] = INVALID_TIME;

	m_cBrain.Killed();
	UnblockPath();
	if (GetUseAltDeathAnims())
		StartRandomAnimation(GetAltDeathAnim(), GetAltDeathNumAnims(), 0);
	else
		StartRandomAnimation(GetDeathAnim(), GetDeathNumAnims(), 0);
}


/*====================
  CEntityChest::GetImmunityType
  ====================*/
uint	CEntityChest::GetImmunityType() const
{
	return Game.LookupImmunityType(Entity_Chest_ImmunityType);
}


/*====================
  CEntityChest::ClientPrecache
  ====================*/
void	CEntityChest::ClientPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
	IUnitEntity::ClientPrecache(pConfig, eScheme, sModifier);

	if (pConfig == NULL)
		return;

	if (!pConfig->GetModelPath().empty())
		s_hModel = Game.RegisterModel(pConfig->GetModelPath());
}


/*====================
  CEntityChest::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void	CEntityChest::ServerPrecache(CEntityConfig *pConfig, EPrecacheScheme eScheme, const tstring &sModifier)
{
	IUnitEntity::ServerPrecache(pConfig, eScheme, sModifier);

	if (pConfig == NULL)
		return;

	if (!pConfig->GetModelPath().empty())
	{
		s_hModel = Game.RegisterModel(pConfig->GetModelPath());
		NetworkResourceManager.GetNetIndex(s_hModel);
	}
}


/*====================
  CEntityChest::GetDisplayName
  ====================*/
const tstring&	CEntityChest::GetDisplayName() const
{
	m_sDisplayName.clear();

	for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
	{
		if (m_apInventory[i] == NULL)
			continue;

		IEntityItem *pItem(m_apInventory[i]->GetAsItem());
		if (pItem == NULL)
			continue;

		CPlayer *pPlayer(Game.GetPlayer(pItem->GetPurchaserClientNumber()));
		if (pPlayer != NULL)
			m_sDisplayName = GetInlineColorString<tstring>(pPlayer->GetColor()) + pPlayer->GetName() + _T("'s^* ");

		m_sDisplayName += pItem->GetDisplayName();

		if (!pItem->HasFlag(ENTITY_TOOL_FLAG_ASSEMBLED))
			m_sDisplayName += _T(" Recipe");

		break;
	}

	return m_sDisplayName;
}

/*====================
  CEntityChest::ServerFrameCleanup
  ====================*/
bool	CEntityChest::ServerFrameCleanup()
{
	// Delete empty chests
	bool bDelete(true);

	for (int i(INVENTORY_START_BACKPACK); i <= INVENTORY_END_BACKPACK; ++i)
	{
		if (!m_apInventory[i])
			continue;

		bDelete = false;
		break;
	}

	if (bDelete)
		return false;

	return IUnitEntity::ServerFrameCleanup();
}
