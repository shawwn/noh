// (C)2006 S2 Games
// c_spellimmolate.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spellimmolate.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, Immolate);
//=============================================================================

/*====================
  CSpellImmolate::CSpellImmolate
  ====================*/
CSpellImmolate::CSpellImmolate() :
ISpellItem(GetEntityConfig()),

m_bActive(false),
m_pGadgetEnt(NULL)
{
    
}


/*====================
  CSpellImmolate::FinishedAction
  ====================*/
void    CSpellImmolate::FinishedAction(int iAction)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return;

    // Deactivate skill, player changed states to non-active
    if (iAction == 1)
    {
        Console << _T("Deactivating spell: ") << m_ySlot << _T(" [") << pOwner->GetSelectedItem() << _T("]") << newl;
        m_bActive = false;

        if (m_pGadgetEnt)
            m_pGadgetEnt->Kill();
    }
}


/*====================
  CSpellImmolate::ActivatePrimary
  ====================*/
bool    CSpellImmolate::ActivatePrimary(int iButtonStatus)
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Information for positioning
    CVec3f v3Pos;
    CVec3f v3Angles;

    // Client does not predict the execution
    if (Game.IsClient())
        return true;

    // Check to see if the spell is ready yet
    if (!IsReady())
        return false;

    // If the spell is not activated, this was a request to activate it
    if (!m_bActive)
    {
        // Check mana
        if (!pOwner->SpendMana(m_pEntityConfig->GetManaCost()))
            return false;

        Console << _T("Activating spell: ") << m_ySlot << _T(" [") << pOwner->GetSelectedItem() << _T("]") << newl;
        m_bActive = true;

        // Spawn gadget
        m_pGadgetEnt = Game.AllocateEntity(_T("Gadget_Immolate"));
        if (m_pGadgetEnt == NULL || m_pGadgetEnt->GetAsGadget() == NULL)
        {
            Console.Warn << _T("Failed to spawn gadget: Immolate") << newl;
            return false;
        }

        v3Angles[YAW] = pOwner->GetAngles()[YAW];
        v3Angles[PITCH] = 0.0f;
        v3Pos = pOwner->GetPosition();
        v3Pos[Z] = Game.GetTerrainHeight(v3Pos[X], v3Pos[Y]);
        m_pGadgetEnt->GetAsGadget()->SetOwner(pOwner->GetIndex());
        m_pGadgetEnt->GetAsGadget()->SetTeam(pOwner->GetTeam());
        m_pGadgetEnt->GetAsGadget()->SetPosition(v3Pos);
        m_pGadgetEnt->GetAsGadget()->SetAngles(v3Angles);
        m_pGadgetEnt->Spawn();
    }
    else
    {
        Console << _T("Deactivating spell: ") << m_ySlot << _T(" [") << pOwner->GetSelectedItem() << _T("]") << newl;
        m_bActive = false;

        if (m_pGadgetEnt)
            m_pGadgetEnt->Kill();
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

