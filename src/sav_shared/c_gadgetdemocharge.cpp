// (C)2007 S2 Games
// c_gadgetdemocharge.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_gadgetdemocharge.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Gadget, DemoCharge);
//=============================================================================


/*====================
  CGadgetDemoCharge::CEntityConfig::CEntityConfig
  ====================*/
CGadgetDemoCharge::CEntityConfig::CEntityConfig(const tstring &sName) :
IGadgetEntity::CEntityConfig(sName),
INIT_ENTITY_CVAR(BlastRadius, 1000.0f),
INIT_ENTITY_CVAR(BlastDamage, 5000.0f)
{
}


/*====================
  CGadgetDemoCharge::CGadgetDemoCharge
  ====================*/
CGadgetDemoCharge::CGadgetDemoCharge() :
IGadgetEntity(GetEntityConfig()),
m_pEntityConfig(GetEntityConfig())
{
}


/*====================
  CGadgetDemoCharge::Spawn
  ====================*/
void    CGadgetDemoCharge::Spawn()
{
    IGadgetEntity::Spawn();

    StartAnimation(_T("countdown"), -1, 1.0f, m_pEntityConfig->GetLifetime());
}


/*====================
  CGadgetDemoCharge::ServerFrame
  ====================*/
bool    CGadgetDemoCharge::ServerFrame()
{
    if (m_pEntityConfig == NULL)
        return true;

    if (GetStatus() == ENTITY_STATUS_DEAD)
    {
        if (Game.GetGameTime() >= m_uiCorpseTime)
            return false;
        return true;
    }

    if (Game.GetGameTime() - m_uiSpawnTime >= m_pEntityConfig->GetLifetime())
    {
        StartAnimation(_T("explosion"), -1);

        uivector setEntities;
        Game.GetEntitiesInRadius(setEntities, CSphere(GetPosition(), m_pEntityConfig->GetBlastRadius()), 0);
        for (uivector_it it(setEntities.begin()); it != setEntities.end(); ++it)
        {
            IVisualEntity *pEntity(Game.GetEntityFromWorldIndex(*it));
            if (pEntity == NULL)
                continue;

            if (pEntity->GetTeam() != GetTeam())
                pEntity->Damage(m_pEntityConfig->GetBlastDamage(), DAMAGE_FLAG_EXPLOSIVE | DAMAGE_FLAG_SPLASH, Game.GetVisualEntity(m_uiOwnerIndex), m_unDamageID);
        }

        SetStatus(ENTITY_STATUS_DEAD);
        m_uiCorpseTime = Game.GetGameTime() + m_pEntityConfig->GetCorpseTime();
        Unlink();
    }

    return true;    
}
