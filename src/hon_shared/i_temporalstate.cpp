// (C)2008 S2 Games
// i_temporalstate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "hon_shared_common.h"

#include "i_temporalstate.h"
//=============================================================================

/*====================
  ITemporalState::CEntityConfig::CEntityConfig
  ====================*/
ITemporalState::CEntityConfig::CEntityConfig(const tstring &sName) :
IEntityState::CEntityConfig(sName),
INIT_TEMPORAL_CVAR(MoveSpeed, 0.0f),
INIT_TEMPORAL_CVAR(Armor, 0.0f),
INIT_TEMPORAL_CVAR(MagicArmor, 0.0f),
INIT_TEMPORAL_CVAR(MaxHealth, 0.0f),
INIT_TEMPORAL_CVAR(MaxMana, 0.0f),
INIT_TEMPORAL_CVAR(HealthRegen, 0.0f),
INIT_TEMPORAL_CVAR(ManaRegen, 0.0f)
{
}


/*====================
  ITemporalState::~ITemporalState
  ====================*/
ITemporalState::~ITemporalState()
{
}


/*====================
  ITemporalState::ITemporalState
  ====================*/
ITemporalState::ITemporalState(CEntityConfig *pConfig) :
m_pEntityConfig(pConfig)
{
}