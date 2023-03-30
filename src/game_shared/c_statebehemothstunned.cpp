// (C)2007 S2 Games
// c_statebehemothstunned.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statebehemothstunned.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, BehemothStunned);
//=============================================================================


/*====================
  CStateBehemothStunned::CEntityConfig::CEntityConfig
  ====================*/
CStateBehemothStunned::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName)
{
}


/*====================
  CStateBehemothStunned::CStateBehemothStunned
  ====================*/
CStateBehemothStunned::CStateBehemothStunned() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateBehemothStunned::Activated
  ====================*/
void    CStateBehemothStunned::Activated()
{
    IEntityState::Activated();
    IPlayerEntity *pOwner(Game.GetPlayerEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Stun(GetExpireTime());
}
