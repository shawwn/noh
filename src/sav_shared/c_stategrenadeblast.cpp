// (C)2006 S2 Games
// c_stategrenadeblast.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_stategrenadeblast.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, GrenadeBlast);
//=============================================================================


/*====================
  CStateGrenadeBlast::CEntityConfig::CEntityConfig
  ====================*/
CStateGrenadeBlast::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 50.0f)
{
}


/*====================
  CStateGrenadeBlast::CStateGrenadeBlast
  ====================*/
CStateGrenadeBlast::CStateGrenadeBlast() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CStateGrenadeBlast::StateFrame
  ====================*/
void    CStateGrenadeBlast::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IGameEntity *pOwner(Game.GetEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), 0, pInflictor, m_unDamageID);

    return;
}
