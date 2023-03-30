// (C)2008 S2 Games
// i_bitentity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_bitentity.h"

#include "c_propdynamic.h"

#include "../k2/c_model.h"
#include "../k2/c_k2model.h"
#include "../k2/c_worldentity.h"
#include "../k2/c_world.h"
#include "../k2/c_texture.h"
#include "../k2/intersection.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENTITY_DESC(IBitEntity, 1)
{
	s_cDesc.pFieldTypes = K2_NEW(ctx_Game,   TypeVector)();
}
//=============================================================================

/*====================
  IBitEntity::CEntityConfig::CEntityConfig
  ====================*/
IBitEntity::CEntityConfig::CEntityConfig(const tstring &sName) :
IUnitEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(BoundsRadius, 0.0f),
INIT_ENTITY_CVAR(BoundsHeight, 0.0f),
INIT_ENTITY_CVAR(OcclusionRadius, 0.0f)
{
}


/*====================
  IBitEntity::IBitEntity
  ====================*/
IBitEntity::IBitEntity(CEntityConfig *pConfig) :
m_pEntityConfig(pConfig)
{
}


/*====================
  IBitEntity::~IBitEntity
  ====================*/
IBitEntity::~IBitEntity()
{
}


/*====================
  IBitEntity::ApplyWorldEntity
  ====================*/
void	IBitEntity::ApplyWorldEntity(const CWorldEntity &ent)
{
	IUnitEntity::ApplyWorldEntity(ent);

	m_fScale = ent.GetScale();
}


/*====================
  IBitEntity::Spawn
  ====================*/
void	IBitEntity::Spawn()
{
}


/*====================
  IBitEntity::Die
  ====================*/
void	IBitEntity::Die(IUnitEntity *pAttacker, ushort unKillingObjectID)
{
	Game.DeactivateBitEntity(m_uiIndex);

	if (m_uiWorldIndex == INVALID_INDEX || !Game.WorldEntityExists(m_uiWorldIndex))
		return;

	CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
	if (pWorldEnt == NULL)
		return;

	// Spawn a dynamic prop to display the death animation
	CPropDynamic *pProp(static_cast<CPropDynamic*>(Game.AllocateEntity(Prop_Dynamic)));
	if (pProp == NULL)
		return;

	pProp->SetModel(pWorldEnt->GetModelHandle());
	pProp->SetPosition(pWorldEnt->GetPosition());
	pProp->SetAngles(pWorldEnt->GetAngles());
	pProp->SetScale(pWorldEnt->GetScale());
	pProp->Spawn();
	if (GetUseAltDeathAnims())
		pProp->StartRandomAnimation(_T("alt_death_%"), 1, 0);
	else
		pProp->StartRandomAnimation(_T("death_%"), 1, 0);
	pProp->SetStatus(ENTITY_STATUS_CORPSE);
	pProp->SetCorpseTime(Game.GetGameTime() + 2500);
	pProp->SetVisibilityFlags(Game.GetVision(GetPosition().x, GetPosition().y));
}


/*====================
  IBitEntity::GetModel
  ====================*/
ResHandle	IBitEntity::GetModel() const
{
	if (m_uiWorldIndex == INVALID_INDEX || !Game.WorldEntityExists(m_uiWorldIndex))
		return INVALID_RESOURCE;
	
	CWorldEntity *pWorldEnt(Game.GetWorldEntity(m_uiWorldIndex));
	if (pWorldEnt == NULL)
		return INVALID_RESOURCE;

	return pWorldEnt->GetModelHandle();
}


/*====================
  IBitEntity::Link
  ====================*/
void	IBitEntity::Link()
{
	Console.Err << _T("Link called on bit entity") << newl;
}


/*====================
  IBitEntity::Unlink
  ====================*/
void	IBitEntity::Unlink()
{
	Console.Err << _T("Unlink called on bit entity") << newl;
}
