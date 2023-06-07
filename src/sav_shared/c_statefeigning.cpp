// (C)2006 S2 Games
// c_statefeigning.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_statefeigning.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
#define STATE_NAME  Feigning

DEFINE_STATE_ALLOCATOR(Feigning);

STATE_CVAR(tstring, IconPath,   _T("/human/units/scout/icons/feigning.tga"));
STATE_CVAR(tstring, EffectPath, _T(""));
STATE_CVAR(bool,    IsDebuff,       false);
STATE_CVAR(bool,    IsMeleeMove,    false);
STATE_CVAR(tstring, Skin,           _T(""));
//=============================================================================

/*====================
  CStateFeigning::~CStateFeigning
  ====================*/
CStateFeigning::~CStateFeigning()
{
}


/*====================
  CStateFeigning::CStateFeigning
  ====================*/
CStateFeigning::CStateFeigning()
{
    ASSIGN_ENTITY_STATE_CVARS;
}
