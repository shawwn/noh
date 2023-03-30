// (C)2006 S2 Games
// c_spellrainoffire.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellrainoffire.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, RainOfFire);
//=============================================================================

/*====================
  CSpellRainOfFire::TryImpact
  ====================*/
bool    CSpellRainOfFire::TryImpact()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Client does not predict the execution
    if (Game.IsClient())
        return false;

    // Information for impact event
    CVec3f v3ImpactPos;
    CVec3f v3ImpactAngles;

    // Validate the target
    CVec3f v3Origin(m_v3TargetPosition);
    m_v3TargetPosition.Clear();
    v3ImpactPos = v3Origin;
    v3ImpactAngles[YAW] = pOwner->GetAngles()[YAW];

    // Spawn gadget
    IGameEntity *pNewEnt(Game.AllocateEntity(_T("Gadget_FireRain")));
    if (pNewEnt == NULL || pNewEnt->GetAsGadget() == NULL)
    {
        Console.Warn << _T("Failed to spawn gadget: FireRain") << newl;
        return false;
    }

    v3ImpactAngles[PITCH] = 0.0f;
    v3ImpactPos[Z] = Game.GetTerrainHeight(v3ImpactPos[X], v3ImpactPos[Y]);
    pNewEnt->GetAsGadget()->SetOwner(pOwner->GetIndex());
    pNewEnt->GetAsGadget()->SetTeam(pOwner->GetTeam());
    pNewEnt->GetAsGadget()->SetPosition(v3ImpactPos);
    pNewEnt->GetAsGadget()->SetAngles(v3ImpactAngles);
    pNewEnt->Spawn();

    // Impact event
    if (!m_pEntityConfig->GetImpactEffectPath().empty())
    {
        CGameEvent evImpact;
        evImpact.SetSourcePosition(v3ImpactPos);
        evImpact.SetSourceAngles(v3ImpactAngles);
        evImpact.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetImpactEffectPath()));
        Game.AddEvent(evImpact);
    }

    return true;
}


/*====================
  CSpellRainOfFire::ActivatePrimary
  ====================*/
bool    CSpellRainOfFire::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Check to see if the spell is ready yet
    if (!IsReady())
        return false;

    // If the spell is not selected, this was a request to activate it
    if (pOwner->GetSelectedItem() != m_ySlot)
    {
        Console << _T("Priming spell: ") << m_ySlot << _T(" [") << pOwner->GetSelectedItem() << _T("]") << newl;
        pOwner->SelectItem(m_ySlot);
        return true;
    }

    // Only activate on an impulse (otherwise it would always fire right away)
    if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
        return false;

    // Check mana
    if (!pOwner->SpendMana(m_pEntityConfig->GetManaCost()))
        return false;

    // Information for cast event
    CVec3f v3CastPos;
    CVec3f v3CastAngles;

    // Determine target position and angles
    v3CastPos = m_v3TargetPosition = GetTargetLocation();
    v3CastAngles[YAW] = pOwner->GetAngles()[YAW];

    // Cast event
    if (!m_pEntityConfig->GetCastEffectPath().empty())
    {
        CGameEvent evCast;
        evCast.SetSourcePosition(v3CastPos);
        evCast.SetSourceAngles(v3CastAngles);
        evCast.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetCastEffectPath()));
        Game.AddEvent(evCast);
    }

    int iAction(PLAYER_ACTION_SPELL);
    if (m_pEntityConfig->GetFreeze())
    {
        pOwner->StopAnimation(0);
        iAction |= PLAYER_ACTION_IMMOBILE;
    }

    CSpellActivateEvent &spellEvent(pOwner->GetSpellActivateEvent());
    spellEvent.Clear();
    spellEvent.SetOwner(pOwner);
    spellEvent.SetSlot(m_ySlot);
    spellEvent.SetActivateTime(Game.GetGameTime() + m_pEntityConfig->GetImpactTime());

    pOwner->SetAction(iAction, Game.GetGameTime() + m_pEntityConfig->GetCastTime());
    pOwner->StartAnimation(m_pEntityConfig->GetAnimName(), 0);
    SetCooldownTimer(Game.GetGameTime(), m_pEntityConfig->GetCooldownTime());
    return true;
}
