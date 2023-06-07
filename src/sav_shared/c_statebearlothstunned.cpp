// (C)2006 S2 Games
// c_statebearlothstunned.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statebearlothstunned.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, BearlothStunned);
//=============================================================================


/*====================
  CStateBearlothStunned::CEntityConfig::CEntityConfig
  ====================*/
CStateBearlothStunned::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(ArmorMult, 1.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f)
{
}


/*====================
  CStateBearlothStunned::CStateBearlothStunned
  ====================*/
CStateBearlothStunned::CStateBearlothStunned() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateBearlothStunned::Activated
  ====================*/
void    CStateBearlothStunned::Activated()
{
    IEntityState::Activated();
    IPlayerEntity *pOwner(Game.GetPlayerEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Stun(GetExpireTime());
}
