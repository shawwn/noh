// (C)2007 S2 Games
// c_gadgethail.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgethail.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, Hail)

CCvarf CGadgetHail::s_cvarRadius(           _T("Gadget_Hail_Radius"),           300.0f, CVAR_GAMECONFIG | CVAR_TRANSMIT);
CCvarf CGadgetHail::s_cvarDamagePerSecond(  _T("Gadget_Hail_DamagePerSecond"),  50.0f,  CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CGadgetHail::ServerFrame
  ====================*/
bool    CGadgetHail::ServerFrame()
{
    if (GetStatus() != ENTITY_STATUS_DEAD)
    {
        uivector vSetResult;
        Game.GetEntitiesInRadius(vSetResult, CSphere(GetPosition(), s_cvarRadius), 0);
        for (uivector_it it(vSetResult.begin()); it != vSetResult.end(); ++it)
        {
            IGameEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
            if (pEnt == NULL)
                continue;

            pEnt->Damage(s_cvarDamagePerSecond, DAMAGE_FLAG_CRUSH | DAMAGE_FLAG_SIEGE, Game.GetVisualEntity(m_uiOwnerIndex));
        }
    }

    if (s_EntityConfig.GetLifetime() > 0 && s_EntityConfig.GetLifetime() + m_uiSpawnTime < Game.GetGameTime())
    {
        Kill();
        return false;
    }

    return true;
}
