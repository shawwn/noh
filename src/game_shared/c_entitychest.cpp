// (C)2007 S2 Games
// c_entitychest.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_entitychest.h"

#include "../k2/c_texture.h"
#include "../k2/c_skeleton.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	CEntityChest::s_pvFields;

DEFINE_ENT_ALLOCATOR2(Entity, Chest)
//=============================================================================


/*====================
  CEntityChest::CEntityConfig::CEntityConfig
  ====================*/
CEntityChest::CEntityConfig::CEntityConfig(const tstring &sName) :
IVisualEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(BoundsRadius, 20.0f),
INIT_ENTITY_CVAR(BoundsHeight, 20.0f),
INIT_ENTITY_CVAR(ExpireTime, 30000)
{
}


/*====================
  CEntityChest::~CEntityChest
  ====================*/
CEntityChest::~CEntityChest()
{
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
IVisualEntity(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig()),
m_uiSpawnTime(INVALID_TIME)
{
}


/*====================
  CEntityChest::Baseline
  ====================*/
void	CEntityChest::Baseline()
{
	m_v3Position = V3_ZERO;
	m_v3Angles = V3_ZERO;

	m_ayAnim[0] = -1;
	m_ayAnimSequence[0] = 0;
	m_afAnimSpeed[0] = 1.0f;

	m_uiSpawnTime = INVALID_TIME;
}


/*====================
  CEntityChest::GetSnapshot
  ====================*/
void	CEntityChest::GetSnapshot(CEntitySnapshot &snapshot) const
{
	snapshot.AddField(m_v3Position);
	snapshot.AddAngle16(m_v3Angles.x);
	snapshot.AddAngle16(m_v3Angles.y);
	snapshot.AddAngle16(m_v3Angles.z);

	snapshot.AddField(m_ayAnim[0]);
	snapshot.AddField(m_ayAnimSequence[0]);
	snapshot.AddField(m_afAnimSpeed[0]);

	snapshot.AddField(m_uiSpawnTime);
}


/*====================
  CEntityChest::ReadSnapshot
  ====================*/
bool	CEntityChest::ReadSnapshot(CEntitySnapshot &snapshot)
{
	snapshot.ReadNextField(m_v3Position);
	snapshot.ReadNextAngle16(m_v3Angles.x);
	snapshot.ReadNextAngle16(m_v3Angles.y);
	snapshot.ReadNextAngle16(m_v3Angles.z);

	snapshot.ReadNextField(m_ayAnim[0]);
	snapshot.ReadNextField(m_ayAnimSequence[0]);
	snapshot.ReadNextField(m_afAnimSpeed[0]);

	snapshot.ReadNextField(m_uiSpawnTime);

	Validate();
	
	return true;
}


/*====================
  CEntityChest::GetTypeVector
  ====================*/
const vector<SDataField>&	CEntityChest::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();
		s_pvFields->push_back(SDataField(_T("m_v3Position"), FIELD_PUBLIC, TYPE_V3F));
		s_pvFields->push_back(SDataField(_T("m_v3Angles[PITCH]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_v3Angles[ROLL]"), FIELD_PUBLIC, TYPE_ANGLE16));
		s_pvFields->push_back(SDataField(_T("m_v3Angles[YAW]"), FIELD_PUBLIC, TYPE_ANGLE16));

		s_pvFields->push_back(SDataField(_T("m_ayAnim[0]"), FIELD_PUBLIC, TYPE_CHAR));
		s_pvFields->push_back(SDataField(_T("m_ayAnimSequence[0]"), FIELD_PUBLIC, TYPE_CHAR));
		s_pvFields->push_back(SDataField(_T("m_afAnimSpeed[0]"), FIELD_PUBLIC, TYPE_FLOAT));

		s_pvFields->push_back(SDataField(_T("m_uiSpawnTime"), FIELD_PUBLIC, TYPE_INT));
	}

	return *s_pvFields;
}


/*====================
  CEntityChest::Spawn
  ====================*/
void	CEntityChest::Spawn()
{
	IVisualEntity::Spawn();

	m_yStatus = ENTITY_STATUS_ACTIVE;
	m_hModel = Game.RegisterModel(GetModelPath());
	m_uiSpawnTime = Game.GetGameTime();

	StartAnimation(_T("idle"), 0);
	m_yDefaultAnim = m_ayAnim[0];

	if (m_uiWorldIndex == INVALID_INDEX)
		m_uiWorldIndex = Game.AllocateNewWorldEntity();

	m_bbBounds.SetCylinder(m_pEntityConfig->GetBoundsRadius(), m_pEntityConfig->GetBoundsHeight());

	Link();
}


/*====================
  CEntityChest::ServerFrame
  ====================*/
bool	CEntityChest::ServerFrame()
{
	PROFILE("CEntityChest::ServerFrame");

	if (!IVisualEntity::ServerFrame())
		return false;

	if (Game.GetGameTime() >= m_uiSpawnTime + m_pEntityConfig->GetExpireTime())
		return false;

	return true;
}


/*====================
  CEntityChest::Touch
  ====================*/
void	CEntityChest::Touch(IGameEntity *pActivator)
{
	if (!Game.IsServer())
		return;

	if (!m_apInventory[0])
		return;

	IPlayerEntity *pPlayer(pActivator->GetAsPlayerEnt());
	if (!pPlayer)
		return;

	if (!pPlayer->CanCarryItem(m_apInventory[0]->GetType()))
		return;

	if (m_apInventory[0]->GetType() == Persistant_Item)
	{
		int iSlot(pPlayer->GiveItem(INVENTORY_AUTO_BACKPACK, m_apInventory[0]->GetType(), false));

		if (iSlot != -1)
		{
			IInventoryItem *pItem(pPlayer->GetItem(iSlot));

			if (pItem)
			{
				if (pItem->GetAsPersistant())
				{
					pItem->GetAsPersistant()->SetItemData(m_apInventory[0]->GetAsPersistant()->GetItemData());
					pItem->GetAsPersistant()->SetItemID(-1);
					pItem->Disable();
				}
			}

			CBufferFixed<4> cBuffer;
			cBuffer << GAME_CMD_PICKUP_ITEM << byte(iSlot) << m_apInventory[0]->GetType();
			Game.SendGameData(pPlayer->GetClientID(), cBuffer, false);

			SetDelete(true);
		}
	}
	else
	{
		int iSlot(pPlayer->GiveItem(INVENTORY_AUTO_BACKPACK, m_apInventory[0]->GetType()));

		if (iSlot != -1)
		{
			CBufferFixed<4> cBuffer;
			cBuffer << GAME_CMD_PICKUP_ITEM << byte(iSlot) << m_apInventory[0]->GetType();
			Game.SendGameData(pPlayer->GetClientID(), cBuffer, false);

			SetDelete(true);
		}
	}
}


/*====================
  CEntityChest::AllocateSkeleton
  ====================*/
CSkeleton*	CEntityChest::AllocateSkeleton()
{
	return m_pSkeleton = K2_NEW(global,   CSkeleton);
}


/*====================
  CEntityChest::Link
  ====================*/
void	CEntityChest::Link()
{
	if (m_uiWorldIndex != INVALID_INDEX)
	{
		CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
		
		if (pWorldEnt != NULL)
		{
			pWorldEnt->SetPosition(GetPosition());
			pWorldEnt->SetScale(GetScale());
			pWorldEnt->SetScale2(GetScale2());
			pWorldEnt->SetAngles(GetAngles());
			pWorldEnt->SetBounds(GetBounds());
			pWorldEnt->SetModelHandle(GetModelHandle());
			pWorldEnt->SetGameIndex(GetIndex());

			Game.LinkEntity(m_uiWorldIndex, LINK_BOUNDS, SURF_ITEM);
		}
	}
}


/*====================
  CEntityChest::Unlink
  ====================*/
void	CEntityChest::Unlink()
{
	if (m_uiWorldIndex != INVALID_INDEX)
		Game.UnlinkEntity(m_uiWorldIndex);
}
