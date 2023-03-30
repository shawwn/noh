// (C)2006 S2 Games
// c_triggerproximity.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_triggerproximity.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Trigger, Proximity)
//=============================================================================

/*====================
  CTriggerProximity::RegisterEntityScripts
  ====================*/
void    CTriggerProximity::RegisterEntityScripts(const CWorldEntity &ent)
{
    Game.RegisterEntityScript(GetIndex(), _T("enter"), ent.GetProperty(_T("triggerenter")));
    Game.RegisterEntityScript(GetIndex(), _T("stay"), ent.GetProperty(_T("triggerstay")));
    Game.RegisterEntityScript(GetIndex(), _T("leave"), ent.GetProperty(_T("triggerleave")));
}

/*====================
  CTriggerProximity::ApplyWorldEntity
  ====================*/
void    CTriggerProximity::ApplyWorldEntity(const CWorldEntity &ent)
{
    ITriggerEntity::ApplyWorldEntity(ent);

    m_fRadius = ent.GetPropertyFloat(_T("triggerradius"));

    m_bTriggerOnPlayer = ent.GetPropertyBool(_T("triggeronplayer"));
    m_bTriggerOnNPC = ent.GetPropertyBool(_T("triggeronnpc"));
    m_bTriggerOnBuilding = ent.GetPropertyBool(_T("triggeronbuilding"));
    m_bTriggerOnProp = ent.GetPropertyBool(_T("triggeronprop"));
    m_bTriggerOnGadget = ent.GetPropertyBool(_T("triggerongadget"));
}

/*====================
  CTriggerProximity::ServerFrame
  ====================*/
bool    CTriggerProximity::ServerFrame()
{
    ITriggerEntity::ServerFrame();

    if (GetStatus() != ENTITY_STATUS_ACTIVE || !Game.IsServer())
        return true;

    uivector vResult;
    uiset setLeft(m_setEntitiesInRadius);
    
    Game.GetEntitiesInRadius(vResult, CSphere(m_v3Position, m_fRadius), 0);
    for (uivector_it it(vResult.begin()); it != vResult.end(); ++it)
    {
        if (*it == m_uiLinkedEntityIndex || *it == GetWorldIndex())
            continue;

        setLeft.erase(*it);

        IGameEntity *pBaseEnt(Game.GetEntityFromWorldIndex(*it));

        if (pBaseEnt == NULL)
            continue;

        IVisualEntity *pEnt(pBaseEnt->GetAsVisualEnt());

        if (pEnt == NULL)
            continue;

        if ((pEnt->IsPlayer() && m_bTriggerOnPlayer) ||
            (pEnt->IsNpc() && m_bTriggerOnNPC) ||
            (pEnt->IsBuilding() && m_bTriggerOnBuilding) ||
            (pEnt->IsProp() && m_bTriggerOnProp) ||
            (pEnt->IsGadget() && m_bTriggerOnGadget))
        {
            if (m_setEntitiesInRadius.find(*it) != m_setEntitiesInRadius.end())
            {
                Trigger(pEnt->GetIndex(), _T("stay"), false);
            }
            else
            {
                Trigger(pEnt->GetIndex(), _T("enter"));
                m_setEntitiesInRadius.insert(*it);
            }
        }
    }

    for (uiset_it it(setLeft.begin()); it != setLeft.end(); ++it)
    {
        if (*it == m_uiLinkedEntityIndex || *it == GetWorldIndex())
            continue;

        m_setEntitiesInRadius.erase(*it);

        IVisualEntity *pEnt(Game.GetEntityFromWorldIndex(*it));
        if (pEnt == NULL)
            continue;

        if ((pEnt->IsPlayer() && m_bTriggerOnPlayer) ||
            (pEnt->IsNpc() && m_bTriggerOnNPC) ||
            (pEnt->IsBuilding() && m_bTriggerOnBuilding) ||
            (pEnt->IsProp() && m_bTriggerOnProp) ||
            (pEnt->IsGadget() && m_bTriggerOnGadget))
        {
            Trigger(pEnt->GetIndex(), _T("leave"));
        }
    }

    return true;
}


/*====================
  CTriggerProximity::Copy
  ====================*/
void    CTriggerProximity::Copy(const IGameEntity &B)
{
    ITriggerEntity::Copy(B);

    const ITriggerEntity *pB(B.GetAsTrigger());

    if (!pB || !pB->IsProximityTrigger())
        return;

    const CTriggerProximity &C(*static_cast<const CTriggerProximity*>(pB));

    m_setEntitiesInRadius = C.m_setEntitiesInRadius;

    m_bTriggerOnPlayer = C.m_bTriggerOnPlayer;
    m_bTriggerOnNPC = C.m_bTriggerOnNPC;
    m_bTriggerOnBuilding = C.m_bTriggerOnBuilding;
    m_bTriggerOnProp = C.m_bTriggerOnProp;
    m_bTriggerOnGadget = C.m_bTriggerOnGadget;

    m_fRadius = C.m_fRadius;
}
