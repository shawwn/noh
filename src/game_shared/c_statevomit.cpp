// (C)2006 S2 Games
// c_statevomit.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statevomit.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(State, Vomit);
//=============================================================================


/*====================
  CStateVomit::CEntityConfig::CEntityConfig
  ====================*/
CStateVomit::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_ENTITY_CVAR(DamagePerSecond, 30.0f),
INIT_ENTITY_CVAR(ArmorAdd, 0.0f),
INIT_ENTITY_CVAR(ArmorMult, 1.0f)
{
}


/*====================
  CStateVomit::CStateVomit
  ====================*/
CStateVomit::CStateVomit() :
IEntityState(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
    m_modArmor.Set(m_pEntityConfig->GetArmorAdd(), m_pEntityConfig->GetArmorMult(), 0.0f);
}


/*====================
  CStateVomit::StateFrame
  ====================*/
void    CStateVomit::StateFrame()
{
    IVisualEntity *pInflictor(Game.GetVisualEntity(m_uiInflictorIndex));
    IVisualEntity *pOwner(Game.GetVisualEntity(m_uiOwnerIndex));
    if (pOwner != NULL)
        pOwner->Damage(m_pEntityConfig->GetDamagePerSecond() * MsToSec(Game.GetFrameLength()), DAMAGE_FLAG_DIRECT, pInflictor, m_unDamageID);

    return;
}
