// (C)2006 S2 Games
// c_abilityresurrect.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_abilityresurrect.h"

#include "../k2/c_skeleton.h"
#include "../k2/s_traceinfo.h"
#include "../k2/c_world.h"
#include "../k2/c_host.h"
//=============================================================================

//=============================================================================
// Declarations
//=============================================================================
//=============================================================================

/*====================
  CAbilityResurrect::~CAbilityResurrect
  ====================*/
CAbilityResurrect::~CAbilityResurrect()
{
}


/*====================
  CAbilityResurrect::CAbilityResurrect
  ====================*/
CAbilityResurrect::CAbilityResurrect()
{
}


/*====================
  CAbilityResurrect::Selected
  ====================*/
void    CAbilityResurrect::Selected()
{
    m_pOwner->StartAnimation(m_pPrimaryAnimName->GetString(), 1);
}


/*====================
  CAbilityResurrect::ActivatePrimary
  ====================*/
bool    CAbilityResurrect::ActivatePrimary()
{
    CAxis axis(m_pOwner->GetAngles());
    CVec3f v3Start(m_pOwner->GetPosition() + V_UP * m_pOwner->GetViewHeight());
    CVec3f v3End(v3Start + axis.Forward() * 1500.0f);

    STraceInfo result;
    Game.TraceLine(result, v3Start, v3End);
    if (result.uiEntityIndex != INVALID_INDEX)
    {
        IGameEntity *pEntity(Game.GetEntityFromWorldIndex(result.uiEntityIndex));
        if (pEntity != NULL && pEntity->GetState() != ENTITY_STATE_DEAD)
            pEntity->Kill();

        //m_pOwner->SetState(ENTITY_STATE_DORMANT);
    }

    return true;
}
