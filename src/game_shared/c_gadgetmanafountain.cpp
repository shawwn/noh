// (C)2006 S2 Games
// c_gadgetmanafountain.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetmanafountain.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, ManaFountain);
//=============================================================================

/*====================
  CGadgetManaFountain::CEntityConfig::CEntityConfig
  ====================*/
CGadgetManaFountain::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetUsable::CEntityConfig(sName),
INIT_ENTITY_CVAR(ManaRefillPercent, 0.5f)
{
}


/*====================
  CGadgetManaFountain::UseEffect
  ====================*/
bool	CGadgetManaFountain::UseEffect(IGameEntity *pActivator)
{
	IPlayerEntity *pPlayer(pActivator->GetAsPlayerEnt());
	if (pPlayer == NULL)
		return false;

	if (pPlayer->GetMana() < pPlayer->GetMaxMana())
	{
		pPlayer->GiveMana(pPlayer->GetMaxMana() * m_pEntityConfig->GetManaRefillPercent());
		return true;
	}

	return false;
}
