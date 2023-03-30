// (C)2006 S2 Games
// i_spellarea.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "i_spellarea.h"

#include "../k2/c_clientsnapshot.h"
//=============================================================================

/*====================
  ISpellArea::ISpellArea
  ====================*/
ISpellArea::ISpellArea() :
ISpellItem(NULL)    // FIXME: NULL settings
{
}


/*====================
  ISpellArea::ActivatePrimary
  ====================*/
bool    ISpellArea::ActivatePrimary(const CClientSnapshot &snapshot)
{
    // Check to see if the spell is ready yet
    if (Game.GetGameTime() < m_pOwner->GetCooldownTimer(m_iSlot))
        return false;

    // If the spell is not selected, this was a request to activate it
    if (m_pOwner->GetCurrentItemSlot() != m_iSlot)
    {
        Console << _T("Priming spell: ") << m_iSlot << _T(" [") << m_pOwner->GetCurrentItemSlot() << _T("]") << newl;
        m_pOwner->SelectItem(m_iSlot);
        return true;
    }

    // Only activate on an impulse (otherwise it would always fire right away)
    int iButtonStatus(snapshot.GetButtonStatus(GAME_BUTTON_QUICK_ATTACK));
    if (!(iButtonStatus & GAME_BUTTON_STATUS_PRESSED))
        return false;

    // Check mana
    if (!m_pOwner->SpendMana(m_pCvarSettings->GetManaCost()))
        return false;

    // Information for cast event
    CVec3f v3CastPos;
    CVec3f v3CastAngles;

    // Determine target position and angles
    v3CastPos = m_v3TargetPosition = GetTargetLocation(snapshot.GetCameraAngles());
    v3CastAngles[YAW] = m_pOwner->GetAngles()[YAW];

    // Cast event
    if (!m_pCastEffectPath->empty())
    {
        CGameEvent evCast;
        evCast.SetPosition(v3CastPos);
        evCast.SetAngles(v3CastAngles);
        evCast.SetEffect(g_ResourceManager.Register(m_pCastEffectPath->GetString(), RES_EFFECT));
        Game.AddEvent(evCast);
    }

    int iAction(PLAYER_ACTION_SPELL);
    if (m_pFreeze->GetValue())
    {
        m_pOwner->StopAnim(0);
        iAction |= PLAYER_ACTION_IMMOBILE;
    }

    CSpellActivateEvent &spellEvent(m_pOwner->GetSpellActivateEvent());
    spellEvent.Clear();
    spellEvent.SetSpellPointer(this);
    spellEvent.SetActivateTime(Game.GetGameTime() + m_pImpactTime->GetValue());

    m_pOwner->SetAction(iAction, Game.GetGameTime() + m_pCvarSettings->GetCastTime());
    m_pOwner->StartAnimation(m_pAnimName->GetString(), 0);
    m_pOwner->SetCooldownTimer(m_iSlot, Game.GetGameTime() + GetCooldownTime());
    m_pOwner->UnselectItem();
    return true;
}
