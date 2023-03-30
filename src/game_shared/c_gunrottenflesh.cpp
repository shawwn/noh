// (C)2007 S2 Games
// c_gunrottenflesh.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gunrottenflesh.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gun, RottenFlesh)
//=============================================================================

/*====================
  CGunRottenFlesh::CEntityConfig::CEntityConfig
  ====================*/
CGunRottenFlesh::CEntityConfig::CEntityConfig(const tstring &sName) :
IGunItem::CEntityConfig(sName),
INIT_ENTITY_CVAR(AmmoPerCorpse, 5)
{
}

/*====================
  CGunRottenFlesh::ConsumeCorpse
  ====================*/
void	CGunRottenFlesh::ConsumeCorpse()
{
	SetAmmo(MIN(ushort(GetAmmoCount()), ushort(GetAmmo() + m_pEntityConfig->GetAmmoPerCorpse())));
}
