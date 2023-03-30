// (C)2005 S2 Games
// i_gameentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_gameentity.h"
#include "c_entityregistry.h"
#include "i_propentity.h"
#include "i_playerentity.h"
#include "c_teaminfo.h"

#include "../k2/c_sceneentity.h"
#include "../k2/c_skeleton.h"
#include "../k2/c_eventscript.h"
#include "../k2/c_entitysnapshot.h"
#include "../k2/c_host.h"
#include "../k2/c_scenemanager.h"
#include "../k2/c_networkresourcemanager.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_model.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_texture.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
vector<SDataField>*	IGameEntity::s_pvFields;
//=============================================================================

/*====================
  IGameEntity::CEntityConfig::CEntityConfig
  ====================*/
IGameEntity::CEntityConfig::CEntityConfig(const tstring &sName)
{
}


/*====================
  IGameEntity::IGameEntity
  ====================*/
IGameEntity::IGameEntity(CEntityConfig *pConfig) :
m_pEntityConfig(pConfig),

m_unType(0),
m_uiIndex(INVALID_INDEX),
m_uiNetFlags(0),
m_bDelete(false),
m_uiFrame(uint(-1)),
m_bValid(false)
{
}


/*====================
  IGameEntity::Baseline
  ====================*/
void	IGameEntity::Baseline()
{
	m_uiNetFlags = 0;
}


/*====================
  IGameEntity::GetAsProp
  ====================*/
IPropEntity*	IGameEntity::GetAsProp()
{
	if (!IsProp())
		return NULL;
	else
		return static_cast<IPropEntity *>(this);
}

const IPropEntity*	IGameEntity::GetAsProp() const
{
	if (!IsProp())
		return NULL;
	else
		return static_cast<const IPropEntity *>(this);
}


/*====================
  IGameEntity::GetAsPlayerEnt
  ====================*/
IPlayerEntity*	IGameEntity::GetAsPlayerEnt()
{
	if (!IsPlayer())
		return NULL;
	else
		return static_cast<IPlayerEntity *>(this);
}

const IPlayerEntity*	IGameEntity::GetAsPlayerEnt() const
{
	if (!IsPlayer())
		return NULL;
	else
		return static_cast<const IPlayerEntity *>(this);
}


/*====================
  IGameEntity::GetAsProjectile
  ====================*/
IProjectile*	IGameEntity::GetAsProjectile()
{
	if (!IsProjectile())
		return NULL;
	else
		return static_cast<IProjectile *>(this);
}

const IProjectile*	IGameEntity::GetAsProjectile() const
{
	if (!IsProjectile())
		return NULL;
	else
		return static_cast<const IProjectile *>(this);
}


/*====================
  IGameEntity::GetAsLight
  ====================*/
ILight*	IGameEntity::GetAsLight()
{
	if (!IsLight())
		return NULL;
	else
		return static_cast<ILight *>(this);
}

const ILight*	IGameEntity::GetAsLight() const
{
	if (!IsLight())
		return NULL;
	else
		return static_cast<const ILight *>(this);
}


/*====================
  IGameEntity::GetAsGadget
  ====================*/
IGadgetEntity*	IGameEntity::GetAsGadget()
{
	if (!IsGadget())
		return NULL;
	else
		return static_cast<IGadgetEntity*>(this);
}

const IGadgetEntity*	IGameEntity::GetAsGadget() const
{
	if (!IsGadget())
		return NULL;
	else
		return static_cast<const IGadgetEntity*>(this);
}


/*====================
  IGameEntity::GetAsBuilding
  ====================*/
IBuildingEntity*	IGameEntity::GetAsBuilding()
{
	if (!IsBuilding())
		return NULL;
	else
		return static_cast<IBuildingEntity*>(this);
}

const IBuildingEntity*	IGameEntity::GetAsBuilding() const
{
	if (!IsBuilding())
		return NULL;
	else
		return static_cast<const IBuildingEntity*>(this);
}


/*====================
  IGameEntity::GetAsNpc
  ====================*/
INpcEntity*		IGameEntity::GetAsNpc()
{
	if (!IsNpc())
		return NULL;
	else
		return static_cast<INpcEntity*>(this);
}

const INpcEntity*	IGameEntity::GetAsNpc() const
{
	if (!IsNpc())
		return NULL;
	else
		return static_cast<const INpcEntity*>(this);
}


/*====================
  IGameEntity::GetAsPet
  ====================*/
IPetEntity*		IGameEntity::GetAsPet()
{
	if (!IsPet())
		return NULL;
	else
		return static_cast<IPetEntity *>(this);
}

const IPetEntity*	IGameEntity::GetAsPet() const
{
	if (!IsPet())
		return NULL;
	else
		return static_cast<const IPetEntity *>(this);
}


/*====================
  IGameEntity::GetAsCombatEnt
  ====================*/
ICombatEntity*	IGameEntity::GetAsCombatEnt()
{
	if (!IsCombat())
		return NULL;
	else
		return static_cast<ICombatEntity *>(this);
}

const ICombatEntity*	IGameEntity::GetAsCombatEnt() const
{
	if (!IsCombat())
		return NULL;
	else
		return static_cast<const ICombatEntity *>(this);
}


/*====================
  IGameEntity::GetAsVisualEnt
  ====================*/
IVisualEntity*	IGameEntity::GetAsVisualEnt()
{
	if (!IsVisual())
		return NULL;
	else
		return static_cast<IVisualEntity *>(this);
}

const IVisualEntity*	IGameEntity::GetAsVisualEnt() const
{
	if (!IsVisual())
		return NULL;
	else
		return static_cast<const IVisualEntity *>(this);
}


/*====================
  IGameEntity::GetAsState
  ====================*/
IEntityState*		IGameEntity::GetAsState()
{
	if (!IsState())
		return NULL;
	else
		return static_cast<IEntityState *>(this);
}

const IEntityState*	IGameEntity::GetAsState() const
{
	if (!IsState())
		return NULL;
	else
		return static_cast<const IEntityState *>(this);
}


/*====================
  IGameEntity::GetAsInventoryItem
  ====================*/
IInventoryItem*		IGameEntity::GetAsInventoryItem()
{
	if (!IsInventoryItem())
		return NULL;
	else
		return static_cast<IInventoryItem *>(this);
}

const IInventoryItem*	IGameEntity::GetAsInventoryItem() const
{
	if (!IsInventoryItem())
		return NULL;
	else
		return static_cast<const IInventoryItem *>(this);
}

/*====================
  IGameEntity::GetAsTrigger
  ====================*/
ITriggerEntity*		IGameEntity::GetAsTrigger()
{
	if (!IsTrigger())
		return NULL;
	else
		return static_cast<ITriggerEntity *>(this);
}

const ITriggerEntity*	IGameEntity::GetAsTrigger() const
{
	if (!IsTrigger())
		return NULL;
	else
		return static_cast<const ITriggerEntity *>(this);
}

/*====================
  IGameEntity::GetSnapshot
  ====================*/
void	IGameEntity::GetSnapshot(CEntitySnapshot &snapshot) const
{
	snapshot.AddField(m_uiNetFlags);
}


/*====================
  IGameEntity::ReadSnapshot
  ====================*/
bool	IGameEntity::ReadSnapshot(CEntitySnapshot &snapshot)
{
	try
	{
		snapshot.ReadNextField(m_uiNetFlags);

		Validate();
		return true;
	}
	catch (CException &ex)
	{
		ex.Process(_T("IGameEntity::ReadSnapshot() - "), NO_THROW);
		return false;
	}
}


/*====================
  IGameEntity::GetTypeVector
  ====================*/
const vector<SDataField>&	IGameEntity::GetTypeVector()
{
	if (!s_pvFields)
	{
		s_pvFields = K2_NEW(global,   vector<SDataField>)();

		s_pvFields->push_back(SDataField(_T("m_uiNetFlags"), FIELD_PUBLIC, TYPE_INT));
	}

	return *s_pvFields;
}


/*====================
  IGameEntity::AddToScene
  ====================*/
bool	IGameEntity::AddToScene(const CVec4f &v4Color, int iFlags)
{
	return true;
}


/*====================
  IGameEntity::Copy
  ====================*/
void	IGameEntity::Copy(const IGameEntity &B)
{
	m_unType		= B.m_unType;
	m_uiNetFlags	= B.m_uiNetFlags;
	m_uiFrame		= B.m_uiFrame;
}


/*====================
  IGameEntity::ClientPrecache
  ====================*/
void	IGameEntity::ClientPrecache(CEntityConfig *pConfig)
{
	if (!pConfig)
		return;
}


/*====================
  IGameEntity::ServerPrecache

  Setup network resource handles and anything else the server needs for this entity
  ====================*/
void	IGameEntity::ServerPrecache(CEntityConfig *pConfig)
{
	if (!pConfig)
		return;
}
