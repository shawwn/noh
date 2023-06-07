// (C)2006 S2 Games
// c_spelldefile.cpp
//
//=============================================================================

//=============================================================================
// Headers
//=============================================================================
#include "game_shared_common.h"

#include "c_spelldefile.h"
//=============================================================================

//=============================================================================
// Definitions
//=============================================================================
DEFINE_ENT_ALLOCATOR2(Spell, Defile);

CCvarf  CSpellDefile::s_cvarDamage(_T("Spell_Defile_Damage"),   0.0f,       CVAR_GAMECONFIG | CVAR_TRANSMIT);
//=============================================================================

/*====================
  CSpellDefile::TryImpact
  ====================*/
bool    CSpellDefile::TryImpact()
{
    ICombatEntity *pOwner(GetOwnerEnt());
    if (!pOwner)
        return false;

    // Client does not predict the execution
    if (Game.IsClient())
        return false;

    // Validate the target
    if (m_pEntityConfig->GetSnapcast())
    {
        IVisualEntity *pTarget(Game.GetVisualEntity(m_uiTargetIndex));
        m_uiTargetIndex = INVALID_INDEX;
        if (pTarget == NULL)
            return false;

        if (!IsValidTarget(pTarget, true))
            return false;

        float fDamageMult(1.0f);

        if (pTarget->IsCombat())
            fDamageMult = pTarget->GetAsCombatEnt()->GetSpellResistance();

        pTarget->Damage(s_cvarDamage.GetValue() * fDamageMult, 0, pOwner, GetType());

        // Impact event
        if (!m_pEntityConfig->GetImpactEffectPath().empty())
        {
            CGameEvent evImpact;
            evImpact.SetSourceEntity(pTarget->GetIndex());
            evImpact.SetEffect(Game.RegisterEffect(m_pEntityConfig->GetImpactEffectPath()));
            Game.AddEvent(evImpact);
        }
    }

    return true;
}
