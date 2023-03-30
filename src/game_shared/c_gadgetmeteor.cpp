// (C)2007 S2 Games
// c_gadgetmeteor.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetmeteor.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, Meteor)

CCvarui CGadgetMeteor::s_cvarImpactTime(    _T("Gadget_Meteor_ImpactTime"),     10000,  CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarf  CGadgetMeteor::s_cvarImpactRadius(  _T("Gadget_Meteor_ImpactRadius"),   300.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarf  CGadgetMeteor::s_cvarImpactDamage(  _T("Gadget_Meteor_ImpactDamage"),   500.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CGadgetMeteor::ServerFrame
  ====================*/
bool    CGadgetMeteor::ServerFrame()
{
    if (GetStatus() == ENTITY_STATUS_DEAD)
    {
        if (Game.GetGameTime() >= m_uiCorpseTime)
            return false;
        return true;
    }

    if (Game.GetGameTime() - m_uiSpawnTime < s_cvarImpactTime)
        return true;

    uivector vSetResult;
    Game.GetEntitiesInRadius(vSetResult, CSphere(GetPosition(), s_cvarImpactRadius), 0);
    for (uivector_it it(vSetResult.begin()); it != vSetResult.end(); ++it)
    {
        IGameEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
        if (pEnt == NULL)
            continue;

        pEnt->Damage(s_cvarImpactDamage, DAMAGE_FLAG_CRUSH | DAMAGE_FLAG_SIEGE, Game.GetVisualEntity(m_uiOwnerIndex));
    }

    Kill();
    return true;
}
